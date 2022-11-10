#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include <inttypes.h>

// Color Code
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[36m"
#define RESET "\x1B[0m"

#define PORT 8880
#define BACKLOG 10
#define rapid_TransMax 2
#define prach_TransMax 2

uint8_t SL = 0;
uint16_t SFN = 0;
uint8_t numerology = 0;
uint8_t rapid_reTrans;
uint8_t prach_reTrans;

// MIB Structure
typedef struct
{
    uint16_t frame;
    uint8_t slot;
} RRC_MIB;

// SIB1 Structure
typedef struct
{
    uint16_t SysFrame; // Set to 20
    uint8_t Slot;      // Set to 7
} RRC_SIB1;

// MSG1 Structure
typedef struct
{
    uint32_t RA_Preamble; // Random value {1000000; 9999999}
    uint8_t RAPID;        // Fix value set to 63
} MSG1;

// MSG2 Structure
typedef struct
{
    uint16_t RAPID;          // Compare to RAPID in MSG1, if not same start over from MSG1
    uint16_t SysFrame;       // Frame to trans MSG3, set to 30
    uint16_t Slot;           // Slot to trans MSG3, set to 9
    uint16_t Timing_Advance; // Set to zero
    uint32_t TC_RTNI;

} MSG2;

// MSG3 Structure
typedef struct
{
    uint32_t UE_ID; // Random value from 1000000 to 9999999
} MSG3;

// MSG4 Structure
typedef struct
{
    uint32_t UE_ID;
} MSG4;

/*
Function declaration
*/

// Independent system frame count thread
void *SFN_Counter_Thread(void *arg);

// Data exchange between UE and gNB function
void data_Trans(void);

// RNTI creation
uint32_t tc_RNTI(void);

//  Print DATE MONT YEAR and TIME
void printf_Real_Time(void);

// Printf current time
void print_Current_Time_Msec(void);

/*
    MAIN FUNCTION
*/
int main()
{
    printf("\n=========================***************=========================\n");
    printf_Real_Time();
    printf(BLU "Start Simulating\n" RESET);
    printf("=================================================================\n");

    pthread_t tid;
    pthread_create(&tid, NULL, SFN_Counter_Thread, NULL);
    data_Trans();

    pthread_exit(NULL);
    return 0;
}

/*
    Data exchange between UE and gNB function
*/

void data_Trans(void)
{
    // Time stamp
    struct timeval msg1_Start, msg2_Start, msg3_Start, msg4_Start;
    srand((int)time(NULL));

    // Create Socket File Descriptor
    int listenfd, connfd;

    // Socket structure
    struct sockaddr_in server_addr, client_addr;
    socklen_t server_addr_len, client_addr_len; // Server and client address struct length
    client_addr_len = sizeof(client_addr);
    server_addr_len = sizeof(server_addr);

    // Create socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);           // TCP socket
    server_addr.sin_family = AF_INET;                     // IP v4
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Local address
    server_addr.sin_port = htons(PORT);                   // Assign server port

    // Check if socket creation failed
    if (listenfd < 0)
    {
        perror(RED "SOCKET CREATION ERROR" RESET);
        exit(EXIT_FAILURE);
    }

    // Fix Address already used
    int option = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) < 0)
    {
        perror(RED "SETSOCKOPT ERROR" RESET);
        exit(EXIT_FAILURE);
    }

    // Binding server address
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror(RED "BIND ERROR" RESET);
        exit(EXIT_FAILURE);
    }

    // Listen incoming connection
    if (listen(listenfd, 10) < 0)
    {
        perror(RED "LISTEN ERROR" RESET);
        exit(EXIT_FAILURE);
    }

    // Accept connection
    if ((connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0)
    {
        perror(RED "ACCEPT ERROR" RESET);
        exit(EXIT_FAILURE);
    }
    printf(YEL "Connection from %s:%d\n" RESET, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // MIB initialize
    RRC_MIB send_MIB;
    send_MIB.frame = 2;
    send_MIB.slot = 5;

    send(connfd, &send_MIB, sizeof(RRC_MIB), 0);
    printf(YEL "[DL]: gNB sent MIB = [%i; %i]\n" RESET, send_MIB.frame, send_MIB.slot);
    SFN = send_MIB.frame;
    SL = send_MIB.slot;

    // SIB1 sends at SFN = 3 and slot = 5
    int check_SIB1_Send_One_Time = 0;
    RRC_SIB1 send_SIB1;
    send_SIB1.SysFrame = 4; // for msg1
    send_SIB1.Slot = 5;     // for msg1

    while (1)
    {
        if ((SFN == 3) && (check_SIB1_Send_One_Time == 0))
        {
            if (SL == 5)
            {
                int bytes_send_SIB1 = send(connfd, &send_SIB1, sizeof(RRC_SIB1), 0);
                if (bytes_send_SIB1 < 0)
                {
                    perror(RED "SEND SIB1 ERROR" RESET);
                    exit(EXIT_FAILURE);
                }
                printf_Real_Time();
                printf(YEL "[DL]: gNB sent SIB1 = " RESET "[%d; %d]" RESET " " YEL "at" RESET " " CYN "SFN = %d" RESET ", Slot = %d\n" RESET, send_SIB1.SysFrame, send_SIB1.Slot, SFN, SL);
                check_SIB1_Send_One_Time = 1;
            }
        }
        if (check_SIB1_Send_One_Time == 1)
            break;
    }

    // MSG1 recvs at SFN = 4 and slot = 5
    int check_MSG1_Recv_One_Time = 0;
    MSG1 recv_MSG1;

MSG1_Label:
    while (1)
    {
        if ((SFN == send_SIB1.SysFrame) && (check_MSG1_Recv_One_Time == 0))
        {
            if (SL == send_SIB1.Slot)
            {
                int bytes_recv_MSG1 = recv(connfd, &recv_MSG1, sizeof(MSG1), 0);
                gettimeofday(&msg1_Start, NULL);
                if (bytes_recv_MSG1 < 0)
                {
                    perror(RED "RECEIVE MSG1 ERROR" RESET);
                    exit(EXIT_FAILURE);
                }

                printf_Real_Time();
                printf(GRN "[UL]: gNB received MSG1 = " RESET "[%d; %d]" RESET " " GRN "at" RESET " " CYN "SFN = %d" RESET ", Slot = %d\n" RESET, recv_MSG1.RA_Preamble, recv_MSG1.RAPID, SFN, SL);
                print_Current_Time_Msec();
                check_MSG1_Recv_One_Time = 1;
            }
        }
        if (check_MSG1_Recv_One_Time == 1)
            break;
    }

    // MSG2 sends at SFN = 5 and slot = 5
    int check_MSG2_Send_One_Time = 0;
    MSG2 send_MSG2;

    send_MSG2.RAPID = recv_MSG1.RAPID;
    // send_MSG2.RAPID = 1;

    send_MSG2.SysFrame = 6;
    send_MSG2.Slot = 5;
    send_MSG2.Timing_Advance = 0;
    // send_MSG2.TC_RTNI = tc_RNTI();
    send_MSG2.TC_RTNI = 89049032;
MSG2_Label:
    while (1)
    {
        if ((SFN == 5) && (check_MSG2_Send_One_Time == 0))
        {
            if (SL == 5)
            {
                int bytes_send_MSG2 = send(connfd, &send_MSG2, sizeof(MSG2), 0);
                gettimeofday(&msg2_Start, NULL);

                if (bytes_send_MSG2 < 0)
                {
                    perror(RED "SEND MSG2 ERROR" RESET);
                    exit(EXIT_FAILURE);
                }

                printf_Real_Time();
                printf(YEL "[DL]: gNB sent MSG2 = " RESET "[%d; %d; %d; %d; %d]" RESET " " YEL "at" RESET " " CYN "SFN = %d" RESET ", Slot = %d\n" RESET, send_MSG2.RAPID, send_MSG2.SysFrame, send_MSG2.Slot, send_MSG2.Timing_Advance, send_MSG2.TC_RTNI, SFN, SL);
                print_Current_Time_Msec();
                printf(CYN "Time interval from Msg1 to Msg2 is: " YEL "%ld" RESET "" CYN " micro seconds\n" RESET "" RESET, ((msg2_Start.tv_sec * 1000000 + msg2_Start.tv_usec) - (msg1_Start.tv_sec * 1000000 + msg1_Start.tv_usec)));
                check_MSG2_Send_One_Time = 1;
            }
        }
        if (check_MSG2_Send_One_Time == 1)
            break;
    }

    // Check preamble ID that gNB resent to UE
    char preamble_ACK_Response[5];

    int bytes_recv_pACK = recv(connfd, preamble_ACK_Response, sizeof(preamble_ACK_Response), 0);
    if (bytes_recv_pACK < 0)
    {
        perror(RED "RECVEIVE PREAMBLE ACK ERROR" RESET);
        exit(EXIT_FAILURE);
    }

    if (strcmp(preamble_ACK_Response, "1") != 0)
    {
        check_MSG1_Recv_One_Time = 0;
        rapid_reTrans++;
        printf_Real_Time();
        printf(BLU "Trying rereceive preamble from UE: [%i]\n" RESET, rapid_reTrans);
        if (rapid_reTrans == rapid_TransMax)
        {
            printf_Real_Time();
            printf(YEL "Stopped trying rereceive random access preamble after %i attempts\n" RESET, rapid_TransMax);
            printf_Real_Time();
            printf(RED "Random acess procedure failed\n" RESET);
            printf("=================================================================\n");
            exit(EXIT_FAILURE);
        }
        goto MSG1_Label;
    }

    // MSG3 recvs at SFN = 6 and slot = 5
    int check_MSG3_Recv_One_Time = 0;
    MSG3 recv_MSG3;

MSG3_Label:
    while (1)
    {
        if ((SFN == send_MSG2.SysFrame) && (check_MSG3_Recv_One_Time == 0))
        {
            if (SL == send_MSG2.Slot)
            {
                int bytes_recv_MSG3 = recv(connfd, &recv_MSG3, sizeof(MSG3), 0);
                gettimeofday(&msg3_Start, NULL);
                if (bytes_recv_MSG3 < 0)
                {
                    perror(RED "RECEIVE MSG3 ERROR" RESET);
                    exit(EXIT_FAILURE);
                }
                printf_Real_Time();
                printf(GRN "[UL]: gNB received MSG3 = " RESET "[%d]" RESET " " GRN "at" RESET " " CYN "SFN = %d" RESET ", Slot = %d\n" RESET, recv_MSG3.UE_ID, SFN, SL);
                print_Current_Time_Msec();
                printf(CYN "Time interval from Msg2 to Msg3 is: " YEL "%ld" RESET "" CYN " micro seconds\n" RESET "" RESET, ((msg3_Start.tv_sec * 1000000 + msg3_Start.tv_usec) - (msg2_Start.tv_sec * 1000000 + msg2_Start.tv_usec)));
                check_MSG3_Recv_One_Time = 1;
            }
        }
        if (check_MSG3_Recv_One_Time == 1)
            break;
    }
    // MSG4 sends at SFN = 7 and slot = 5
    int check_MSG4_Send_One_Time = 0;
    MSG4 send_MSG4;

    send_MSG4.UE_ID = recv_MSG3.UE_ID;
    // send_MSG4.UE_ID = 1;

MSG4_Label:
    while (1)
    {
        if ((SFN == 7) && (check_MSG4_Send_One_Time == 0))
        {
            if (SL == 5)
            {
                int bytes_send_MSG4 = send(connfd, &send_MSG4, sizeof(MSG4), 0);
                gettimeofday(&msg4_Start, NULL);
                if (bytes_send_MSG4 < 0)
                {
                    perror(RED "SEND MSG4 ERROR" RESET);
                    exit(EXIT_FAILURE);
                }
                printf_Real_Time();

                printf(YEL "[DL]: gNB sent MSG4 = " RESET "[%d]" RESET " " YEL "at" RESET " " CYN "SFN = %d" RESET ", Slot = %d\n" RESET, send_MSG4.UE_ID, SFN, SL);
                print_Current_Time_Msec();
                printf(CYN "Time interval from Msg3 to Msg4 is: " YEL "%ld" RESET "" CYN " micro seconds\n" RESET "" RESET, ((msg4_Start.tv_sec * 1000000 + msg4_Start.tv_usec) - (msg3_Start.tv_sec * 1000000 + msg3_Start.tv_usec)));
                check_MSG4_Send_One_Time = 1;
            }
        }
        if (check_MSG4_Send_One_Time == 1)
            break;
    }

    // Contention Resolution
    char cri_ACK[5];

    int bytes_recv_cACK = recv(connfd, cri_ACK, sizeof(cri_ACK), 0);
    if (bytes_recv_cACK < 0)
    {
        perror(RED "RECEIVE CRI ACK ERROR" RESET);
        exit(EXIT_FAILURE);
    }
    if (strcmp(cri_ACK, "1") == 0)
    {
        printf_Real_Time();
        printf(GRN "Contention Resolution Successfully\n" RESET);
        printf_Real_Time();
        printf(GRN "Connection Established\n" RESET);
        printf("=================================================================\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        prach_reTrans++;
        printf_Real_Time();
        printf(BLU "Trying start random access procedure: [%i]\n" RESET, prach_reTrans);
        check_MSG1_Recv_One_Time = 0;
        check_MSG2_Send_One_Time = 0;
        check_MSG3_Recv_One_Time = 0;
        check_MSG4_Send_One_Time = 0;

        if (prach_reTrans == prach_TransMax)
        {
            printf_Real_Time();
            printf(RED "Abandon the random access procedure after %i attempts\n" RESET, prach_TransMax);
            printf_Real_Time();
            printf(RED "Contention resolution identity failed\n" RESET);
            printf_Real_Time();
            printf(RED "Random acess procedure failed\n" RESET);
            printf("=================================================================\n");
            exit(EXIT_FAILURE);
        }
        goto MSG1_Label;
    }
    close(connfd);
    shutdown(listenfd, SHUT_RDWR);
}

void *SFN_Counter_Thread(void *arg)
{
    while (1)
    {
        if (SL < (pow(2, numerology) * 10))
        {
            printf("Slot number: %i\n", SL);
            SL++;
        }
        else
        {
            SL = 0;
            printf(CYN "Frame number: %i\n" RESET, SFN);
            if (SFN < 8)
            {
                SFN++;
            }
            else
            {
                SFN = 0;
            }
        }
        usleep(10000);
    }
}

uint32_t tc_RNTI(void)
{
    srand((int)time(NULL));
    return 4100000000 + rand() % (4150000000 - 4100000000 + 1);
}

void print_Current_Time_Msec(void)
{
    long ms;  // Milliseconds
    time_t s; // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    s = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds
    if (ms > 999)
    {
        s++;
        ms = 0;
    }

    printf("Current time: " YEL "%" PRIdMAX ".%03ld" RESET " seconds since the Epoch\n", (intmax_t)s, ms);
}

void printf_Real_Time()
{
    time_t t;
    char buffer[26];
    struct tm *tm_info;

    t = time(NULL);
    tm_info = localtime(&t);

    strftime(buffer, 20, "%d-%m-%Y %H:%M:%S", tm_info);
    printf(YEL "[%s]: " RESET, buffer);
}