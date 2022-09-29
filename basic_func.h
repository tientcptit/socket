#include <string.h>

char *cell_State(char *cell_barred)
{
    char cell_state[30];
    if (strcmp(cell_barred, "notBarred") == 0)
    {
        strcpy(cell_state, "UE can access this cell");
        return "UE can access this cell";
    }
    else
    {
        strcpy(cell_state, "UE can not access this cell");
        return "UE can not access this cell";
    }
}

char time_buf[20];
void *get_time()
{
    time_t t;
    struct tm *tm_info;
    t = time(NULL);
    tm_info = localtime(&t);
    strftime(time_buf, 20, "%H:%M:%S", tm_info);
    return time_buf;
}
