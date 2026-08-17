// Single-owner transport for WarcraftXL's post-3.3.5 opcode namespace.
// Published through WXL_Api::GetInterface("wxl.network", WXL_NETWORK_API_VERSION).
// Copyright (C) 2026 WarcraftXL. GPLv3.

#ifndef WXL_NETWORK_API_H
#define WXL_NETWORK_API_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WXL_NETWORK_API_VERSION 1

typedef void(__cdecl* WXL_NetworkPacketHandler)(
    const uint8_t* payload, uint32_t payloadSize, void* user);

typedef struct WXL_NetworkApi
{
    uint32_t structSize;
    uint32_t apiVersion;

    int(__cdecl* RegisterClientOpcode)(uint16_t opcode, const char* name);
    int(__cdecl* RegisterServerOpcode)(uint16_t opcode, const char* name,
                                       WXL_NetworkPacketHandler handler, void* user);
    int(__cdecl* Send)(uint16_t opcode, const uint8_t* payload, uint32_t payloadSize);
} WXL_NetworkApi;

#ifdef __cplusplus
}
#endif

#endif // WXL_NETWORK_API_H
