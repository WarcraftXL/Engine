// The single-owner transport for opcodes the stock protocol leaves free.
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

#ifndef WXL_NETWORK_API_H
#define WXL_NETWORK_API_H

#include <stdint.h>

// The engine dispatches an incoming message through a fixed table that stops short of the top of the
// opcode range, so everything above that ceiling reaches no handler and is dropped. Claiming one of
// those opcodes means detouring the single receive entry -- and two parties detouring it would each
// see the other's traffic and have to agree on who drops what. So the transport has ONE owner, and
// every feature that wants a custom opcode registers with it instead.
//
// A feature and its server-side counterpart must agree on the numeric opcode and the payload layout.
// Neither is modelled here: this interface moves bytes, it does not know what they mean. Where the
// assignments are recorded is the transport's business, not the core's.
//
// Plain C, __cdecl spelled out, POD only -- see PluginApi.h for why.

#ifdef __cplusplus
extern "C" {
#endif

#define WXL_NETWORK_API_VERSION 1

/**
 * @brief Receives one message body, header already consumed.
 * @param payload      the bytes after the opcode.
 * @param payloadSize  their count.
 * @param user         the opaque pointer given at registration.
 */
typedef void(__cdecl* WXL_NetworkPacketHandler)(
    const uint8_t* payload, uint32_t payloadSize, void* user);

typedef struct WXL_NetworkApi
{
    uint32_t structSize;
    uint32_t apiVersion;

    /**
     * @brief Claims an opcode this client sends.
     *
     * Reserving it is what stops two features from each believing they own the number. Sending does
     * not require the engine to know the opcode, only the server.
     * @param opcode  the numeric opcode.
     * @param name    label used in logging, and in the refusal when the opcode is already claimed.
     * @return non-zero when the claim succeeded; zero when another party holds it.
     */
    int(__cdecl* RegisterClientOpcode)(uint16_t opcode, const char* name);

    /**
     * @brief Claims an opcode this client receives, and the handler that consumes it.
     *
     * One handler per opcode: it is the party that decides what the message means. A second
     * registration for the same opcode is refused rather than chained.
     * @param opcode   the numeric opcode.
     * @param name     label used in logging.
     * @param handler  invoked on the thread the message arrives on.
     * @param user     opaque pointer passed back to @p handler.
     * @return non-zero when the claim succeeded; zero when another party holds it.
     */
    int(__cdecl* RegisterServerOpcode)(uint16_t opcode, const char* name,
                                       WXL_NetworkPacketHandler handler, void* user);

    /**
     * @brief Sends a message under a claimed opcode.
     * @param opcode       an opcode this extension claimed with RegisterClientOpcode.
     * @param payload      the body, without the header the transport stamps itself.
     * @param payloadSize  its byte count.
     * @return non-zero when the message was handed to the connection.
     */
    int(__cdecl* Send)(uint16_t opcode, const uint8_t* payload, uint32_t payloadSize);

    /**
     * @brief Adds a listener on an opcode without claiming it.
     *
     * Where RegisterServerOpcode appoints the one party that interprets a message, this adds parties
     * that only watch: a debug overlay, a bridge that mirrors the message to script. Observers run
     * after the owning handler and cannot suppress it, so adding one cannot change what a feature
     * does. An opcode with observers but no owner is still intercepted, which is what lets something
     * be watched before anyone implements it.
     * @param opcode   the numeric opcode.
     * @param name     label used in logging.
     * @param handler  invoked after the owning handler, if there is one.
     * @param user     opaque pointer passed back to @p handler.
     * @return non-zero when the observer was added.
     */
    int(__cdecl* RegisterServerObserver)(uint16_t opcode, const char* name,
                                         WXL_NetworkPacketHandler handler, void* user);
} WXL_NetworkApi;

#ifdef __cplusplus
}
#endif

#endif // WXL_NETWORK_API_H
