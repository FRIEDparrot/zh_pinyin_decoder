/**
 ***************************** Declaration ********************************
 * @file           : zh_hash_boost.c
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
#include <string.h>
#include  "zh_hash_boost.h"

/* this array gives the sequence for vague search */ 
const uint8_t idx_an[] = { 3,2 };
const uint8_t idx_ai[] = { 1 };
const uint8_t idx_ao[] = { 4 };
const uint8_t idx_ba[] = { 3,1,2,4,0 };
const uint8_t idx_be[] = { 6,7,5 };
const uint8_t idx_bi[] = { 9,10,13,11,12,8 };
const uint8_t idx_bo[] = { 14 };
const uint8_t idx_bu[] = { 15 };
const uint8_t idx_ch[] = { 1,7,11,14,3,0,5,6,8,10,13,17,2,16,4,9,12,15 };
const uint8_t idx_ca[] = { 23,21,22,24,20 };
const uint8_t idx_ce[] = { 27,26,25 };
const uint8_t idx_co[] = { 30,29 };
const uint8_t idx_cu[] = { 18,33,19,32,31 };
const uint8_t idx_ci[] = { 28 };
const uint8_t idx_da[] = { 5,3,4,6,2 };
const uint8_t idx_de[] = { 8,9,7 };
const uint8_t idx_di[] = { 12,13,15,11,14,16,10 };
const uint8_t idx_do[] = { 18,17 };
const uint8_t idx_du[] = { 0,21,1,20,19 };
const uint8_t idx_ei[] = { 2 };
const uint8_t idx_en[] = { 1 };
const uint8_t idx_er[] = { 3 };
const uint8_t idx_fa[] = { 2,1,0 };
const uint8_t idx_fe[] = { 4,5,3 };
const uint8_t idx_fo[] = { 7,6 };
const uint8_t idx_fu[] = { 8 };
const uint8_t idx_gu[] = { 2,4,1,0,18,3,17,16 };
const uint8_t idx_ga[] = { 8,6,7,9,5 };
const uint8_t idx_ge[] = { 12,13,11,10 };
const uint8_t idx_go[] = { 15,14 };
const uint8_t idx_hu[] = { 3,5,2,1,19,4,18,17 };
const uint8_t idx_ha[] = { 9,7,8,10,6 };
const uint8_t idx_he[] = { 13,14,12,11 };
const uint8_t idx_ho[] = { 16,15 };
const uint8_t idx_hn[] = { 0 };
const uint8_t idx_ji[] = { 6,10,4,5,9,3,7,8,11,2 };
const uint8_t idx_ju[] = { 0,1,12 };
const uint8_t idx_ku[] = { 2,4,1,0,17,3,16,15 };
const uint8_t idx_ka[] = { 8,6,7,9,5 };
const uint8_t idx_ke[] = { 12,11,10 };
const uint8_t idx_ko[] = { 14,13 };
const uint8_t idx_li[] = { 16,14,15,19,13,17,18,20,12 };
const uint8_t idx_la[] = { 7,5,6,8,4 };
const uint8_t idx_le[] = { 10,11,9 };
const uint8_t idx_lo[] = { 23,22,21 };
const uint8_t idx_lu[] = { 0,1,2,25,24 };
const uint8_t idx_lv[] = { 3 };
const uint8_t idx_ma[] = { 4,2,3,5,1 };
const uint8_t idx_me[] = { 8,9,7,6 };
const uint8_t idx_mi[] = { 11,12,15,13,14,16,10 };
const uint8_t idx_mo[] = { 18,17 };
const uint8_t idx_mu[] = { 19 };
const uint8_t idx_ni[] = { 16,14,15,19,17,18,20,13 };
const uint8_t idx_na[] = { 7,5,6,8,4 };
const uint8_t idx_ne[] = { 11,12,10,9 };
const uint8_t idx_no[] = { 23,22 };
const uint8_t idx_nu[] = { 1,2,25,24 };
const uint8_t idx_ng[] = { 21 };
const uint8_t idx_nv[] = { 3 };
const uint8_t idx_ou[] = { 1 };
const uint8_t idx_pa[] = { 3,1,2,4,0 };
const uint8_t idx_pe[] = { 6,7,5 };
const uint8_t idx_pi[] = { 9,10,13,11,12,8 };
const uint8_t idx_po[] = { 15,14 };
const uint8_t idx_pu[] = { 16 };
const uint8_t idx_qi[] = { 7,11,5,6,10,4,8,9,12,3 };
const uint8_t idx_qu[] = { 0,1,2,13 };
const uint8_t idx_ra[] = { 3,2,4 };
const uint8_t idx_re[] = { 7,6,5 };
const uint8_t idx_ro[] = { 10,9 };
const uint8_t idx_ru[] = { 0,13,1,12,11 };
const uint8_t idx_ri[] = { 8 };
const uint8_t idx_sh[] = { 2,8,12,4,1,6,7,9,11,14,0,17,3,16,5,10,13,15 };
const uint8_t idx_sa[] = { 23,21,22,24,20 };
const uint8_t idx_se[] = { 27,26,25 };
const uint8_t idx_so[] = { 30,29 };
const uint8_t idx_su[] = { 18,33,19,32,31 };
const uint8_t idx_si[] = { 28 };
const uint8_t idx_ta[] = { 5,3,4,6,2 };
const uint8_t idx_te[] = { 8,9,7 };
const uint8_t idx_ti[] = { 11,12,14,13,10 };
const uint8_t idx_to[] = { 16,15 };
const uint8_t idx_tu[] = { 0,19,1,18,17 };
const uint8_t idx_wa[] = { 3,1,2,0 };
const uint8_t idx_we[] = { 5,6,4 };
const uint8_t idx_wo[] = { 7 };
const uint8_t idx_wu[] = { 8 };
const uint8_t idx_xi[] = { 7,11,5,6,10,4,8,9,12,3 };
const uint8_t idx_xu[] = { 0,1,2,13 };
const uint8_t idx_ya[] = { 5,4,6,3 };
const uint8_t idx_yi[] = { 10,9,8 };
const uint8_t idx_yo[] = { 13,12,11 };
const uint8_t idx_yu[] = { 0,1,2,14 };
const uint8_t idx_ye[] = { 7 };
const uint8_t idx_zh[] = {19, 25, 29, 33, 21, 18, 23, 24, 26, 30, 28, 32, 17, 36, 20, 35, 22, 27, 31, 34};
const uint8_t idx_za[] = {5, 3, 4, 6, 2};
const uint8_t idx_ze[] = {9, 10, 8, 7};
const uint8_t idx_zo[] = {13, 12};
const uint8_t idx_zu[] = {0, 16, 1, 15, 14};
const uint8_t idx_zi[] = {11};

