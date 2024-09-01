/**
 ***************************** Declaration ********************************
 * @file           : GB2312search.cpp
 * @author         : FriedParrot (https://github.com/FriedParrot)
 * @version        : v1.5
 * @date           : 2024-09-01  (last modified)
 * @brief          : decoder source file for chinese pinyin inputting method
 * @license        : MIT license (https://opensource.org/license/mit)
 *****************************************************************************
 * @attention
 * This file contains the 'main' function. Program execution begins and ends there.
 * this is also the c++ example program for the whole inputting method.
 * 
 * test1 : font base file integrity test (字库完整性测试)
 * test2 : character code vague match test (单个汉字模糊匹配测试)
 * test3 : pinyin split method test  (拼音分词测试) 
 * test4 : comprehensive input method test with lexcion (带词库检索的完整输入法测试)
 * 
 * @note
 * this inputting method is a lightweight, portable Chinese pinyin inputting method.
 * you can port it to your embedded platform (for example, stm32, esp32 or audrino) 
 * to give support for Chinese pinyin input recognition. 
 *****************************************************************************
 */

#include <iostream>
#include <string>
#include <cstdio>
#include <time.h>
#include "CJSON/cJSON.h"
#include "codeconv/codeconv.h"
#include "zh_pinyin_decoder/zh_pinyin_decoder.h"
#include "zh_pinyin_decoder/zh_code_table.h"
#include "zh_pinyin_decoder/zh_hash_boost.h"

using namespace std;

void zh_pinyin_show_split(const char* str, const __split_method_list_t* m_list) {
    if (m_list == NULL || m_list->head == NULL) {
        printf("can't find the pinyin string to match\n");
        return;
    }
    __split_method_t* m = m_list->head;
    printf("-------------- split method length: %d ----------------\n", m_list->num);
    for (int i = 0; i < m_list->num; i++) {
        int j = 0;
        for (int k = 0; k < strlen(str); k++) {
            if (m->spm[j] == k) {
                printf(" ");
                j++;
            }
            printf("%c", str[k]);
        }
        char s_tmp[10];
        _itoa(m->wt, s_tmp, 2);
        printf("\t length: %d, weight: %d (%s)\n", m->length, m->wt, s_tmp);
        m = m->next;
    }
}

void test1() {
    /*  test 1 all code test */
    printf("*************** Test 1 : library integrity test **********************\n");
    printf("==================== press any key to continue ======================\n ");
    while (1) {
        char c = getchar();
        int start_time = clock();
        char res_str[MAX_CODE_BUFF_SZ];
        for (int i = 0; i < 26; i++) {
            for (int j = 0; j < code_index[i].table_length; j++) {
                const char* str = code_index[i].code_table[j];
                memset(res_str, 0, sizeof(res_str));
                uint8_t br;
                if (zh_match_code_prec(str, res_str, MAX_CODE_SEARCH_TYPES, &br) == 0) {
                    printf("%s : %d ", str, br);
                    printf("%s\n", Utf8ToGbk((const char*)res_str).c_str());
                }
                else {
                    /* do nothing -> may raise error if no file */
                }
            }
        }
        int end_time = clock();
        printf("executing time : %d ms \n", end_time - start_time);
        break;
    }
}

void test2() {
    /* test 2 input method test */
    printf("*********** Test2: vague match test : input string for vague match ******************\n");
    printf("===================    enter \"exit()\" to exit  ====================================\n");
    while (1) {
        string input_str;
        std::getline(std::cin, input_str);
        if (input_str == "exit()") break;

        uint8_t br;
        char res_str2[MAX_CODE_BUFF_SZ];
        uint32_t end_time;
        uint32_t start_time = clock();
        uint8_t res = zh_match_code_vague(input_str.c_str(), res_str2, MAX_CODE_SEARCH_TYPES, &br);
        end_time = clock();
        if (res)
        {
            printf("match failed : nothing to match\n");
        }
        else
        {
            string s = Utf8ToGbk(res_str2);
            for (int i = br - 1, j = 0; i >= 0; i--, j++) {
                string s2 = s.substr(2 * i, 2);  // since it is Utf-8 code (each 2 bytes)
                std::cout << j + 1 << ":" << s2;
                if (i != 0) {
                    cout << ",";
                }
            }
            std::cout << endl;
        }
        std::cout << "time token for search : " << end_time - start_time << " ms" << endl;
    }
}

void test3() {
    printf("*********** Test3: pinyin split test : input mixed pinyin string (no space) *****************\n");
    printf("===================    enter \"exit()\" to exit  ====================================\n");

    while (1) {
        string input_str;
        std::getline(std::cin, input_str);
        if (input_str == "exit()") break;
        const char* s = input_str.c_str();
        uint32_t start_time = clock();
        __split_method_list_t* m_list = zh_pinyin_get_split(s);
        zh_pinyin_filter_split(m_list);  /* use the filter option to eliminate unwanted result */
        uint32_t end_time = clock();
        zh_pinyin_show_split(s, m_list);
        printf("splitting string take time : %d ms\n", end_time - start_time);
        zh_pinyin_free_split(m_list);  /* free the space */
    }
}

#if (USE_ZH_WORD_MATCH == 1)
void test4() {
    printf("*********** Test4: word match test : input mixed pinyin string (no space) *****************\n");
    printf("===================    enter \"exit()\" to exit  ====================================\n");
    /* this is a simple code for declare the storge structure of __word_block_t */
    while (1) {
        string input_str;
        std::getline(std::cin, input_str);
        if (input_str == "exit()") break;

        __split_method_t sp;
        uint16_t idx = 0;
        
        uint32_t start_time = clock();
        __word_block_t* blk = zh_match_word(input_str.c_str(), &sp);
        uint32_t end_time = clock();

        uint8_t loc = 0;
        for (int i = 0; i < strlen(input_str.c_str()); i++) {
            if (i == sp.spm[loc]) {
                cout << "'";
                loc++;
            }
            cout << input_str.c_str()[i];
        }
        cout << endl;
        char code_str[3 * MAX_WORD_LENGTH + 1];
        for (__word_block_t* w = blk; w != NULL; w = w->next) {
            if (w->type == WORD_BLK_TYPE_CODES) {
                for (int i = 0; i < w->num.code_nbr; i++) {
                    idx++;
                    strncpy(code_str, w->buf + 3 * i, 3);
                    code_str[3] = '\0';
                    printf("%d : %s ", idx, Utf8ToGbk(code_str).c_str());
                }
            }
            else { /* WORD_BLK_TYPE_WORDS */
                uint16_t buf_idx = 0;
                for (int i = 0; w->num.word_nbr[i] != 0; i++) {
                    idx++;
                    strncpy(code_str, w->buf + buf_idx, 3 * w->num.word_nbr[i]);
                    buf_idx += 3 * w->num.word_nbr[i];
                    code_str[3 * w->num.word_nbr[i]] = '\0';
                    printf("%d : %s ", idx, Utf8ToGbk(code_str).c_str());
                }
            }
        }
        printf("\n");
        zh_word_free_match(blk);
        printf("match word take time : %d ms\n", end_time - start_time);
    }
}
#endif

/**
* @brief : this is the example test function , but also the most important function to tell you 
*          how to use parse the data processed by the functions 
*/
void zh_code_table_test()
{
    test1();
    test2();
    test3();
#if (USE_ZH_WORD_MATCH == 1)
    test4();
#endif
    printf("============================= Test END, Enjoy! ==================================\n");
}

int main() {
    zh_code_table_test();
    return 0;
}
