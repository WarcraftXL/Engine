// Shared FrameScript registration service for out-of-core extensions.
// Copyright (C) 2026 WarcraftXL. GPLv3.

#ifndef WXL_FRAME_SCRIPT_API_H
#define WXL_FRAME_SCRIPT_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WXL_FRAME_SCRIPT_API_VERSION 1

typedef int(__cdecl* WXL_LuaCFunction)(void* state);

typedef struct WXL_FrameScriptApi
{
    uint32_t structSize;
    uint32_t apiVersion;

    int(__cdecl* RegisterFunction)(const char* name, WXL_LuaCFunction function);
    int(__cdecl* RegisterScript)(const char* name, const char* source);
    int(__cdecl* ExecuteCurrent)(const char* name, const char* source);

    /*
     * Appended in the v1 ABI. Consumers must check structSize before using it so an extension built
     * against this header can still run with an older v1 provider. The CVar belongs to the native
     * client registry and survives FrameScript-state recreation; registration is idempotent when
     * name and defaultValue match.
     */
    int(__cdecl* RegisterCVar)(const char* name, const char* defaultValue);
} WXL_FrameScriptApi;

#ifdef __cplusplus
}
#endif

#endif // WXL_FRAME_SCRIPT_API_H