const __zh_hash_node_t hash_node_ai = { 1, &idx_ai };
const __zh_hash_node_t hash_node_an = { 2, &idx_an };
const __zh_hash_node_t hash_node_ao = { 1, &idx_ao };
const __zh_hash_node_t hash_node_ba = { 5, &idx_ba };
const __zh_hash_node_t hash_node_be = { 3, &idx_be };
const __zh_hash_node_t hash_node_bi = { 6, &idx_bi };
const __zh_hash_node_t hash_node_bo = { 1, &idx_bo };
const __zh_hash_node_t hash_node_bu = { 1, &idx_bu };
const __zh_hash_node_t hash_node_ch = { 18,&idx_ch };
const __zh_hash_node_t hash_node_cu = { 5, &idx_cu };
const __zh_hash_node_t hash_node_ca = { 5, &idx_ca };
const __zh_hash_node_t hash_node_ce = { 3, &idx_ce };
const __zh_hash_node_t hash_node_ci = { 1, &idx_ci };
const __zh_hash_node_t hash_node_co = { 2, &idx_co };
const __zh_hash_node_t hash_node_du = { 5, &idx_du };
const __zh_hash_node_t hash_node_da = { 5, &idx_da };
const __zh_hash_node_t hash_node_de = { 3, &idx_de };
const __zh_hash_node_t hash_node_di = { 7, &idx_di };
const __zh_hash_node_t hash_node_do = { 2, &idx_do };
const __zh_hash_node_t hash_node_en = { 1, &idx_en };
const __zh_hash_node_t hash_node_ei = { 1, &idx_ei };
const __zh_hash_node_t hash_node_er = { 1, &idx_er };
const __zh_hash_node_t hash_node_fa = { 3, &idx_fa };
const __zh_hash_node_t hash_node_fe = { 3, &idx_fe };
const __zh_hash_node_t hash_node_fo = { 2, &idx_fo };
const __zh_hash_node_t hash_node_fu = { 1, &idx_fu };
const __zh_hash_node_t hash_node_gu = { 8, &idx_gu };
const __zh_hash_node_t hash_node_ga = { 5, &idx_ga };
const __zh_hash_node_t hash_node_ge = { 4, &idx_ge };
const __zh_hash_node_t hash_node_go = { 2, &idx_go };
const __zh_hash_node_t hash_node_hn = { 1, &idx_hn };
const __zh_hash_node_t hash_node_hu = { 8, &idx_hu };
const __zh_hash_node_t hash_node_ha = { 5, &idx_ha };
const __zh_hash_node_t hash_node_he = { 4, &idx_he };
const __zh_hash_node_t hash_node_ho = { 2, &idx_ho };
const __zh_hash_node_t hash_node_ju = { 3, &idx_ju };
const __zh_hash_node_t hash_node_ji = { 10,&idx_ji };
const __zh_hash_node_t hash_node_ku = { 8, &idx_ku };
const __zh_hash_node_t hash_node_ka = { 5, &idx_ka };
const __zh_hash_node_t hash_node_ke = { 3, &idx_ke };
const __zh_hash_node_t hash_node_ko = { 2, &idx_ko };
const __zh_hash_node_t hash_node_lu = { 5, &idx_lu };
const __zh_hash_node_t hash_node_lv = { 1, &idx_lv };
const __zh_hash_node_t hash_node_la = { 5, &idx_la };
const __zh_hash_node_t hash_node_le = { 3, &idx_le };
const __zh_hash_node_t hash_node_li = { 9, &idx_li };
const __zh_hash_node_t hash_node_lo = { 3, &idx_lo };
const __zh_hash_node_t hash_node_ma = { 5, &idx_ma };
const __zh_hash_node_t hash_node_me = { 4, &idx_me };
const __zh_hash_node_t hash_node_mi = { 7, &idx_mi };
const __zh_hash_node_t hash_node_mo = { 2, &idx_mo };
const __zh_hash_node_t hash_node_mu = { 1, &idx_mu };
const __zh_hash_node_t hash_node_nu = { 4, &idx_nu };
const __zh_hash_node_t hash_node_nv = { 1, &idx_nv };
const __zh_hash_node_t hash_node_na = { 5, &idx_na };
const __zh_hash_node_t hash_node_ne = { 4, &idx_ne };
const __zh_hash_node_t hash_node_ni = { 8, &idx_ni };
const __zh_hash_node_t hash_node_ng = { 1, &idx_ng };
const __zh_hash_node_t hash_node_no = { 2, &idx_no };
const __zh_hash_node_t hash_node_ou = { 1, &idx_ou };
const __zh_hash_node_t hash_node_pa = { 5, &idx_pa };
const __zh_hash_node_t hash_node_pe = { 3, &idx_pe };
const __zh_hash_node_t hash_node_pi = { 6, &idx_pi };
const __zh_hash_node_t hash_node_po = { 2, &idx_po };
const __zh_hash_node_t hash_node_pu = { 1, &idx_pu };
const __zh_hash_node_t hash_node_qu = { 4, &idx_qu };
const __zh_hash_node_t hash_node_qi = { 10,&idx_qi };
const __zh_hash_node_t hash_node_ru = { 5, &idx_ru };
const __zh_hash_node_t hash_node_ra = { 3, &idx_ra };
const __zh_hash_node_t hash_node_re = { 3, &idx_re };
const __zh_hash_node_t hash_node_ri = { 1, &idx_ri };
const __zh_hash_node_t hash_node_ro = { 2, &idx_ro };
const __zh_hash_node_t hash_node_sh = { 18,&idx_sh };
const __zh_hash_node_t hash_node_su = { 5, &idx_su };
const __zh_hash_node_t hash_node_sa = { 5, &idx_sa };
const __zh_hash_node_t hash_node_se = { 3, &idx_se };
const __zh_hash_node_t hash_node_si = { 1, &idx_si };
const __zh_hash_node_t hash_node_so = { 2, &idx_so };
const __zh_hash_node_t hash_node_tu = { 5, &idx_tu };
const __zh_hash_node_t hash_node_ta = { 5, &idx_ta };
const __zh_hash_node_t hash_node_te = { 3, &idx_te };
const __zh_hash_node_t hash_node_ti = { 5, &idx_ti };
const __zh_hash_node_t hash_node_to = { 2, &idx_to };
const __zh_hash_node_t hash_node_wa = { 4, &idx_wa };
const __zh_hash_node_t hash_node_we = { 3, &idx_we };
const __zh_hash_node_t hash_node_wo = { 1, &idx_wo };
const __zh_hash_node_t hash_node_wu = { 1, &idx_wu };
const __zh_hash_node_t hash_node_xu = { 4, &idx_xu };
const __zh_hash_node_t hash_node_xi = { 10,&idx_xi };
const __zh_hash_node_t hash_node_yu = { 4, &idx_yu };
const __zh_hash_node_t hash_node_ya = { 4, &idx_ya };
const __zh_hash_node_t hash_node_ye = { 1, &idx_ye };
const __zh_hash_node_t hash_node_yi = { 3, &idx_yi };
const __zh_hash_node_t hash_node_yo = { 3, &idx_yo };
const __zh_hash_node_t hash_node_zu = { 5, &idx_zu };
const __zh_hash_node_t hash_node_za = { 5, &idx_za };
const __zh_hash_node_t hash_node_ze = { 4, &idx_ze };
const __zh_hash_node_t hash_node_zi = { 1, &idx_zi };
const __zh_hash_node_t hash_node_zo = { 2, &idx_zo };
const __zh_hash_node_t hash_node_zh = { 20,&idx_zh };

