#ifndef  __ZH_PINYIN_DECODER_H
#define  __ZH_PINYIN_DECODER_H

#ifdef __cplusplus
    extern "C"{
#endif // __cplusplus

#include <stdint.h>
#include <stdlib.h>

#define USE_ZH_WORD_MATCH   1   /* use match word support option  */
#define USE_ZH_HASH_BOOST   0   /* option to use the hash table method for speed up search  */

#if (USE_ZH_WORD_MATCH == 1) && (USE_ZH_HASH_BOOST == 0)
    #pragma message("USE_ZH_HASH_BOOST is recommended for better performance when matching word is required")
#endif

#define ZH_CODE_TABLE_FILE_NAME      "zh_pinyin_decoder/bin/zh_pinyin.bin"   // code table name 
#define ZH_WORD_DICTIONARY_FILE_NAME "zh_pinyin_decoder/bin/zh_word_dict.json"  // dictionary json file name 

#define zh_buffer_malloc  malloc
#define zh_buffer_free    free

/********************************** LOG Setttings *********************************/

#define ZH_USE_LOG        1
#if (ZH_USE_LOG == 0)
#define ZH_LOG_INFO(...)       do{}while(0)
#define ZH_LOG_ERROR(...)      do{}while(0)
#define ZH_LOG_WARNING(...)    do{}while(0)
#else
#define ZH_LOG_INFO(fmt,...)       printf("[Info]    " fmt " at %s : %s, line %d\n", __VA_ARGS__,__FILE__ ,__func__,__LINE__)
#define ZH_LOG_WARNING(fmt,...)    printf("[Warning] " fmt " at %s : %s, line %d\n", __VA_ARGS__,__FILE__ ,__func__,__LINE__)
#define ZH_LOG_ERROR(fmt,...)      printf("[Error]   " fmt " at %s : %s, line %d\n", __VA_ARGS__,__FILE__ ,__func__,__LINE__)
#endif

/**************************** CODE MATCH Settings *********************************/

#define ZH_MAX_STRING_LENGTH         20      // not match 
#define ZH_VAGUE_MATCH_HEAD_DEPTH    10      // 10 characters at first if there is match

/* when length of string for vague search = 1 */
#define ZH_VAGUE_SEARCH_TYPES_SINGLE 25      // most 25 types of other vague search 
#define ZH_VAGUE_SEARCH_DEPTH_SINGLE 1       // 1 search depth when 1 character is use 

/* when length of string for vague search > 1 */
#define ZH_VAGUE_SEARCH_TYPES        10      // most 10 types of other vague search
#define ZH_VAGUE_SEARCH_DEPTH        3       // 2 search depth for vague match 


#if (USE_ZH_WORD_MATCH  == 1)
#define ZH_WORD_MAX_CODE_LENGTH       6      // max length of pinyin (not change it)

#define ZH_WORD_MAX_INPUT_LENGTH      16     // not match when input too long 
#define ZH_WORD_MAX_MATCH_DEPTH       4      // cursive split string depth 
#define ZH_WORD_MAX_MATCH_TYPES_PREC  2      // max split types for precise match 
#define ZH_WORD_MAX_MATCH_TYPES_VAGE  8      // max number for vague match 
#define ZH_WORD_MAX_SEARCH_TYPES      40     // 
#define ZH_WORD_MAX_MATCH_NUMBER      20     // 
#endif

/*************************** PUBLIC MACROS (don't change) ******************************/

#define MAX_PINYIN_BUFFER_SZ        400     /** Recommended buffer size for code match */
#define MAX_PINYIN_MAX_TYPES         150     /** Recommended search length if want all vague search */

#if (USE_ZH_WORD_MATCH  == 1)
/** word match definitions */
#define MAX_WORD_MATCH_BUFFER_SZ      310   /**  Recommended buffer size for word match */

/* circular signle linked list for store match case */
//typedef struct match_case_list {
//    uint8_t length;
//    node_case* head;
//}list_case;
//
//typedef struct match_case_node{
//    uint8_t match_num;
//    node_case* next;
//}node_case;

typedef struct {
    uint8_t is_prec;                                 // record the index
    uint8_t match_loc[ZH_WORD_MAX_MATCH_DEPTH - 1];  // record the location of split sign location 
}__word_match_method_t;

#endif


uint8_t zh_match_code_prec(const char* str, char* res_str, uint8_t num, uint8_t* br);
uint8_t zh_match_code_vague(const char* str, char* res_str, uint8_t num, uint8_t* br);

uint8_t zh_match_word_prec(const char* str, char* res_str, uint8_t word_num, uint8_t* word_br);


#ifdef __cplusplus
    }
#endif // 

#endif 
