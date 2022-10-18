
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

void RACH_gen(PRACH_Config_Index *prach[2])
{
    prach[0]->n_SFN = 10;
    prach[0]->PRACH_Index = 70;
    char preamble_4mat0[3] = "A3";
    strcpy(prach[0]->Preamble_format, preamble_4mat0);
    prach[0]->slot_Number = 19;
    prach[0]->subFrame_Number = 1;

    prach[1]->n_SFN = 10;
    prach[1]->PRACH_Index = 71;
    char preamble_4mat1[3] = "A3";
    strcpy(prach[1]->Preamble_format, preamble_4mat1);
    prach[1]->slot_Number = 39;
    prach[1]->subFrame_Number = 1;
}

void *send_MIB_func(void *arg)
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
        int sib1_int = 70;
        struct SIB1 sib1 = {sib1_int};
        sendto(connfd, &sib1, sizeof(sib1), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
        printf("[%d] ", sib1.PRACH_Index);
    }
    printf("\n");
}

void *message_Handler(void *arg)
{
    pthread_arg_t *pthread_arg = (pthread_arg_t *)arg;
    int connfd = pthread_arg->connfd;
    struct sockaddr_in client_addr = pthread_arg->client_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

    printf(YEL "Sent 5 SIB1 to UE: " RESET);
    srand(time(NULL));
    for (int i = 0; i < 5; i++)
    {
        int sib1_int = 71;
        struct SIB1 sib1 = {sib1_int};
        sendto(connfd, &sib1, sizeof(sib1), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
        printf("[%d] ", sib1.PRACH_Index);
    }
    printf("\n");
}

int main()
{
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;

    pthread_arg_t *pthread_arg;
    socklen_t client_addr_sz = sizeof(struct sockaddr_in);

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
        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_sz);
        if (connfd < 0)
        {
            perror("Accept");
            exit(1);
        }
        printf(YEL "Connection accepted from %s:%d\n" RESET, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        pthread_arg->connfd = connfd;
        pthread_arg->client_addr = client_addr;

        pthread_t mib_tid, sib1_tid;
        pthread_create(&mib_tid, NULL, send_MIB_func, (void *)pthread_arg);
        // pthread_create(&sib1_tid, NULL, send_SIB1_func, (void *)pthread_arg);
        pthread_join(mib_tid, NULL);
        // pthread_join(sib1_tid, NULL);

        printf("---------------------------------------------------------------------------------------\n");
        close(connfd);
    }
    return 0;
}