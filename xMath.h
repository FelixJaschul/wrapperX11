// ============================================================================
// xMath.h - Vector math 
// ============================================================================
#ifndef XMATH_H
#define XMATH_H

#include <math.h>

typedef struct { 
    float x, y, z; 
} Vec3;

typedef struct { 
    Vec3 origin, direction; 
} Ray;

Vec3  vec3(float x, float y, float z);
Vec3  add(Vec3 a, Vec3 b);
Vec3  sub(Vec3 a, Vec3 b);
Vec3  mul(Vec3 v, float t);
Vec3  vdiv(Vec3 v, float t);
float dot(Vec3 a, Vec3 b);
Vec3  cross(Vec3 a, Vec3 b);
float len(Vec3 v);
Vec3  norm(Vec3 v);
Vec3  reflect(Vec3 v, Vec3 n);

#ifdef XMATH_IMPLEMENTATION

inline Vec3 vec3(const float x, const float y, const float z)
{ 
    return (Vec3) {x, y, z}; 
}

inline Vec3 add(const Vec3 a, const Vec3 b)
{
    return (Vec3) {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z,
    };
}

inline Vec3 sub(const Vec3 a, const Vec3 b)
{
    return (Vec3) {
        a.x - b.x,
        a.y - b.y,
        a.z - b.z,
    };
}

inline Vec3 mul(Vec3 v, const float t)
{
    return (Vec3) {
        v.x * t,
        v.y * t,
        v.z * t,
    };
}

inline Vec3 vdiv(const Vec3 v, const float t)
{
    const float inv = 1.0f / t;
    return (Vec3) {
        v.x * inv,
        v.y * inv,
        v.z * inv,
    };
}

inline float dot(const Vec3 a, const Vec3 b)
{
    return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

inline Vec3 cross(const Vec3 a, const Vec3 b)
{
    return (Vec3) {
        (a.y * b.z) - (a.z * b.y),
        (a.z * b.x) - (a.x * b.z),
        (a.x * b.y) - (a.y * b.x),
    };
}

inline float len(const Vec3 v)
{
    return sqrtf(dot(v,v));
}

inline Vec3 norm(const Vec3 v)
{
    const float l2 = dot(v,v);
    if (l2 > 0.0f) 
    {
        const float inv = 1.0f / sqrtf(l2);
        return (Vec3) {
            v.x * inv,
            v.y * inv,
            v.z * inv,
        };
    }
    return v;
}

inline Vec3 reflect(const Vec3 v, const Vec3 n)
{
    const float d = 2.0f * dot(v,n);
    return (Vec3) {
        v.x - (d * n.x),
        v.y - (d * n.y),
        v.z - (d * n.z),
    };
}

#endif // XMATH_IMPLEMENTATION
#endif // XMATH_H
