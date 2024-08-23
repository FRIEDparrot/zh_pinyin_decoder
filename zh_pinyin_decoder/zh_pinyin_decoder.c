#include <stdio.h>
#include <string.h>
#include "zh_pinyin_decoder.h"
#include "zh_code_table.h"
#include "../CJSON/cJSON.h"

/**
 * @brief       Match the utf-8 code in PinYin table precisely, else return 1
 * @param       str : string to match
 * @param       res_str : result string (must pre-malloc size at least 3 * num bytes + 1(MAX_PINYIN_BUFFER_SZ is recommended)
 * @param       num : number of zh Character to read (set to ZH_VAGUE_MAX_LENGTH if want all)
 * @param       br : number of zh Character readed
 * @retval      0: match succeed , 1: read error or nothing to match
 */
uint8_t zh_match_code_prec(const char* str, char* res_str, uint8_t num, uint8_t* br){
    if (str == NULL || res_str == NULL) return 1;
    uint8_t res = 1; int i;
    uint8_t idx = str[0] - 'a';
    if (code_index[idx].code_table == NULL){
        return 1;
    };
    FILE* fp = fopen(ZH_CODE_TABLE_FILE_NAME, "rb");
    if (!fp) return 1;
    const __code_index_t* codex = (&code_index[idx]);
    for (i = 0; i < codex->table_length; i++) {
        if (strcmp(str, codex->code_table[i]) != 0) continue;
        uint16_t read_length = 0;
        if (br!= NULL) {
            (*br) = num > codex->code_table_num[i] ? codex->code_table_num[i] : num;
            read_length = (*br) * 3;
        }
        else {
            read_length = num * 3;
        }
        uint32_t read_loc = codex->char_start + codex->code_offset[i] + (codex->code_table_num[i] * 3 - read_length);
        fseek(fp, (long)(read_loc), SEEK_SET);
        fread(res_str, sizeof(uint8_t), read_length, fp);
        res_str[read_length] = '\0';
        res = 0;
        break;
    }
    fclose(fp);
    return res;
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
    if (str == NULL || res_str == NULL) return 1;
    uint8_t idx = str[0] - 'a';
    if (code_index[idx].code_table == NULL) {
        return 1;
    };
    FILE* fp = fopen(ZH_CODE_TABLE_FILE_NAME, "rb");
    if (!fp) return 1;
    
    const __code_index_t* codex = (&code_index[idx]);

    uint8_t chars_left = num > ZH_VAGUE_MAX_LENGTH? ZH_VAGUE_MAX_LENGTH: num;
    uint8_t array_size = chars_left;        /* array size used */ 
    int8_t match_idx = -1, match_num = 0;    /* fill last "match num" */
    
    uint8_t search_width, search_depth;
    if (strlen(str) == 1) {
        search_width = ZH_VAGUE_SEARCH_TYPES_SINGLE; search_depth = ZH_VAGUE_SEARCH_DEPTH_SINGLE;
    }
    else {
        search_width = ZH_VAGUE_SEARCH_TYPES; search_depth = ZH_VAGUE_SEARCH_DEPTH;
    }
    res_str[3 * chars_left] = '\0';

    for (int i = 0; i < codex->table_length; i++) {  
        /** check for any precise match and record, put it in the end of res_str */
        if (strcmp(str, codex->code_table[i]) == 0) {
            match_idx = i;
            match_num = codex->code_table_num[i] > ZH_VAGUE_MATCH_HEAD_DEPTH ? ZH_VAGUE_MATCH_HEAD_DEPTH : codex->code_table_num[i];
            match_num = match_num > chars_left ? chars_left : match_num;
            chars_left -= match_num;
            fseek(fp, codex->char_start + codex->code_offset[i] + 3 * (codex->code_table_num[i] - match_num) , SEEK_SET);
            fread(res_str + 3 * (chars_left), sizeof(uint8_t),  3 * match_num, fp);
            break;
        }
    }

    uint8_t types = 0;      /* type number of vague match cases */
    uint8_t any_match = 0;
    /* search and fill the buffer from tail to head */
    for (int i = 0; i < codex->table_length; i++) {
        if (chars_left == 0 || types == search_width) break;
        if (match_idx == i) {
            any_match = 1; continue;
        }
        if (strncmp(str, codex->code_table[i], strlen(str)) == 0) {
            types++;
            uint8_t vague_match_num = codex->code_table_num[i] > search_depth ? search_depth : codex->code_table_num[i];
            vague_match_num = vague_match_num > chars_left ? chars_left : vague_match_num;
            chars_left -= vague_match_num;
            fseek(fp, codex->char_start + codex->code_offset[i] + 3 * (codex->code_table_num[i] - vague_match_num), SEEK_SET);
            fread(res_str + (3 * chars_left), sizeof(uint8_t), 3 * vague_match_num, fp);
            any_match = 1;
        }
    }
    if (!any_match) return 1;

    /* when the first match still not match end */
    if (match_num == ZH_VAGUE_MATCH_HEAD_DEPTH){
        uint8_t match_num2 = codex->code_table_num[match_idx] - ZH_VAGUE_MATCH_HEAD_DEPTH;
        match_num2 = match_num2 > chars_left ? chars_left : match_num2;
        if (match_num2 > 0) {
            chars_left -= match_num2;
            fseek(fp, codex->char_start + codex->code_offset[match_idx] + 3 * (codex->code_table_num[match_idx] - ZH_VAGUE_MATCH_HEAD_DEPTH - match_num2), SEEK_SET);
            fread(res_str + (3 * chars_left), sizeof(uint8_t), 3 * match_num2, fp);
        }
    }
    if (br)(*br) = array_size - chars_left;
    if (chars_left > 0) {
        memmove(res_str, res_str + 3 * (chars_left), 3 * (array_size - chars_left) + 1);
    }
    return 0;
}

//uint8_t zh_match_word_prec(const char* str, char* res_str, ) {
//    
//}
