#include "myheader.h"

int main()
{
    time_t t = time(NULL) + 10;
    int i = 0;
    while (1)
    {
        if (time(NULL) >= t)
        {
            break;
        }
        printf("%d\n", i);
        i++;
    }
}
