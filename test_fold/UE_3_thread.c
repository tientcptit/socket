#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <pthread.h>

#define PORT 6969
#define BUF_SIZE 4096

#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"

typedef struct
{
    int sockfd;
    struct sockaddr_in server_addr;
} pthread_arg_t;

typedef struct
{
    int pow;
    int SFN;
} MIB;

typedef struct
{
    int PRACH_Index;
} SIB1;

typedef struct
{
    int pow_rcv;
    int SFN_rcv;
} MIB_RCV;

typedef struct
{
    MIB_RCV mib_rcv[7];
} BEAM_RCV;

typedef struct
{
    int PRACH_Index_rcv;
} SIB1_RCV;

#pragma pack(1)
typedef struct
{
    int RA_Preamble;
} MSG1;
#pragma pack(0)

typedef struct
{
    int RAPID;
    int TimingAdvance;
    int UL_grant;
    int TC_RNTI;
} MSG2;

typedef struct
{
    int RAPID_RCV;
    int TimingAdvance_RCV;
    int UL_grant_RCV;
    int TC_RNTI_RCV;
} MSG2_RCV;

#pragma pack(1)
typedef struct
{
    int UE_ID;
    char cause[20];
} MSG3;
#pragma pack(0)

typedef struct
{
    int UE_ID;
} MSG4;

typedef struct
{
    int UE_ID_RCV;
} MSG4_RCV;

void *recv_MIB_func(void *arg)
{
    BEAM_RCV beam_rcv[5];
    MIB mib;

    pthread_arg_t *pthread_arg = (pthread_arg_t *)arg;
    int sockfd = pthread_arg->sockfd;
    struct sockaddr_in server_addr = pthread_arg->server_addr;

    socklen_t server_addr_sz = sizeof(struct sockaddr_in);

    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 7; j++)
        {
            recvfrom(sockfd, &mib, sizeof(mib), MSG_WAITALL, (struct sockaddr *)&server_addr, &server_addr_sz);
            beam_rcv[i].mib_rcv[j].pow_rcv = mib.pow;
            beam_rcv[i].mib_rcv[j].SFN_rcv = mib.SFN;
        }
    }
    int max_pow = -150;
    int max_index = -1;
    printf(YEL "7 SSB of the first transmission: " RESET);
    for (int j = 0; j < 7; j++)
    {
        printf("[%d and %d] ", beam_rcv[0].mib_rcv[j].pow_rcv, beam_rcv[0].mib_rcv[j].SFN_rcv);
        if (beam_rcv[0].mib_rcv[j].pow_rcv > max_pow)
        {
            max_pow = beam_rcv[0].mib_rcv[j].pow_rcv;
            max_index = j;
        }
    }
    printf("\n");
    // printf(YEL "\nBest pair of beam: [%d and %d]\n" RESET, beam_rcv[0].mib_rcv[max_index].pow_rcv, beam_rcv[0].mib_rcv[max_index].SFN_rcv);
}

void *recv_SIB1_func(void *arg)
{
    SIB1 sib1;
    SIB1_RCV sib1_rcv[5];

    pthread_arg_t *pthread_arg = (pthread_arg_t *)arg;
    int sockfd = pthread_arg->sockfd;
    struct sockaddr_in server_addr = pthread_arg->server_addr;

    socklen_t server_addr_sz = sizeof(struct sockaddr_in);

    printf(YEL "5 SIB1 received: " RESET);
    for (int i = 0; i < 5; i++)
    {
        recvfrom(sockfd, &sib1, sizeof(sib1), MSG_WAITALL, (struct sockaddr *)&server_addr, &server_addr_sz);
        sib1_rcv[i].PRACH_Index_rcv = sib1.PRACH_Index;
        printf("[%d] ", sib1_rcv[i].PRACH_Index_rcv);
    }
    printf("\n");
    // printf(YEL "\nThe first SIB1:  [%d]\n", sib1_rcv[0].PRACH_Index_rcv);
}

void *main_handler(void *arg)
{
    pthread_arg_t *pthread_arg = (pthread_arg_t *)arg;
    int sockfd = pthread_arg->sockfd;

    struct sockaddr_in server_addr = pthread_arg->server_addr;
    socklen_t server_addr_sz = sizeof(struct sockaddr_in);

    MSG2 msg2;
    MSG2_RCV msg2_rcv;

    MSG4 msg4;
    MSG4_RCV msg4_rcv;

    MSG1 msg1 = {12345};
    sendto(sockfd, &msg1, sizeof(msg1), MSG_CONFIRM, (struct sockaddr *)&server_addr, sizeof(server_addr));

    recvfrom(sockfd, &msg2, sizeof(msg2), MSG_WAITALL, (struct sockaddr *)&server_addr, &server_addr_sz);
    msg2_rcv.RAPID_RCV = msg2.RAPID;
    msg2_rcv.TC_RNTI_RCV = msg2.TC_RNTI;
    msg2_rcv.TimingAdvance_RCV = msg2.TimingAdvance;
    msg2_rcv.UL_grant_RCV = msg2.UL_grant;

    MSG3 msg3 = {9999, "mo-signaling"};
    sendto(sockfd, &msg3, sizeof(msg3), MSG_CONFIRM, (struct sockaddr *)&server_addr, sizeof(server_addr));

    recvfrom(sockfd, &msg4, sizeof(msg4), MSG_WAITALL, (struct sockaddr *)&server_addr, &server_addr_sz);
    msg4_rcv.UE_ID_RCV = msg4.UE_ID;

    printf("MSG1 [SEND]: Preamble %d\n", msg1.RA_Preamble);
    printf("MSG2 [RECV]: RAPID=%d; TA=%d; UL=%d; RNTI=%d\n", msg2_rcv.RAPID_RCV, msg2_rcv.TimingAdvance_RCV, msg2_rcv.UL_grant_RCV, msg2_rcv.TC_RNTI_RCV);
    printf("MSG3 [SEND]: UEID=%d; cause=%s\n", msg3.UE_ID, msg3.cause);
    printf("MSG4 [RECV]: UEID=%d\n", msg4_rcv.UE_ID_RCV);
}

int main()
{
    int sockfd;
    struct sockaddr_in server_addr;
    pthread_arg_t *pthread_arg;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8888);

    int connect_val = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (connect_val < 0)
    {
        perror("Connect");
        exit(1);
    }
    printf("Connected to server via port 8888\n");

    pthread_arg = (pthread_arg_t *)malloc(sizeof(*pthread_arg));
    pthread_arg->sockfd = sockfd;
    pthread_arg->server_addr = server_addr;

    pthread_t recv_MIB, recv_SIB1, main_th;
    pthread_create(&recv_MIB, NULL, recv_MIB_func, (void *)pthread_arg);
    // pthread_create(&recv_SIB1, NULL, recv_SIB1_func, (void *)pthread_arg);
    // pthread_create(&main_th, NULL, main_handler, (void *)pthread_arg);

    pthread_join(recv_MIB, NULL);
    pthread_join(recv_SIB1, NULL);
    // pthread_join(main_th, NULL);
    return 0;
}