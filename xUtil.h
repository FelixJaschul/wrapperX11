// ============================================================================
// xUtil.h - Camera, Scene utilities
// ============================================================================
#ifndef XUTIL_H
#define XUTIL_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define INITIAL_VERTEX_CAPACITY 1024
#define INITIAL_TRIANGLE_CAPACITY 2048

#ifdef __cplusplus
extern "C" {
#endif

// Camera structure
typedef struct xCamera {
    Vec3 position;
    float yaw;     // Rotation in degrees
    float pitch;   // Rotation in degrees
    float fov;     // Field of view
    Vec3 front;    // Forward direction vector
    Vec3 right;    // Right direction vector
    Vec3 up;       // Up direction vector
} xCamera;

// Material properties for rendering
typedef struct {
    Vec3 color;
    float reflectivity;  // 0.0 = matte, 1.0 = mirror
    float specular;      // Specular highlight strength
} xMaterial;

// Triangle primitive for meshes
typedef struct {
    Vec3 v0, v1, v2;
} xTriangle;

// 3D model with transform and material
typedef struct {
    xTriangle* triangles;              // Original mesh triangles
    xTriangle* transformed_triangles;  // World-space transformed triangles
    int num_triangles;
    int capacity;                      // Allocated triangle capacity
    Vec3 position;
    float rot_x, rot_y, rot_z;        // Euler angles in radians
    Vec3 scale;
    xMaterial mat;
} xModel;

// Initialize camera with default position and orientation
/*  -> Example:
 *  xCamera camera;
 *  xCameraInit(&camera);
 *  camera.position = vec3(0.0f, 1.0f, 5.0f);
 */
void xCameraInit(xCamera *cam);

// Update camera vectors based on yaw/pitch (automatically clamps pitch)
/*  -> Example:
 *  camera.yaw += 10.0f;
 *  xCameraUpdate(&camera);
 */
void xCameraUpdate(xCamera *cam);

// Move camera in given direction by speed amount
/*  -> Example:
 *  if (xIsKeyDown(&input, KEY_W)) xCameraMove(&camera, camera.front, 0.1f);
 */
void xCameraMove(xCamera *cam, Vec3 direction, float speed);

// Rotate camera by delta angles in degrees
/*  -> Example:
 *  xCameraRotate(&camera, mouse_dx * sensitivity, -mouse_dy * sensitivity);
 */
void xCameraRotate(xCamera *cam, float dyaw, float dpitch);

// Generate ray from camera with pre-scaled viewport offsets
/*  -> Example:
 *  float* u_offsets = (float*)malloc(win.width * sizeof(float));
    float* v_offsets = (float*)malloc(win.height * sizeof(float));
    assert(u_offsets && v_offsets && "Failed to allocate viewport offset buffers");

    for (int x = 0; x < win.width; x++)  u_offsets[x] = ((float)x / (float)(win.width - 1) - 0.5f) * viewport_width;
    for (int y = 0; y < win.height; y++) v_offsets[y] = ((float)(win.height - 1 - y) / (float)(win.height - 1) - 0.5f) * viewport_height;

    while (1) {
        ...
        #pragma omp parallel for schedule(dynamic) default(none) shared(win, camera, u_offsets, v_offsets)
        for (int y = 0; y < win.height; y++) {
            for (int x = 0; x < win.width; x++) {
                const Ray ray = xCameraGetRay(&camera, u_offsets[x], v_offsets[y]);
                const Vec3 color = calculate_ray_color(ray, MAX_BOUNCES);
                win.buffer[y * win.width + x] = uint32(color);
        }
    }
}*/
Ray xCameraGetRay(const xCamera* cam, float u_scaled, float v_scaled);

// Create new model in storage array (returns NULL if array is full)
/*  -> Example:
 *  xModel* cube = xModelCreate(scene_models, &num_models, MAX_MODELS, vec3(1,0,0), 0.5f);
 */
xModel* xModelCreate(xModel* storage, int* count, int max, Vec3 color, float refl);

// Load OBJ file into model (dynamically allocates triangles)
/*  -> Example:
 *  xModelLoad(cube, "res/cube.obj");
 */
void xModelLoad(xModel* m, const char* path);

// Free model triangle data
/*  -> Example:
 *  xModelFree(cube);
 */
void xModelFree(xModel* m);

// Set model transform (position, rotation in radians, scale)
/*  -> Example:
 *  xModelTransform(cube, vec3(0,0,0), vec3(0, M_PI/4, 0), vec3(1,1,1));
 */
void xModelTransform(xModel* m, Vec3 pos, Vec3 rot, Vec3 scale);

// Apply transforms to all models in array (call after changing transforms)
/*  -> Example:
 *  xModelUpdate(scene_models, num_models);
 */
void xModelUpdate(const xModel* models, int count);

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
    // Clamp pitch to avoid gimbal lock
    if (cam->pitch > 89.0f) cam->pitch = 89.0f;
    if (cam->pitch < -89.0f) cam->pitch = -89.0f;

    // Calculate front vector from yaw/pitch
    const float yaw_rad   = cam->yaw * M_PI / 180.0f;
    const float pitch_rad = cam->pitch * M_PI / 180.0f;
    cam->front = vec3(
        cosf(yaw_rad) * cosf(pitch_rad),
        sinf(pitch_rad),
        sinf(yaw_rad) * cosf(pitch_rad)
    );

    // Calculate right, up and front vectors
    cam->front = norm(cam->front);
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

inline Ray xCameraGetRay(const xCamera* cam, const float u_scaled, const float v_scaled)
{
    Vec3 rd = add(cam->front, add(mul(cam->up, v_scaled), mul(cam->right, u_scaled)));
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
    v = vmul(v, m->scale);
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
        .triangles = NULL,
        .transformed_triangles = NULL,
        .num_triangles = 0,
        .capacity = 0,
        .position = {0, 0, 0},
        .scale = {1.0f, 1.0f, 1.0f},
        .rot_x = 0, .rot_y = 0, .rot_z = 0,
        .mat = {color, refl, 0.0f}
    };
    return m;
}

