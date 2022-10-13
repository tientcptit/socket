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
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"

typedef struct
{
    int SFN;
    int power;
} MIB;

typedef struct
{
    int PRACH_Config_Index;
    int SysFrame;
} SIB1;

int SFN_RCV;
int power_rcv;

int PRACH_Config_Index_RCV;
int SysFrame_RCV;

typedef struct pthread_arg_t
{
    int connfd;
    struct sockaddr_in client_addr;
} pthread_arg_t;

char time_buf[20];
void *get_time()
{
    time_t t;
    struct tm *timex;
    t = time(NULL);
    timex = localtime(&t);
    strftime(time_buf, 20, "%H:%M:%S", timex);
    return time_buf;
}

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

    MIB mib;
    int loop = 0;
    struct timeval begin, end;
    socklen_t server_addr_len = sizeof(struct sockaddr_in);
    gettimeofday(&begin, NULL);
    char *start_time = get_time();
    printf(YEL "Reading from gNB at %s\n" RESET, start_time);
    int max_power = -100;
    while (loop < 5)
    {
        memset(recv_buf, 0, sizeof(recv_buf));

        recvfrom(sockfd, &mib, sizeof(mib), 0, (struct sockaddr *)&server_addr, &server_addr_len);
        usleep(sleep_time_ms);
        if (mib.power > max_power)
        {
            max_power = mib.power;
            SFN_RCV = mib.SFN;
        }
        printf(YEL "power [%d] = %d\n" RESET, loop, mib.power);
        loop++;
    }
    printf("Max beam power %d\n", max_power);
    printf("SFN received = %d\n", SFN_RCV);
    gettimeofday(&end, NULL);
    char *end_time = get_time();
    long time_exec = (end.tv_sec - begin.tv_sec) * 1000.0;
    time_exec += (end.tv_usec - begin.tv_usec) / 1000.0;
    printf(YEL "Finished reading from gNB at %s \n" RESET, end_time);
    printf("Time spent in execution %ld ms\n", time_exec);
    printf("==============================\n");

    return 0;
}