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

#define PORT 8080
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

struct MSG1
{
    int RA_Preamble;
    int RAPID;
};

#pragma pack(1)
struct MSG2
{
    int RAPID;
    int TimingAdvance;
    int UL_grant;
    int TC_RNTI;
};
#pragma pack(0)

struct MSG3
{
    long UE_ID;
};

#pragma pack(1)
struct MSG4
{
    long UE_ID;
};
#pragma pack(0)

int main()
{
    printf("\n");
    struct MIB mib = {-70, 10};
    struct SIB1 sib1 = {70};
    struct MSG1 msg1 = {0, 0};
    struct MSG3 msg3 = {0};

    int listenfd, connfd, valread;
    struct sockaddr_in server_addr, client_addr;
    int opt = 1;
    // int client_addr_sz = sizeof(server_addr);
    socklen_t client_addr_sz = sizeof(struct sockaddr_in);

    // SOCKET
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
    {
        perror("SOCKET CREATION FAILED...\n");
        exit(EXIT_FAILURE);
    }
    printf(YEL "SOCKET IS CREATED...\n" RESET);

    // REUSE ADDRESS
    int sso_stt = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt));
    if (sso_stt < 0)
    {
        printf("CAN NOT REUSE THIS ADDRESS...\n");
        exit(EXIT_FAILURE);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    // BIND
    int bind_stt = bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_stt < 0)
    {
        printf("BINDING FAILED...\n");
        exit(EXIT_FAILURE);
    }

    // LISTEN
    int lis_stt = listen(listenfd, 5);
    if (lis_stt < 0)
    {
        printf("LISTENING ERROR...\n");
        exit(EXIT_FAILURE);
    }
    printf(YEL "LISTENING INCOMING CONNECTION...\n" RESET);
    printf("========================================\n");
    // ACCEPT
    connfd = accept(listenfd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_sz);
    if (connfd < 0)
    {
        printf("ACCEPT FAILED\n");
        exit(EXIT_FAILURE);
    }
    printf(YEL "Connection accepted from ");
    printf(MAG "%s:%d\n" RESET, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    int bytes_read;
    int bytes_send;

    bytes_send = sendto(connfd, &mib, sizeof(mib), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
    printf("MIB sent: %d %d\n", mib.beam_power, mib.SFN);
    bytes_send = sendto(connfd, &sib1, sizeof(sib1), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
    printf("SIB1 sent: %d\n", sib1.PRACH_Index);

START1:
    bytes_read = recvfrom(connfd, &msg1, sizeof(msg1), 0, (struct sockaddr *)&client_addr, &client_addr_sz);
    printf("Msg1 rcv: %d %d\n", msg1.RA_Preamble, msg1.RAPID);

    struct MSG2 msg2 = {msg1.RAPID, 12, 34, 56};
    bytes_send = sendto(connfd, &msg2, sizeof(msg2), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
    printf("Msg2 sent: %d %d %d %d\n", msg2.RAPID, msg2.TimingAdvance, msg2.UL_grant, msg2.TC_RNTI);

    /*char preamble_ack[5];
    bytes_read = read(connfd, preamble_ack, sizeof(preamble_ack));
    printf("Preamble ACK: %s\n", preamble_ack);

    if (strcmp(preamble_ack, "1") == 0)
    {
        goto START2;
    }
    else
    {
        goto START1;
    }
*/
    char buf[20];
    bytes_read = recvfrom(connfd, &buf, sizeof(buf), 0, (struct sockaddr *)&client_addr, &client_addr_sz);
    printf(YEL "%s\n" RESET, buf);
    if (strcmp(buf, "1") == 0)
    {
        printf("ACK\n");
        goto START2;
    }
    else
    {
        printf("NACK\n");
        goto START1;
    }
START2:
    bytes_read = recvfrom(connfd, &msg3, sizeof(msg3), 0, (struct sockaddr *)&client_addr, &client_addr_sz);
    printf("Msg3 rcv: %ld\n", msg3.UE_ID);

    struct MSG4 msg4 = {1};
    bytes_send = sendto(connfd, &msg4, sizeof(msg4), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
    printf("Msg4 sent: %ld\n", msg4.UE_ID);
    printf("========================================\n");

    // MSG4 ACK
    char msg4_ACK[5];
    bytes_read = recvfrom(connfd, &msg4_ACK, sizeof(msg4_ACK), 0, (struct sockaddr *)&client_addr, &client_addr_sz);

    if (strcmp(msg4_ACK, "1") == 0)
    {
        printf(YEL "Random Access Procedure Done\n" RESET);
        printf("========================================\n");
        printf(YEL "Connection has been established\n\n" RESET); // closing the connected socket
    }
    else
    {
        printf("Re Receiving preamble\n");
        goto START1;
    }

    close(connfd);
    // closing the listening socket
    shutdown(listenfd, SHUT_RDWR);

    return 0;
}