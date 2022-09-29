#define prach_configurationindex 10
#define msg1_frequencystart 273
#define preamblereceivedtargetpower -50
#define preambletransmax 20
#define MCC "452"
#define MNC "04"
#define totalnumberofra_preambles 63
#define q_rxlevmin -70
#define q_rxlevminoffset -80
#define pading 0

struct MIB
{
    char systemFrameNumber[7];
    char subCarrierSpacingCommon[10];
    int ssb_SubcarrierOffset;
    int pdcch_ConfigSIB1;
    char cellBarred[10];
} MIB = {"101010", "scs15or69", 15, 1, "notBarred"};

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

struct SIB1
{
    int sib1_pad;
    struct cellSelectionInfo cell_selection_info;
    struct cellAccessRelatedInfo cell_access_related_info;
    struct ServingCellConfigCommonSIB serving_cell_config_common_sib;
} SIB1 = {pading,
          {q_rxlevmin, q_rxlevminoffset},
          {pading, {pading, {MCC, MNC}}},
          {pading, {pading, {pading, {totalnumberofra_preambles, {prach_configurationindex, msg1_frequencystart, preamblereceivedtargetpower, preambletransmax}}}}}};
