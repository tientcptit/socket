#include "myheader.h"

typedef struct pthread_arg_t
{
    int connfd;
    struct sockaddr_in client_addr;
} pthread_arg_t;

void *thread_func(void *arg);

int main()
{
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    char send_buf[BUF_SIZE];
    char recv_buf[BUF_SIZE];
    socklen_t addr_len;

    pthread_arg_t *pthread_arg;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (listenfd == -1)
    {
        perror("Socket");
        exit(1);
    }
    printf("Socket is created!\n");

    int bind_stt = bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (bind_stt == -1)
    {
        perror("Bind");
        exit(1);
    }

    int lis_stt = listen(listenfd, 10);
    if (lis_stt == -1)
    {
        perror("Listen");
        exit(1);
    }
    printf("Listening connection\n");
    for (int i = 0; i < 30; i++)
    {
        printf("=");
    }
    printf("\n");

    int j, max = 3;
    while (j < max)
    {
        pthread_arg = (pthread_arg_t *)malloc(sizeof(*pthread_arg));

        connfd = accept(listenfd, (struct sockaddr *)&client_addr, &addr_len);
        if (connfd == -1)
        {
            perror("Accept");
            exit(1);
        }
        char *cl_addr = inet_ntoa(client_addr.sin_addr);
        int cl_port = ntohs(client_addr.sin_port);
        printf("Connection accepted from %s:%d\n", cl_addr, cl_port);

        int i, loop = 3;
        char *msg = "This is server msg ";
        for (i = 0; i < loop; i++)
        {
            memset(recv_buf, 0, sizeof(recv_buf));
            int bytes_recv = recv(connfd, recv_buf, sizeof(recv_buf), 0);
            printf(YEL "[UID=%d]:" RESET, cl_port);
            printf(GRN " client-to-server: %s\n" RESET, recv_buf);
            if (bytes_recv == -1)
            {
                perror("Recv");
                continue;
            }

            memset(send_buf, 0, sizeof(send_buf));
            char c = i;
            strcat(send_buf, msg);
            strncat(send_buf, &c, 1);
            printf(YEL "[UID=%d]:" RESET, cl_port);
            printf(BLU " server-to-client: %s\n" RESET, send_buf);
            int bytes_send = send(connfd, send_buf, sizeof(send_buf), 0);
            if (bytes_send == -1)
            {
                perror("Send");
                continue;
            }
        }
        j++;
        if (j == max)
            break;
        ;
    }

    close(connfd);
    close(listenfd);

    return 0;
}