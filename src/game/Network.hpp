// Network bindings: building an outgoing packet the way the engine builds its own.
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

#include "game/Binding.hpp"
#include "offsets/game/Network.hpp"

/**
 * @brief Typed accessors for the engine's own packet buffer.
 *
 * A packet of ours goes out through the same three calls the engine uses for its own, so it is
 * encrypted, framed and queued identically and needs nothing of its own on the wire.
 */
namespace wxl::game::network
{
    namespace off = wxl::offsets::game::network;

    /// The buffer an outgoing packet is built in. Caller-owned, sized as the writes accumulate.
    using ClientPacket = off::ClientPacket;

    /// @brief Prepares a buffer for writing. Must run before the first byte is appended.
    inline void Initialize(ClientPacket* packet)
    { Native<off::InitializePacketFn>(off::kInitializePacket)(packet); }

    /// @brief Stamps the header over what was written. Runs once, after the last byte.
    inline void Finalize(ClientPacket* packet)
    { Native<off::FinalizePacketFn>(off::kFinalizePacket)(packet); }

    /// @brief Hands a finalized packet to the connection.
    inline void Send(ClientPacket* packet)
    { Native<off::SendPacketFn>(off::kSendPacket)(packet); }
}