const __zh_hash_node_t* node_a[] = { NULL,NULL,&hash_node_an,&hash_node_ao,NULL,NULL,NULL,NULL,&hash_node_ai };
const __zh_hash_node_t* node_b[] = { &hash_node_ba,NULL,NULL,&hash_node_bo,&hash_node_be,NULL,NULL,NULL,&hash_node_bi,&hash_node_bu };
const __zh_hash_node_t* node_c[] = { &hash_node_ca,NULL,NULL,&hash_node_co,&hash_node_ce,NULL,NULL,&hash_node_ch,&hash_node_ci,&hash_node_cu };
const __zh_hash_node_t* node_d[] = { &hash_node_da,NULL,NULL,&hash_node_do,&hash_node_de,NULL,NULL,NULL,&hash_node_di,&hash_node_du };
const __zh_hash_node_t* node_e[] = { NULL,NULL,&hash_node_en,NULL,NULL,NULL,&hash_node_er,NULL,&hash_node_ei };
const __zh_hash_node_t* node_f[] = { &hash_node_fa,NULL,NULL,&hash_node_fo,&hash_node_fe,NULL,NULL,NULL,NULL,&hash_node_fu };
const __zh_hash_node_t* node_g[] = { &hash_node_ga,NULL,NULL,&hash_node_go,&hash_node_ge,NULL,NULL,NULL,NULL,&hash_node_gu };
const __zh_hash_node_t* node_h[] = { &hash_node_ha,NULL,&hash_node_hn,&hash_node_ho,&hash_node_he,NULL,NULL,NULL,NULL,&hash_node_hu };

