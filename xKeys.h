// ============================================================================
// xKeys.h - Keyboard and Mouse Input
// ============================================================================
#ifndef XKEYS_H
#define XKEYS_H

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    KEY_UNKNOWN = 0,

    KEY_ESCAPE, KEY_SPACE,
    KEY_ENTER,  KEY_TAB, KEY_BACKSPACE,

    KEY_LEFT, KEY_RIGHT,
    KEY_UP,   KEY_DOWN,

    KEY_A, KEY_B, KEY_C,
    KEY_D, KEY_E, KEY_F,
    KEY_G, KEY_H, KEY_I,
    KEY_J, KEY_K, KEY_L,
    KEY_M, KEY_N, KEY_O,
    KEY_P, KEY_Q, KEY_R,
    KEY_S, KEY_T, KEY_U,
    KEY_V, KEY_W, KEY_X,

    KEY_Y, KEY_Z,

    KEY_NUM0, KEY_NUM1, KEY_NUM2,
    KEY_NUM3, KEY_NUM4, KEY_NUM5,
    KEY_NUM6, KEY_NUM7, KEY_NUM8,
    KEY_NUM9,

    KEY_LSHIFT, KEY_RSHIFT,
    KEY_LCTRL,  KEY_RCTRL,
    KEY_LALT,   KEY_RALT,

    KEY_F1, KEY_F2,
    KEY_F3, KEY_F4,

    KEY_COUNT
} Key;

typedef enum {
    MOUSE_LEFT = 0,
    MOUSE_MIDDLE,
    MOUSE_RIGHT,
    MOUSE_COUNT
} MouseButton;

// Input state structure - must be initialized and passed to all functions
typedef struct xInputState {
    bool key_curr[KEY_COUNT];
    bool key_prev[KEY_COUNT];
    bool mouse_curr[MOUSE_COUNT];
    bool mouse_prev[MOUSE_COUNT];
    int mouse_x, mouse_y;
    int mouse_dx, mouse_dy;
    int last_x, last_y;
    bool mouse_grabbed;
    Window grab_window;
    int center_x, center_y;
} xInput;

// Initialize input state to zero
/*  -> Example:
 *  xInputState input;
 *  xInputInit(&input);
 */
void xInputInit(xInput *input);

// Poll X11 events and update input state, returns true if window should close
/*  -> Example:
 *  if (xPollEvents(win.display, &input)) break;
 */
bool xPollEvents(Display *display, xInput *input);

// Copy current frame state to previous (call once per frame after processing)
/*  -> Example:
 *  while(1) {
 *      ...
 *      xUpdateInput(&input);
 *  }
 */
void xUpdateInput(xInput *input);

// Check if key is currently held down
/*  -> Example:
 *  if (xIsKeyDown(&input, KEY_W)) // Move forward
 */
bool xIsKeyDown(const xInput *input, Key key);

// Check if key was just pressed this frame
/*  -> Example:
 *  if (xIsKeyPressed(&input, KEY_SPACE)) // Jump
 */
bool xIsKeyPressed(const xInput *input, Key key);

// Check if key was just released this frame
/*  -> Example:
 *  if (xIsKeyReleased(&input, KEY_E)) // Use item
 */
bool xIsKeyReleased(const xInput *input, Key key);

// Check if mouse button is currently held down
/*  -> Example:
 *  if (xIsMouseDown(&input, MOUSE_LEFT)) // Shoot
 */
bool xIsMouseDown(const xInput *input, MouseButton btn);

// Check if mouse button was just pressed this frame
bool xIsMousePressed(const xInput *input, MouseButton btn);

// Check if mouse button was just released this frame
bool xIsMouseReleased(const xInput *input, MouseButton btn);

// Get current mouse position in window coordinates
/*  -> Example:
 *  int mx, my;
 *  xGetMousePosition(&input, &mx, &my);
 */
void xGetMousePosition(const xInput *input, int *x, int *y);

// Get mouse delta since last frame (for FPS camera controls)
/*  -> Example:
 *  int dx, dy;
 *  xGetMouseDelta(&input, &dx, &dy);
 */
void xGetMouseDelta(const xInput *input, int *dx, int *dy);

// Grab mouse cursor (hides and locks to window center)
/*  -> Example:
 *  xGrabMouse(win.display, win.window, win.width, win.height, &input);
 */
void xGrabMouse(Display *display, Window window, int width, int height, xInput *input);

// Release grabbed mouse cursor
/*  -> Example:
 *  xReleaseMouse(win.display, win.window, &input);
 */
void xReleaseMouse(Display *display, Window window, xInput *input);

// Check if mouse is currently grabbed
bool xIsMouseGrabbed(const xInput *input);

#ifdef __cplusplus
}
#endif

#ifdef XKEYS_IMPLEMENTATION

