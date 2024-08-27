/**
 ***************************** Declaration ********************************
 * @file           : zh_pinyin_decoder.c
 * @author         : FriedParrot (https://github.com/FriedParrot)
 * @version        : v1.3
 * @date           : 2024-08-27  (last modified)
 * @brief          : decoder source file for chinese pinyin inputting method
 * @license        : MIT license (https://opensource.org/license/mit)
 *****************************************************************************
 * @attention
 * this file is the core decoder file for chinese pinyin input method.
 * 
 * @note : in static function, we consider input string valid and not check it.
 *         "chk_valid_string" is called at the beginning of public functions.
 *****************************************************************************
 */
#include <stdio.h>
#include <string.h>
#include "zh_pinyin_decoder.h"
#include "zh_code_table.h"

#if (USE_ZH_HASH_BOOST == 1)
#include "zh_hash_boost.h"
#endif

#if (USE_ZH_WORD_MATCH == 1)
#include "../CJSON/cJSON.h"
#endif

static int g_word_match_number = 0;
static int g_word_match_max = ZH_PINYIN_MAX_SPLIT_METHODS;

/*******************   static function prototypes     ****************************/

static uint8_t chk_valid_string(const char* str);
static uint8_t common_prefix_length(const char* str1, const char* str2);

static __split_method_t* mnode_init(void);
static int mnode_cmp(__split_method_t* m1, __split_method_t* m2);
static __split_method_list_t* mlist_init(void);
static void mlist_insert(__split_method_list_t* m_list, __split_method_t* m);
static void mlist_destroy(__split_method_list_t* m_list);

static uint8_t get_match_idx(const char* str, int8_t* match_idx, const uint8_t vag_num, uint8_t* vag_idx_arr, int8_t* vag_br);
static uint8_t pinyin_dfs(__split_method_list_t* m_list, const char* str, uint8_t* spm, uint8_t m_wt, uint8_t depth);

/*
* @brief check if a string input is valid for input method
*/
static uint8_t chk_valid_string(const char* str) {
    if (str == NULL || strlen(str) < 1 || strlen(str) > ZH_MAX_STRING_LENGTH) return 1;
    uint8_t s = strlen(str);
    for (int i = 0; i < s; i++) {
        if (str[i] < 'a' || str[i] > 'z') return 1;
    }
    return 0;
}

/* get the common prefix length of two string */
static uint8_t common_prefix_length(const char* str1, const char* str2) {
    const char* start1 = str1;
    const char* start2 = str2;
    while (*str1 && *str2 && *str1 == *str2) {
        str1++; str2++;
    }
    return str1 - start1;
}

/* init method node list (sigly linked list) */
static __split_method_t* mnode_init(void) {
    __split_method_t* m = zh_buffer_malloc(sizeof(__split_method_t));
    if (m == NULL) {
        ZH_LOG_ERROR("zh_buffer_malloc failed");
        return NULL;
    }
    memset(m->spm, 0, MAX_WORD_LENGTH);
    m->wt = 0;
    m->length = 0;
    m->next = NULL;
    return m;
}

/**
* @compare the sequence that method in list. 
* @note compare insert sequence : insert seq : 1.m->length(shorter)  2.m->wt(larger)  3.m->spm(smaller)
* @note  m1 and m2 must not be NULL 
* @return < 0 m1 is before m2,  = 0: same , >0: m1 is after m2
*/
static int mnode_cmp(__split_method_t* m1, __split_method_t* m2) {
    if (m1->length != m2->length) return m1->length - m2->length;
    if (m1->wt != m2->wt) return  m2->wt - m1->wt;
    for (int i = 0; i < m1->length; i++) {
        if (m1->spm[i] != m2->spm[i]) return (m1->spm[i] - m2->spm[i]);
    }
    return 0;
}

/* init the split string method list */
static __split_method_list_t* mlist_init(void) {
    __split_method_list_t* m_list = zh_buffer_malloc(sizeof(__split_method_list_t));
    if (m_list == NULL) {
        ZH_LOG_ERROR("zh_buffer_malloc fail");
        return NULL;
    }
    m_list->num = 0;
    m_list->head = NULL;
    // m_list->tail = NULL;
    return m_list;
}

