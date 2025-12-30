// ============================================================================
// xUtil.h - Camera, Scene utilities
// ============================================================================
#ifndef XUTIL_H
#define XUTIL_H

#include <math.h>
#include <stdio.h>
#include <string.h>
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
    Vec3 v0, v1, v2;
} xTriangle;

typedef struct {
    xTriangle* triangles;
    xTriangle* transformed_triangles;
    int num_triangles;
    Vec3 position;
    float rot_x, rot_y, rot_z;
    float scale;
    xMaterial mat;
} xModel;

// Camera functions
void xCameraInit(xCamera *cam);
void xCameraUpdate(xCamera *cam);
void xCameraMove(xCamera *cam, Vec3 direction, float speed);
void xCameraRotate(xCamera *cam, float dyaw, float dpitch);
Ray  xCameraGetRay(const xCamera *cam, float u, float v, float aspect_ratio);

// Model functions
xModel* xModelCreate(xModel* storage, int* count, int max, Vec3 color, float refl);
void    xModelLoad(xModel* m, const char* path);
void    xModelTransform(xModel* m, Vec3 pos, Vec3 rot, float scale);
void    xModelUpdate(const xModel* models, int count);

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

    Vec3 rd = add(cam->front, add(mul(cam->up, v * viewport_height), mul(cam->right, u * viewport_width)));
    rd = norm(rd);

    return (Ray){cam->position, rd};
}

// Rotation helpers
static inline Vec3 rotate_x(const Vec3 v, const float a)
{
    const float c = cosf(a), s = sinf(a);
    return vec3(v.x, v.y * c - v.z * s, v.y * s + v.z * c);
}

static inline Vec3 rotate_y(const Vec3 v, const float a)
{
    const float c = cosf(a), s = sinf(a);
    return vec3(v.x * c - v.z * s, v.y, v.x * s + v.z * c);
}

static inline Vec3 rotate_z(const Vec3 v, const float a)
{
    const float c = cosf(a), s = sinf(a);
    return vec3(v.x * c - v.y * s, v.x * s + v.y * c, v.z);
}

static inline Vec3 transform_vertex(Vec3 v, const xModel* m)
{
    v = mul(v, m->scale);
    if (m->rot_z) v = rotate_z(v, m->rot_z);
    if (m->rot_x) v = rotate_x(v, m->rot_x);
    if (m->rot_y) v = rotate_y(v, m->rot_y);
    return add(v, m->position);
}

inline xModel* xModelCreate(xModel* storage, int* count, const int max, const Vec3 color, const float refl)
{
    if (*count >= max) return NULL;

    xModel* m = &storage[(*count)++];
    *m = (xModel){
        .position = {0, 0, 0},
        .scale = 1.0f,
        .mat = {color, refl, 0.0f}
    };
    return m;
}

inline void xModelLoad(xModel* m, const char* path)
{
    FILE* f = fopen(path, "r");
    if (!f) {
        printf("Failed to load: %s\n", path);
        return;
    }

    Vec3 verts[1000];
    xTriangle tris[1000];
    int nv = 0, nt = 0;

    char buf[256];
    while (fgets(buf, sizeof(buf), f))
    {
        if (buf[0] == 'v' && buf[1] == ' ')
        {
            float x, y, z;
            sscanf(buf + 2, "%f %f %f", &x, &y, &z);
            verts[nv++] = vec3(x, y, z);
        }
        else if (buf[0] == 'f') {
            int a, b, c;
            sscanf(buf + 2, "%d %d %d", &a, &b, &c);
            tris[nt++] = (xTriangle){verts[a-1], verts[b-1], verts[c-1]};
        }
    }
    fclose(f);

    m->triangles = (xTriangle*)malloc(nt * sizeof(xTriangle));
    m->transformed_triangles = (xTriangle*)malloc(nt * sizeof(xTriangle));
    memcpy(m->triangles, tris, nt * sizeof(xTriangle));
    m->num_triangles = nt;
}

inline void xModelTransform(xModel* m, const Vec3 pos, const Vec3 rot, const float scale)
{
    m->position = pos;
    m->rot_x = rot.x;
    m->rot_y = rot.y;
    m->rot_z = rot.z;
    m->scale = scale;
}

inline void xModelUpdate(const xModel* models, const int count)
{
    for (int i = 0; i < count; i++)
    {
        const xModel* m = &models[i];
        for (int j = 0; j < m->num_triangles; j++)
        {
            m->transformed_triangles[j].v0 = transform_vertex(m->triangles[j].v0, m);
            m->transformed_triangles[j].v1 = transform_vertex(m->triangles[j].v1, m);
            m->transformed_triangles[j].v2 = transform_vertex(m->triangles[j].v2, m);
        }
    }
}

#endif // XUTIL_IMPLEMENTATION
#endif // XUTIL_H
