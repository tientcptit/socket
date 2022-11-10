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
uint32_t C_RNTI = 0;

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

// Contention Resolution in MSG4
int contention_Resolution(uint32_t msg3_UEID, uint32_t msg4_UEID);

// Check if gNB has received correct preamble sent by UE function
int check_Preamble(uint32_t msg1_RAPID, uint32_t msg2_RAPID);

// Create Preamble function
uint32_t preamble_Create(uint32_t minN, uint32_t maxN);

// Create UEID function
uint32_t ueid_Create(void);

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

    /* TIME METER*/

    /*
    struct timeval timeStamp1, timeStamp2;
    srand((int)time(NULL));
    while (1)
    {
        if (SFN == 3)
        {
            if (SL == 1)
            {
                gettimeofday(&timeStamp1, NULL);
            }
            if (SL == 2)
            {
                gettimeofday(&timeStamp2, NULL);
            }
        }
    }
    printf("Interval time between 2 slot: %ld\n", ((timeStamp2.tv_sec * 1000000 + timeStamp2.tv_usec) - (timeStamp1.tv_sec * 1000000 + timeStamp1.tv_usec)));
    */

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
    int sockfd;

    // Socket structure
    struct sockaddr_in server_addr;
    socklen_t server_addr_len; // Server address struct length
    server_addr_len = sizeof(server_addr);

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);             // TCP socket
    server_addr.sin_family = AF_INET;                     // IP v4
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Local address
    server_addr.sin_port = htons(PORT);                   // Assign server port

    // Check if socket creation failed
    if (sockfd < 0)
    {
        perror(RED "SOCKET CREATION ERROR" RESET);
        exit(EXIT_FAILURE);
    }

    // Check if connection to server failed
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror(RED "CONNECTION ERROR" RESET);
        exit(EXIT_FAILURE);
    }

    // Recv MIB
    RRC_MIB recv_MIB;

    int bytes_recv_MIB = recv(sockfd, &recv_MIB, sizeof(RRC_MIB), 0); // Function to receive data
    if (bytes_recv_MIB < 0)                                           // Check if has no data in recv_MIB
    {
        perror(RED "RECEIVE MIB ERROR" RESET);
        exit(EXIT_FAILURE);
    }
    printf_Real_Time();                                                                                      // Display time received data
    printf(YEL "[DL]: UE received MIB = " RESET "[%d; %d]" RESET "\n" RESET, recv_MIB.frame, recv_MIB.slot); // Display MIB
    SFN = recv_MIB.frame;                                                                                    // System Frame Synchronization
    SL = recv_MIB.slot;                                                                                      // Slot Synchronization

    // SIB1 recvs at SFN = 3 and slot = 5
    int check_SIB1_Recv_One_Time = 0;
    RRC_SIB1 recv_SIB1; // Struct to storage SIB1

    while (1)
    {
        if ((SFN == 3) && (check_SIB1_Recv_One_Time == 0)) // Check Frame that recv SIB1 and flag assure not re-recv SIB1
        {
            if (SL == 5) // Check slot
            {
                int bytes_recv_SIB1 = recv(sockfd, &recv_SIB1, sizeof(RRC_SIB1), 0);
                if (bytes_recv_SIB1 < 0)
                {
                    perror(RED "RECEIVE SIB1 ERROR" RESET);
                    exit(EXIT_FAILURE);
                }
                printf_Real_Time();
                printf(YEL "[DL]: UE received SIB1 = " RESET "[%d; %d]" RESET " " YEL "at" RESET " " CYN "SFN = %d" RESET ", Slot = %d\n" RESET, recv_SIB1.SysFrame, recv_SIB1.Slot, SFN, SL);
                check_SIB1_Recv_One_Time = 1;
            }
        }
        if (check_SIB1_Recv_One_Time == 1)
            break;
    }

    //  MSG1 sends ats SFN = 4 and slot = 4
    int check_MSG1_Send_One_Time = 0;
    MSG1 send_MSG1;
    // send_MSG1.RA_Preamble = preamble_Create(1000000000, 1500000000);
    send_MSG1.RA_Preamble = 68688686;
    send_MSG1.RAPID = 63;

MSG1_Label:
    while (1)
    {
        if ((SFN == recv_SIB1.SysFrame) && (check_MSG1_Send_One_Time == 0))
        {
            if (SL == recv_SIB1.Slot)
            {
                int bytes_send_MSG1 = send(sockfd, &send_MSG1, sizeof(MSG1), 0);
                gettimeofday(&msg1_Start, NULL);
                if (bytes_send_MSG1 < 0)
                {
                    perror(RED "SEND MSG1 ERROR" RESET);
                    exit(EXIT_FAILURE);
                }

                printf_Real_Time();
                printf(GRN "[UL]: UE sent MSG1 = " RESET "[%d; %d]" RESET " " GRN "at" RESET " " CYN "SFN = %d" RESET ", Slot = %d\n" RESET, send_MSG1.RA_Preamble, send_MSG1.RAPID, SFN, SL);
                print_Current_Time_Msec();
                check_MSG1_Send_One_Time = 1;
            }
        }
        if (check_MSG1_Send_One_Time == 1)
            break;
    }

    // MSG2 recvs at SFN = 5 and slot = 5
    int check_MSG2_Recv_One_Time = 0;
    MSG2 recv_MSG2;

