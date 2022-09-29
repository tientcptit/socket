struct PSS
{
    int N_id_2;
};

struct SSS
{
    int N_id_1;
};

struct MIB
{
    char systemFrameNumber[7];
    char subCarrierSpacingCommon[10];
    int ssb_SubcarrierOffset;
    int pdcch_ConfigSIB1;
    char cellBarred[10];
};

struct PBCH_DMRS
{
    int SSB_Index;
    int RSRP;
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

struct UE_decoded
{
    int prach_configurationindex_decoded;
    int msg1_frequencystart_decoded;
    int preamblereceivedtargetpower_decoded;
    int preambletransmax_decoded;
    char MCC_decoded[4];
    char MNC_decoded[3];
    int totalnumberofra_preambles_decoded;
    int q_rxlevmin_decoded;
    int q_rxlevminoffset_decoded;
    int n_id_2_decoded;
    int n_id_1_decoded;
    int ssb_index_decoded;
    int rsrp_decoded;
    char systemframenumber_decoded[20];
    char susubcarrierspacingcommon_decoded[20];
    int ssb_subcarrieroffset_decoded;
    int pdcch_configsib1_decoded;
    char cellbarred_decoded[20];
};