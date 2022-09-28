typedef struct MIB
{
    char systemFrameNumber[7] = "100100";
    char subCarrierSpacingCommon[10] = "scs15or60";
    int ssb_SubcarrierOffset = 10;
    int pdcch_ConfigSIB1 = 0;
    char cellBarred[10] = "notBarred";
} MIB;

struct RACH_ConfigGeneric
{
    int prach_ConfigurationIndex = 999;
    int msg1_FrequencyStart = 10;
    int preambleReceivedTargetPower = -100;
    int preambleTransMax = 10;
};

struct RACH_ConfigCommon
{
    int totalNumberOfRA_Preambles = 63;
    struct RACH_ConfigGeneric rach_configgeneric;
};

struct BWP_UplinkCommon
{
    struct RACH_ConfigCommon rach_configcommon;
};

struct UplinkConfigCommonSIB
{
    struct BWP_UplinkCommon bwp_uplinkcommon;
};

struct ServingCellConfigCommonSIB
{
    struct UplinkConfigCommonSIB uplink_config_common_sib;
};

struct PLMN_Identity
{
    int mcc = 452;
    int mnc = 4;
};

struct plmn_IdentityList
{
    struct PLMN_Identity plmn_identity;
};

struct cellAccessRelatedInfo
{
    struct plmn_IdentityList plmn_identitylist;
};

struct cellSelectionInfo
{
    int q_RxLevMin = -50;
    int q_RxLevMinOffset = -70;
};

typedef struct SIB1
{
    struct cellSelectionInfo cell_selection_info;
    struct cellAccessRelatedInfo cell_access_related_info;
    struct ServingCellConfigCommonSIB serving_cell_config_common_sib;
    ;

} SIB1;
