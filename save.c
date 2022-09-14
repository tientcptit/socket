#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define YEL "\x1B[33m"
#define BLU "\x1B[34m"
#define MAG "\x1B[35m"
#define CYN "\x1B[36m"
#define WHT "\x1B[37m"
#define RESET "\x1B[0m"

char buffer[26];

void *get_time()
{
    time_t timer;
    struct tm *tm_info;
    timer = time(NULL);
    tm_info = localtime(&timer);
    strftime(buffer, 26, "%d-%m-%Y %H:%M:%S", tm_info);
    return buffer;
}

int main()
{
    char *tim = get_time();
    char buf[10] = "Edited at ";
    char buf1[100] = "git commit -m ";
    char c = '"';

    strncat(buf1, &c, 1);
    strcat(buf1, buf);
    strcat(buf1, tim);
    strncat(buf1, &c, 1);

    system("git add .");
    system(buf1);
    system("git push");
    system("clear");

    char pad[50] = "Pushed to Github at ";
    printf(RED "%s\n" RESET, pad);
    printf(YEL "%s\n" RESET, tim);
}