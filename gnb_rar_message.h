#include "myheader.h"

int ra_RNTI_Generator()
{
    int s_id = -1 + rand() % 16;
    int t_id = -1 + rand() % 81;
    int f_id = -1 + rand() % 10;
    int ul_carrier_id = 0;
    int ra_rnti = 1 + s_id + 14 * t_id + 14 * 80 * f_id + 14 * 80 * 8 * ul_carrier_id;
    return ra_rnti;
}

char arr_Val[BUF_SIZE];
char *preamble_Generator() // Ctrl H => find & replace
{
    int num_Bit = 15;
    srand(time(NULL));
    long long dec_Num = pow(2, num_Bit) - rand() % 12345;
    long long bin_Num = 0;
    int p = 0;
    while (dec_Num > 0)
    {
        bin_Num += (dec_Num % 2) * pow(10, p);
        ++p;
        dec_Num /= 2;
    }
    sprintf(arr_Val, "%lld", bin_Num);
    return arr_Val;
}

struct MSG1
{
    char rach_Preamble[30];
    int ra_RNTI;
};

struct MSG2
{
    int RAPID;
    int timing_Advance;
    struct UL_grant;
    int TC_RNTI;
};

struct MSG3
{
    int UE_ID;
    char clause[BUF_SIZE];
};

struct MSG4
{
    int UE_ID_Trunkcate;
};