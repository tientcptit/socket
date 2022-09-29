#include "myheader.h"
#include "ue_message.h"
#include "basic_func.h"

int main()
{
    int sockfd;
    struct sockaddr_in server_addr;
    char send_buf[BUF_SIZE];
    char recv_buf[BUF_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    char *sock_time = get_time();
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        perror("Setsockopt");
        exit(1);
    }

    int connect_val = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (connect_val == -1)
    {
        perror("Connect");
        exit(0);
    }
    printf("Connected to server via port %d\n", PORT);

    struct MIB mib;
    struct SIB1 sib1;
    int loop = 0;
    struct timeval begin, end;
    socklen_t server_addr_len = sizeof(struct sockaddr_in);
    gettimeofday(&begin, NULL);
    char *start_time = get_time();
    printf(YEL "Reading from gNB at %s\n" RESET, start_time);
    while (loop < 5)
    {
        memset(recv_buf, 0, sizeof(recv_buf));

        recvfrom(sockfd, &mib, sizeof(mib), 0, (struct sockaddr *)&server_addr, &server_addr_len);
        usleep(sleep_time_ms / 10);
        recvfrom(sockfd, &sib1, sizeof(sib1), 0, (struct sockaddr *)&server_addr, &server_addr_len);
        usleep(sleep_time_ms);
        loop++;

        printf("MIB from gNB: %d %s (%s)\n", atoi(mib.systemFrameNumber), mib.cellBarred, cell_State(mib.cellBarred));
        printf("SIB1 from gNB: %d %d %d %d\n", sib1.serving_cell_config_common_sib.uplink_config_common_sib.bwp_uplinkcommon.rach_configcommon.rach_configgeneric.prach_ConfigurationIndex,
               sib1.serving_cell_config_common_sib.uplink_config_common_sib.bwp_uplinkcommon.rach_configcommon.rach_configgeneric.msg1_FrequencyStart,
               sib1.serving_cell_config_common_sib.uplink_config_common_sib.bwp_uplinkcommon.rach_configcommon.rach_configgeneric.preambleReceivedTargetPower,
               sib1.serving_cell_config_common_sib.uplink_config_common_sib.bwp_uplinkcommon.rach_configcommon.rach_configgeneric.preambleTransMax);
    }
    gettimeofday(&end, NULL);
    char *end_time = get_time();
    long time_exec = (end.tv_sec - begin.tv_sec) * 1000.0;
    time_exec += (end.tv_usec - begin.tv_usec) / 1000.0;
    printf(YEL "Finished reading from gNB at %s \n" RESET, end_time);
    printf("Time spent in execution %ld ms\n", time_exec);
    printf("==============================\n");

    return 0;
}