static void set_key(xInput *input, const Key key, const bool down)
{
    if (key < KEY_COUNT) input->key_curr[key] = down;
}

static void set_mouse(xInput *input, const MouseButton btn, const bool down)
{
    if (btn < MOUSE_COUNT) input->mouse_curr[btn] = down;
}

inline void xInputInit(xInput *input)
{
    for (int i = 0; i < KEY_COUNT; i++) {
        input->key_curr[i] = false;
        input->key_prev[i] = false;
    }
    for (int i = 0; i < MOUSE_COUNT; i++) {
        input->mouse_curr[i] = false;
        input->mouse_prev[i] = false;
    }
    input->mouse_x = 0;
    input->mouse_y = 0;
    input->mouse_dx = 0;
    input->mouse_dy = 0;
    input->last_x = 0;
    input->last_y = 0;
    input->mouse_grabbed = false;
    input->grab_window = 0;
    input->center_x = 400;
    input->center_y = 300;
}

inline bool xPollEvents(Display *display, xInput *input)
{
    const Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
    bool shouldClose = false;

    input->mouse_dx = 0;
    input->mouse_dy = 0;

    while (XPending(display) > 0)
    {
        XEvent event;
        XNextEvent(display, &event);

        switch (event.type)
        {
            case ClientMessage:
                if ((Atom)event.xclient.data.l[0] == wmDeleteMessage) shouldClose = true;
                break;

            case MotionNotify:
                if (input->mouse_grabbed)
                {
                    input->mouse_dx += event.xmotion.x - input->last_x;
                    input->mouse_dy += event.xmotion.y - input->last_y;
                }
                input->mouse_x = event.xmotion.x;
                input->mouse_y = event.xmotion.y;
                input->last_x = input->mouse_x;
                input->last_y = input->mouse_y;
                break;

            case ButtonPress:
            case ButtonRelease:
            {
                const bool down = (event.type == ButtonPress);
                switch (event.xbutton.button)
                {
                    case Button1: set_mouse(input, MOUSE_LEFT, down); break;
                    case Button2: set_mouse(input, MOUSE_MIDDLE, down); break;
                    case Button3: set_mouse(input, MOUSE_RIGHT, down); break;
                    default: break;
                }
                break;
            }

            case KeyPress:
            case KeyRelease:
            {
                const bool down = (event.type == KeyPress);
                const KeySym sym = XLookupKeysym(&event.xkey, 0);
                Key k = KEY_UNKNOWN;

                switch (sym)
                {
                    case XK_Escape: k = KEY_ESCAPE; break;
                    case XK_space: k = KEY_SPACE; break;
                    case XK_Return: k = KEY_ENTER; break;
                    case XK_Tab: k = KEY_TAB; break;
                    case XK_BackSpace: k = KEY_BACKSPACE; break;
                    case XK_Left: k = KEY_LEFT; break;
                    case XK_Right: k = KEY_RIGHT; break;
                    case XK_Up: k = KEY_UP; break;
                    case XK_Down: k = KEY_DOWN; break;
                    case XK_a: case XK_A: k = KEY_A; break;
                    case XK_b: case XK_B: k = KEY_B; break;
                    case XK_c: case XK_C: k = KEY_C; break;
                    case XK_d: case XK_D: k = KEY_D; break;
                    case XK_e: case XK_E: k = KEY_E; break;
                    case XK_f: case XK_F: k = KEY_F; break;
                    case XK_g: case XK_G: k = KEY_G; break;
                    case XK_h: case XK_H: k = KEY_H; break;
                    case XK_i: case XK_I: k = KEY_I; break;
                    case XK_j: case XK_J: k = KEY_J; break;
                    case XK_k: case XK_K: k = KEY_K; break;
                    case XK_l: case XK_L: k = KEY_L; break;
                    case XK_m: case XK_M: k = KEY_M; break;
                    case XK_n: case XK_N: k = KEY_N; break;
                    case XK_o: case XK_O: k = KEY_O; break;
                    case XK_p: case XK_P: k = KEY_P; break;
                    case XK_q: case XK_Q: k = KEY_Q; break;
                    case XK_r: case XK_R: k = KEY_R; break;
                    case XK_s: case XK_S: k = KEY_S; break;
                    case XK_t: case XK_T: k = KEY_T; break;
                    case XK_u: case XK_U: k = KEY_U; break;
                    case XK_v: case XK_V: k = KEY_V; break;
                    case XK_w: case XK_W: k = KEY_W; break;
                    case XK_x: case XK_X: k = KEY_X; break;
                    case XK_y: case XK_Y: k = KEY_Y; break;
                    case XK_z: case XK_Z: k = KEY_Z; break;
                    case XK_0: k = KEY_NUM0; break;
                    case XK_1: k = KEY_NUM1; break;
                    case XK_2: k = KEY_NUM2; break;
                    case XK_3: k = KEY_NUM3; break;
                    case XK_4: k = KEY_NUM4; break;
                    case XK_5: k = KEY_NUM5; break;
                    case XK_6: k = KEY_NUM6; break;
                    case XK_7: k = KEY_NUM7; break;
                    case XK_8: k = KEY_NUM8; break;
                    case XK_9: k = KEY_NUM9; break;
                    case XK_Shift_L: k = KEY_LSHIFT; break;
                    case XK_Shift_R: k = KEY_RSHIFT; break;
                    case XK_Control_L: k = KEY_LCTRL; break;
                    case XK_Control_R: k = KEY_RCTRL; break;
                    case XK_Alt_L: k = KEY_LALT; break;
                    case XK_Alt_R: k = KEY_RALT; break;
                    case XK_F1: k = KEY_F1; break;
                    case XK_F2: k = KEY_F2; break;
                    case XK_F3: k = KEY_F3; break;
                    case XK_F4: k = KEY_F4; break;
                    default: break;
                }

                if (k != KEY_UNKNOWN) set_key(input, k, down);
                break;
            }

            default: break;
        }
    }

    // Recenter mouse if grabbed and moved
    if (input->mouse_grabbed && (input->mouse_dx != 0 || input->mouse_dy != 0))
    {
        XWarpPointer(display, None, input->grab_window, 0, 0, 0, 0, input->center_x, input->center_y);
        XFlush(display);
        input->last_x = input->center_x;
        input->last_y = input->center_y;
    }

    return shouldClose;
}