const __zh_hash_node_t* node_j[] = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,&hash_node_ji,&hash_node_ju };
const __zh_hash_node_t* node_k[] = { &hash_node_ka,NULL,NULL,&hash_node_ko,&hash_node_ke,NULL,NULL,NULL,NULL,&hash_node_ku };
const __zh_hash_node_t* node_l[] = { &hash_node_la,NULL,NULL,&hash_node_lo,&hash_node_le,NULL,NULL,NULL,&hash_node_li,&hash_node_lu,&hash_node_lv };
const __zh_hash_node_t* node_m[] = { &hash_node_ma,NULL,NULL,&hash_node_mo,&hash_node_me,NULL,NULL,NULL,&hash_node_mi,&hash_node_mu };
const __zh_hash_node_t* node_n[] = { &hash_node_na,NULL,NULL,&hash_node_no,&hash_node_ne,NULL,&hash_node_ng,NULL,&hash_node_ni,&hash_node_nu,&hash_node_nv };
const __zh_hash_node_t* node_o[] = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,&hash_node_ou };
const __zh_hash_node_t* node_p[] = { &hash_node_pa,NULL,NULL,&hash_node_po,&hash_node_pe,NULL,NULL,NULL,&hash_node_pi,&hash_node_pu };
const __zh_hash_node_t* node_q[] = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,&hash_node_qi,&hash_node_qu };
const __zh_hash_node_t* node_r[] = { &hash_node_ra,NULL,NULL,&hash_node_ro,&hash_node_re,NULL,NULL,NULL,&hash_node_ri,&hash_node_ru };
const __zh_hash_node_t* node_s[] = { &hash_node_sa,NULL,NULL,&hash_node_so,&hash_node_se,NULL,NULL,&hash_node_sh,&hash_node_si,&hash_node_su };
const __zh_hash_node_t* node_t[] = { &hash_node_ta,NULL,NULL,&hash_node_to,&hash_node_te,NULL,NULL,NULL,&hash_node_ti,&hash_node_tu };

