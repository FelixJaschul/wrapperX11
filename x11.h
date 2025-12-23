#include <X11/Xlib.h>
#include <unistd.h>
#include <array>
#include <cstddef>
#include <cstdint>

#ifndef WRAPPER_H
#define WRAPPER_H

struct xWindow
{
    Display *display = XOpenDisplay(nullptr);
    Window window = DefaultScreen(display);
    int screen = 0;
	GC gc = DefaultGC(display, screen);
    int width = 800,
        height = 600;
    int x = 100,
        y = 100;
    const char *title = "CHANGE NAME BY wWindow window = { .title='TITLE', };";
    uint32_t fcolor = 0x000000;
    uint32_t bcolor = 0xFFFFFF;
};

/*
 * WINDOW MANAGEMENT
 */

inline bool xCreateWindow(xWindow &w)
{
    w.window = XCreateSimpleWindow(
        w.display,
        RootWindow(w.display, w.screen),
        w.x, w.y,
        w.width, w.height, 0,
        BlackPixel(w.display, w.screen),
        WhitePixel(w.display, w.screen));

    if (!w.window) return false;

    // Basic setup: title,
    //              input masks,
    //              map the window so it becomes visible

    XStoreName(w.display, w.window, w.title);
    XSelectInput(w.display, w.window, ExposureMask | KeyPressMask | KeyReleaseMask | StructureNotifyMask);

    Atom wmDeleteMessage = XInternAtom(w.display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(w.display, w.window, &wmDeleteMessage, 1);
    XMapWindow(w.display, w.window);
    XFlush(w.display);

    XEvent ev;
	XWindowEvent(w.display, w.window, ExposureMask, &ev);

    return true;
}

inline void xDestroyWindow(xWindow &w)
{
    XDestroyWindow(w.display, w.window);
    XSync(w.display, False);
    w.window = 0;
}

/*
 * DRAWING
 */

inline void xDrawPixel(xWindow &w, int x, int y, uint32_t color)
{
    XSetForeground(w.display, w.gc, color);
    XDrawPoint(w.display, w.window, w.gc, x, y);
    XFlush(w.display);
}

inline void xDrawTriangle(xWindow &w, int x1, int y1, int x2, int y2, int x3, int y3, uint32_t color)
{
    XSetForeground(w.display, w.gc, color);
    XPoint pts[3] = {
        { static_cast<short>(x1), static_cast<short>(y1) },
        { static_cast<short>(x2), static_cast<short>(y2) },
        { static_cast<short>(x3), static_cast<short>(y3) }
    };
    XFillPolygon(w.display, w.window, w.gc, pts, 3, Convex, CoordModeOrigin);
    XFlush(w.display);
}

/*
 * INPUT EVENTS
 */

#include <X11/keysym.h>

enum class xKey : uint16_t
{
    Unknown = 0,

    Escape, Space, Enter,
    Tab, Backspace,

    Left, Right,
    Up, Down,

    A, B, C, D, E,
    F, G, H, I, J,
    K, L, M, N, O,
    P, Q, R, S, T,
    U, V, W, X, Y,
    Z,

    Num0, Num1, Num2,
    Num3, Num4, Num5,
    Num6, Num7, Num8,
    Num9,

    LeftShift, RightShift,
    LeftControl, RightControl,

    LeftAlt, RightAlt,
    LeftSuper, RightSuper,

    F1, F2,
    COUNT
};

constexpr size_t KEY_COUNT = static_cast<size_t>(xKey::COUNT);
static std::array<bool, KEY_COUNT> s_curr{};
static std::array<bool, KEY_COUNT> s_prev{};

void xSetKey(xKey key, bool down)
{
    auto idx = static_cast<size_t>(key);
    if (idx < s_curr.size()) s_curr[idx] = down;
}

bool xPollEvents(Display *display)
{
    static Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
    bool shouldClose = false;

    while (XPending(display) > 0)
    {
        XEvent event;
        XNextEvent(display, &event);

        switch (event.type)
        {
            case ClientMessage:
                if ((Atom)event.xclient.data.l[0] == wmDeleteMessage) shouldClose = true;
                break;

            case KeyPress:
            {
                KeySym sym = XLookupKeysym(&event.xkey, 0);
                auto mapKey = [](KeySym s) -> xKey
				{
                    switch (s)
                    {
                        case XK_Escape: return xKey::Escape;
                        case XK_space: return xKey::Space;
                        case XK_Return: return xKey::Enter;
                        case XK_Tab: return xKey::Tab;
                        case XK_BackSpace: return xKey::Backspace;

                        case XK_Left: return xKey::Left;
                        case XK_Right: return xKey::Right;
                        case XK_Up: return xKey::Up;
                        case XK_Down: return xKey::Down;

                        case XK_a: case XK_A: return xKey::A;
                        case XK_b: case XK_B: return xKey::B;
                        case XK_c: case XK_C: return xKey::C;
                        case XK_d: case XK_D: return xKey::D;
                        case XK_e: case XK_E: return xKey::E;
                        case XK_f: case XK_F: return xKey::F;
                        case XK_g: case XK_G: return xKey::G;
                        case XK_h: case XK_H: return xKey::H;
                        case XK_i: case XK_I: return xKey::I;
                        case XK_j: case XK_J: return xKey::J;
                        case XK_k: case XK_K: return xKey::K;
                        case XK_l: case XK_L: return xKey::L;
                        case XK_m: case XK_M: return xKey::M;
                        case XK_n: case XK_N: return xKey::N;
                        case XK_o: case XK_O: return xKey::O;
                        case XK_p: case XK_P: return xKey::P;
                        case XK_q: case XK_Q: return xKey::Q;
                        case XK_r: case XK_R: return xKey::R;
                        case XK_s: case XK_S: return xKey::S;
                        case XK_t: case XK_T: return xKey::T;
                        case XK_u: case XK_U: return xKey::U;
                        case XK_v: case XK_V: return xKey::V;
                        case XK_w: case XK_W: return xKey::W;
                        case XK_x: case XK_X: return xKey::X;
                        case XK_y: case XK_Y: return xKey::Y;
                        case XK_z: case XK_Z: return xKey::Z;

                        case XK_0: return xKey::Num0;
                        case XK_1: return xKey::Num1;
                        case XK_2: return xKey::Num2;
                        case XK_3: return xKey::Num3;
                        case XK_4: return xKey::Num4;
                        case XK_5: return xKey::Num5;
                        case XK_6: return xKey::Num6;
                        case XK_7: return xKey::Num7;
                        case XK_8: return xKey::Num8;
                        case XK_9: return xKey::Num9;

                        case XK_Shift_L: return xKey::LeftShift;
                        case XK_Shift_R: return xKey::RightShift;
                        case XK_Control_L: return xKey::LeftControl;
                        case XK_Control_R: return xKey::RightControl;
                        case XK_Alt_L: return xKey::LeftAlt;
                        case XK_Alt_R: return xKey::RightAlt;
                        case XK_Super_L: return xKey::LeftSuper;
                        case XK_Super_R: return xKey::RightSuper;

                        case XK_F1: return xKey::F1;
                        case XK_F2: return xKey::F2;

                        default: return xKey::Unknown;
                    }
                };

                xKey k = mapKey(sym);
                if (k != xKey::Unknown) xSetKey(k, true);
                break;
            }

            case KeyRelease:
            {
                KeySym sym = XLookupKeysym(&event.xkey, 0);
                auto mapKey = [](KeySym s) -> xKey
				{
                    switch (s)
                    {
                        case XK_Escape: return xKey::Escape;
                        case XK_space: return xKey::Space;
                        case XK_Return: return xKey::Enter;
                        case XK_Tab: return xKey::Tab;
                        case XK_BackSpace: return xKey::Backspace;

                        case XK_Left: return xKey::Left;
                        case XK_Right: return xKey::Right;
                        case XK_Up: return xKey::Up;
                        case XK_Down: return xKey::Down;

                        case XK_a: case XK_A: return xKey::A;
                        case XK_b: case XK_B: return xKey::B;
                        case XK_c: case XK_C: return xKey::C;
                        case XK_d: case XK_D: return xKey::D;
                        case XK_e: case XK_E: return xKey::E;
                        case XK_f: case XK_F: return xKey::F;
                        case XK_g: case XK_G: return xKey::G;
                        case XK_h: case XK_H: return xKey::H;
                        case XK_i: case XK_I: return xKey::I;
                        case XK_j: case XK_J: return xKey::J;
                        case XK_k: case XK_K: return xKey::K;
                        case XK_l: case XK_L: return xKey::L;
                        case XK_m: case XK_M: return xKey::M;
                        case XK_n: case XK_N: return xKey::N;
                        case XK_o: case XK_O: return xKey::O;
                        case XK_p: case XK_P: return xKey::P;
                        case XK_q: case XK_Q: return xKey::Q;
                        case XK_r: case XK_R: return xKey::R;
                        case XK_s: case XK_S: return xKey::S;
                        case XK_t: case XK_T: return xKey::T;
                        case XK_u: case XK_U: return xKey::U;
                        case XK_v: case XK_V: return xKey::V;
                        case XK_w: case XK_W: return xKey::W;
                        case XK_x: case XK_X: return xKey::X;
                        case XK_y: case XK_Y: return xKey::Y;
                        case XK_z: case XK_Z: return xKey::Z;

                        case XK_0: return xKey::Num0;
                        case XK_1: return xKey::Num1;
                        case XK_2: return xKey::Num2;
                        case XK_3: return xKey::Num3;
                        case XK_4: return xKey::Num4;
                        case XK_5: return xKey::Num5;
                        case XK_6: return xKey::Num6;
                        case XK_7: return xKey::Num7;
                        case XK_8: return xKey::Num8;
                        case XK_9: return xKey::Num9;

                        case XK_Shift_L: return xKey::LeftShift;
                        case XK_Shift_R: return xKey::RightShift;
                        case XK_Control_L: return xKey::LeftControl;
                        case XK_Control_R: return xKey::RightControl;
                        case XK_Alt_L: return xKey::LeftAlt;
                        case XK_Alt_R: return xKey::RightAlt;
                        case XK_Super_L: return xKey::LeftSuper;
                        case XK_Super_R: return xKey::RightSuper;

                        case XK_F1: return xKey::F1;
                        case XK_F2: return xKey::F2;

                        default: return xKey::Unknown;
                    }
                };

                xKey k = mapKey(sym);
                if (k != xKey::Unknown) xSetKey(k, false);
                break;
            }

            default:
                break;
        }
    }
    return shouldClose;
}

bool xIsKeyDown(xKey key)
{
    auto idx = static_cast<size_t>(key);
    return idx < s_curr.size() ? s_curr[idx] : false;
}

bool xIsKeyPressed(xKey key)
{
    auto idx = static_cast<size_t>(key);
    return idx < s_curr.size() ? (s_curr[idx] && !s_prev[idx]) : false;
}

bool xIsKeyReleased(xKey key)
{
    auto idx = static_cast<size_t>(key);
    return idx < s_curr.size() ? (!s_curr[idx] && s_prev[idx]) : false;
}

// Call once per frame after processing events
inline void xUpdateInput()
{
    s_prev = s_curr;
}
#endif // WRAPPER_H
