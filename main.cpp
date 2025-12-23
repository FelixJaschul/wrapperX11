#include <unistd.h>
#include "x11.h"

int main()
{
    // INIT
    xWindow win 
    {
        .title  = "X11",
        .fcolor = 0xf088F0, 
    };
 
    bool run = true;

    xCreateWindow(win);

    xDrawTriangle(win, 100, 100, 200, 200, 300, 100, 0x0000FF);

    while (true)
    {
        if (xPollEvents(win.display)) break;
        if (xIsKeyPressed(xKey::Escape)) break;

        xUpdateInput();
        usleep(16000);
    }

    xDestroyWindow(win);

    return 0;
}

