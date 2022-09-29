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