#include "myheader.h"

// random number
int rand_int()
{
    int result;
    int min = 1;
    int max = 8;
    return 1 + rand() % 8;
}

int main()
{
    srand((unsigned int)time(NULL));
    int sockfd;
    struct sockaddr_in server_addr;

    char send_buf[BUF_SIZE];
    char recv_buf[BUF_SIZE];
    char name_send[BUF_SIZE];
    char pass_send[BUF_SIZE];

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
    printf("Socket is created!!!\n");

    int enable = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

    int stt = connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (stt == -1)
    {
        perror("Connect");
        exit(0);
    }
    printf("Connected to server via port %d!!!\n", PORT);

    int i, loop = 3;
    for (i = 0; i < 1; i++)
    {
        memset(recv_buf, 0, sizeof(recv_buf));
        read(sockfd, recv_buf, sizeof(recv_buf));
        strcpy(name_send, recv_buf);

        memset(recv_buf, 0, sizeof(recv_buf));
        read(sockfd, recv_buf, sizeof(recv_buf));
        strcpy(pass_send, recv_buf);

        // printf("%s - %s", name_send, pass_send);

        write(sockfd, name_send, strlen(name_send));
        write(sockfd, pass_send, strlen(pass_send));

        memset(recv_buf, 0, sizeof(recv_buf));
        read(sockfd, recv_buf, sizeof(recv_buf));
        printf("Server replied %s\n", recv_buf);
        printf("Login status: Sucessfully!!\n");
        printf("==============================\n");

        // choise
        memset(recv_buf, 0, sizeof(recv_buf));
        read(sockfd, recv_buf, sizeof(recv_buf));
        printf("%s\n", recv_buf);

        // send client choise
        memset(send_buf, 0, sizeof(send_buf));
        int choise = rand_int();
        snprintf(send_buf, sizeof(send_buf), "%d", choise);
        printf("Client choise: %s\n", send_buf);
        write(sockfd, send_buf, strlen(send_buf));

        memset(recv_buf, 0, sizeof(recv_buf));
        read(sockfd, recv_buf, sizeof(recv_buf));

        float point = atof(recv_buf);
        printf("Point of %s is %.2f\n", name_send, point);
        printf("==============================\n");
    }
    close(sockfd);
    return 0;
}