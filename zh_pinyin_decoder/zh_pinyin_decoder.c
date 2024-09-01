/**
 ***************************** Declaration ********************************
 * @file           : zh_pinyin_decoder.c
 * @author         : FriedParrot (https://github.com/FriedParrot)
 * @version        : v1.5
 * @date           : 2024-09-01  (last modified)
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
#include <ctype.h>
#include "../CJSON/cJSON.h"
#endif

/************************* private vairables ***************************************/

static int g_word_match_number = 0;
static int g_word_match_max = ZH_PINYIN_MAX_SPLIT_METHODS;

#if (USE_ZH_WORD_MATCH == 1)
static uint8_t word_dict_buffer[ZH_WORD_DICT_BUFFER_SZ];

const uint32_t word_dict_offset[26] = { 0x00, 0x2A92 ,0x12CDB ,0x24184, 0x36696, 0x37ACC, 0x434A0, 0x52F50,
                                        0x00 , 0x621CC, 0x787EB, 0x809A8, 0x8E942, 0x98543, 0x9E815, 0x9ECC8,
                                        0xA4FD2, 0xAFA24, 0xB448E, 0xCE4C5, 0x00, 0x00, 0xDA638, 0xE499F, 0xF745F, 0x10D105};

#endif

/*******************   private function prototypes     ****************************/

static uint8_t chk_valid_string(const char* str);
static uint8_t common_prefix_length(const char* str1, const char* str2);

static __split_method_t* mnode_init(void);
static int mnode_cmp(__split_method_t* m1, __split_method_t* m2);
static __split_method_list_t* mlist_init(void);
static uint8_t mnode_prec(__split_method_t* m);
static void mlist_insert(__split_method_list_t* m_list, __split_method_t* m);
static void mlist_remove(__split_method_list_t* m_list, uint8_t idx);
static void mlist_destroy(__split_method_list_t* m_list);

static uint8_t get_match_idx(const char* str, int8_t* match_idx, const uint8_t vag_num, uint8_t* vag_idx_arr, int8_t* vag_br);
static uint8_t pinyin_dfs(__split_method_list_t* m_list, const char* str, uint8_t* spm, uint8_t m_wt, uint8_t depth);

#if (USE_ZH_WORD_MATCH == 1)

static __word_block_t* wordblock_init(uint8_t type);
static void wordblock_append(__word_block_t** head, __word_block_t* w);
static void wordblock_destroy(__word_block_t* w);

static int str_match_cjson(const char* str, __split_method_t* m, cJSON* item);
static cJSON* cjson_parse_piece(char* buf, uint32_t* bytes_left);
static __word_block_t* word_dict_exit(char** res_str);
static __word_block_t* word_dict_search(const char* str, __split_method_list_t* m_list);

#endif

/************************   private functions   *********************************/

/**
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

/**
 * @brief  get the common prefix length of two string
 * @return common length
 */
static uint8_t common_prefix_length(const char* str1, const char* str2) {
    const char* start1 = str1;
    const char* start2 = str2;
    while (*str1 && *str2 && *str1 == *str2) {
        str1++; str2++;
    }
    return str1 - start1;
}

/**
 * @brief init method node (sigle linked list node)
 * @return __split_method_t* node pointer
 */
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
    m->cm_num = 0;
    return m;
}

/**
* @brief compare the method (decide the sequence that method in list.) 
* @note compare insert sequence : insert seq : 1.m->length(shorter)  2.m->wt(larger)  3.m->spm(smaller)
* @note  m1 and m2 must not be NULL 
* @return < 0 m1 is before m2,  = 0: same , >0: m1 is after m2
*/
static int mnode_cmp(__split_method_t* m1, __split_method_t* m2) {
    if (m1->length != m2->length) return m1->length - m2->length;
    if (m1->wt != m2->wt) return  m2->wt - m1->wt;
    return memcmp(m1->spm, m2->spm, m1->length - 1);
}

/**
 * @brief check if a split method is precise match method
 * @param m method for check
 * @return uint8_t 0: not precise (or NULL), 1: precise
 */
