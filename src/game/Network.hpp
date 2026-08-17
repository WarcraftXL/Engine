// Typed bindings for the native 3.3.5a packet allocator and sender.
// Copyright (C) 2026 WarcraftXL. GPLv3.

#pragma once

#include "game/Binding.hpp"
#include "offsets/game/Network.hpp"

namespace wxl::game::network
{
    namespace off = wxl::offsets::game::network;
    using ClientPacket = off::ClientPacket;

    inline void Initialize(ClientPacket* packet)
    {
        Native<off::InitializePacketFn>(off::kInitializePacket)(packet);
    }

    inline void Finalize(ClientPacket* packet)
    {
        Native<off::FinalizePacketFn>(off::kFinalizePacket)(packet);
    }

    inline void Send(ClientPacket* packet)
    {
        Native<off::SendPacketFn>(off::kSendPacket)(packet);
    }
}
