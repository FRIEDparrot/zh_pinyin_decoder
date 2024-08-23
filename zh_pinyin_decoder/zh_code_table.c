/**
 ***************************** Declaration ********************************
 * @file           : zh_code_table.c
 * @author         : FriedParrot (https://github.com/FriedParrot)
 * @version        : v1.0
 * @date           : 2024-08-23  (last modified)
 * @brief          : Chinese pinyin code table definition source file
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
#include  "zh_code_table.h"

const char* code_table_a[] = { "a","ai","an","ang","ao" };
const uint8_t code_table_a_num[] = { 7, 23, 20, 3, 24 };
const uint16_t code_table_a_off[] = { 0x0,0x17,0x5e,0x9c,0xa7 };

const char* code_table_b[] = { "ba", "bai", "ban", "bang", "bao", "ben", "beng", "bei", "bi", "bian", "biao", "bie", "bin", "bing", "bo", "bu" };
const uint8_t code_table_b_num[] = { 26, 13, 21, 15, 30, 9, 9, 28, 58, 27, 17, 5, 16, 15, 37, 20 };
const uint16_t  code_table_b_off[] = { 0x0,0x50,0x79,0xba,0xe9,0x145,0x162,0x17f,0x1d5,0x285,0x2d8,0x30d,0x31e,0x350,0x37f, 0x3f0 };

const char* code_table_c[] = { "chuan", "chuang", "chun", "chuai", "cha", "chai", "chan", "chang", "chao", "che", "chen", "cheng", "chi", "chou", "chong", "chu", "chuo", "chui", "cuan", "cun", "ca", "cai", "can", "cang", "cao", "ce", "cen", "ceng", "ci", "cou", "cong", "cu", "cuo", "cui" };
const uint8_t code_table_c_num[] = { 13, 7, 10, 6, 25, 8, 26, 28, 15, 10, 23, 27, 39, 17, 13, 29, 7, 10 , 8, 6, 4, 11, 12, 7, 11, 6, 3, 4, 19, 4, 12, 12, 15, 15 };
const uint16_t code_table_c_off[] = { 0x0, 0x29, 0x40, 0x60, 0x74, 0xc1, 0xdb, 0x12b, 0x181, 0x1b0, 0x1d0, 0x217, 0x26a, 0x2e1, 0x316, 0x33f, 0x398, 0x3af, 0x3cf, 0x3e9, 0x3fd, 0x40b, 0x42e, 0x454, 0x46b, 0x48e, 0x4a2, 0x4ad, 0x4bb, 0x4f6, 0x504, 0x52a, 0x550, 0x57f };

const char* code_table_d[] = { "duan", "dun", "da", "dai", "dan", "dang", "dao", "de", "deng", "dei", "di", "dia", "dian", "diao", "die", "ding", "diu", "dou", "dong", "du", "duo", "dui" };
const uint8_t   code_table_d_num[] = { 9, 17, 16, 21, 29, 12, 19, 6, 14, 1, 39, 1, 26, 14, 15, 18, 2, 14, 20, 25, 21, 9 };
const uint16_t code_table_d_off[] = { 0x0,0x1d,0x52,0x84,0xc5,0x11e,0x144,0x17f,0x193,0x1bf,0x1c4,0x23b,0x240,0x290,0x2bc,0x2eb,0x323,0x32b,0x357,0x395,0x3e2,0x423 };

const char* code_table_e[] = { "e", "en", "ei", "er" };
const uint8_t   code_table_e_num[] = { 34, 5, 1, 14 };
const uint16_t code_table_e_off[] = { 0x0,0x68,0x79,0x7e };

const char* code_table_f[] = { "fa", "fan", "fang", "fen", "feng", "fei", "fo", "fou", "fu" };
const uint8_t   code_table_f_num[] = { 10, 26, 18, 21, 21, 30, 1, 2, 84 };
const uint16_t code_table_f_off[] = { 0x0,0x20,0x70,0xa8,0xe9,0x12a,0x186,0x18b,0x193 };

const char* code_table_g[] = { "gua", "guan", "guang", "gun", "guai", "ga", "gai", "gan", "gang", "gao", "ge", "gen", "geng", "gei", "gou", "gong", "gu", "guo", "gui" };
const uint8_t   code_table_g_num[] = { 13, 20, 7, 8, 4, 11, 12, 25, 13, 19, 36, 6, 12, 1, 22, 21, 43, 20, 28 };
const uint16_t code_table_g_off[] = { 0x0,0x29,0x67,0x7e,0x98,0xa6,0xc9,0xef,0x13c,0x165,0x1a0,0x20e,0x222,0x248,0x24d,0x291,0x2d2,0x355,0x393 };

const char* code_table_h[] = { "hng", "hua", "huan", "huang", "hun", "huai", "ha", "hai", "han", "hang", "hao", "he", "hen", "heng", "hei", "hou", "hong", "hu", "huo", "hui" };
const uint8_t code_table_h_num[] = { 1, 13, 29, 25, 11, 7, 4, 13, 31, 9, 21, 30, 4, 8, 3, 15, 17, 49, 22, 40 };
const uint16_t code_table_h_off[] = { 0x0,0x5,0x2e,0x87,0xd4,0xf7,0x10e,0x11c,0x145,0x1a4,0x1c1,0x202,0x25e,0x26c,0x286,0x291,0x2c0,0x2f5,0x38a,0x3ce };

/** there is no pinyin code table startwith  "i" */

