// wxl-modern-m2's extended animation resolver, published through
// WXL_Api::GetInterface("wxl.m2-animation", WXL_M2_ANIMATION_API_VERSION).
// Copyright (C) 2026 WarcraftXL. GPLv3.

#ifndef WXL_M2_ANIMATION_API_H
#define WXL_M2_ANIMATION_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WXL_M2_ANIMATION_API_VERSION 1

typedef int32_t(__cdecl* WXL_M2AnimationResolveOverrideFn)(
    void* unit, int32_t requestedAnimation, void* model, int32_t resolvedAnimation);

typedef struct WXL_M2AnimationApi
{
    uint32_t structSize;
    uint32_t apiVersion;

    // One high-level animation owner may override the resolved result. Passing NULL removes it.
    // Returning a negative value keeps the resolver's result; zero or greater replaces it.
    void(__cdecl* SetResolveOverride)(WXL_M2AnimationResolveOverrideFn callback);
} WXL_M2AnimationApi;

#ifdef __cplusplus
}
#endif

#endif // WXL_M2_ANIMATION_API_H