inline void xUpdateInput(xInput *input)
{
    for (int i = 0; i < KEY_COUNT; i++) input->key_prev[i] = input->key_curr[i];
    for (int i = 0; i < MOUSE_COUNT; i++) input->mouse_prev[i] = input->mouse_curr[i];
}

inline bool xIsKeyDown(const xInput *input, const Key key)
{
    return key < KEY_COUNT ? input->key_curr[key] : false;
}

inline bool xIsKeyPressed(const xInput *input, const Key key)
{
    return key < KEY_COUNT ? (input->key_curr[key] && !input->key_prev[key]) : false;
}

inline bool xIsKeyReleased(const xInput *input, const Key key)
{
    return key < KEY_COUNT ? (!input->key_curr[key] && input->key_prev[key]) : false;
}

inline bool xIsMouseDown(const xInput *input, const MouseButton btn)
{
    return btn < MOUSE_COUNT ? input->mouse_curr[btn] : false;
}

inline bool xIsMousePressed(const xInput *input, const MouseButton btn)
{
    return btn < MOUSE_COUNT ? (input->mouse_curr[btn] && !input->mouse_prev[btn]) : false;
}

inline bool xIsMouseReleased(const xInput *input, const MouseButton btn)
{
    return btn < MOUSE_COUNT ? (!input->mouse_curr[btn] && input->mouse_prev[btn]) : false;
}

inline void xGetMousePosition(const xInput *input, int *x, int *y)
{
    if (x) *x = input->mouse_x;
    if (y) *y = input->mouse_y;
}

inline void xGetMouseDelta(const xInput *input, int *dx, int *dy)
{
    if (dx) *dx = input->mouse_dx;
    if (dy) *dy = input->mouse_dy;
}

inline void xGrabMouse(Display *display, const Window window, const int width, const int height, xInput *input)
{
    if (input->mouse_grabbed) return;

    input->center_x = width / 2;
    input->center_y = height / 2;
    input->grab_window = window;

    XColor dummy;
    const char data[1] = {0};
    const Pixmap blank = XCreateBitmapFromData(display, window, data, 1, 1);
    const Cursor cursor = XCreatePixmapCursor(display, blank, blank, &dummy, &dummy, 0, 0);
    XFreePixmap(display, blank);

    XDefineCursor(display, window, cursor);
    XGrabPointer(display, window, True,
                 PointerMotionMask | ButtonPressMask | ButtonReleaseMask,
                 GrabModeAsync, GrabModeAsync, window, None, CurrentTime);

    XWarpPointer(display, None, window, 0, 0, 0, 0, input->center_x, input->center_y);
    XFlush(display);

    input->last_x = input->center_x;
    input->last_y = input->center_y;
    input->mouse_grabbed = true;
    input->mouse_dx = 0;
    input->mouse_dy = 0;
}

inline void xReleaseMouse(Display *display, const Window window, xInput *input)
{
    if (!input->mouse_grabbed) return;

    XUngrabPointer(display, CurrentTime);
    XDefineCursor(display, window, None);
    input->mouse_grabbed = false;
    input->mouse_dx = 0;
    input->mouse_dy = 0;
}

inline bool xIsMouseGrabbed(const xInput *input)
{
    return input->mouse_grabbed;
}

#endif // XKEYS_IMPLEMENTATION
#endif // XKEYS_H