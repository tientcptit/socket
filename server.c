#include "myheader.h"

#define MAX_STUDENT 10
#define MAX_LEN 100

typedef struct students
{
    char username[MAX_LEN];
    char password[MAX_LEN];
    float maths;
    float physics;
    float chemistry;
    float biology;
    float history;
    float geography;
    float literature;
    float english;
} students;

students student[MAX_STUDENT];

// Struct to pass parameters to the thread
typedef struct pthread_arg_t
{
    int connfd;
    struct sockaddr_in client_addr;
} pthread_arg_t;

void *thread_func(void *arg);

// Generate a random string to assign to the name and password
void rand_text(int length, char *result)
{

    int i, rand_int;
    char char_set[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    for (i = 0; i < length; i++)
    {
        result[i] = char_set[rand() % sizeof(char_set)];
    }
    result[length] = 0;
}

// Initialize data
void data_init()
{

    int uname_len = 10;
    int pass_len = 20;
    for (int i = 0; i < MAX_STUDENT; i++)
    {

        char name_result[uname_len + 1];
        char pass_result[pass_len + 1];

        rand_text(uname_len, name_result);
        rand_text(pass_len, pass_result);

        strcpy(student[i].username, name_result);
        strcpy(student[i].password, pass_result);

        student[i].maths = 5 + (float)i / 4;
        student[i].physics = 6 + (float)i / 4;
        student[i].chemistry = 7 + (float)i / 4;
        student[i].biology = 8 + (float)i / 4;
        student[i].history = 9 + (float)i / 4;
        student[i].geography = 7 + (float)i / 4;
        student[i].literature = 8 + (float)i / 4;
        student[i].english = 9 + (float)i / 4;
    }
}

// random number
int rand_int()
{
    int result;
    int min = 1;
    int max = 8;
    return 1 + rand() % 8;
}

int user_auth(char *username_recv, char *password_recv, int *loc)
{

    int i;
    int auth_result = 0;
    for (i = 0; i < MAX_STUDENT; i++)
    {
        if ((strcmp(student[i].username, username_recv) == 0) && (strcmp(student[i].password, password_recv) == 0))
        {
            auth_result = 1;
            *loc = i;
            break;
        }
    }
    return auth_result;
}
int main()
{
    data_init();
    srand((unsigned int)time(NULL));
    int listenfd, connfd;
    struct sockaddr_in server_addr, client_addr;
    char send_buf[BUF_SIZE];
    char recv_buf[BUF_SIZE];
    socklen_t addr_len;
    int bytes_recv;
    int bytes_send;
    char name_recv[MAX_LEN];
    char pass_recv[MAX_LEN];
    int loc;
    char buffer[MAX_LEN];

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

    int enable = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));

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
    printf("Listening connection!\n");
    printf("==============================\n");

    int max = 3;
    int j = 0;
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
        for (i = 0; i < 1; i++)
        {
            // Sent username and pass to client
            int rand_value = rand_int();
            memset(send_buf, 0, sizeof(send_buf));
            strcpy(send_buf, student[rand_value].username);
            // printf("%s\n", send_buf);
            bytes_send = write(connfd, send_buf, strlen(send_buf));
            if (bytes_send < 0)
            {
                printf(RED "Sent username to [UID=%d] failed...\n" RESET, cl_port);
            }
            memset(send_buf, 0, sizeof(send_buf));
            strcpy(send_buf, student[rand_value].password);
            // printf("%s\n", send_buf);
            bytes_send = write(connfd, send_buf, strlen(send_buf));
            if (bytes_send < 0)
            {
                printf(RED "Sent password to [UID=%d] failed...\n" RESET, cl_port);
            }
            printf("Sent username and password to [UID=%d] sucessfully!\n", cl_port);
            printf("==============================\n");

            // receive user name and pass from cliet and author
            memset(name_recv, 0, sizeof(name_recv));
            memset(pass_recv, 0, sizeof(pass_recv));
            read(connfd, name_recv, sizeof(name_recv));
            read(connfd, pass_recv, sizeof(pass_recv));

            if (name_recv[strlen(name_recv) - 1] == '\n')
            {
                name_recv[strlen(name_recv) - 1] = '\0';
                pass_recv[strlen(pass_recv) - 1] = '\0';
            }
            printf("[UID=%d]: Username received: %s\n", cl_port, name_recv);
            printf("[UID=%d]: Password received: %s\n", cl_port, pass_recv);

            if (user_auth(name_recv, pass_recv, &loc) == 1)
            {
                // login successfully and send status to client
                printf("[UID=%d]: Logged in successfully!\n", cl_port);
                memset(send_buf, 0, sizeof(send_buf));
                strcpy(send_buf, "ok");
                write(connfd, send_buf, strlen(send_buf));

                // send choise to client
                char success_msg[125] = "Enter your option:\n 1. Maths\n 2. Physics\n 3. Chemistry\n 4. Biology\n 5. History\n 6. Geography\n 7. English\n 8. Literature\n";
                printf("==============================\n");
                write(connfd, success_msg, strlen(success_msg));

                // recv client choise
                memset(recv_buf, 0, sizeof(recv_buf));
                read(connfd, recv_buf, sizeof(connfd));

                // detect client choise and send request
                if (strcmp(recv_buf, "1") == 0)
                {
                    printf("[UID=%d]: Requested math point of %s\n", cl_port, student[loc].username);

                    memset(send_buf, 0, sizeof(send_buf));
                    gcvt(student[loc].maths, 4, send_buf);
                    write(connfd, send_buf, strlen(send_buf));
                    printf("[UID=%d]: Sent data successfully!\n", cl_port);
                    printf("==============================\n");
                }
                else if (strcmp(recv_buf, "2") == 0)
                {
                    printf("[UID=%d]: Requested physics point of %s\n", cl_port, student[loc].username);

                    memset(send_buf, 0, sizeof(send_buf));
                    gcvt(student[loc].physics, 4, send_buf);
                    write(connfd, send_buf, strlen(send_buf));
                    printf("[UID=%d]: Sent data successfully!\n", cl_port);
                    printf("==============================\n");
                }
                else if (strcmp(recv_buf, "3") == 0)
                {
                    printf("[UID=%d]: Requested chemistry point of %s\n", cl_port, student[loc].username);

                    memset(send_buf, 0, sizeof(send_buf));
                    gcvt(student[loc].chemistry, 4, send_buf);
                    write(connfd, send_buf, strlen(send_buf));
                    printf("[UID=%d]: Sent data successfully!\n", cl_port);
                    printf("==============================\n");
                }
                else if (strcmp(recv_buf, "4") == 0)
                {
                    printf("[UID=%d]: Requested biology point of %s\n", cl_port, student[loc].username);

                    memset(send_buf, 0, sizeof(send_buf));
                    gcvt(student[loc].biology, 4, send_buf);
                    write(connfd, send_buf, strlen(send_buf));
                    printf("[UID=%d]: Sent data successfully!\n", cl_port);
                    printf("==============================\n");
                }
                else if (strcmp(recv_buf, "5") == 0)
                {
                    printf("[UID=%d]: Requested history point of %s\n", cl_port, student[loc].username);

                    memset(send_buf, 0, sizeof(send_buf));
                    gcvt(student[loc].history, 4, send_buf);
                    write(connfd, send_buf, strlen(send_buf));
                    printf("[UID=%d]: Sent data successfully!\n", cl_port);
                    printf("==============================\n");
                }
                else if (strcmp(recv_buf, "6") == 0)
                {
                    printf("[UID=%d]: Requested geography point of %s\n", cl_port, student[loc].username);

                    memset(send_buf, 0, sizeof(send_buf));
                    gcvt(student[loc].geography, 4, send_buf);
                    write(connfd, send_buf, strlen(send_buf));
                    printf("[UID=%d]: Sent data successfully!\n", cl_port);
                    printf("==============================\n");
                }
                else if (strcmp(recv_buf, "7") == 0)
                {
                    printf("[UID=%d]: Requested english point of %s\n", cl_port, student[loc].username);

                    memset(send_buf, 0, sizeof(send_buf));
                    gcvt(student[loc].english, 4, send_buf);
                    write(connfd, send_buf, strlen(send_buf));
                    printf("[UID=%d]: Sent data successfully!\n", cl_port);
                    printf("==============================\n");
                }
                else if (strcmp(recv_buf, "8") == 0)
                {
                    printf("[UID=%d]: Requested literature point of %s\n", cl_port, student[loc].username);

                    memset(send_buf, 0, sizeof(send_buf));
                    gcvt(student[loc].literature, 4, send_buf);
                    write(connfd, send_buf, strlen(send_buf));
                    printf("[UID=%d]: Sent data successfully!\n", cl_port);
                    printf("==============================\n");
                }
                else
                {
                    printf("[UID=%d]: Something was wrong...\n", cl_port);
                    printf("==============================\n");
                }
                break;
            }

            // login failed
            else
            {
                printf("[UID=%d]: Login failed...\n", cl_port);
                memset(send_buf, 0, sizeof(send_buf));
                strcpy(send_buf, "!ok");
                send(connfd, send_buf, strlen(send_buf), 0);
            }
        }
        j++;
        if (j == max)
        {
            break;
        }
    }

    close(connfd);
    close(listenfd);

    return 0;
}