// GB2312search.cpp : This file contains the 'main' function. Program execution begins and ends there.
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

void zh_code_table_test()
{
    /*  test 1 all code test */
    printf("*************** Test 1 : library integrity test **********************\n");
    printf("==================== press any key to continue ======================\n ");
    while (1) {
        char c = getchar();
        int start_time = clock();
        char res_str[MAX_CODE_BUFFER_SZ];
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
        break;
        int end_time = clock();
        printf("executing time : %d ms \n", end_time - start_time);
    }
    /* test 2 input method test */
    printf("*********** Test2: vague match test : input string for vague match ******************\n");
    printf("===================    enter \"exit()\" to exit  ====================================\n");
    while (1) {
        string input_str;
        std::getline(std::cin, input_str);
        if (input_str == "exit()") break;

        uint8_t br;
        char res_str2[MAX_CODE_BUFFER_SZ];
        float end_time;
        float start_time = clock();
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

    printf("*********** Test3: pinyin split test : input mixed pinyin string (no space) *****************\n");
    printf("===================    enter \"exit()\" to exit  ====================================\n");

    while (1) {
        string input_str;
        std::getline(std::cin, input_str);
        if (input_str == "exit()") break;
        const char* s = input_str.c_str();
        __split_method_list_t* m_list = zh_pinyin_get_split(s);
        if (m_list == NULL || m_list->num == 0 ) {
            printf("can't find the pinyin string to match\n");
        }
        else {
            __split_method_t* m = m_list->head;
            for (int i = 0; i < m_list->num; i++) {
                int j = 0;
                for (int k = 0; k < strlen(s); k++) {
                    if (j < m->length && m->spm[j] == k) {
                        printf(" ");
                        j++;
                    }
                    printf("%c", s[k]);
                }

                char s_tmp[10];
                _itoa(m->wt, s_tmp, 2);
                printf("\t length: %d, weight: %d (%s)\n", m->length, m->wt, s_tmp);
                m = m->next;
            }
        }
        zh_pinyin_free_split(m_list);  /* free the space */
    }
    printf("============================= Test END, Enjoy! ==================================\n");
}

int main() {
    string input_str;
    zh_code_table_test();
    
    return 0;
}
