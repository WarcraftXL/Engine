// 3.3.5a packet transport addresses and native packet layout.
// Copyright (C) 2026 WarcraftXL. GPLv3.

#pragma once

#include <cstdint>

namespace wxl::offsets::game::network
{
#pragma pack(push, 1)
    struct ClientPacket
    {
        uint32_t padding;
        uint8_t* buffer;
        uint32_t base;
        uint32_t allocation;
        uint32_t size;
        uint32_t read;
    };
#pragma pack(pop)
    static_assert(sizeof(ClientPacket) == 24, "ClientPacket");

    constexpr uintptr_t kInitializePacket = 0x00401050;
    constexpr uintptr_t kFinalizePacket   = 0x00401130;
    constexpr uintptr_t kSendPacket       = 0x006B0B50;
    constexpr uintptr_t kProcessMessage   = 0x00631FE0;

    using InitializePacketFn = void(__thiscall*)(ClientPacket* packet);
    using FinalizePacketFn = void(__thiscall*)(ClientPacket* packet);
    using SendPacketFn = void(__cdecl*)(ClientPacket* packet);
    using ProcessMessageFn =
        void(__thiscall*)(void* client, int time, ClientPacket* packet, int unused);
}
