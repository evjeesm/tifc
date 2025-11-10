#include "border.h"
#include <wchar.h>

int main(void)
{
    border_set_t bs;

    bs.top_left   = U'A';
    bs.top_right  = U'B';
    bs.bot_right  = U'C';
    bs.bot_left   = U'D';
    bs.vertical   = U'E';
    bs.horizontal = U'F';

    for (size_t i = 0; i < BORDER_SET_SIZE; ++i)
        wprintf(L"%c\n", bs._[i]);
}
