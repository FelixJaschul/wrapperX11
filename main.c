#include <unistd.h>
#include <stdint.h>
#include <x11.h>

#define XTITLE "X11"
#define XFPS 60

int main()
{
    xWindow win;
    xInit(&win);
    win.title = XTITLE;

    xCreateWindow(&win);
    uint32_t c[] = { 0x602020, 0x206020, 0x202060 };
    size_t   cc  = sizeof(c)/sizeof(c[0]);
    size_t   ccc = 0;
    while (1)
    {   // Event Loop
        if (xPollEvents(win.display)) break;
        if (xIsKeyPressed(Escape)) break;

        xDrawRectangle(&win, 0, 0, win.width, win.height, c[ccc]);

        ccc = (ccc + 1) % cc;
        xUpdateInput(); usleep(3000*XFPS);
    } xDestroyWindow(&win);
    return 0;
}