/* add a split method element to the list */
static void mlist_insert(__split_method_list_t* m_list, __split_method_t* m) {
    if (m_list == NULL || m == NULL) return;
    int res = m_list->head == NULL ? -1 : mnode_cmp(m, m_list->head);
    if (res < 0) {
        m->next = m_list->head;
        m_list->head = m;
        m_list->num++;
        return;
    }
    if (res == 0) return; // do nothing
    __split_method_t* pre = m_list->head;
    __split_method_t* nxt = m_list->head->next;
    while (1) {
        res = (nxt == NULL) ? -1 : mnode_cmp(m, nxt);
        if (res < 0) {
            m->next = nxt;
            pre->next = m;
            m_list->num++;
            return;
        }
        if (res == 0) return;
        nxt = pre->next->next;
        pre = pre->next;
    }
}

/* free single linked list */
static void mlist_destroy(__split_method_list_t* m_list) {
    if (m_list == NULL) return;
    while (m_list->head != NULL) {
        __split_method_t* m = m_list->head;
        m_list->head = m_list->head->next;
        zh_buffer_free(m);
    }
    zh_buffer_free(m_list);
}

/**
* @brief match string from pinyin table 
* @note "str" must be a legal string consits of 'a' to 'z' 
* @param str            string for match 
* @param match_idx      index of precise match  (if no precise match found,return -1)
* @param vag_num        max number for vague match 
* @param vag_idx_arr    index array of vague match (at least size vague num)
* @param vag_br         number of vague match found 
*/
static uint8_t get_match_idx(const char* str, int8_t* match_idx, const uint8_t vag_num, uint8_t* vag_idx_arr, int8_t* vag_br) {
    uint8_t idx = str[0] - 'a';
    (*match_idx) = -1;
    const __code_index_t* codex = (&code_index[idx]);
    if (codex->table_length == 0) return 1;
    uint8_t cur_vague_num = 0;

#if (USE_ZH_HASH_BOOST == 0)
    /* search through the whole table  and compare string */
    for (int i = 0; i < codex->table_length; i++) {
        if (strcmp(str, codex->code_table[i]) ==  0) {
            (*match_idx) = i;
        }
        else if (cur_vague_num < vag_num && strncmp(str, codex->code_table[i], strlen(str)) == 0) {
            vag_idx_arr[cur_vague_num] = i;
            cur_vague_num++;
        }
    }
#else
    if (strlen(str) == 1) {
        if (strlen(codex->code_table[0]) == 1) {
            /* 1st result precise, other vague result */
            (*match_idx) = 0;
            cur_vague_num = codex->table_length - 1 > vag_num ? vag_num : codex->table_length - 1;
            for (int i = 0; i < cur_vague_num; i++) {
                vag_idx_arr[i] = i + 1;
            }
        }
        else {
            /* all are vague result */
            cur_vague_num = codex->table_length > vag_num ? vag_num : codex->table_length;
            for (int i = 0; i < cur_vague_num; i++) {
                vag_idx_arr[i] = i;
            }
        }
    }
    else 
    {
        if (!zh_hash_inlist(str)) return 1;
        uint8_t idx2 = (str[1] - 'a') % ZH_HASH_TABLE_DIV;
        for (int i = 0; i < hash_list[idx][idx2]->num; i++) {
            uint8_t index = hash_list[idx][idx2]->idx_list[i];
            if (strcmp(str, codex->code_table[index]) == 0) {
                (*match_idx) = index;
            }
            else if (cur_vague_num < vag_num && strncmp(str, codex->code_table[index], strlen(str)) == 0) {
                vag_idx_arr[cur_vague_num] = index;
                cur_vague_num++;
            }
        }
    }
#endif
    if (vag_br != NULL) {
        (*vag_br) = cur_vague_num;
    }
    return 0;
}