const __zh_hash_node_t* node_w[] = { &hash_node_wa,NULL,NULL,&hash_node_wo,&hash_node_we,NULL,NULL,NULL,NULL,&hash_node_wu };
const __zh_hash_node_t* node_x[] = { NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,&hash_node_xi,&hash_node_xu };
const __zh_hash_node_t* node_y[] = { &hash_node_ya,NULL,NULL,&hash_node_yo,&hash_node_ye,NULL,NULL,NULL,&hash_node_yi,&hash_node_yu };
const __zh_hash_node_t* node_z[] = { &hash_node_za,NULL,NULL,&hash_node_zo,&hash_node_ze,NULL,NULL,&hash_node_zh,&hash_node_zi,&hash_node_zu };

const __zh_hash_node_t** hash_list[26] = { node_a, node_b, node_c, node_d, node_e, node_f, node_g, node_h, NULL, node_j, node_k, node_l, node_m, node_n, node_o, node_p, node_q, node_r, node_s, node_t, NULL,NULL, node_w, node_x, node_y, node_z };

/* this is generated for  recording if 2 character match case exist */
const uint32_t zh_hash_chk_arr[26] = { 0x6100,0x104111,0x104191,0x104111,0x22100,0x104011,0x104011,0x106011,0x0,0x100100,0x104011,0x304111,0x104111,0x304151,0x100000,0x104111,0x100100,0x104111,0x104191,0x104111,0x0,0x0,0x104011,0x100100,0x104111,0x104191 };

/// <summary>
/// check if a string (length >= 2) exists in hash list
/// </summary>
/// <warning>str must consits of a-z or this will crash</warning>
/// <param name="str"></param>
/// <returns> 0: not in list, 1: in list (need further search)</returns>
uint8_t zh_hash_inlist(const char* str) {
    if (zh_hash_chk_arr[str[0] - 'a'] & (1 << (str[1] - 'a'))) return 1;
    return 0;
}