const char* code_table_j[] = { "juan", "jue", "ji", "jia", "jian", "jiao", "jiang", "jie", "jin", "jing", "jiong", "jiu", "ju" };
const uint8_t code_table_j_num[] = { 16, 31, 109, 37, 70, 48, 25, 48, 35, 41, 6, 26, 55 };
const uint16_t code_table_j_off[] = { 0x0,0x32,0x91,0x1da,0x24b,0x31f,0x3b1,0x3fe,0x490,0x4fb,0x578,0x58c,0x5dc };

const char* code_table_k[] = { "kua", "kuan", "kuang", "kun", "kuai", "ka", "kai", "kan", "kang", "kao", "ke", "ken", "keng", "kou", "kong", "ku", "kuo", "kui" };
const uint8_t code_table_k_num[] = { 6, 3, 16, 11, 11, 7, 13, 16, 10, 8, 34, 6, 3, 9, 7, 12, 6, 29 };
const uint16_t code_table_k_off[] = { 0x0,0x14,0x1f,0x51,0x74,0x97,0xae,0xd7,0x109,0x129,0x143,0x1ab,0x1bf,0x1ca,0x1e7,0x1fe,0x224,0x238 };

const char* code_table_l[] = { "luan", "lue", "lun", "lv", "la", "lai", "lan", "lang", "lao", "le", "leng", "lei", "li", "lia", "lian", "liao", "liang", "lie", "lin", "ling", "liu", "lo", "lou", "long", "lu", "luo" };
const uint8_t code_table_l_num[] = { 11, 3, 8, 21, 13, 12, 22, 14, 20, 8, 5, 20, 78, 1, 26, 21, 17, 13, 25, 28, 23, 1, 16, 18, 43, 31 };
const uint16_t code_table_l_off[] = { 0x0,0x23,0x2e,0x48,0x89,0xb2,0xd8,0x11c,0x148,0x186,0x1a0,0x1b1,0x1ef,0x2db,0x2e0,0x330,0x371,0x3a6,0x3cf,0x41c,0x472,0x4b9,0x4be,0x4f0,0x528,0x5ab };

const char* code_table_m[] = { "m", "ma", "mai", "man", "mang", "mao", "me", "men", "meng", "mei", "mi", "mian", "miao", "mie", "min", "ming", "miu", "mo", "mou", "mu" };
const uint8_t code_table_m_num[] = { 1, 16, 10, 19, 10, 26, 2, 8, 20, 27, 31, 14, 16, 6, 16, 12, 2, 37, 9, 25 };
const uint16_t code_table_m_off[] = { 0x0,0x5,0x37,0x57,0x92,0xb2,0x102,0x10a,0x124,0x162,0x1b5,0x214,0x240,0x272,0x286,0x2b8,0x2de,0x2e6,0x357,0x374 };

const char* code_table_n[] = { "n", "nuan", "nue", "nv", "na", "nai", "nan", "nang", "nao", "ne", "nen", "neng", "nei", "ni", "nian", "niao", "niang", "nie", "nin", "ning", "niu", "ng", "nou", "nong", "nu", "nuo" };
const uint8_t code_table_n_num[] = { 2, 1, 2, 4, 11, 10, 10, 5, 13, 5, 2, 1, 4, 21, 15, 7, 2, 15, 1, 10, 8, 2, 1, 6, 8, 9 };
const uint16_t code_table_n_off[] = { 0x0,0x8,0xd,0x15,0x23,0x46,0x66,0x86,0x97,0xc0,0xd1,0xd9,0xde,0xec,0x12d,0x15c,0x173,0x17b,0x1aa,0x1af,0x1cf,0x1e9,0x1f1,0x1f6,0x20a,0x224 };

