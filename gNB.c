#include "message.h"
#include "myheader.h"

char time_buf[20];

#define sleep_time_ms 0.5 * 1000000

typedef struct pthread_arg_t
{
    int connfd;
    struct sockaddr_in client_addr;
} pthread_arg_t;

void *get_time();
void *connected_thread_func(void *arg);

int main()
{
    struct MIB;
    struct SIB1;
    // printf("UE would transmit at SFN: %s\n", MIB.systemFrameNumber);
    // printf("Cell Status: %s\n", MIB.cellBarred);
    // printf("Mobile Country Code : %s\n", SIB1.cell_access_related_info.plmn_identitylist.plmn_identity.mcc);
    // printf("Mobile Network Code: %s\n", SIB1.cell_access_related_info.plmn_identitylist.plmn_identity.mnc);

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

        printf("Connection accepted from %s:%d at %s\n", cl_addr, cl_port, acc_time);
        int thread_val = pthread_create(&tid, 0, connected_thread_func, (void *)pthread_arg);
        if (thread_val != 0)
        {
            perror("Thread");
            exit(1);
        }
    }
    return 0;
}
void *get_time()
{
    time_t t;
    struct tm *tm_info;
    t = time(NULL);
    tm_info = localtime(&t);
    strftime(time_buf, 20, "%H:%M:%S", tm_info);
    return time_buf;
}

void *connected_thread_func(void *arg)
{
    pthread_arg_t *pthread_arg = (pthread_arg_t *)arg;
    int connfd = pthread_arg->connfd;
    struct sockaddr_in client_addr = pthread_arg->client_addr;
    int cl_port = ntohs(client_addr.sin_port);

    char send_buf[BUF_SIZE];
    char recv_buf[BUF_SIZE];

    struct timeval begin, end;
    int loop = 0;
}
/*
struct timeval begin, end;
int i = 0;
gettimeofday(&begin, NULL);
dis_time();
while (i < 10)
{
    cout << i << " ";
    usleep(1 * 1000000);
    i++;
}
cout << endl;
dis_time();
gettimeofday(&end, NULL);
long seconds = (end.tv_sec - begin.tv_sec) * 1000.0;
seconds += (end.tv_usec - begin.tv_usec) / 1000.0;
cout << "Elapsed time is " << YEL << seconds << RESET << " ms" << endl;
*/