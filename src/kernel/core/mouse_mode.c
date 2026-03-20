#include "mouse_mode.h"

static int mouse_enabled = 0;

int mouse_is_enabled(void)
{
    return mouse_enabled;
}

void mouse_enable(int enable)
{
    mouse_enabled = (enable != 0);
}
