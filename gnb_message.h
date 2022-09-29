#include "myheader.h"

#define prach_configurationindex 15
#define msg1_frequencystart 0
#define preamblereceivedtargetpower -70
#define preambletransmax 20
#define MCC "452"
#define MNC "04"
#define totalnumberofra_preambles 63
#define q_rxlevmin -70
#define q_rxlevminoffset -100
#define pading 0
#define n_id_2 2
#define n_id_1 335
#define ssb_index 7
#define rsrp -50
#define systemframenumber "100111"
#define susubcarrierspacingcommon "scs15or60"
#define ssb_subcarrieroffset 15
#define pdcch_configsib1 15
#define cellbarred "notBarred"

// SSB structure
struct PSS
{
    int N_id_2;
};

struct SSS
{
    int N_id_1;
};

struct PBCH_DMRS
{
    int SSB_Index;
    int RSRP;
};

struct MIB
{
    char systemFrameNumber[7];
    char subCarrierSpacingCommon[10];
    int ssb_SubcarrierOffset;
    int pdcch_ConfigSIB1;
    char cellBarred[10];
};

struct PBCH_Data
{
    struct MIB mib;
};

struct SSB
{
    int ssb_pad;
    struct PSS pss;
    struct SSS sss;
    struct PBCH_DMRS pbch_dmrs;
    struct PBCH_Data pbch_data;
};

#pragma pack(1)
struct BEAM
{
    int beam_power;
    struct SSB ssb;
};
#pragma pack(0)

// SIB1 Structure
struct RACH_ConfigGeneric
{
    int prach_ConfigurationIndex;
    int msg1_FrequencyStart;
    int preambleReceivedTargetPower;
    int preambleTransMax;
};

struct RACH_ConfigCommon
{
    int totalNumberOfRA_Preambles;
    struct RACH_ConfigGeneric rach_configgeneric;
};

struct BWP_UplinkCommon
{
    int buc_pad;
    struct RACH_ConfigCommon rach_configcommon;
};

struct UplinkConfigCommonSIB
{
    int ucc_pad;
    struct BWP_UplinkCommon bwp_uplinkcommon;
};

struct ServingCellConfigCommonSIB
{
    int sccc_pad;
    struct UplinkConfigCommonSIB uplink_config_common_sib;
};

struct PLMN_Identity
{
    char mcc[4];
    char mnc[3];
};

struct plmn_IdentityList
{
    int pil_pad;
    struct PLMN_Identity plmn_identity;
} plmn_identitylist;

struct cellAccessRelatedInfo
{
    int cari_pad;
    struct plmn_IdentityList plmn_identitylist;
};

struct cellSelectionInfo
{
    int q_RxLevMin;
    int q_RxLevMinOffset;
};

#pragma pack(1)
struct SIB1
{
    int sib1_pad;
    struct cellSelectionInfo cell_selection_info;
    struct cellAccessRelatedInfo cell_access_related_info;
    struct ServingCellConfigCommonSIB serving_cell_config_common_sib;
};
#pragma pack(0)

// RACH Message
// RA_RNTI
int ra_RNTI_Generator()
{
    int s_id = -1 + rand() % 16;
    int t_id = -1 + rand() % 81;
    int f_id = -1 + rand() % 10;
    int ul_carrier_id = 0;
    int ra_rnti = 1 + s_id + 14 * t_id + 14 * 80 * f_id + 14 * 80 * 8 * ul_carrier_id;
    return ra_rnti;
}

// Create Preamble
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

// Four Messages
struct MSG1
{
    char rach_Preamble[30];
    int ra_RNTI;
};

struct UL_grant
{
    int PUSCH_FrequencyDomainResoureAllocation;
    int PUSCH_TimeDomainResoureAllocation;
};
struct MSG2
{
    int RAPID;
    int timing_Advance;
    struct UL_grant ul_grant;
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