const char* code_table_o[] = { "o", "ou" };
const uint8_t code_table_o_num[] = { 3, 12 };
const uint16_t code_table_o_off[] = { 0x0,0xb };

const char* code_table_p[] = { "pa", "pai", "pan", "pang", "pao", "pen", "peng", "pei", "pi", "pian", "piao", "pie", "pin", "ping", "po", "pou", "pu" };
const uint8_t code_table_p_num[] = { 12, 10, 18, 11, 12, 3, 18, 15, 45, 12, 15, 5, 11, 13, 21, 3, 27 };
const uint16_t code_table_p_off[] = { 0x0,0x26,0x46,0x7e,0xa1,0xc7,0xd2,0x10a,0x139,0x1c2,0x1e8,0x217,0x228,0x24b,0x274,0x2b5,0x2c0 };

const char* code_table_q[] = { "quan", "que", "qun", "qi", "qia", "qian", "qiao", "qiang", "qie", "qin", "qing", "qiong", "qiu", "qu" };
const uint8_t code_table_q_num[] = { 22, 11, 4, 73, 7, 45, 29, 20, 14, 24, 25, 9, 25, 35 };
const uint16_t code_table_q_off[] = { 0x0,0x44,0x67,0x75,0x152,0x169,0x1f2,0x24b,0x289,0x2b5,0x2ff,0x34c,0x369,0x3b6 };

const char* code_table_r[] = { "ruan", "run", "ran", "rang", "rao", "re", "ren", "reng", "ri", "rou", "rong", "ru", "ruo", "rui" };
const uint8_t code_table_r_num[] = { 3, 2, 7, 7, 6, 3, 18, 2, 1, 6, 15, 20, 4, 8 };
const uint16_t code_table_r_off[] = { 0x0,0xb,0x13,0x2a,0x41,0x55,0x60,0x98,0xa0,0xa5,0xb9,0xe8,0x126,0x134 };

const char* code_table_s[] = { "shua", "shuan", "shuang", "shun", "shuai", "sha", "shai", "shan", "shang", "shao", "she", "shen", "sheng", "shi", "shou", "shu", "shuo", "shui", "suan", "sun", "sa", "sai", "san", "sang", "sao", "se", "sen", "seng", "si", "sou", "song", "su", "suo", "sui" };
const uint8_t code_table_s_num[] = { 3, 4, 5, 5, 6, 20, 5, 38, 14, 20, 21, 29, 17, 70, 15, 49, 10, 4, 4, 8, 8, 5, 9, 6, 10, 7, 1, 1, 35, 14, 16, 22, 17, 19 };
const uint16_t code_table_s_off[] = { 0x0,0xb,0x19,0x2a,0x3b,0x4f,0x8d,0x9e,0x112,0x13e,0x17c,0x1bd,0x216,0x24b,0x31f,0x34e,0x3e3,0x403,0x411,0x41f,0x439,0x453,0x464,0x481,0x495,0x4b5,0x4cc,0x4d1,0x4d6,0x541,0x56d,0x59f,0x5e3,0x618 };

const char* code_table_t[] = { "tuan", "tun", "ta", "tai", "tan", "tang", "tao", "te", "teng", "tei", "ti", "tian", "tiao", "tie", "ting", "tou", "tong", "tu", "tuo", "tui" };
const uint8_t code_table_t_num[] = { 5, 9, 20, 18, 26, 26, 18, 5, 5, 1, 25, 15, 16, 6, 18, 6, 24, 16, 26, 8 };
const uint16_t code_table_t_off[] = { 0x0,0x11,0x2e,0x6c,0xa4,0xf4,0x144,0x17c,0x18d,0x19e,0x1a3,0x1f0,0x21f,0x251,0x265,0x29d,0x2b1,0x2fb,0x32d,0x37d };

/*  there is no pinyin code table startwith  "u" */

/*  there is no pinyin code table startwith  "v" */

const char* code_table_w[] = { "wa", "wai", "wan", "wang", "wen", "weng", "wei", "wo", "wu" };
const uint8_t code_table_w_num[] = { 10, 3, 28, 14, 18, 5, 62, 17, 57 };
const uint16_t code_table_w_off[] = { 0x0,0x20,0x2b,0x81,0xad,0xe5,0xf6,0x1b2,0x1e7 };

