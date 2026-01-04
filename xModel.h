// ============================================================================
// xModel.h - 3D model with materials and transforms
// ============================================================================

#ifndef XMODEL_H
#define XMODEL_H

#include "xMath.h"

#define INITIAL_VERTEX_CAPACITY 1024
#define INITIAL_TRIANGLE_CAPACITY 2048

#ifdef __cplusplus
extern "C" {
#endif

// Material properties for rendering
typedef struct {
    Vec3 color;
    float reflectivity;  // 0.0 = matte, 1.0 = mirror
	float specular;
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
    float rot_x, rot_y, rot_z;         // Euler angles in radians
    Vec3 scale;
    xMaterial mat;
} xModel;

// Create new model in storage array (returns NULL if array is full)
/*  -> Example:
 *  xModel* cube = xModelCreate(scene_models, &num_models, MAX_MODELS, vec3(1,0,0), 0.5f);
 */
xModel* xModelCreate(xModel* storage, int* count, int max, Vec3 color, float refl, float spec);

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

#ifdef XMODEL_IMPLEMENTATION

#include <stdlib.h>
#include <string.h>

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

inline xModel* xModelCreate(xModel* storage, int* count, const int max, const Vec3 color, const float refl, const float spec)
{
    if (*count >= max) return NULL;
    xModel* m = &storage[(*count)++];
    m->triangles = NULL;
    m->transformed_triangles = NULL;
    m->num_triangles = 0;
    m->capacity = 0;
    m->position = (Vec3){0, 0, 0};
    m->scale = (Vec3){1.0f, 1.0f, 1.0f};
    m->rot_x = 0; m->rot_y = 0; m->rot_z = 0;
    m->mat = (xMaterial){color, refl, spec};
    return m;
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

inline void xModelLoad(xModel* m, const char* path)
{
    FILE* f = fopen(path, "r");
    if (!f) {
        fprintf(stderr, "Failed to open OBJ file: %s\n", path);
        return;
    }

    // Dynamic arrays for vertices and triangles
    int vert_capacity = INITIAL_VERTEX_CAPACITY;
    int tri_capacity  = INITIAL_TRIANGLE_CAPACITY;

    Vec3* verts     = malloc(vert_capacity * sizeof(Vec3));
    xTriangle* tris = malloc(tri_capacity * sizeof(xTriangle));
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
    m->triangles             = malloc(nt * sizeof(xTriangle));
    m->transformed_triangles = malloc(nt * sizeof(xTriangle));
    assert(m->triangles && m->transformed_triangles && "Failed to allocate model triangles");

    memcpy(m->triangles, tris, nt * sizeof(xTriangle));
    m->num_triangles = nt;
    m->capacity = nt;

    // Free temporary buffers
    free(verts);
    free(tris);

    printf("Loaded %s: %d vertices, %d triangles\n", path, nv, nt);
}

#endif // XMODEL_IMPLEMENTATION
#endif // XMODEL_H