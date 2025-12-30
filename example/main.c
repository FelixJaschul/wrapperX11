// Minimal example showing FPS camera with mouse control
#include <math.h>

#define XKEYS_IMPLEMENTATION
#define XMATH_IMPLEMENTATION
#define XUTIL_IMPLEMENTATION

#include "../x11.h"

#define SKY_COLOR 0x000000

// Simple checkerboard ground plane raytracer
static inline uint32_t raytrace_ground(const Vec3 origin, const Vec3 direction)
{
    if (fabsf(direction.y) < 0.001f) return SKY_COLOR; // Sky
    
    const float t = -origin.y / direction.y;
    if (t < 0.0f) return SKY_COLOR; // Sky
    
    const Vec3 hit = add(origin, mul(direction, t));
    const int checker = ((int)floorf(hit.x) + (int)floorf(hit.z)) & 1;
    
    const float fade = fmaxf(0.0f, 1.0f - t / 40.0f);
    const uint32_t base = checker ? 0x404040 : 0x909090;
    
    return ((int)(((base >> 16) & 0xFF) * fade) << 16) |
           ((int)(((base >>  8) & 0xFF) * fade) <<  8) |
           ((int)(((base      ) & 0xFF) * fade));
}

int main(void)
{
    xWindow win;
    xWindowInit(&win);
    win.title = "FPS Camera - Click to grab mouse, SPACE to release";
    win.fps = 60;

    xCamera camera;
    xCameraInit(&camera);
    camera.position = vec3(0.0f, 2.0f, 0.0f);

    xCreateWindow(&win);
    
    const float viewport_height = 2.0f;
    const float viewport_width = (float)win.width / win.height * viewport_height;
    
    bool mouse_grabbed = false;

    while (1)
    {
        if (xPollEvents(win.display)) break;
        if (xIsKeyPressed(KEY_ESCAPE)) break;
        
        // Toggle mouse grab
        if (xIsMousePressed(MOUSE_LEFT) && !mouse_grabbed)
        {
            xGrabMouse(win.display, win.window, win.width, win.height);
            mouse_grabbed = true;
        }
        
        if (xIsKeyPressed(KEY_SPACE) && mouse_grabbed)
        {
            xReleaseMouse(win.display, win.window);
            mouse_grabbed = false;
        }
        
        // Mouse look (only when grabbed)
        if (mouse_grabbed)
        {
            const float sensitivity_v = 0.15f;
            const float sensitivity_h = 0.30f;

            int dx, dy;
            xGetMouseDelta(&dx, &dy);
            if (dx != 0 || dy != 0) xCameraRotate(&camera, dx * sensitivity_h, -dy * sensitivity_v);
        }
        
        // Keyboard movement
        const float speed = 0.05f;
        if (xIsKeyDown(KEY_W)) xCameraMove(&camera, camera.front, speed);
        if (xIsKeyDown(KEY_S)) xCameraMove(&camera, mul(camera.front, -1), speed);
        if (xIsKeyDown(KEY_A)) xCameraMove(&camera, mul(camera.right, -1), speed);
        if (xIsKeyDown(KEY_D)) xCameraMove(&camera, camera.right, speed);
        
        // Render checkerboard ground
        for (int y = 0; y < win.height; y++)
        {
            const float v = ((float)(win.height - 1 - y) / (win.height - 1) - 0.5f) * viewport_height;
            
            for (int x = 0; x < win.width; x++)
            {
                const float u = ((float)x / (win.width - 1) - 0.5f) * viewport_width;
                const Vec3 dir = norm(add(camera.front, add(mul(camera.right, u), mul(camera.up, v))));
                win.buffer[y * win.width + x] = raytrace_ground(camera.position, dir);
            }
        }
        
        xUpdateFramebuffer(&win);
        xUpdateFrame(&win);
        xUpdateInput();
    }
    
    xDestroyWindow(&win);
    return 0;
}