const char* code_table_x[] = { "xuan", "xue", "xun", "xi", "xia", "xian", "xiao", "xiang", "xie", "xin", "xing", "xiong", "xiu", "xu" };
const uint8_t code_table_x_num[] = { 28, 13, 32, 77, 22, 50, 30, 30, 42, 19, 23, 8, 19, 35 };
const uint16_t code_table_x_off[] = { 0x0,0x56,0x7f,0xe1,0x1ca,0x20e,0x2a6,0x302,0x35e,0x3de,0x419,0x460,0x47a,0x4b5 };

const char* code_table_y[] = { "yuan", "yue", "yun", "ya", "yan", "yang", "yao", "ye", "yi", "yin", "ying", "yo", "you", "yong", "yu" };
const uint8_t code_table_y_num[] = { 35, 19, 25, 31, 69, 25, 37, 25, 108, 36, 39, 2, 43, 25, 97 };
const uint16_t code_table_y_off[] = { 0x0,0x6b,0xa6,0xf3,0x152,0x223,0x270,0x2e1,0x32e,0x474,0x4e2,0x559,0x561,0x5e4,0x631 };

const char* code_table_z[] = { "zuan", "zun", "za", "zai", "zan", "zang", "zao", "ze", "zen", "zeng", "zei", "zi", "zou", "zong", "zu", "zuo", "zui", "zhua", "zhuan", "zhuang", "zhun", "zhuai", "zha", "zhai", "zhan", "zhang", "zhao", "zhe", "zhen", "zheng", "zhei", "zhi", "zhou", "zhong", "zhu", "zhuo", "zhui" };
const uint8_t code_table_z_num[] = { 6, 5, 7, 10, 11, 7, 15, 14, 2, 9, 1, 39, 10, 11, 11, 19, 7 , 3, 10, 11, 5, 1, 27, 12, 21, 24, 19, 23, 34, 20, 1, 80, 24, 18, 51, 23, 10 };
const uint16_t code_table_z_off[] = { 0x0,0x14,0x25,0x3c,0x5c,0x7f,0x96,0xc5,0xf1,0xf9,0x116,0x11b,0x192,0x1b2,0x1d5,0x1f8,0x233,0x24a,0x255,0x275,0x298,0x2a9,0x2ae,0x301,0x327,0x368,0x3b2,0x3ed,0x434,0x49c,0x4da,0x4df,0x5d1,0x61b,0x653,0x6ee,0x735 };

const __code_index_t code_index[26] = {
{     5, 0x56F1, code_table_a, code_table_a_num,code_table_a_off },
{    16, 0x0000, code_table_b, code_table_b_num,code_table_b_off },  /* start from  b offset 0x000000 */
{    34, 0x3857, code_table_c, code_table_c_num,code_table_c_off },
{    22, 0x0D9B, code_table_d, code_table_d_num,code_table_d_off },
{     4, 0x57E4, code_table_e, code_table_e_num,code_table_e_off },
{     9, 0x0B08, code_table_f, code_table_f_num,code_table_f_off },
{    19, 0x1DC5, code_table_g, code_table_g_num,code_table_g_off },
{    20, 0x2443, code_table_h, code_table_h_num,code_table_h_off },
{     0,  0x0,   NULL        , NULL            ,NULL             },
{    13, 0x288D, code_table_j, code_table_j_num,code_table_j_off },
{    18, 0x21B0, code_table_k, code_table_k_num,code_table_k_off },
{    26, 0x17B9, code_table_l, code_table_l_num,code_table_l_off },
{    20, 0x0745, code_table_m, code_table_m_num,code_table_m_off },
{    26, 0x1576, code_table_n, code_table_n_num,code_table_n_off },
{     2, 0x5890, code_table_o, code_table_o_num,code_table_o_off },
{    17, 0x0430, code_table_p, code_table_p_num,code_table_p_off },
{    14, 0x2F12, code_table_q, code_table_q_num,code_table_q_off },
{    14, 0x445C, code_table_r, code_table_r_num,code_table_r_off },
{    34, 0x3E07, code_table_s, code_table_s_num,code_table_s_off },
{    20, 0x11DD, code_table_t, code_table_t_num,code_table_t_off },
{     0, 0x0,    NULL        , NULL            ,NULL             },
{     0, 0x0,    NULL        , NULL            ,NULL             },
{     9, 0x545B, code_table_w, code_table_w_num,code_table_w_off },
{    14, 0x3335, code_table_x, code_table_x_num,code_table_x_off },
{    15, 0x4D03, code_table_y, code_table_y_num,code_table_y_off },
{    37, 0x45AC, code_table_z, code_table_z_num,code_table_z_off },
};
