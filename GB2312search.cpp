// GB2312search.cpp : This file contains the 'main' function. Program execution begins and ends there.
#include <iostream>
#include <cstdio>
#include "CJSON/cJSON.h"
#include "codeconv/codeconv.h"
#include "zh_pinyin_decoder/zh_pinyin_decoder.h"
#include "zh_pinyin_decoder/zh_code_table.h"

using namespace std;

void zh_code_table_test()
{
    /*  test 1 all code test */
    printf("*********************  begin test 1 : press any key **********************\n");
    char c = getchar();
    for (int i = 0; i < 26; i++) {
        for (int j = 0; j < code_index[i].table_length; j++) {
            const char* str = code_index[i].code_table[j];
            uint8_t res_str[MAX_PINYIN_BUFFER_SZ];
            uint8_t br;
            if (zh_match_code_prec(str, (char*)res_str, 120, &br) == 0) {
                printf("%s : %d ", str, br);
                printf("%s\n", Utf8ToGbk((const char*)res_str).c_str());
            }
            else {
                printf("no file zh_pinyin.bin, check if file path set correctly!\n");
            }
        }
    }
    /* test 2 input method test */
    printf("***********  begin test 2 : input string for vague match *****************\n");
    printf("===================    enter \"exit()\" to exit  ====================================\n");
    string input_str;
    while (1) {
        std::cin >> input_str;
        if (input_str == "exit()") break;
        uint8_t br;
        char res_str[MAX_PINYIN_BUFFER_SZ];
        if (zh_match_code_vague(input_str.c_str(), res_str, 150, &br))
        {
            printf("match failed : nothing to match\r\n");
        }
        else
        {
            string s = Utf8ToGbk(res_str);
            for (int i = br - 1, j = 0; i >= 0; i--, j++) {
                string s2 = s.substr(2 * i, 2);  // since it is Utf-8 code (each 2 bytes)
                std::cout << j + 1 << ":" << s2;
                if (i != 0) {
                    cout << ",";
                }
            }
            std::cout << endl;
        }
    }
}

int main() {
    
    zh_code_table_test();
    /*
    string s;
    char str_res[MAX_PINYIN_BUFFER_SZ];
    uint8_t br;
    */
    
    // cout << cJSON_Version() << endl;
    return 0;
}


// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