static uint8_t mnode_prec(__split_method_t* m){
    if (m == NULL) return 0;
    uint16_t num = (((1 << MAX_WORD_LENGTH) -1) & ~((1 << MAX_WORD_LENGTH - m->length) - 1));
    return num == m->wt;
}

/* init the split method linked list */
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

/* insert a split method element to linked list */
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

/* remove the element on the index of list */
static void mlist_remove(__split_method_list_t* m_list, uint8_t idx){
    if (m_list == NULL|| m_list->head == NULL || idx >= m_list->num) return;
    __split_method_t* pre = m_list->head;
    if (idx == 0) {
        m_list->head = pre->next;
        zh_buffer_free(pre);
        m_list->num--;
        return;
    }
    for (uint8_t i = 0; i < idx - 1; i++) {
        pre = pre->next;
    }
    __split_method_t* nxt = pre->next->next;
    zh_buffer_free(pre->next);
    pre->next = nxt;
    m_list->num--;
}

/* free single linked list */
static void mlist_destroy(__split_method_list_t* m_list) {
    if (m_list == NULL) return;

    __split_method_t* m = m_list->head;
    while (m != NULL) {
        __split_method_t* tmp = m;
        m = m->next;
        zh_buffer_free(tmp);
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
* @brief  dfs algorithm function for pinyin split method
* @note   dfs function terminate when search number >= ZH_PINYIN_MAX_SPLIT_METHODS to optimize performance.
*         change ZH_PINYIN_MAX_SPLIT_METHODS if you need more split method.
* @param  m_list split method list (we concatenate the split method into this list)
* @param  str input string
* @param  spm split method array (use MAX_WORD_LENGTH zero array when input) 
* @param  m_wt split method weight (use 0 when input)
* @param  depth split method depth (use 0 when input)
* @return 0: success, 1: fail
*/
static uint8_t pinyin_dfs(__split_method_list_t* m_list, const char* str, uint8_t* spm, uint8_t m_wt, uint8_t depth) {
    uint8_t start_idx = (depth == 0) ? 0 : spm[depth - 1];
    /* termiate conditions -> string end */
    if (start_idx == strlen(str)) {  /* note : depth must not be 0 here */
        /* word split successfully, concanate the split method into m_list */
        __split_method_t* m = mnode_init();
        if (m == NULL) return 1;
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
        // ZH_LOG_INFO("reach the maximum search length, search terminated");
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

#if (USE_ZH_WORD_MATCH == 1)

/// @param type refer to @defgroup word_block_type in zh_pinyin_decoder.h
static __word_block_t* wordblock_init(uint8_t type) {
    __word_block_t* w = zh_buffer_malloc(sizeof(__word_block_t));
    if (!w) return NULL;
    w->type = type;
    switch(type){
        case WORD_BLK_TYPE_CODES:
            w->num.code_nbr = 0;
            break;
        case WORD_BLK_TYPE_WORDS:
            w->num.word_nbr = NULL;
            break;
        default:
            return NULL;
    }
    w->buf = NULL;
    w->next = NULL;
    return w;
}

/** append a word block to the end of the head */
static void wordblock_append(__word_block_t** head, __word_block_t* w){
    if (head == NULL || w == NULL) return;
    if (*head == NULL) {
        *head = w;
        return;
    }
    __word_block_t* tmp = (*head);
    while (tmp->next != NULL) tmp = tmp->next; /* find the last block */
    tmp->next = w;
}

/** destroy the block one by one */
static void wordblock_destroy(__word_block_t* w){
    if (w == NULL) return;
    while (w != NULL){
        if (w->type == WORD_BLK_TYPE_WORDS){
            zh_buffer_free(w->num.word_nbr);
        }
        zh_buffer_free(w->buf);
        __word_block_t* tmp = w;
        w = w->next;
        zh_buffer_free(tmp);
    }
}

/**
* @brief test if the string split matches a json object 
* @param str string to match 
* @param m   split method of this string 
* @param item cjson object 
* @return 0: search succeed  1: not match (match failed)
*/
static int str_match_cjson(const char* str, __split_method_t* m, cJSON* item) {
    char* str2 = item->string;
    if (strlen(str) + m->length - 1 > strlen(str2)) return 1; /* string length check */

    uint8_t loc1 = 0, loc2 = 0;
    /* compare the string and the name of item */
    for (int i = 0; i < m->length - 1; i++) {
        uint8_t p = m->spm[i], sz = p - loc1;
        if (strncmp(str + loc1, str2 + loc2, sz) != 0) return 1;
        if ((m->wt & 1 << (MAX_WORD_LENGTH - 1 - i)) && str2[loc2 + sz] != ' ') return 1; /* for precise match */
        /* locate the pointer after ] signal */
        loc1 += sz;
        for (loc2 += sz; str2[loc2] && str2[loc2] != ' '; loc2++);
        if (str2[loc2] == '\0') return 1;  /* terminate when str2 is end */
        loc2++; /* move to next char after ' ' */
    }
    /* check if the last piece fit */
    uint8_t sz = m->spm[m->length - 1] - loc1;
    if (m->wt & 1 << (MAX_WORD_LENGTH - m->length)) {
        if (strncmp(str + loc1, str2 + loc2, sz) == 0 && str2[loc2 + sz] == '\0') return 0;
    }
    else {
        if (strncmp(str + loc1, str2 + loc2, sz) != 0) return 1;
        for (loc2 += sz; str2[loc2]; loc2++) {
            if (str2[loc2] == ' ') return 1;
        }
        return 0;
    }
    return 1;
}

/* parse cJSON file piece, return parse result */
static cJSON* cjson_parse_piece(char* buf, uint32_t* bytes_left) {
    uint8_t* pstart, * pend;
    uint8_t  conn[2];           /* file piece connector array */
    pstart = buf;
    while (*pstart && isspace(*pstart)) pstart++;
    if (pstart == '\0') return NULL;
    /* build the left bracket "{" for piece */
    if (*pstart != '{') {
        while (*pstart && *pstart != ']') pstart++;
        if (!pstart) return NULL;
        pstart++;
        /* can't find location for parse */
        while (isspace(*pstart)) pstart++;
        if (pstart - buf == ZH_WORD_DICT_BUFFER_SZ) return NULL;
        if (*pstart == ',') {
            *pstart = '{';
        }
        else { /* *pstart == '}' or other case */
            return NULL;
        }
    }
    /* construct the final "}" for piece */
    for (pend = buf + ZH_WORD_DICT_BUFFER_SZ - 2; pend != pstart; pend--) {
        if (*pend == ']') {
            uint8_t* pend_tmp = pend;  /* record address */
            pend++;
            while (isspace(*pend)) pend++;
            /* if read to the end but can't find signal */
            if (pend - buf == ZH_WORD_DICT_BUFFER_SZ) {
                pend = pend_tmp;      /* relocate pointer */
                continue;
            }
            memcpy(conn, pend, 2);
            *pend = '}';
            if (ZH_WORD_DICT_BUFFER_SZ - (pend - buf) != 1) {
                *(pend + 1) = '\0';
            }
            break;
        }
    }
    if (pend == pstart) return NULL;   /* can't get reasonable parse location */
    cJSON* item = cJSON_Parse(pstart);
    memcpy(pend, conn, 2);
    if (bytes_left) *bytes_left = ZH_WORD_DICT_BUFFER_SZ - (pend - buf);  /* size left in current buffer */
    return item;
}

/* auxiliary function for exit */
static __word_block_t* word_dict_exit(char** res_str) {
    zh_buffer_free(*res_str);
    res_str = NULL;
    return NULL;
}

/**
* @brief reshape the word block according to search type
* @param w_res   the word block to be modified  
* @param search_state  refer to @defgroup word_search_state
* @return the modified w_res pointer
*/
static __word_block_t* wordblock_reshape(__word_block_t* w_res, uint8_t search_state) {
    if (!w_res) return NULL;
    if (search_state == WORD_SEARCH_STATE_CODE_PREC_MATCH && w_res->num.code_nbr > ZH_WORD_CODE_DISP_NUM) {
        /* when it not reach, do nothing */
        __word_block_t* w3 = wordblock_init(WORD_BLK_TYPE_CODES);
        if (w3 == NULL) return NULL;

        uint16_t buf1_sz = 3 * ZH_WORD_CODE_DISP_NUM;
        uint16_t buf2_sz = 3 * (w_res->num.code_nbr - ZH_WORD_CODE_DISP_NUM);
        uint8_t* new_buf1 = zh_buffer_malloc(buf1_sz + 1);
        uint8_t* new_buf2 = zh_buffer_malloc(buf2_sz + 1);
        if (!new_buf1 || !new_buf2)
        {
            if (new_buf1) zh_buffer_free(new_buf1);
            if (new_buf2) zh_buffer_free(new_buf2);
            return NULL;
        }
        else
        {
            memcpy(new_buf1, w_res->buf, buf1_sz); new_buf1[buf1_sz] = '\0';
            memcpy(new_buf2, w_res->buf + 3 * ZH_WORD_CODE_DISP_NUM, buf2_sz); new_buf2[buf2_sz] = '\0';
            w3->buf = new_buf2;
            w3->num.code_nbr = w_res->num.code_nbr - ZH_WORD_CODE_DISP_NUM;
            zh_buffer_free(w_res->buf);
            w_res->buf = new_buf1;
            w_res->num.code_nbr = ZH_WORD_CODE_DISP_NUM;
            wordblock_append(&w_res, w3);
        }
    }
    else { 
        // no match or vague match
        /* reshape the search code after the word result */
        __word_block_t* tmp = w_res;
        if (w_res->next!=NULL) {
            w_res = w_res->next;
            tmp->next = NULL;
            w_res->next = tmp;
        }
    }
    return w_res;
}

/**
 * @brief search the split method in word dictionary and store the result in res_str 
 * @param str       input string
 * @param m_list    split method list
 * @note            MAX_WORD_BLK_WORD_NUM is assumed to be maximum word results, and 
 *                  MAX_CODE_SEARCH_TYPES is assumed to be maximum code search types 
 * @return pointer of resulted word block 
 * @note : after filtering buffer, there may be at most 1 split method of single code. 
 *       in this case, no matter the signal(corresponding bit of wt) is vague or precise, we would use vague search for result
 *       whether signal is precise determines whether we split the block into 2 parts for better search.
 */
static __word_block_t* word_dict_search(const char* str, __split_method_list_t* m_list){
    uint16_t read_buf_num = 0;   /* number of buffers readed */
    uint8_t  search_state = WORD_SEARCH_STATE_CODE_NO_MATCH;
    if (m_list == NULL || m_list->head == NULL) return NULL;
    __word_block_t* w_res = NULL;
    
    char* res_str = zh_buffer_malloc(MAX_WORD_BLK_BUFFER_SZ);
    if (!res_str) return NULL;  /* Memory allocation failed  */ 

    /** process single code match case */
    char code_str[MAX_WORD_CODE_LENGTH + 1];
    strncpy(code_str, str, m_list->head->spm[0]);
    code_str[m_list->head->spm[0]] = '\0';
    uint8_t br = 0;
    uint8_t* buf = NULL;
    __word_block_t* w1 = wordblock_init(WORD_BLK_TYPE_CODES);
    uint8_t res_tmp = zh_match_code_vague(code_str, res_str, MAX_CODE_SEARCH_TYPES, &br);
    if (res_tmp == 0) buf = zh_buffer_malloc(3 * br + 1);
    if (w1 == NULL || res_tmp || buf == NULL) return word_dict_exit(&res_str);
    
    for (int i = 0; i < br; i++) {
        memcpy(buf + 3 * i, res_str + 3 * (br - 1 - i), 3); 
    }
    buf[3 * br] = '\0';
    w1->num.code_nbr = br;
    w1->buf = buf;
    wordblock_append(&w_res, w1); /* append the result to word block */
    if (m_list->head->length == 1) {
        search_state = mnode_prec(m_list->head) ? WORD_SEARCH_STATE_CODE_PREC_MATCH : WORD_SEARCH_STATE_CODE_VAGUE_MATCH;
        memset(res_str, 0, MAX_WORD_BLK_BUFFER_SZ);
        mlist_remove(m_list, 0);          /* delete head node */
    }
    else {
        search_state = WORD_SEARCH_STATE_CODE_NO_MATCH;
    }
    if (m_list->num == 0) {
        zh_buffer_free(*res_str);
        return w_res;
    };

    /** process multi-code word match case */
    FILE* fp = fopen(ZH_WORD_DICTIONARY_FILE_NAME, "r");
    __word_block_t* w2 = wordblock_init(WORD_BLK_TYPE_WORDS);
    uint8_t* word_nbr = zh_buffer_malloc(MAX_WORD_BLK_WORD_NUM + 1);
    word_nbr[0] = 0;

    if (!fp || !w2 || !word_nbr) {
        ZH_LOG_WARNING("Word Dictionary file \"zh_word_dict.json\" not exist");
        zh_buffer_free(res_str);
        wordblock_destroy(w2);
        wordblock_destroy(w_res);
        return NULL;
    }
    w2->num.word_nbr = word_nbr;
    fseek(fp, word_dict_offset[str[0] - 'a'], SEEK_SET);

    uint8_t  word_buff_idx = 0;    /* index of word_nbr */
    uint8_t  word_buff_ptr = 0;    /* location pointer  */
    if (fread(word_dict_buffer, sizeof(uint8_t), sizeof(word_dict_buffer), fp) == 0) {
        zh_buffer_free(*res_str);
        return w_res;
    }
    /*  parse word dictionary json file */
    while (m_list->num > 0 && read_buf_num < ZH_WORD_MAX_BUFFER_READ) {
        /* Parse JSON object and do search operation */
        uint32_t bytes_left = 0;
        cJSON *item = cjson_parse_piece(word_dict_buffer, &bytes_left);
        if (item == NULL || item->child == NULL || item->child->string[0] > str[0]) {
            break;  /* json file end or can't parse */
        }
        for (cJSON* js = item->child; js != NULL; js = js->next) {
            __split_method_t* m = m_list->head;
            for (int i = 0; i < m_list->num; i++) {
                if (str_match_cjson(str, m, js)) {
                    m = m->next;
                    continue;
                }
                /* the string match the json object */
                uint8_t sz = cJSON_GetArraySize(js);
                for (int j = 0; j < sz; j++) {
                    char* m_str = cJSON_GetArrayItem(js, j)->valuestring;
                    uint8_t len = m->length * 3;
                    memcpy(res_str + word_buff_ptr, m_str, len);
                    res_str[word_buff_ptr + len] = '\0';
                    w2->num.word_nbr[word_buff_idx] = m->length;
                    word_buff_ptr += len;
                    word_buff_idx ++;
                    if (word_buff_idx == MAX_WORD_BLK_WORD_NUM) break;
                }
                m->cm_num++;
                if (mnode_prec(m) || m->cm_num >= ZH_WORD_VAGE_SEARCH_DEPTH)
                {
                    mlist_remove(m_list, i);
                }
                break;    /* once a case match, we don't consider other case */
            }
            if (word_buff_idx >= MAX_WORD_BLK_WORD_NUM) break;
        }
        cJSON_Delete(item);
        /** re-read file and concanate the buffer */
        memmove(word_dict_buffer, word_dict_buffer + ZH_WORD_DICT_BUFFER_SZ - bytes_left, bytes_left);
        word_dict_buffer[0] = '{';
        if (feof(fp)) break;
        fread(word_dict_buffer + bytes_left, sizeof(uint8_t), ZH_WORD_DICT_BUFFER_SZ - bytes_left, fp);
        read_buf_num++;
    }
    w2->num.word_nbr[word_buff_idx] = 0;

    size_t tmp = strlen(res_str);
    if (tmp > 0) {
        uint8_t* buf = zh_buffer_malloc(tmp + 1);
        if (buf == NULL) {
            ZH_LOG_ERROR("buffer malloc failed");
            wordblock_destroy(w2);  /* we just not append w2, but still retain w1 */ 
        }
        else {
            memcpy(buf, res_str, tmp + 1);
            w2->buf = buf;
            wordblock_append(&w_res, w2);
        }
    }
    else wordblock_destroy(w2);

    /* reshape the search result */
    w_res = wordblock_reshape(w_res, search_state);
    
    zh_buffer_free(res_str);
    return w_res;
}

#endif

/********************************** public functions ***************************************/

/**
 * @brief       Match the utf-8 code in PinYin table precisely 
 * @param       str : string to match
 * @param       res_str : result string (must pre-malloc size at least 3 * num bytes + 1(MAX_CODE_BUFF_SZ is recommended)
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
    size_t read_loc = codex->char_start + codex->code_offset[match_idx] + (codex->code_table_num[match_idx] - br_read) * 3;
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
 * @param       res_str : result string (must pre-malloc size at least 3 * num bytes + 1(MAX_CODE_BUFF_SZ is recommended)
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
        zh_buffer_free(v_idx);
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
    zh_buffer_free(v_idx);
    if (br!= NULL) (*br) = br_read;
    fclose(fp);
    if (chars_left > 0){
        memmove(res_str, res_str + 3 * chars_left, 3 * br_read + 1);
    }
    return 0;
}

/**
 * @brief get split method object in a mixed pinyin string (not filtered)
 * @param str string to get 
 * @return NULL if no match found
 */
__split_method_list_t* zh_pinyin_get_split(const char* str) {
    if (chk_valid_string(str)) return NULL;
    __split_method_list_t* m_list = mlist_init();
    if (m_list == NULL) return NULL;

    g_word_match_number = 0;
    uint8_t spm[MAX_WORD_LENGTH] = { 0, 0, 0, 0 };
    if (pinyin_dfs(m_list, str, spm, 0, 0) || m_list->head == NULL) return NULL;
    return m_list;
}

/**
 * @brief filter the split method list (remove repeat vague split method)
 * @note  it can filter the method that have the same splitting method but different weight
 * @param m_list split method list
 * @return 0: success, 1: fail
 */
uint8_t zh_pinyin_filter_split(__split_method_list_t* m_list) {
    if (m_list == NULL || m_list->num == 0) return 1;
    __split_method_t* m1 = m_list->head, * m2 = m1->next;
    uint8_t num = 1;   /*  since m1 is head -> alerady 1 method */
    while (m2 != NULL && num < ZH_PINYIN_MAX_FILTER_TYPES) {
        uint8_t rep = 0;
        for (__split_method_t* mt = m_list->head; mt != m2; mt = mt->next) {
            if (mt->length == m2->length && memcmp(mt->spm, m2->spm, mt->length - 1) == 0) {
                rep = 1;
                break;
            }
        }
        if (rep) {
            __split_method_t* m = m2;
            m2 = m2->next;
            m1->next = m2;
            zh_buffer_free(m);
            m_list->num--;
        }
        else { /* not repeat */
            m1 = m2;
            m2 = m2->next;
            num++;
        }
    }
    /* deposite the element after m1  */
    __split_method_t* m = m1->next;
    m1->next = NULL;
    while (m != NULL) {
        __split_method_t* tmp = m;
        m = m->next;
        zh_buffer_free(tmp);
        m_list->num--;
    }
    return 0;
}

/**
 * @brief free the split method object
 */
void zh_pinyin_free_split(__split_method_list_t* m_list) {
    if (m_list != NULL) mlist_destroy(m_list);
    m_list = NULL;
}

#if (USE_ZH_WORD_MATCH == 1)

/// @brief match the word in a mixed pinyin string
/// @param str 
/// @param num      maximum number of words and codes for read (MAX_WORD_MATCH_NUM is recommended)
/// @param sp       prior split method for str (transfer an object for return result)
/// @return 
__word_block_t* zh_match_word(const char* str, __split_method_t *sp) {
    if (chk_valid_string(str)) return NULL;
    uint8_t res = 0;

    /* split pinyin */
    __split_method_list_t* m_list = zh_pinyin_get_split(str);
    if (zh_pinyin_filter_split(m_list)) return NULL;   /* filter the split string method */
    
    if (sp != NULL) memcpy(sp, m_list->head, sizeof(__split_method_t));
    __word_block_t *w = word_dict_search(str, m_list);
    zh_pinyin_free_split(m_list);
    return w;
}

void zh_word_free_match(__word_block_t* blk){
    wordblock_destroy(blk);
}

#endif
