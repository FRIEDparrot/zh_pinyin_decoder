#include <stdio.h>
#include <string.h>
#include "zh_pinyin_decoder.h"
#include "zh_code_table.h"
#include "../CJSON/cJSON.h"

#if (USE_ZH_HASH_BOOST == 1)
#include "zh_hash_boost.h"
#endif

/*
* @brief check if a string input is valid for input method
*/
static uint8_t zh_chk_valid_string(const char* str) {
    if (str == NULL || strlen(str) >  ZH_MAX_STRING_LENGTH) return 1;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] < 'a' || str[i] > 'z') return 1;
    }
    return 0;
}

/*
* @brief match string from pinyin table 
* @note "str" must be a legal string consits of 'a' to 'z' 
* @param str            string for match 
* @param match_idx      index of precise match  (if no precise match found,return -1)
* @param vag_num        max number for vague match 
* @param vag_idx_arr    index array of vague match (at least size vague num)
* @param vag_br         number of vague match found 
*/
static uint8_t zh_get_match_idx(const char* str, int8_t* match_idx, const uint8_t vag_num, uint8_t* vag_idx_arr, int8_t* vag_br) {
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
 * @brief       Match the utf-8 code in PinYin table precisely 
 * @param       str : string to match
 * @param       res_str : result string (must pre-malloc size at least 3 * num bytes + 1(MAX_PINYIN_BUFFER_SZ is recommended)
 * @param       num : number of zh Character to read (set to ZH_VAGUE_MAX_LENGTH if want all)
 * @param       br : number of zh Character readed
 * @retval      0: match succeed , 1: read error or nothing to match
 */
uint8_t zh_match_code_prec(const char* str, char* res_str, uint8_t num, uint8_t* br){
    if (res_str == NULL || zh_chk_valid_string(str)) return 1;
    FILE* fp = fopen(ZH_CODE_TABLE_FILE_NAME, "rb");
    if (fp == NULL) {
        ZH_LOG_ERROR("code table file \"zh pinyin.bin\" not exist");
        return 1;
    }
    int8_t  match_idx = -1;
    uint8_t v_br = 0;

    if (zh_get_match_idx(str, &match_idx, 0, NULL, &v_br) || match_idx < 0) {
        fclose(fp);
        return 1;
    }
    uint8_t idx = str[0] - 'a';
    const __code_index_t* codex = (&code_index[idx]);
    
    uint8_t br_read = codex->code_table_num[match_idx] > num ? num :codex->code_table_num[match_idx];
    uint32_t read_loc = codex->char_start + codex->code_offset[match_idx] + \
        (codex->code_table_num[match_idx] - br_read) * 3;
    uint16_t read_length = 3 * br_read;
    
    fseek(fp, (long)read_loc, SEEK_SET);
    fread(res_str,sizeof(uint8_t), read_length ,fp);
    res_str[read_length] = '\0';
    if (br != NULL) (*br) = br_read;
    fclose(fp);
    return 0;
}

/**
 * @brief       vague match for the input pinyin code
 * @param       str : string to match
 * @param       res_str : result string (must pre-malloc size at least 3 * num bytes + 1(MAX_PINYIN_BUFFER_SZ is recommended)
 * @param       num : number of zh Character to read (set to ZH_VAGUE_MAX_LENGTH if want all)
 * @param       br : number of zh Character readed
 * @return      0: match succeed , 1: read error or nothing to match
 * @bug         when str starts with '0' may cause fault 
 */
uint8_t zh_match_code_vague(const char* str, char* res_str, uint8_t num, uint8_t* br) {
    if (res_str == NULL || zh_chk_valid_string(str)) return 1;

    int8_t mid = 0;
    uint8_t* v_idx = zh_buffer_malloc(num);
    if (v_idx == NULL) {
        ZH_LOG_ERROR("storge full, allocate buffer failed");
        return 1;
    }
    uint8_t  v_br = 0;
    if (zh_get_match_idx(str, &mid, num, v_idx, &v_br)) return 1;
    
    FILE* fp = fopen(ZH_CODE_TABLE_FILE_NAME, "rb");
    if (!fp) {
        free(v_idx);
        ZH_LOG_ERROR("code table file \"zh pinyin.bin\" not exist");
        return 1;
    }
    uint8_t idx = str[0] - 'a';
    
    const __code_index_t* codex = (&code_index[idx]);
    
    res_str[3 * num] = '\0';

    uint8_t chars_left = num;  /* left space in num */
    uint8_t br_read = 0;       /* character that has readed */
    if (chars_left > 0 &&  mid >= 0) {
        uint8_t match_num = codex->code_table_num[mid] > chars_left ? chars_left : codex->code_table_num[mid];
        match_num = __min(match_num, ZH_VAGUE_MATCH_HEAD_DEPTH);
        chars_left -= match_num; br_read += match_num;
        
        uint32_t read_loc = codex->char_start + codex->code_offset[mid] + 3 *(codex->code_table_num[mid] - match_num);
        uint16_t read_length = 3 * (match_num);
        
        fseek(fp, (long)read_loc, SEEK_SET);
        fread(res_str + 3 * chars_left, sizeof(uint8_t), read_length, fp);
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
        memmove(res_str, res_str + 3 * chars_left, 3 * (num - chars_left) + 1);
    }
    
    return 0;
}

/* match word but input must be precise */
//uint8_t zh_word_split(const char* str, __word_match_method_t* methods, uint8_t size){
//    uint8_t idx = str[0];
//    __code_index_t* codex = (&code_index[idx]);
//    uint8_t cmp_num = strlen(str) > ZH_WORD_MAX_CODE_LENGTH ? ZH_WORD_MAX_CODE_LENGTH : strlen(str);
//    /* check precise compare */
//
//#if (USE_ZH_BLOOM_HASH_BOOST == 0)
//    for (int i = 0; i < codex->table_length; i++) {
//        if (strncmp(codex->code_table[i], str, strlen(codex->code_table[i])) == 0) {
//
//        }
//    }
//#endif
//}

/* the assumed size of the res_str is  word_num * 3 * (ZH_WORD_MAX_MATCH_DEPTH + 1)+ 1
* if want match maximum number, use MAX_WORD_MATCH_BUFFER_SZ  
*/
//uint8_t zh_match_word_prec(const char* str, char* res_str, uint8_t word_num, uint8_t* word_br) {
//    if (str == NULL || res_str == NULL) return 1;
//    if (strlen(str) > ZH_WORD_MAX_INPUT_LENGTH) return 1;   /* must not be too long */
//    for (int i = 0; i < strlen(str); i++) {  /* check all the string is legal */
//        if (str[i] < 'a' || str[i] > 'z') return 1;
//    }
//    FILE* fp = fopen(ZH_WORD_DICTIONARY_FILE_NAME, 'r');
//    if (!fp) return 1;
//    
//    uint8_t match_num = word_num > ZH_WORD_MAX_MATCH_NUMBER ? ZH_WORD_MAX_MATCH_NUMBER : word_num;
//    /* 1: just slit and find if there is existing code in table */
//    __word_match_method_t* match_methods = zh_buffer_malloc(sizeof(__word_match_method_t) * match_num);
//    if (match_methods == NULL) {
//        ZH_LOG_ERROR("Allocate word buffer failed");
//        return 1;
//    }
//}
