#ifndef  __ZH_PINYIN_DECODER_H
#define  __ZH_PINYIN_DECODER_H

#ifdef __cplusplus
    extern "C"{
#endif // __cplusplus

#include <stdint.h>

#define ZH_CODE_TABLE_FILE_NAME "zh_pinyin_decoder/bin/zh_pinyin.bin"   // code table name 
#define ZH_WORD_DICTIONARY_FILE_NAME "zh_pinyin_decoder/bin/zh_word_dict.json"  // dictionary json file name 

#define MAX_PINYIN_BUFFER_SZ      400

/* when length of string for vague search = 1 */
#define ZH_VAGUE_SEARCH_TYPES_SINGLE 25      // most 25 types of other vague search 
#define ZH_VAGUE_SEARCH_DEPTH_SINGLE 1       // 1 search depth when 1 character is use 

/* when length of string for vague search > 1 */
#define ZH_VAGUE_SEARCH_TYPES        10      // most 10 types of other vague search
#define ZH_VAGUE_SEARCH_DEPTH        3       // 2 search depth for vague match 


#define ZH_VAGUE_MATCH_HEAD_DEPTH   12      // 12 characters at first if there is match
#define ZH_VAGUE_MAX_LENGTH         130     // maximum 150 length of vague search 

uint8_t zh_match_code_prec(const char* str, char* res_str, uint8_t num, uint8_t* br);
uint8_t zh_match_code_vague(const char* str, char* res_str, uint8_t num, uint8_t* br);

#ifdef __cplusplus
    }
#endif // 

#endif 
