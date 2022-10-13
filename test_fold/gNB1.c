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

#pragma pack(1)
typedef struct
{
    int SFN;
    int power;
} MIB;
#pragma pack(0)

#pragma pack(1)
typedef struct
{
    int PRACH_Config_Index;
    int SysFrame;
} SIB1;
#pragma pack(0)

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

void *timer_thread(void *arg);
void *connected_thread_func(void *arg);
int main()
{
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    char send_buf[BUF_SIZE];
    char recv_buf[BUF_SIZE];
    pthread_arg_t *pthread_arg;
    pthread_t tid;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    char *sock_time = get_time();
    if (listenfd == -1)
    {
        perror("Socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    printf(YEL "Starting simulation!!!\n" RESET);
    if (listenfd < 0)
    {
        perror("Socket");
        exit(1);
    }
    printf("Socket is created at ");
    printf(YEL "%s\n" RESET, sock_time);

    int enable = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    {
        perror("Setsockopt");
        exit(1);
    }

    int bind_val = bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_val < 0)
    {
        perror("Binding");
        exit(1);
    }
    listen(listenfd, 5);
    printf("Listening for incoming connections\n");
    printf("==============================\n");

    socklen_t client_addr_size;
    client_addr_size = sizeof(struct sockaddr_in);

    while (1)
    {
        pthread_arg = (pthread_arg_t *)malloc(sizeof(*pthread_arg));

        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &client_addr_size);
        char *acc_time = get_time();
        if (connfd < 0)
        {
            perror("Accept");
            exit(1);
        }

        char *cl_addr = inet_ntoa(client_addr.sin_addr);
        int cl_port = ntohs(client_addr.sin_port);

        pthread_arg->client_addr = client_addr;
        pthread_arg->connfd = connfd;
        printf("Connection accepted from %s:%d at ", cl_addr, cl_port);
        printf(YEL "%s\n" RESET, acc_time);
        int thread_val = pthread_create(&tid, 0, connected_thread_func, (void *)pthread_arg);
        if (thread_val != 0)
        {
            perror("Thread");
            exit(1);
        }
    }
    return 0;
}
void *connected_thread_func(void *arg)
{
    pthread_arg_t *pthread_arg = (pthread_arg_t *)arg;
    int connfd = pthread_arg->connfd;
    struct sockaddr_in client_addr = pthread_arg->client_addr;
    int cl_port = ntohs(client_addr.sin_port);

    char send_buf[BUF_SIZE];
    char recv_buf[BUF_SIZE];

    memset(send_buf, 0, sizeof(send_buf));
    strcpy(send_buf, "Hello from gNB");
    struct timeval begin, end;
    int loop = 0;
    gettimeofday(&begin, NULL);
    char *start_time = get_time();

    printf("Started sending data to UE at %s\n", start_time);
    srand(time(NULL));
    while (loop < 5)
    {
        int pow_rand = (-1) * (50 + rand() % 50);
        MIB mib = {10, pow_rand};
        sendto(connfd, &mib, sizeof(mib), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
        usleep(sleep_time_ms);
        loop++;
    }
    gettimeofday(&end, NULL);
    char *end_time = get_time();
    long time_exec = (end.tv_sec - begin.tv_sec) * 1000.0;
    time_exec += (end.tv_usec - begin.tv_usec) / 1000.0;
    printf(YEL "Finished sending to UE at %s \n" RESET, end_time);
    printf("Time spent in execution %ld ms\n", time_exec);
    printf("==============================\n");
}

void *timer_thread(void *arg)
{
}