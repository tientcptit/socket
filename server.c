#include "myheader.h"

int main()
{
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    char send_buf[BUF_SIZE];
    char recv_buf[BUF_SIZE];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    return 0;
}