inline void xModelLoad(xModel* m, const char* path)
{
    FILE* f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "Failed to open OBJ file: %s\n", path);
        return;
    }

    // Dynamic arrays for vertices and triangles
    int vert_capacity = INITIAL_VERTEX_CAPACITY;
    int tri_capacity = INITIAL_TRIANGLE_CAPACITY;

    Vec3* verts = (Vec3*)malloc(vert_capacity * sizeof(Vec3));
    xTriangle* tris = (xTriangle*)malloc(tri_capacity * sizeof(xTriangle));
    assert(verts && tris && "Failed to allocate OBJ parsing buffers");

    int nv = 0, nt = 0;
    char buf[256];

    while (fgets(buf, sizeof(buf), f))
    {
        if (buf[0] == 'v' && buf[1] == ' ')
        {
            // Vertex line
            if (nv >= vert_capacity)
            {
                vert_capacity *= 2;
                verts = (Vec3*)realloc(verts, vert_capacity * sizeof(Vec3));
                assert(verts && "Failed to reallocate vertex buffer");
            }

            float x, y, z;
            sscanf(buf + 2, "%f %f %f", &x, &y, &z);
            verts[nv++] = vec3(x, y, z);
        }
        else if (buf[0] == 'f')
        {
            // Face line (only supports triangulated meshes)
            if (nt >= tri_capacity)
            {
                tri_capacity *= 2;
                tris = (xTriangle*)realloc(tris, tri_capacity * sizeof(xTriangle));
                assert(tris && "Failed to reallocate triangle buffer");
            }

            int a, b, c;
            if (sscanf(buf + 2, "%d %d %d", &a, &b, &c) == 3)
            {
                // OBJ indices are 1-based
                if (a > 0 && a <= nv && b > 0 && b <= nv && c > 0 && c <= nv)
                {
                    tris[nt++] = (xTriangle){verts[a-1], verts[b-1], verts[c-1]};
                }
            }
        }
    }
    fclose(f);

    // Allocate exact size for model
    m->triangles = (xTriangle*)malloc(nt * sizeof(xTriangle));
    m->transformed_triangles = (xTriangle*)malloc(nt * sizeof(xTriangle));
    assert(m->triangles && m->transformed_triangles && "Failed to allocate model triangles");

    memcpy(m->triangles, tris, nt * sizeof(xTriangle));
    m->num_triangles = nt;
    m->capacity = nt;

    // Free temporary buffers
    free(verts);
    free(tris);

    printf("Loaded %s: %d vertices, %d triangles\n", path, nv, nt);
}

inline void xModelFree(xModel* m)
{
    if (m->triangles)
    {
        free(m->triangles);
        m->triangles = NULL;
    }
    if (m->transformed_triangles)
    {
        free(m->transformed_triangles);
        m->transformed_triangles = NULL;
    }
    m->num_triangles = 0;
    m->capacity = 0;
}

inline void xModelTransform(xModel* m, const Vec3 pos, const Vec3 rot, const Vec3 scale)
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