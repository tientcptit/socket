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

#define PORT 8080

struct MIB
{
    int beam_power;
    int SFN;
};

struct SIB1
{
    int PRACH_Index;
};

struct BEAM
{
    struct MIB mib[6];
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
    char Preamble_format[5];
    int n_SFN;
    int subFrame_Number;
    int slot_Number;
} PRACH_Config_Index;

PRACH_Config_Index rach_congig;

#pragma pack(1)
struct MSG1
{
    int RA_Preamble;
    int RAPID;
};
#pragma pack(0)

struct MSG2
{
    int RAPID;
    int TimingAdvance;
    int UL_grant;
    int TC_RNTI;
};

#pragma pack(1)
struct MSG3
{
    long UE_ID;
};
#pragma pack(0)

struct MSG4
{
    long UE_ID;
};

// INIT MIB SIB1 MSG VALUE
struct MIB mib = {0, 0};
struct SIB1 sib1 = {0};
struct MSG1 msg1 = {1014028, 63};
struct MSG2 msg2 = {0, 0, 0, 0};
struct MSG3 msg3 = {9753893836127};
struct MSG4 msg4 = {0};

// MSG1 FUNC
void msg1_Preamble(int nvalue, int sockfd);
// MSG2 FUNC
int msg2_RAR(int rapid, int sockfd);
// MSG3 FUNC
int msg3_RRC(long uid);
// MSG4 FUCN
int msg4_contention_Resolution(long UID);

int main()
{
    printf("\n");
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_addr_sz = sizeof(struct sockaddr_in);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    if (sockfd < 0)
    {
        printf(RED "\nSOCKET CREATION ERROR\n" RESET);
        exit(EXIT_FAILURE);
    }
    printf(YEL "SOCKET IS CREATED...\n" RESET);

    int conn_stt = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (conn_stt < 0)
    {
        printf(RED "\nCONNECTION FAILED...\n" RESET);
        printf(YEL "EXITING...\n" RESET);
        exit(EXIT_FAILURE);
    }

    printf("CONNECTED TO SERVER VIA PORT %d...\n", PORT);
    printf("START SIMULATING...\n");
    printf("========================================\n");

    // RECV MIB AND SIB1
    int bytes_read = recvfrom(sockfd, &mib, sizeof(mib), 0, (struct sockaddr *)&server_addr, &server_addr_sz);
    printf("MIB rcv: %d %d\n", mib.beam_power, mib.SFN);
    bytes_read = recvfrom(sockfd, &sib1, sizeof(sib1), 0, (struct sockaddr *)&server_addr, &server_addr_sz);
    printf("SIB1 rcv: %d\n", sib1.PRACH_Index);

START1:
    // SEND MSG1
    msg1_Preamble(sib1.PRACH_Index, sockfd);
    printf("%s %d %d %d\n", rach_congig.Preamble_format, rach_congig.n_SFN, rach_congig.subFrame_Number, rach_congig.slot_Number);
    printf("========================================\n");

    int bytes_send = sendto(sockfd, &msg1, sizeof(msg1), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("Msg1 sent: %d %d\n", msg1.RA_Preamble, msg1.RAPID);

    // RECV MSG2
    bytes_read = recvfrom(sockfd, &msg2, sizeof(msg2), 0, (struct sockaddr *)&server_addr, &server_addr_sz);
    printf("Msg2 rcv: %d %d %d %d\n", msg2.RAPID, msg2.TimingAdvance, msg2.UL_grant, msg2.TC_RNTI);

    // Check preamble id

    /*char preamble_ack[5];
    if (msg2_Handle(msg2.RAPID) == 1)
    {
        goto START2;
        strcpy(preamble_ack, "1");
        bytes_send = write(sockfd, preamble_ack, strlen(preamble_ack));
    }
    else
    {
        goto START1;
        strcpy(preamble_ack, "1");
        bytes_send = write(sockfd, preamble_ack, strlen(preamble_ack));
    }

    usleep(100000);
 */
    if (msg2_RAR(msg2.RAPID, sockfd) == 1)
    {
        char preamble_ACK[5];
        strcpy(preamble_ACK, "1");
        bytes_send = sendto(sockfd, &preamble_ACK, sizeof(preamble_ACK), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        printf(YEL "Sent: %s\n" RESET, preamble_ACK);
        goto START2;
    }
    else
    {
        char preamble_ACK[5];
        strcpy(preamble_ACK, "0");
        bytes_send = sendto(sockfd, &preamble_ACK, sizeof(preamble_ACK), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        printf(YEL "Sent: %s\n" RESET, preamble_ACK);
        goto START1;
    }

START2:
    // SEND MSG3
    bytes_send = sendto(sockfd, &msg3, sizeof(msg3), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("Msg3 sent: %ld\n", msg3.UE_ID);

    // RECV MSG4
    bytes_read = recvfrom(sockfd, &msg4, sizeof(msg4), 0, (struct sockaddr *)&server_addr, &server_addr_sz);
    printf("Msg4 rcv: %ld\n", msg4.UE_ID);
    printf("========================================\n");

    if (msg4_contention_Resolution(msg4.UE_ID) == 1)
    {
        char msg4_ACK[5];
        strcpy(msg4_ACK, "1");
        bytes_send = sendto(sockfd, &msg4_ACK, sizeof(msg4_ACK), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        printf(GRN "Contention resulution successfully\n" RESET);
        printf(YEL "Random Access Procedure Done\n" RESET);
        printf("========================================\n");
        printf(YEL "Connection has been established\n\n" RESET);
    }
    else
    {
        char msg4_ACK[5];
        strcpy(msg4_ACK, "0");
        bytes_send = sendto(sockfd, &msg4_ACK, sizeof(msg4_ACK), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
        printf(RED "Contention resulution failed\n" RESET);
        printf(YEL "Trying to retransmit preamble...\n" RESET);
        printf("========================================\n\n");
        goto START1;
    }
    // closing the connected socket
    close(sockfd);
    return 0;
}

// MAPPING VALUE FOR MSG1
void msg1_Preamble(int nvalue, int sockfd)
{
    if (nvalue == 70)
    {
        printf("UE will transmit preamble A3 at SFN 20, subframe 5 and slot 7\n");
        // rach_congig.= {"A3", 20, 5, 7};
        strcpy(rach_congig.Preamble_format, "A3");
        rach_congig.n_SFN = 20;
        rach_congig.subFrame_Number = 5;
        rach_congig.slot_Number = 7;
    }
    else
    {
        printf("Not likely\n");
        exit(1);
        // exit(EXIT_FAILURE);
    }
}

// CHECK RAPID
int msg2_RAR(int rapid, int sockfd)
{
    if (rapid == msg1.RAPID)
    {
        printf(YEL "The base station received the correct preamble\n" RESET);
        return 1;
    }
    else
    {
        printf(YEL "The base station received the wrong preamble\n" RESET);
        return 0;
    }
}

// CHECK UEID
int msg4_contention_Resolution(long UID)
{
    if (UID == msg3.UE_ID)
    {
        return 1;
    }
    else
    {
        return -1;
    }
}