// ============================================================================
// x11.h - Core window management
// ============================================================================
#ifndef X11_WRAPPER_H
#define X11_WRAPPER_H

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>

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
    uint32_t *buffer;    // Direct pixel buffer for software rendering
    double fps;          // Target frames per second
    double deltat;       // Delta time of last frame in seconds
    struct timespec lastt;
} xWindow;

// Initialize window structure with default values
/*  -> Example:
 *  xWindow win;
 *  xWindowInit(&win);
 *  win.title = "My Window";
 *  win.width = 1280;
 *  win.height = 720;
 */
void xWindowInit(xWindow *w);

// Create and display X11 window, returns false on failure
/*  -> Example:
 *  if (!xCreateWindow(&win)) return 1;
 */
bool xCreateWindow(xWindow *w);

// Cleanup and destroy window resources
/*  -> Example:
 *  xDestroyWindow(&win);
 */
void xDestroyWindow(xWindow *w);

// Update frame timing (sleeps to maintain target FPS)
/*  -> Example:
 *  while(1) {
 *      ...
 *      xUpdateFrame(&win);
 *  }
 */
void xUpdateFrame(xWindow *w);

// Push pixel buffer to screen
/*  -> Example:
 *  // After drawing to win.buffer
 *  xUpdateFramebuffer(&win);
 */
void xUpdateFramebuffer(const xWindow *w);

// Get current FPS based on actual frame time
/*  -> Example:
 *  printf("FPS: %.2f\n", xGetFPS(&win));
 */
double xGetFPS(const xWindow *w);

// Draw single pixel at (x,y) with bounds checking
/*  -> Example:
 *  xDrawPixel(&win, 100, 100, 0xFF0000); // Draw red pixel
 */
void xDrawPixel(const xWindow *w, int x, int y, uint32_t color);

#ifdef __cplusplus
}
#endif

#ifdef XKEYS_IMPLEMENTATION
#include "xKeys.h"
#endif

#ifdef XMATH_IMPLEMENTATION
#include "xMath.h"
#endif

#ifdef XUTIL_IMPLEMENTATION
#include "xUtil.h"
#endif

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
    if (!w->display)
    {
        fprintf(stderr, "Failed to open X11 display\n");
        return false;
    }

    w->window = XCreateSimpleWindow(
        w->display, RootWindow(w->display, w->screen),
        w->x, w->y, w->width, w->height, 0,
        BlackPixel(w->display, w->screen),
        WhitePixel(w->display, w->screen)
    );

    if (!w->window)
    {
        fprintf(stderr, "Failed to create X11 window\n");
        return false;
    }

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
    assert(w->buffer && "Failed to allocate framebuffer");

    w->image = XCreateImage(
        w->display,
        DefaultVisual(w->display, w->screen),
        DefaultDepth(w->display, w->screen),
        ZPixmap, 0, (char*) w->buffer,
        w->width, w->height, 32, 0
    );
    assert(w->image && "Failed to create XImage");

    clock_gettime(CLOCK_MONOTONIC, &w->lastt);
    return true;
}

inline void xDestroyWindow(xWindow *w)
{
    if (!w->display) return;

    if (w->image)
    {
        XDestroyImage(w->image);
        w->image = NULL;
        w->buffer = NULL;
    }

    if (w->window)
    {
        XDestroyWindow(w->display, w->window);
        w->window = 0;
    }

    XSync(w->display, False);
    XCloseDisplay(w->display);
    w->display = NULL;
}

inline void xUpdateFrame(xWindow *w)
{
    struct timespec current_time;
    clock_gettime(CLOCK_MONOTONIC, &current_time);

    double elapsed = (current_time.tv_sec - w->lastt.tv_sec) +
                     (current_time.tv_nsec - w->lastt.tv_nsec) / 1e9;

    const double target_frame_time = 1.0 / w->fps;
    if (elapsed < target_frame_time) {
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
    w->lastt  = current_time;
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