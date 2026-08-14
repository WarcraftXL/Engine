// Packet transport entries and the client's own outgoing-packet layout.
// Copyright (C) 2026 WarcraftXL
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cstdint>

// INTERNAL to the core. The send path a custom opcode has to go through to look like any other
// packet, plus the receive entry the hook-point table exposes by name. Modules never include this;
// they use wxl::game::network or the named hook point.
namespace wxl::offsets::game::network
{
    // The growable byte buffer the client builds an outgoing packet in. Initialize seeds it, the
    // writes append, Finalize stamps the header, Send hands it to the socket.
#pragma pack(push, 1)
    struct ClientPacket
    {
        uint32_t vtable;      // set by Initialize; never written by a caller
        uint8_t* buffer;      // the bytes themselves, reallocated as the packet grows
        uint32_t base;        // where the payload starts, past the header Finalize stamps
        uint32_t allocation;  // bytes currently allocated in buffer
        uint32_t size;        // bytes written so far
        uint32_t read;        // read cursor, used on the receive side
    };
#pragma pack(pop)
    static_assert(sizeof(ClientPacket) == 24, "ClientPacket");

    constexpr uintptr_t kInitializePacket = 0x00401050;
    constexpr uintptr_t kFinalizePacket   = 0x00401130;
    constexpr uintptr_t kSendPacket       = 0x006B0B50;
    // Receive entry: reads the opcode off the front of the packet, then dispatches through the
    // handler table. The table is only consulted below kOpcodeCeiling, so an opcode at or above it
    // reaches no stock handler at all and is dropped unless a detour here claims it first.
    constexpr uintptr_t kProcessMessage = 0x00631FE0;

    /// First opcode the stock dispatch table does not cover, and therefore the first one free to
    /// carry something of ours.
    constexpr uint16_t kOpcodeCeiling = 0x051F;

    using InitializePacketFn = void(__thiscall*)(ClientPacket* packet);
    using FinalizePacketFn   = void(__thiscall*)(ClientPacket* packet);
    using SendPacketFn       = void(__cdecl*)(ClientPacket* packet);
    // Two stack arguments, not three: this is callee-cleaned, so a detour carrying one more would
    // unwind too far and corrupt the caller's stack on every message.
    using ProcessMessageFn   = void(__thiscall*)(void* client, uint32_t connectionId,
                                                 ClientPacket* packet);
}
