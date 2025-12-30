// ============================================================================
// x11.h - Core window management 
// ============================================================================
#ifndef X11_WRAPPER_H
#define X11_WRAPPER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xWindow {
    Window window;
    Display *display;
    XImage *image;
    int screen;
    GC gc;
    int width;
    int height;
    int x, y;
    const char *title;
    uint32_t *buffer;
    double fps;
    double deltat;
    struct timespec lastt;
} xWindow;

void   xWindowInit(xWindow *w);
bool   xCreateWindow(xWindow *w);
void   xDestroyWindow(xWindow *w);
void   xUpdateFrame(xWindow *w);
void   xUpdateFramebuffer(const xWindow *w);
double xGetFPS(const xWindow *w);
void   xDrawPixel(const xWindow *w, int x, int y, uint32_t color);

#ifdef __cplusplus
}
#endif

#ifdef XKEYS_IMPLEMENTATION
#include <xKeys.h>
#endif // XKEYS_IMPLEMENTATION
#ifdef XMATH_IMPLEMENTATION
#include <xMath.h>
#endif // XMATH_IMPLEMENTATION
#ifdef XUTIL_IMPLEMENTATION
#include <xUtil.h>
#endif // XUTIL_IMPLEMENTATION

inline void xWindowInit(xWindow *w)
{
    w->display = XOpenDisplay(NULL);
    w->screen = 0;
    w->window = 0;
    w->gc = 0;
    w->width = 800;
    w->height = 600;
    w->x = 100;
    w->y = 100;
    w->title = "X11 Window";
    w->image = NULL;
    w->buffer = NULL;
    w->fps = 60.0;
    w->deltat = 0.0;
    clock_gettime(CLOCK_MONOTONIC, &w->lastt);
}

inline bool xCreateWindow(xWindow *w)
{
    if (!w->display) return false;

    w->window = XCreateSimpleWindow(
        w->display, RootWindow(w->display, w->screen),
        w->x, w->y, w->width, w->height, 0,
        BlackPixel(w->display, w->screen),
        WhitePixel(w->display, w->screen)
    );

    if (!w->window) return false;

    XStoreName(w->display, w->window, w->title);
    XSelectInput(w->display, w->window, 
                 ExposureMask | KeyPressMask | KeyReleaseMask | 
                 StructureNotifyMask | PointerMotionMask | 
                 ButtonPressMask | ButtonReleaseMask);

    Atom wmDeleteMessage = XInternAtom(w->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(w->display, w->window, &wmDeleteMessage, 1);
    XMapWindow(w->display, w->window);
    XFlush(w->display);

    XEvent ev;
    XWindowEvent(w->display, w->window, ExposureMask, &ev);
    w->gc = DefaultGC(w->display, w->screen);

    w->buffer = (uint32_t *) malloc(w->width * w->height * sizeof(uint32_t));
    w->image = XCreateImage(
        w->display, 
        DefaultVisual(w->display, w->screen),
        DefaultDepth(w->display, w->screen),
        ZPixmap, 
        0, 
        (char*) w->buffer,
        w->width, 
        w->height, 32, 0
    );

    clock_gettime(CLOCK_MONOTONIC, &w->lastt);
    return true;
}

inline void xDestroyWindow(xWindow *w)
{
    if (!w->display || !w->window) return;
    if (w->image) XDestroyImage(w->image);
    XDestroyWindow(w->display, w->window);
    XSync(w->display, False);
    w->window = 0;
}

inline void xUpdateFrame(xWindow *w)
{
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    double elapsed = (current_time.tv_sec - w->lastt.tv_sec) +
                     (current_time.tv_nsec - w->lastt.tv_nsec) / 1e9;

    const double target_frame_time = 1.0 / w->fps;
    if (elapsed < target_frame_time) 
    {
        const double sleep_time = target_frame_time - elapsed;
        struct timespec sleep_spec;
        sleep_spec.tv_sec = (time_t)sleep_time;
        sleep_spec.tv_nsec = (long)((sleep_time - sleep_spec.tv_sec) * 1e9);
        nanosleep(&sleep_spec, NULL);

        clock_gettime(CLOCK_MONOTONIC, &current_time);
        elapsed = (current_time.tv_sec - w->lastt.tv_sec) +
                  (current_time.tv_nsec - w->lastt.tv_nsec) / 1e9;
    }

    w->deltat = elapsed;
    w->lastt = current_time;
}

inline void xUpdateFramebuffer(const xWindow *w)
{
    XPutImage(w->display, w->window, w->gc, w->image, 0, 0, 0, 0, w->width, w->height);
    XFlush(w->display);
}

inline double xGetFPS(const xWindow *w)
{
    return (w->deltat > 0.0) ? (1.0 / w->deltat) : 0.0;
}

inline void xDrawPixel(const xWindow *w, int x, int y, uint32_t color)
{
    if (x >= 0 && x < w->width && y >= 0 && y < w->height) w->buffer[y * w->width + x] = color;
}

#endif // X11_WRAPPER_H
