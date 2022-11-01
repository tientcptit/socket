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

#pragma pack(1)
struct MIB
{
    int beam_power;
    int SFN;
};
#pragma pack(0)

#pragma pack(1)
struct SIB1
{
    int PRACH_Index;
};
#pragma pack(0)

typedef struct
{
    int connfd;
    struct sockaddr_in client_addr;
} pthread_arg_t;

typedef struct
{
    int SFN;
    int PRACH_Index;
} pthread_msg_t;

typedef struct
{
    int PRACH_Index;
    char Preamble_format[5];
    int n_SFN;
    int subFrame_Number;
    int slot_Number;
} PRACH_Config_Index;

typedef struct
{
    int RA_Preamble;
} MSG1;

typedef struct
{
    int RA_Preamble_RCV;
} MSG1_RCV;

#pragma pack(1)
typedef struct
{
    int RAPID;
    int TimingAdvance;
    int UL_grant;
    int TC_RNTI;
} MSG2;
#pragma pack(0)

typedef struct
{
    int UE_ID;
    char cause[20];
} MSG3;

typedef struct
{
    int UE_ID_RCV;
    char cause_RCV[20];
} MSG3_RCV;

#pragma pack(1)
typedef struct
{
    int UE_ID;
} MSG4;
#pragma pack(0)

void *msg_exchange(void *arg)
{
    pthread_arg_t *pthread_arg = (pthread_arg_t *)arg;
    int connfd = pthread_arg->connfd;
    struct sockaddr_in client_addr = pthread_arg->client_addr;

    socklen_t client_addr_sz = sizeof(struct sockaddr_in);

    MSG1 msg1;
    MSG1_RCV msg1_rcv;

    MSG3 msg3;
    MSG3_RCV msg3_rcv;

    recvfrom(connfd, &msg1, sizeof(msg1), 0, (struct sockaddr *)&client_addr, &client_addr_sz);
    msg1_rcv.RA_Preamble_RCV = msg1.RA_Preamble;

    MSG2 msg2 = {1111, 2222, 3333, 4444};
    sendto(connfd, &msg2, sizeof(msg2), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));

    recvfrom(connfd, &msg3, sizeof(msg3), 0, (struct sockaddr *)&client_addr, &client_addr_sz);
    msg3_rcv.UE_ID_RCV = msg3.UE_ID;
    strcpy(msg3_rcv.cause_RCV, msg3.cause);

    MSG4 msg4 = {msg3_rcv.UE_ID_RCV};
    sendto(connfd, &msg4, sizeof(msg4), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));

    printf("MSG1 [RECV]: Preamble %d\n", msg1_rcv.RA_Preamble_RCV);
    printf("MSG2 [SEND]: RAPID=%d; TA=%d; UL=%d; RNTI=%d\n", msg2.RAPID, msg2.TimingAdvance, msg2.UL_grant, msg2.TC_RNTI);
    printf("MSG3 [RECV]: UEID=%d; cause=%s\n", msg3_rcv.UE_ID_RCV, msg3_rcv.cause_RCV);
    printf("MSG4 [SEND]: UEID=%d\n", msg4.UE_ID);
}

void *handler_func(void *arg)
{
    pthread_arg_t *pthread_arg = (pthread_arg_t *)arg;
    int connfd = pthread_arg->connfd;
    struct sockaddr_in client_addr = pthread_arg->client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    printf(YEL "Sent MIB and SIB1 to UE: " RESET);
    srand(time(NULL));
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 6; j++)
        {
            int pow_int = rand() % 10;
            int sfn_int = rand() % 20;
            struct MIB mib = {pow_int, sfn_int};
            sendto(connfd, &mib, sizeof(mib), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
            if (i == 0)
            {
                printf("[%d and %d] ", mib.beam_power, mib.SFN);
            }
        }
        int sib1_int = rand() % 2 + 70;
        struct SIB1 sib1 = {sib1_int};
        sendto(connfd, &sib1, sizeof(sib1), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
        printf("[%d] ", sib1.PRACH_Index);
    }
    printf("\n");
    socklen_t client_addr_sz = sizeof(struct sockaddr_in);

    MSG1 msg1;
    MSG1_RCV msg1_rcv;

    MSG3 msg3;
    MSG3_RCV msg3_rcv;

    recvfrom(connfd, &msg1, sizeof(msg1), 0, (struct sockaddr *)&client_addr, &client_addr_sz);
    msg1_rcv.RA_Preamble_RCV = msg1.RA_Preamble;

    MSG2 msg2 = {1111, 2222, 3333, 4444};
    sendto(connfd, &msg2, sizeof(msg2), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));

    recvfrom(connfd, &msg3, sizeof(msg3), 0, (struct sockaddr *)&client_addr, &client_addr_sz);
    msg3_rcv.UE_ID_RCV = msg3.UE_ID;
    strcpy(msg3_rcv.cause_RCV, msg3.cause);

    MSG4 msg4 = {msg3_rcv.UE_ID_RCV};
    sendto(connfd, &msg4, sizeof(msg4), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));

    printf("MSG1 [RECV]: Preamble %d\n", msg1_rcv.RA_Preamble_RCV);
    printf("MSG2 [SEND]: RAPID=%d; TA=%d; UL=%d; RNTI=%d\n", msg2.RAPID, msg2.TimingAdvance, msg2.UL_grant, msg2.TC_RNTI);
    printf("MSG3 [RECV]: UEID=%d; cause=%s\n", msg3_rcv.UE_ID_RCV, msg3_rcv.cause_RCV);
    printf("MSG4 [SEND]: UEID=%d\n", msg4.UE_ID);
}

int main()
{
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;

    pthread_arg_t *pthread_arg;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(8888);
    printf("Starting simulation...\n");

    int enable = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        perror("Setsockopt");
        exit(1);
    }
    int bin_val = bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bin_val < 0)
    {
        perror("Binding");
        exit(1);
    }
    listen(listenfd, 5);
    printf("Listening incoming connection...\n");

    while (1)
    {
        pthread_arg = (pthread_arg_t *)malloc(sizeof(*pthread_arg));
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_len);
        if (connfd < 0)
        {
            perror("Accept");
            exit(1);
        }

        printf(YEL "Connection accepted from %s:%d\n" RESET, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        pthread_arg->connfd = connfd;
        pthread_arg->client_addr = client_addr;

        pthread_t tid;
        pthread_create(&tid, NULL, handler_func, (void *)pthread_arg);
        // pthread_join(tid, NULL);
    }

    return 0;
}