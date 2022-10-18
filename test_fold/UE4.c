#define sleep_time_ms 0.01 * 1000000

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
#define WHT "\x1B[36m"
#define RESET "\x1B[0m"

struct MIB
{
    int beam_power;
    int SFN;
};

struct MIB_RCV
{
    int beam_power_rcv;
    int SFN_rcv;
};

struct SIB1
{
    int PRACH_Index;
};

struct SIB1_RCV
{
    int PRACH_Index_rcv;
};

struct BEAM_RCV
{
    struct MIB_RCV mib_rcv[6];
};

typedef struct
{
    int sockfd;
    int SFN;
    int PRACH_Index;
    struct sockaddr_in server_addr;
} pthread_arg_t;

typedef struct
{
    int PRACH_Index;
    char Preamble_format[5];
    int n_SFN;
    int subFrame_Number;
    int slot_Number;
} PRACH_Config_Index;

PRACH_Config_Index prach[3];

void *message_Handler(void *arg);

void RACH_gen()
{
    prach[0].n_SFN = 10;
    prach[0].PRACH_Index = 70;
    char preamble_4mat0[3] = "A3";
    strcpy(prach[0].Preamble_format, preamble_4mat0);
    prach[0].slot_Number = 19;
    prach[0].subFrame_Number = 1;

    prach[1].n_SFN = 10;
    prach[1].PRACH_Index = 71;
    char preamble_4mat1[3] = "A3";
    strcpy(prach[1].Preamble_format, preamble_4mat1);
    prach[1].slot_Number = 39;
    prach[1].subFrame_Number = 1;
}

void *MIB_SIB1_func(void *arg)
{
    pthread_arg_t *pthread_arg = (pthread_arg_t *)arg;
    int sockfd = pthread_arg->sockfd;
    struct sockaddr_in server_addr = pthread_arg->server_addr;

    struct BEAM_RCV beam_rcv[5];
    struct MIB mib;
    struct SIB1 sib1;
    struct SIB1_RCV sib1_rcv[6];

    socklen_t server_addr_len = sizeof(struct sockaddr_in);
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            recvfrom(sockfd, &mib, sizeof(mib), 0, (struct sockaddr *)&server_addr, &server_addr_len);
            beam_rcv[i].mib_rcv[j].beam_power_rcv = mib.beam_power;
            beam_rcv[i].mib_rcv[j].SFN_rcv = mib.SFN;
        }
        recvfrom(sockfd, &sib1, sizeof(sib1), 0, (struct sockaddr *)&server_addr, &server_addr_len);
        sib1_rcv[i].PRACH_Index_rcv = sib1.PRACH_Index;
    }
    int max_power = -150;
    int max_index = -1;
    for (int i = 0; i < 6; i++)
    {
        if (beam_rcv[0].mib_rcv[i].beam_power_rcv > max_power)
        {
            max_power = beam_rcv[0].mib_rcv[i].beam_power_rcv;
            max_index = i;
        }
    }
    pthread_arg->SFN = beam_rcv[0].mib_rcv[max_index].SFN_rcv;
    pthread_arg->PRACH_Index = sib1_rcv[0].PRACH_Index_rcv;
    pthread_t msg_tid;
    pthread_create(&msg_tid, NULL, message_Handler, (void *)pthread_arg);

    printf(YEL "6 SSB first: " RESET);
    for (int i = 0; i < 6; i++)
    {
        printf("[%d and %d] ", beam_rcv[0].mib_rcv[i].beam_power_rcv, beam_rcv[0].mib_rcv[i].SFN_rcv);
    }
    printf("\n");
    printf(YEL "5 SIB1 recv: " RESET);
    for (int i = 0; i < 5; i++)
    {
        printf("[%d] ", sib1_rcv[i].PRACH_Index_rcv);
    }
    printf("\n");
}

void *message_Handler(void *arg)
{
    pthread_arg_t *pthread_arg = (pthread_arg_t *)arg;
    int sockfd = pthread_arg->sockfd;
    struct sockaddr_in server_addr = pthread_arg->server_addr;
    int SFN = pthread_arg->SFN;
    int prach_index = pthread_arg->PRACH_Index;

    printf(YEL "SFN rec: [%d]\n" RESET, SFN);
    printf(YEL "RACH index: [%d]\n" RESET, prach_index);

    int ind = -1;
    for (int i = 0; i < 2; i++)
    {
        if (prach_index == prach[i].PRACH_Index)
        {
            ind = i;
        }
        // printf("%d ", prach[i].PRACH_Index);
    }
    printf("Index %d", ind);
    // printf(YEL "UE will trans this config: [Index:%d] [Pream:%s] [nSFN: %d] [subFN%d] [slot:%d]\n" RESET,
    //        prach[ind]->PRACH_Index,
    //        prach[ind]->Preamble_format,
    //        prach[ind]->n_SFN,
    //        prach[ind]->subFrame_Number,
    //        prach[ind]->slot_Number);
    printf("\n");
}

int main()
{
    RACH_gen();
    int sockfd;
    struct sockaddr_in server_addr;
    pthread_arg_t *pthread_arg;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8888);

    connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("Connected to server via port 8888\n");

    pthread_arg = (pthread_arg_t *)malloc(sizeof(*pthread_arg));
    pthread_arg->sockfd = sockfd;
    pthread_arg->server_addr = server_addr;

    pthread_t mibsib_tid, msg_tid;
    pthread_create(&mibsib_tid, NULL, MIB_SIB1_func, (void *)pthread_arg);

    pthread_join(mibsib_tid, NULL);
    pthread_join(msg_tid, NULL);

    printf("---------------------------------------------------------------------------\n");
    return 0;
}
