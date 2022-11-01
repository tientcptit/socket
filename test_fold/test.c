#include <time.h>
#include <stdio.h>
#include <unistd.h>

int main()
{
    int msec = 0, trigger = 10; /* 10ms */
    clock_t before = clock();
    int iterations;
    do
    {
        usleep(500000);

        clock_t difference = clock() - before;
        msec = difference * 1000 / CLOCKS_PER_SEC;
        iterations++;
    } while (msec < trigger);

    printf("Time taken %d seconds %d milliseconds (%d iterations)\n",
           msec / 1000, msec % 1000, iterations);

    return (0);
}