/**
* @brief  dfs function  for pinyin split algorithm 
*/
static uint8_t pinyin_dfs(__split_method_list_t* m_list, const char* str, uint8_t* spm, uint8_t m_wt, uint8_t depth) {
    uint8_t start_idx = (depth == 0) ? 0 : spm[depth - 1];
    /* termiate conditions -> string end */
    if (start_idx == strlen(str)) {
        /* word split successfully, concanate the split method into m_list */
        __split_method_t* m = mnode_init();
        if (m == NULL) return 1;
        if (depth == 0) {
            ZH_LOG_ERROR("depth is not 0 here!!!");
            return 1;
        }
        m->length = depth;
        m->wt = m_wt;
        memcpy(m->spm, spm, sizeof(uint8_t) * depth);
        memset(m->spm + depth, 0, MAX_WORD_LENGTH - depth);
        mlist_insert(m_list, m);
        g_word_match_number++;
        return 0;
    }
    else if (depth == MAX_WORD_LENGTH) {
        /* still not find the split method */
        g_word_match_number++; return 1; 
    }
    if (g_word_match_number >= g_word_match_max) {
        ZH_LOG_INFO("reach the maximum search length, search terminated");
        return 1;
    }

    uint8_t idx1 = str[start_idx] - 'a';
    __code_index_t* codex = (&code_index[idx1]);
    if (codex->table_length == 0) {
        g_word_match_number++; return 1;
    }
    uint8_t sl = __min(strlen(str) - start_idx, MAX_WORD_CODE_LENGTH);   /* strlen (left_str) */
    uint8_t res = 1;
    if (sl > 1) {
#if (USE_ZH_HASH_BOOST == 0)
        for (int i = 0; i < codex->table_length; i++) {
            uint8_t sr = strlen(codex->code_table[i]); /* strlen (result_str) */
            uint8_t sc = common_prefix_length(str + start_idx, codex->code_table[i]);
            if (sc <= 1) continue;  /* for */
            res = 0;   /* have at least 1 vague match (1 letter occasion needn't to be considered) */
            m_wt = sc == sr ? (m_wt | (1 << (MAX_WORD_LENGTH - 1 - depth))) : m_wt & ~(1 << (MAX_WORD_LENGTH - 1 - depth));  /* record precise match location */
            spm[depth] = start_idx + sc;     /* the match part */
            pinyin_dfs(m_list, str, spm, m_wt, depth + 1);  /* If any result is found, set res to 0 */
        }
#else
        uint8_t idx2 = (str[start_idx + 1] - 'a') % ZH_HASH_TABLE_DIV;
        char s_tmp[2] = {str[start_idx], str[start_idx + 1]};
        if (zh_hash_inlist(s_tmp)) {
            for (int i = 0; i < hash_list[idx1][idx2]->num; i++) {
                uint8_t idx = hash_list[idx1][idx2]->idx_list[i];
                uint8_t sr = strlen(codex->code_table[idx]);
                uint8_t sc = common_prefix_length(str + start_idx, codex->code_table[idx]);
                if (sc <= 1) continue;
                res = 0;
                m_wt = sc == sr ? (m_wt | (1 << (MAX_WORD_LENGTH - 1 - depth))) : m_wt & ~(1 << (MAX_WORD_LENGTH - 1 - depth));  /* record precise match location */
                spm[depth] = start_idx + sc;     /* the match part */
                pinyin_dfs(m_list, str, spm, m_wt, depth + 1);
            }
        }
#endif
    }
    /* when all result is 1, no other split ways */
    if (res) { /**  no match found for len >=2, we can just split it in 1*/
        uint8_t tmp = __min(codex->table_length, ZH_PINYIN_SIGNLE_SEARCH_DEPTH);
        code_index[idx1].code_table;
        /* when 1 code can be fully matched */
        m_wt = strlen(codex->code_table[0]) == 1 ? (m_wt | (1 << (MAX_WORD_LENGTH - 1 - depth))) : m_wt & ~(1 << (MAX_WORD_LENGTH - 1 - depth));
        spm[depth] = start_idx + 1;   /* sc = 1 */
        pinyin_dfs(m_list, str, spm, m_wt, depth + 1);
    }
    return 0;
}

/**
 * @brief       Match the utf-8 code in PinYin table precisely 
 * @param       str : string to match
 * @param       res_str : result string (must pre-malloc size at least 3 * num bytes + 1(MAX_CODE_BUFFER_SZ is recommended)
 * @param       num : number of zh Character to read (set to ZH_VAGUE_MAX_LENGTH if want all)
 * @param       br : number of zh Character readed
 * @retval      0: match succeed , 1: read error or nothing to match
 */
