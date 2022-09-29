#include "ue_rar_message.h"

int main()
{
    struct MSG1 msg1;
    printf("%s %d\n", msg1.rach_Preamble, msg1.ra_RNTI);
    return 0;
}