MSG2_Label:
    while (1)
    {
        if ((SFN == 5) && (check_MSG2_Recv_One_Time == 0))
        {
            if (SL == 5)
            {
                int bytes_recv_MSG2 = recv(sockfd, &recv_MSG2, sizeof(MSG2), 0);
                gettimeofday(&msg2_Start, NULL);

                if (bytes_recv_MSG2 < 0)
                {
                    perror(RED "RECEIVE MSG2 ERROR" RESET);
                    exit(EXIT_FAILURE);
                }

                printf_Real_Time();
                printf(YEL "[DL]: UE received MSG2 = " RESET "[%d; %d; %d; %d; %d]" RESET " " YEL "at" RESET " " CYN "SFN = %d" RESET ", Slot = %d\n" RESET, recv_MSG2.RAPID, recv_MSG2.SysFrame, recv_MSG2.Slot, recv_MSG2.Timing_Advance, recv_MSG2.TC_RTNI, SFN, SL);
                print_Current_Time_Msec();
                printf(CYN "Time interval from Msg1 to Msg2 is: " YEL "%ld" RESET "" CYN " micro seconds\n" RESET "" RESET, ((msg2_Start.tv_sec * 1000000 + msg2_Start.tv_usec) - (msg1_Start.tv_sec * 1000000 + msg1_Start.tv_usec)));
                check_MSG2_Recv_One_Time = 1;
            }
        }
        if (check_MSG2_Recv_One_Time == 1)
            break;
    }

    // Check preamble ID to assure that gNB not resent to UE
    char preamble_ACK_Response[5];

    if (check_Preamble(send_MSG1.RAPID, recv_MSG2.RAPID) == 1)
    {
        strcpy(preamble_ACK_Response, "1");
        int bytes_send_pACK = send(sockfd, preamble_ACK_Response, sizeof(preamble_ACK_Response), 0);
        if (bytes_send_pACK < 0)
        {
            perror(RED "SEND PREAMBLE ACK ERROR" RESET);
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        strcpy(preamble_ACK_Response, "0");
        int bytes_send_pACK = send(sockfd, preamble_ACK_Response, sizeof(preamble_ACK_Response), 0);
        if (bytes_send_pACK < 0)
        {
            perror(RED "SEND PREAMBLE ACK ERROR" RESET);
            exit(EXIT_FAILURE);
        }

        check_MSG1_Send_One_Time = 0;
        rapid_reTrans++;
        printf_Real_Time();
        printf(BLU "Trying to resend preamble to gNB: [%d]\n" RESET, rapid_reTrans);
        if (rapid_reTrans == rapid_TransMax)
        {
            printf_Real_Time();
            printf(YEL "Stopped trying resend random access preamble after %d attempts\n" RESET, rapid_TransMax);
            printf_Real_Time();
            printf(RED "Random acess procedure failed\n" RESET);
            printf("=================================================================\n");
            exit(EXIT_FAILURE);
        }
        goto MSG1_Label;
    }

    // MSG3 sends at SFN = 6 and slot = 5
    int check_MSG3_Send_One_Time = 0;
    MSG3 send_MSG3;
    // send_MSG3.UE_ID = ueid_Create();
    send_MSG3.UE_ID = 35750427;

MSG3_Label:
    while (1)
    {
        if ((SFN == recv_MSG2.SysFrame) && (check_MSG3_Send_One_Time == 0))
        {
            if (SL == recv_MSG2.Slot)
            {
                int bytes_send_MSG3 = send(sockfd, &send_MSG3, sizeof(MSG3), 0);
                gettimeofday(&msg3_Start, NULL);
                if (bytes_send_MSG3 < 0)
                {
                    perror(RED "SEND MSG3 ERROR" RESET);
                    exit(EXIT_FAILURE);
                }
                printf_Real_Time();
                printf(GRN "[UL]: UE sent MSG3 = " RESET "[%d]" RESET " " GRN "at" RESET " " CYN "SFN = %d" RESET ", Slot = %d\n" RESET, send_MSG3.UE_ID, SFN, SL);
                print_Current_Time_Msec();
                printf(CYN "Time interval from Msg2 to Msg3 is: " YEL "%ld" RESET "" CYN " micro seconds\n" RESET "" RESET, ((msg3_Start.tv_sec * 1000000 + msg3_Start.tv_usec) - (msg2_Start.tv_sec * 1000000 + msg2_Start.tv_usec)));
                check_MSG3_Send_One_Time = 1;
            }
        }
        if (check_MSG3_Send_One_Time == 1)
            break;
    }

    // MSG4 recvs at SFN = 7 and slot = 5
    int check_MSG4_Recv_One_Time = 0;
    MSG4 recv_MSG4;

MSG4_Label:

    while (1)
    {
        if ((SFN == 7) && (check_MSG4_Recv_One_Time == 0))
        {
            if (SL == 5)
            {
                int bytes_recv_MSG4 = recv(sockfd, &recv_MSG4, sizeof(MSG4), 0);
                gettimeofday(&msg4_Start, NULL);
                if (bytes_recv_MSG4 < 0)
                {
                    perror(RED "RECEIVE MSG4 ERROR" RESET);
                    exit(EXIT_FAILURE);
                }

                printf_Real_Time();
                printf(YEL "[DL]: UE received MSG4 = " RESET "[%d]" RESET " " YEL "at" RESET " " CYN "SFN = %d" RESET ", Slot = %d\n" RESET, recv_MSG4.UE_ID, SFN, SL);
                print_Current_Time_Msec();
                printf(CYN "Time interval from Msg3 to Msg4 is: " YEL "%ld" RESET "" CYN " micro seconds\n" RESET "" RESET, ((msg4_Start.tv_sec * 1000000 + msg4_Start.tv_usec) - (msg3_Start.tv_sec * 1000000 + msg3_Start.tv_usec)));
                check_MSG4_Recv_One_Time = 1;
            }
        }
        if (check_MSG4_Recv_One_Time == 1)
            break;
    }

    // Contention Resolution
    char cri_ACK[5];

    if (contention_Resolution(send_MSG3.UE_ID, recv_MSG4.UE_ID) == 1)
    {
        strcpy(cri_ACK, "1");
        int bytes_send_cACK = send(sockfd, cri_ACK, sizeof(cri_ACK), 0);
        if (bytes_send_cACK < 0)
        {
            perror(RED "SEND CRI ACK ERROR" RESET);
            exit(EXIT_FAILURE);
        }
        C_RNTI = recv_MSG2.TC_RTNI;

        printf_Real_Time();
        printf(YEL "TC RNTI has promoted to C RNTI for UE\n" RESET);
        printf_Real_Time();
        printf(GRN "Contention resolution identity successfully\n" RESET);
        printf_Real_Time();
        printf(GRN "Connection established\n" RESET);
        printf("=================================================================\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        strcpy(cri_ACK, "0");
        int bytes_send_cACK = send(sockfd, cri_ACK, sizeof(cri_ACK), 0);
        if (bytes_send_cACK < 0)
        {
            perror(RED "SEND CRI ACK ERROR" RESET);
            exit(EXIT_FAILURE);
        }

        prach_reTrans++;
        printf_Real_Time();
        printf(BLU "Trying start random access procedure: [%i]\n" RESET, prach_reTrans);
        check_MSG1_Send_One_Time = 0;
        check_MSG2_Recv_One_Time = 0;
        check_MSG3_Send_One_Time = 0;
        check_MSG4_Recv_One_Time = 0;

        if (prach_reTrans == prach_TransMax)
        {
            printf_Real_Time();
            printf(YEL "Abandon the random access procedure after %d attempts\n" RESET, prach_TransMax);
            printf_Real_Time();
            printf(RED "Contention resolution identity failed\n" RESET);
            printf_Real_Time();
            printf(RED "Random acess procedure failed\n" RESET);
            printf("=================================================================\n");
            exit(EXIT_FAILURE);
        }
        goto MSG1_Label;
    }
}

void *SFN_Counter_Thread(void *arg)
{
    while (1)
    {
        if (SL < (pow(2, numerology) * 10))
        {
            printf("Slot number: %d\n", SL);
            SL++;
        }
        else
        {
            SL = 0;
            printf(CYN "Frame number: %d\n" RESET, SFN);
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

int contention_Resolution(uint32_t msg3_UEID, uint32_t msg4_UEID)
{
    if (msg3_UEID == msg4_UEID)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int check_Preamble(uint32_t msg1_RAPID, uint32_t msg2_RAPID)
{
    if (msg1_RAPID == msg2_RAPID)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// Create Preamble
uint32_t preamble_Create(uint32_t minN, uint32_t maxN)
{
    return minN + rand() % (maxN - minN + 1);
}

// Create UEID
uint32_t ueid_Create(void)
{
    srand((int)time(NULL));
    return 3900000000 + rand() % (4000000000 - 3900000000 + 1);
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