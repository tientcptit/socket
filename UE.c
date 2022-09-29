#include "myheader.h"
#include "message.h"

char time_buf[20];
void *get_time();

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
    printf("Connected to server via port %d!\n", PORT);

    int loop = 0;
    struct timeval begin, end;
    gettimeofday(&begin, NULL);
    char *start_time = get_time();
    printf("Reading from gNB at %s\n", start_time);
    while (loop < 5)
    {
        memset(recv_buf, 0, sizeof(recv_buf));
        read(sockfd, recv_buf, sizeof(recv_buf));
        char *recv_time = get_time();
        loop++;
        usleep(sleep_time_ms);
        printf("Message from gNB: %s at %s\n", recv_buf, recv_time);
    }
    gettimeofday(&end, NULL);
    char *end_time = get_time();
    long time_exec = (end.tv_sec - begin.tv_sec) * 1000.0;
    time_exec += (end.tv_usec - begin.tv_usec) / 1000.0;
    printf("Finished reading from gNB at %s \n", end_time);
    printf("Time spent in execution %ld ms\n", time_exec);
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
