/**
 ***************************** Declaration ********************************
 * @file           : zh_code_table.h
 * @author         : FriedParrot (https://github.com/FriedParrot)
 * @version        : v1.0 
 * @date           : 2024-08-23  (last modified)
 * @brief          : Chinese pinyin code table definition header file
 * @license        : MIT license (https://opensource.org/license/mit)
 *****************************************************************************
 * @attention 
 * zh_code_table.h and zh_code.c relys on files "zh_pinyin.bin"
 * which path is specified in zh_pinyin_decoder.h 
 * 
 * @warning don't modify either code_table.c file or .bin file, or decoding
 *   would not work correctly
 *****************************************************************************
 */
#ifndef __ZH_CODE_TABLE_H
#define __ZH_CODE_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <stdint.h>

typedef struct {
    const uint8_t   table_length;       // length of code table 
    const uint32_t  char_start;         // start location of each character

    const char**    code_table;         // following pinyin table
    const uint8_t*  code_table_num;     // number of chinese character
    const uint16_t* code_offset;        // each code table offset from start location
}__code_index_t;

extern const __code_index_t code_index[];

#ifdef __cplusplus
    }
#endif //

#endif

