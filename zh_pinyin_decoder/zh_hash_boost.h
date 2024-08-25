/**
 ***************************** Declaration ********************************
 * @file           : zh_hash_boost.h
 * @author         : FriedParrot (https://github.com/FriedParrot)
 * @version        : v1.2
 * @date           : 2024-08-24  (last modified)
 * @brief          : hash code table file for pinyins match
 * @license        : MIT license (https://opensource.org/license/mit)
 *****************************************************************************
 * @attention
 * this file is need when option USE_ZH_HASH_BOOST is set to 1
 * using this option would cost 2kb more ROM space but can search much faster
 *
 * @warning don't modify this file, or decoding would not work correctly
 *****************************************************************************
 */
#ifndef __ZH_HASH_BOOST_H
#define __ZH_HASH_BOOST_H

#ifdef __cplusplus
    extern "C"{
#endif

#include <stdint.h>
#include  <stdlib.h>

#define ZH_HASH_TABLE_DIV     11

typedef struct zh_hash_node_t{
    uint8_t num;
    uint8_t* idx_list;
}__zh_hash_node_t;

extern const __zh_hash_node_t** hash_list[];

uint8_t zh_hash_inlist(const char* str);

#ifdef __cplusplus
    }
#endif


#endif