uint8_t zh_match_code_prec(const char* str, char* res_str, uint8_t num, uint8_t* br){
    if (res_str == NULL || chk_valid_string(str)) return 1;
    FILE* fp = fopen(ZH_CODE_TABLE_FILE_NAME, "rb");
    if (fp == NULL) {
        ZH_LOG_ERROR("code table file \"zh pinyin.bin\" not exist");
        return 1;
    }
    int8_t  match_idx = -1;
    uint8_t v_br = 0;

    if (get_match_idx(str, &match_idx, 0, NULL, &v_br) || match_idx < 0) {
        fclose(fp);
        return 1;
    }
    uint8_t idx = str[0] - 'a';
    const __code_index_t* codex = (&code_index[idx]);
    
    uint8_t br_read = codex->code_table_num[match_idx] > num ? num :codex->code_table_num[match_idx];
    size_t read_loc = codex->char_start + codex->code_offset[match_idx] + \
        (codex->code_table_num[match_idx] - br_read) * 3;
    uint16_t read_length = 3 * br_read;
    
    fseek(fp, read_loc, SEEK_SET);
    fread(res_str, sizeof(uint8_t), read_length ,fp);
    res_str[read_length] = '\0';
    if (br != NULL) (*br) = br_read;
    fclose(fp);
    return 0;
}

/**
 * @brief       vague match for the input pinyin code
 * @param       str : string to match
 * @param       res_str : result string (must pre-malloc size at least 3 * num bytes + 1(MAX_CODE_BUFFER_SZ is recommended)
 * @param       num : number of zh Character to read (set to ZH_VAGUE_MAX_LENGTH if want all)
 * @param       br : number of zh Character readed
 * @return      0: match succeed , 1: read error or nothing to match
 * @bug         when str starts with '0' may cause fault 
 */
uint8_t zh_match_code_vague(const char* str, char* res_str, uint8_t num, uint8_t* br) {
    if (res_str == NULL || chk_valid_string(str)) return 1;

    int8_t mid = 0;
    uint8_t* v_idx = zh_buffer_malloc(num);
    if (v_idx == NULL) {
        ZH_LOG_ERROR("storge full, allocate buffer failed");
        return 1;
    }
    uint8_t  v_br = 0;
    if (get_match_idx(str, &mid, num, v_idx, &v_br)) return 1;

    FILE* fp = fopen(ZH_CODE_TABLE_FILE_NAME, "rb");
    if (!fp) {
        free(v_idx);
        ZH_LOG_ERROR("code table file \"zh pinyin.bin\" not exist");
        return 1;
    }
    uint8_t idx = str[0] - 'a';
    const __code_index_t* codex = (&code_index[idx]);
    res_str[(uint16_t)num * 3] = '\0';
    
    uint8_t chars_left = num;  /* left space in num */
    uint8_t br_read = 0;       /* character that has been readed */

    if (chars_left > 0 &&  mid >= 0) {
        uint8_t match_num = __min(chars_left, codex->code_table_num[mid]);
        match_num = __min(match_num, ZH_VAGUE_MATCH_HEAD_DEPTH);
        chars_left -= match_num; br_read += match_num;
        
        size_t read_loc = codex->char_start + codex->code_offset[mid] + 3 * (codex->code_table_num[mid] - match_num);
        fseek(fp, read_loc, SEEK_SET);
        fread(res_str + (size_t)chars_left * 3, sizeof(uint8_t), (size_t)match_num * 3, fp);
    }

    if (chars_left > 0 && v_br > 0) {
        uint8_t match_depth = strlen(str) == 1 ? ZH_VAGUE_SEARCH_DEPTH_SINGLE : ZH_VAGUE_SEARCH_DEPTH;
        uint8_t match_types = strlen(str) == 1 ? ZH_VAGUE_SEARCH_TYPES_SINGLE : ZH_VAGUE_SEARCH_TYPES;

        uint8_t cur_types = 0;
        /* search and fill the buffer from tail to head */
        for (int i = 0; i < v_br && cur_types < match_types; i++, cur_types++) {
            uint8_t index = v_idx[i];      /* index of the vague match */
            uint8_t match_num = codex->code_table_num[index] > chars_left ? chars_left : codex->code_table_num[index];
            match_num = __min(match_num, match_depth);
            chars_left -= match_num; br_read += match_num;
            
            uint32_t read_loc = codex->char_start + codex->code_offset[index] + 3 * (codex->code_table_num[index] - match_num);
            uint16_t read_length = 3 * (match_num);
            fseek(fp, (long)read_loc, SEEK_SET);
            fread(res_str + 3 * chars_left, sizeof(uint8_t), read_length, fp);
        }
    }

    /* append other search result in the end of strin */
    if (chars_left > 0 && mid>=0 && codex->code_table_num[mid] > ZH_VAGUE_MATCH_HEAD_DEPTH) {
        uint8_t match_num = __min(codex->code_table_num[mid] - ZH_VAGUE_MATCH_HEAD_DEPTH, chars_left);
        chars_left -= match_num; br_read += match_num;
        
        uint32_t read_loc = codex->char_start + codex->code_offset[mid] + 3 * (codex->code_table_num[mid] - ZH_VAGUE_MATCH_HEAD_DEPTH - match_num);
        uint16_t read_length = 3 * (match_num);
        fseek(fp, (long)read_loc, SEEK_SET);
        fread(res_str + 3 * chars_left, sizeof(uint8_t), read_length, fp);
    }
    free(v_idx);
    if (br!= NULL) (*br) = br_read;
    fclose(fp);
    if (chars_left > 0){
        memmove(res_str, res_str + 3 * chars_left, 3 * br_read + 1);
    }
    return 0;
}


