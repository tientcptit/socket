#include "myheader.h"

int main()
{
    int sockfd;
    struct sockaddr_in server_addr;

    char send_buf[BUF_SIZE];
    char recv_buf[BUF_SIZE];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (sockfd < 0)
    {
        perror("Socket");
        exit(0);
    }
    printf("Socket is created!\n");

    int stt = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (stt == -1)
    {
        perror("Connect");
        exit(0);
    }
    printf("Connected to server via port %d\n", PORT);

    int i, loop = 10;
    char *msg = "msg ";
    for (i = 0; i < loop; i++)
    {
        memset(send_buf, 0, sizeof(send_buf));
        char c = i;
        strcat(send_buf, msg);
        strncat(send_buf, &c, 1);
        printf("client-to-server: ");
        fgets(send_buf, sizeof(send_buf), stdin);
        int bytes_send = send(sockfd, send_buf, sizeof(send_buf), 0);
        if (bytes_send == -1)
        {
            perror("Send");
            continue;
        }

        memset(recv_buf, 0, sizeof(recv_buf));
        int bytes_recv = recv(sockfd, recv_buf, sizeof(recv_buf), 0);
        printf("server-to-client: ");
        printf("%s\n", recv_buf);
        if (bytes_recv == -1)
        {
            perror("Recv");
            continue;
        }
    }
    close(sockfd);
    return 0;
}