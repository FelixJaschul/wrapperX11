// ============================================================================
// xUtil.h - Camera, Scene utilities
// ============================================================================
#ifndef XUTIL_H
#define XUTIL_H

#include <math.h>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Camera structure
typedef struct xCamera {
    Vec3 position;
    float yaw;   // degrees
    float pitch; // degrees
    float fov;
    Vec3 front;
    Vec3 right;
    Vec3 up;
} xCamera;

// Scene object structures
typedef struct {
    Vec3 color;
    float reflectivity;
    float specular;
} xMaterial;

typedef struct {
    Vec3 center;
    float radius;
    xMaterial mat;
} xSphere;

typedef struct {
    Vec3 point;
    Vec3 normal;
    Vec3 u;
    Vec3 v;
    float width;
    float height;
    xMaterial mat;
} xRect;

// Camera functions
void xCameraInit(xCamera *cam);
void xCameraUpdate(xCamera *cam);
void xCameraMove(xCamera *cam, Vec3 direction, float speed);
void xCameraRotate(xCamera *cam, float dyaw, float dpitch);
Ray  xCameraGetRay(const xCamera *cam, float u, float v, float aspect_ratio);

// Scene helpers
void xAddSphere(xSphere *spheres, int *count, int max, Vec3 center, float radius, Vec3 color, float refl);
void xAddRect(xRect *rects, int *count, int max, Vec3 point, Vec3 normal, Vec3 u, Vec3 v, float width, float height, Vec3 color, float refl);
void xAddCube(xRect *rects, int *count, int max, Vec3 center, float sx, float sy, float sz, Vec3 color, float refl);

#ifdef __cplusplus
}
#endif

#ifdef XUTIL_IMPLEMENTATION

inline void xCameraInit(xCamera *cam)
{
    cam->position = vec3(0.0f, 0.0f, 0.0f);
    cam->yaw = 0.0f;
    cam->pitch = 0.0f;
    cam->fov = 60.0f;
    xCameraUpdate(cam);
}

inline void xCameraUpdate(xCamera *cam)
{
    // Clamp pitch
    if (cam->pitch > 89.0f) cam->pitch = 89.0f;
    if (cam->pitch < -89.0f) cam->pitch = -89.0f;
    
    // Calculate front vector
    const float yaw_rad = cam->yaw * M_PI / 180.0f;
    const float pitch_rad = cam->pitch * M_PI / 180.0f;
    
    cam->front = vec3(
        cosf(yaw_rad) * cosf(pitch_rad),
        sinf(pitch_rad),
        sinf(yaw_rad) * cosf(pitch_rad)
    );
    cam->front = norm(cam->front);
    
    // Calculate right and up vectors
    cam->right = norm(cross(cam->front, vec3(0, 1, 0)));
    cam->up    = cross(cam->right, cam->front);
}

inline void xCameraMove(xCamera *cam, const Vec3 direction, const float speed)
{
    cam->position = add(cam->position, mul(direction, speed));
}

inline void xCameraRotate(xCamera *cam, const float dyaw, const float dpitch)
{
    cam->yaw += dyaw;
    cam->pitch += dpitch;
    xCameraUpdate(cam);
}

inline Ray xCameraGetRay(const xCamera *cam, const float u, const float v, const float aspect_ratio)
{
    const float viewport_height = 2.0f;
    const float viewport_width = aspect_ratio * viewport_height;
    
    Vec3 rd = add(cam->front, 
                  add(mul(cam->up, v * viewport_height),
                      mul(cam->right, u * viewport_width)));
    rd = norm(rd);
    
    return (Ray){cam->position, rd};
}

inline void xAddSphere(xSphere *spheres, int *count, const int max, const Vec3 center, const float radius, const Vec3 color, const float refl)
{
    if (*count < max) {
        spheres[*count] = (xSphere) {
            center, radius, 
            {color, refl, 0.0f}
        };
        (*count)++;
    }
}

inline void xAddRect(xRect *rects, int *count, const int max, const Vec3 point, const Vec3 normal, const Vec3 u, const Vec3 v, const float width, const float height, const Vec3 color, const float refl)
{
    if (*count < max) {
        rects[*count] = (xRect) {
            point, norm(normal), norm(u), norm(v),
            width, height,
            {color, refl, 0.0f}
        };
        (*count)++;
    }
}

inline void xAddCube(xRect *rects, int *count, const int max, const Vec3 center, const float sx, const float sy, const float sz, const Vec3 color, const float refl)
{
    const float hx = sx / 2.0f, hy = sy / 2.0f, hz = sz / 2.0f;
    
    xAddRect(rects, count, max, vec3(center.x, center.y - hy, center.z), vec3(0,-1,0), vec3(1,0,0), vec3(0,0,1), sx, sz, color, refl);
    xAddRect(rects, count, max, vec3(center.x, center.y + hy, center.z), vec3(0,1,0),  vec3(1,0,0), vec3(0,0,1), sx, sz, color, refl);
    xAddRect(rects, count, max, vec3(center.x - hx, center.y, center.z), vec3(-1,0,0), vec3(0,0,1), vec3(0,1,0), sz, sy, color, refl);
    xAddRect(rects, count, max, vec3(center.x + hx, center.y, center.z), vec3(1,0,0),  vec3(0,0,1), vec3(0,1,0), sz, sy, color, refl);
    xAddRect(rects, count, max, vec3(center.x, center.y, center.z - hz), vec3(0,0,-1), vec3(1,0,0), vec3(0,1,0), sx, sy, color, refl);
    xAddRect(rects, count, max, vec3(center.x, center.y, center.z + hz), vec3(0,0,1),  vec3(1,0,0), vec3(0,1,0), sx, sy, color, refl);
}

#endif // XUTIL_IMPLEMENTATION
#endif // XUTIL_H
