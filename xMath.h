#ifndef XMATH_H
#define XMATH_H

#include <math.h>

typedef struct {
    float x, y, z;
} Vec3;

typedef struct {
    Vec3 origin;
    Vec3 direction;
} Ray;

static inline Vec3 vec3(float x, float y, float z) {
    return (Vec3){x, y, z};
}

static inline Vec3 add(Vec3 a, Vec3 b) {
    return (Vec3){a.x + b.x, a.y + b.y, a.z + b.z};
}

static inline Vec3 sub(Vec3 a, Vec3 b) {
    return (Vec3){a.x - b.x, a.y - b.y, a.z - b.z};
}

static inline Vec3 mul(Vec3 v, float t) {
    return (Vec3){v.x * t, v.y * t, v.z * t};
}

static inline Vec3 vdiv(Vec3 v, float t) {
    return (Vec3){v.x / t, v.y / t, v.z / t};
}

static inline float dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline float len(Vec3 v) {
    return sqrtf(dot(v, v));
}

static inline Vec3 norm(Vec3 v) {
    return vdiv(v, len(v));
}

static inline Vec3 reflect(Vec3 v, Vec3 n) {
    return sub(v, mul(n, 2.0f * dot(v, n)));
}

static inline Vec3 cross(Vec3 a, Vec3 b) {
    return (Vec3){
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x
    };
}

#endif // XMATH_H
