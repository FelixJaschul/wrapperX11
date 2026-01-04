// ============================================================================
// xLight.h - TODO
// ============================================================================
#ifndef XLIGHT_H
#define XLIGHT_H

#include "xMath.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct xLight {
    Vec3 position;
    float intensity;
} xLight;

// Initialize light with default values
/*  -> Example:
 *  xLight light;
 *  xLightInit(&light);
 *  light.position = vec3(0.0f, 5.0f, 0.0f);
 */
void xLightInit(xLight *light);

#ifdef __cplusplus
}
#endif

#ifdef XLIGHT_IMPLEMENTATION

inline void xLightInit(xLight *light)
{
    light->position = vec3(0.0f, 0.0f, 0.0f);
    light->intensity = 1.0f;
}

#endif // XLIGHT_IMPLEMENTATION
#endif // XLIGHT_H