/* get split method for pinyin string */
__split_method_list_t* zh_pinyin_get_split(const char* str) {
    if (chk_valid_string(str)) return NULL;
    __split_method_list_t* m_list = mlist_init();
    if (m_list == NULL) return NULL;

    g_word_match_number = 0;
    uint8_t spm[MAX_WORD_LENGTH] = { 0, 0, 0, 0 };
    if (pinyin_dfs(m_list, str, spm, 0, 0)) return NULL;
    return m_list;
}

/* free the */
void zh_pinyin_free_split(__split_method_list_t* m_list) {
    if (m_list != NULL) mlist_destroy(m_list);
    m_list = NULL;
}



#if (USE_ZH_WORD_MATCH == 1)
/* the assumed size of the res_str is  word_num * 3 * (ZH_WORD_MAX_MATCH_DEPTH + 1)+ 1
* if want match maximum number, use MAX_WORD_MATCH_BUFFER_SZ
*/
/*** search for word split method (only used when string length >=2 ) */
static __split_method_t* split_word(const char* str, uint8_t max_split_size) {
    if (max_split_size == 0) return NULL;

    __split_method_list_t* m_list = mlist_init();
    if (m_list == NULL) return NULL;

    g_word_match_number = 0;
    if (code_index[str[0] - 'a'].table_length == 0) return NULL;     /* check if the first char exists */

    /** search for method to split the string */
    uint8_t spm[MAX_WORD_LENGTH] = { 0, 0, 0, 0 };   /* record the  current split method */
    if (pinyin_dfs(m_list, str, spm, 0, 0)) return NULL;

    return m_list;
}


uint8_t zh_match_word(const char* str, char* res_str, uint8_t max_word_num, uint8_t* word_br) {
    if (res_str == NULL || chk_valid_string(str)) return 1; /* must not be too long */ 
    uint8_t split_size = __min(ZH_WORD_MAX_SPLIT_METHODS, max_word_num);
    
    /* 1: just slit and find if there is existing code in table */
    __split_method_list_t* mlist = split_word(str, split_size);
    if (mlist == NULL) return 1;
    
    FILE* fp = fopen(ZH_WORD_DICTIONARY_FILE_NAME, "r");
    if (!fp) {
        ZH_LOG_ERROR("Word Dictionary file \"zh_word_dict.json\" not exist");
        return 1;
    }

    mlist_destroy(mlist);
}

#endif 