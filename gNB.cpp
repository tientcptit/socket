#include <iostream>

#include "message.h"
#include "myheader.h"

using namespace std;

void *dis_time()
{
    time_t timer;
    struct tm *tm_info;
    timer = time(NULL);
    tm_info = localtime(&timer);
    strftime(time_buf, 26, "%d-%m-%Y %H:%M:%S", tm_info);
    return time_buf;
}

int main()
{
    MIB mib;
    SIB1 sib1;

    int listenfd, connfd;
    struct sockaddr_in server_addr;
    char send_buf[BUF_SIZE];
    char recv_buf[BUF_SIZE];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        perror("Socket");
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);
    char *get_time = dis_time();
    cout << "Socket is created at " << get_time << endl;

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
    cout << "Listening for incoming connections" << endl;
    cout << "==============================" << endl;

    /*struct timeval begin, end;
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
    return 0;
}
