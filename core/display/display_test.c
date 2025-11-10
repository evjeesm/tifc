#include "display.h"
#include "input.h"
#include <termio.h>
#include <unistd.h>

void resize_hook(const display_t *const display, void *data)
{
    (void) display;
    (void) data;
}

int main(void)
{
    resize_hook_with_data_t hook = {.hook = resize_hook};
    display_t display;
    input_enable_mouse();
    display_set_resize_handler(&display, hook);
    while (1)
    {
        display_clear(&display);
        for (unsigned int i = 0; i < display.size.y; ++i)
        {
            for (unsigned int j = 0; j < display.size.x; ++j)
            {
                display_set_char(&display, U'=', (disp_pos_t){j, i});
            }
        }
        usleep(100);
        display_render(&display);
    }
    input_disable_mouse();
    return 0;
}
