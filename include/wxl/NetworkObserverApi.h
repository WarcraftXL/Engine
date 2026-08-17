// Additive observers for WarcraftXL's single-owner custom-opcode transport.
// Copyright (C) 2026 WarcraftXL. GPLv3.

#ifndef WXL_NETWORK_OBSERVER_API_H
#define WXL_NETWORK_OBSERVER_API_H

#include "wxl/NetworkApi.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WXL_NETWORK_OBSERVER_API_VERSION 1

typedef struct WXL_NetworkObserverApi
{
    uint32_t structSize;
    uint32_t apiVersion;

    /*
     * Adds a non-owning listener for an SMSG opcode. Unlike RegisterServerOpcode, this does not
     * replace or consume the feature's primary handler. Observer-only opcodes are still intercepted
     * by the shared transport, permitting compatibility Lua consumers without another packet hook.
     */
    int(__cdecl* RegisterServerObserver)(uint16_t opcode, const char* name,
                                         WXL_NetworkPacketHandler handler, void* user);
} WXL_NetworkObserverApi;

#ifdef __cplusplus
}
#endif

#endif // WXL_NETWORK_OBSERVER_API_H
