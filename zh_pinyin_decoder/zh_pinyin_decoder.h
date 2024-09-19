/**
 ***************************** Declaration ********************************
 * @file           : zh_pinyin_decoder.h
 * @author         : FriedParrot (https://github.com/FriedParrot)
 * @version        : v1.5
 * @date           : 2024-09-01  (last modified)
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

#define USE_ZH_WORD_MATCH           1   /* use match word support option  */
#define USE_ZH_HASH_BOOST           1   /* use the hash table method (search faster but take more ROM)  */

#if (USE_ZH_WORD_MATCH == 1) && (USE_ZH_HASH_BOOST == 0)
    #pragma message("USE_ZH_HASH_BOOST is recommended for better performance when matching word is required")
#endif

#define ZH_CODE_TABLE_FILE_NAME      "zh_pinyin_decoder/bin/zh_pinyin.bin"      // code table file name
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
#define ZH_LOG_INFO(msg)       printf("[Info]    " msg " at %s : %s, line %d\n",  __FILE__ ,__func__,__LINE__)
#define ZH_LOG_WARNING(msg)    printf("[Warning] " msg " at %s : %s, line %d\n",  __FILE__ ,__func__,__LINE__)
#define ZH_LOG_ERROR(msg)      printf("[Error]   " msg " at %s : %s, line %d\n",  __FILE__ ,__func__,__LINE__)
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

#define ZH_PINYIN_MAX_SPLIT_METHODS     100    // maximum possible split methods number (used for truncate bfs)
#define ZH_PINYIN_MAX_FILTER_TYPES      3         // maximum word match types for filter
#define ZH_PINYIN_SIGNLE_SEARCH_DEPTH   10     // maximum search depth for single word search

#if (USE_ZH_WORD_MATCH  == 1)

#define ZH_WORD_DICT_BUFFER_SZ        4 * 1024  // 4kb buffer for json parse

#define ZH_WORD_MAX_BUFFER_READ       1500      // search buffer depth after the first read (100 later)
#define ZH_WORD_MAX_MATCH_LENGTH      25        // maximum numbfer of words in result

#define ZH_WORD_VAGE_SEARCH_DEPTH     10        // max search depth per vague match case 
#define ZH_WORD_CODE_DISP_NUM         4         // code displayed number before the word (if precise match)

#endif

/*************************** PUBLIC MACROS (don't modify) ******************************/

#define MAX_CODE_BUFF_SZ           430     /** Recommended buffer size for code match (>= 3 * MAX_CODE_SEARCH_TYPES + 1) */
#define MAX_CODE_SEARCH_TYPES      140     /** Recommended search length if want (almost) all vague search result */

#define MAX_WORD_CODE_LENGTH          6    /** zhuang, chuang, are maximum 6 length */ 
#define MAX_WORD_LENGTH               4    /** max split number for word match (depend on max length of word dict) */

#if (USE_ZH_WORD_MATCH  == 1)
/** word match definitions */

/* >= 3 * MAX_WORD_BLK_CODE_NUM + 1, also >= 3 * MAX_WORD_BLK_WORD_NUM * MAX_WORD_LENGTH + 1 */
#define MAX_WORD_BLK_BUFFER_SZ      MAX_CODE_BUFF_SZ    /** search buffer size for word      */
#define MAX_WORD_BLK_WORD_NUM       20                  /** max number of words in one block */
#define MAX_WORD_MATCH_NUM          160                 /** max number for  */

#endif

/************************** PUBLIC TYPEDEFS *******************************************/
typedef struct match_case_node_t {
    uint8_t  length;                 /* size of word */
    uint8_t  wt;                     /* match weight */
    uint8_t  spm[MAX_WORD_LENGTH];   /* split method (location of split sign) */
    struct   match_case_node_t* next;/* next node pointer */
    uint8_t  cm_num;                 /* current match case number */
}__split_method_t;


/* single link list for store match case */
typedef struct match_case_list_t {
    uint8_t num;
    struct  match_case_node_t* head;
}__split_method_list_t;


#if (USE_ZH_WORD_MATCH == 1)

/* word match result (may be codes or words, 1 match method -> 1 result) */
typedef struct word_dict_search_result_t{
    uint8_t type;   /* 0: codes ; 1: words */
    union {
        uint32_t code_nbr;     /* if type is 0, store code numbfer */
        uint8_t* word_nbr;     /* terminates when it reach 0 */
    }num;
    char* buf;
    struct word_dict_search_result_t* next;
}__word_block_t;

/**
* @defgroup word_block_type
*/
#define WORD_BLK_TYPE_CODES       0
#define WORD_BLK_TYPE_WORDS       1

/**
* @defgroup word_search_state
*/
#define WORD_SEARCH_STATE_CODE_NO_MATCH      0
#define WORD_SEARCH_STATE_CODE_VAGUE_MATCH   1
#define WORD_SEARCH_STATE_CODE_PREC_MATCH    2

#endif 

/************************** PUBLIC FUNCTIONS *******************************************/

uint8_t zh_match_code_prec(const char* str, char* res_str, uint8_t num, uint8_t* br);
uint8_t zh_match_code_vague(const char* str, char* res_str, uint8_t num, uint8_t* br);

__split_method_list_t* zh_pinyin_get_split(const char* str);
uint8_t zh_pinyin_filter_split(__split_method_list_t* m_list);
void zh_pinyin_free_split(__split_method_list_t* m_list);


#if (USE_ZH_WORD_MATCH == 1)

__word_block_t* zh_match_word(const char* str, __split_method_t* sp);
void zh_word_free_match(__word_block_t* blk);


#endif


#ifdef __cplusplus
    }
#endif // 

#endif 
