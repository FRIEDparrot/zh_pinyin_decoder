/**
 ***************************** Declaration ********************************
 * @file           : zh_pinyin_decoder.h
 * @author         : FriedParrot (https://github.com/FriedParrot)
 * @version        : v1.3
 * @date           : 2024-08-27  (last modified)
 * @brief          : header file for chinese pinyin inputting method
 * @license        : MIT license (https://opensource.org/license/mit)
 *****************************************************************************
 * @attention
 * this file is cofiguration file for chinese pinyin decoder  input method.
 * you can modify the settings in this file to suit your needs. 
 * 
 * @warning not modify the public macros below. keep them as they are. 
 * 
 *****************************************************************************
 */
#ifndef  __ZH_PINYIN_DECODER_H
#define  __ZH_PINYIN_DECODER_H

#ifdef __cplusplus
    extern "C"{
#endif // __cplusplus

#include <stdint.h>
#include <stdlib.h>

/********************************** Basic Settings *********************************/

#define USE_ZH_WORD_MATCH   0   /* use match word support option  */
#define USE_ZH_HASH_BOOST   1   /* use the hash table method (search faster but take more ROM)  */

#if (USE_ZH_WORD_MATCH == 1) && (USE_ZH_HASH_BOOST == 0)
    #pragma message("USE_ZH_HASH_BOOST is recommended for better performance when matching word is required")
#endif

#define ZH_CODE_TABLE_FILE_NAME      "zh_pinyin_decoder/bin/zh_pinyin_bkp.bin"      // code table file name
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
#define ZH_LOG_INFO(fmt)       printf("[Info]    " fmt " at %s : %s, line %d\n",__FILE__ ,__func__,__LINE__)
#define ZH_LOG_WARNING(fmt)    printf("[Warning] " fmt " at %s : %s, line %d\n",__FILE__ ,__func__,__LINE__)
#define ZH_LOG_ERROR(fmt)      printf("[Error]   " fmt " at %s : %s, line %d\n",__FILE__ ,__func__,__LINE__)
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

#define ZH_PINYIN_MAX_SPLIT_METHODS     500    // maximum possible split methods number (used for truncate bfs)
#define ZH_PINYIN_SIGNLE_SEARCH_DEPTH   10     // maximum search depth for single word search

#if (USE_ZH_WORD_MATCH  == 1)

#define ZH_WORD_RES_LENGTH_MAX        25     // maximum numbfer of words in result

#define ZH_WORD_PREC_MATCH_DEPTH      3      // max search precise match case
#define ZH_WORD_VAGE_MATCH_DEPTH      8      // max search vague match case 


#define ZH_WORD_MAX_MATCH_TYPES       4      // used for match word truncation 

#endif

/*************************** PUBLIC MACROS (don't modify) ******************************/

#define MAX_CODE_BUFFER_SZ         430     /** Recommended buffer size for code match (>= 3 * MAX_CODE_SEARCH_TYPES + 1) */
#define MAX_CODE_SEARCH_TYPES      140     /** Recommended search length if want (almost) all vague search result */

#define MAX_WORD_CODE_LENGTH          6     /** zhuang, chuang, are maximum 6 length */ 
#define MAX_WORD_LENGTH               4     /** max split number for word match (depend on max length of word dict) */

#if (USE_ZH_WORD_MATCH  == 1)
/** word match definitions */
#define MAX_WORD_BUFFER_SZ           400    /**  Recommended buffer size for word match (> MAX_WORD_MATCH_NUMBER * 3 * (MAX_WORD_SPLIT_NUMBER + 1)) */
#endif

/************************** PUBLIC TYPEDEFS *******************************************/

typedef struct match_case_node_t {
    uint8_t  length;                /* size of word */
    uint8_t  wt;                    /* match weight */
    uint8_t  spm[MAX_WORD_LENGTH];  /* split method (location of split sign) */
    struct  match_case_node_t* next;
}__split_method_t;

/* single link list for store match case */
typedef struct match_case_list_t {
    uint8_t num;
    struct  match_case_node_t* head;
    // struct  match_case_node_t* tail;
}__split_method_list_t;


/************************** PUBLIC FUNCTIONS *******************************************/

uint8_t zh_match_code_prec(const char* str, char* res_str, uint8_t num, uint8_t* br);
uint8_t zh_match_code_vague(const char* str, char* res_str, uint8_t num, uint8_t* br);

__split_method_list_t* zh_pinyin_get_split(const char* str);
void zh_pinyin_free_split(__split_method_list_t* m_list);

// uint8_t zh_match_word(const char* str, char* res_str, uint8_t word_num, uint8_t* word_br);

#ifdef __cplusplus
    }
#endif // 

#endif 
