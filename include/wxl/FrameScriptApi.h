// Registering script-side surface that survives the script state being rebuilt.
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

#ifndef WXL_FRAME_SCRIPT_API_H
#define WXL_FRAME_SCRIPT_API_H

#include <stdint.h>

// An extension can reach the script engine on its own -- wxl::game::script wraps every call it
// needs. What it cannot do on its own is survive: the script state is torn down and rebuilt (a
// reload, a return to the login screen), and everything registered on the old one is gone. Each
// extension re-registering for itself means each extension having to notice the rebuild, and the
// order they notice in decides which of them sees a half-built environment.
//
// So one owner holds the registrations and replays them onto every new state, in the order they were
// made. What an extension registers here it registers once, at load, and never thinks about again.
//
// Plain C, __cdecl spelled out, POD only -- see PluginApi.h for why.

#ifdef __cplusplus
extern "C" {
#endif

#define WXL_FRAME_SCRIPT_API_VERSION 1

/**
 * @brief A native function callable from script.
 * @param state  the script state the call arrived on.
 * @return the number of values left on the stack as return values.
 */
typedef int(__cdecl* WXL_LuaCFunction)(void* state);

typedef struct WXL_FrameScriptApi
{
    uint32_t structSize;
    uint32_t apiVersion;

    /**
     * @brief Registers a native function under a global name, on this state and every later one.
     * @param name      the name script calls it by.
     * @param function  the implementation.
     * @return non-zero when registered; zero when the name is already taken.
     */
    int(__cdecl* RegisterFunction)(const char* name, WXL_LuaCFunction function);

    /**
     * @brief Registers script source run on every state, this one included.
     *
     * For the script-side half of a feature: the tables, frames and handlers that have to exist
     * before anything calls into them. Sources run in registration order.
     * @param name    label used in an error message, so a failure names its source.
     * @param source  the chunk text; copied, so the caller need not keep it.
     * @return non-zero when registered and its first run succeeded.
     */
    int(__cdecl* RegisterScript)(const char* name, const char* source);

    /**
     * @brief Runs a chunk once, on the live state, without registering it.
     *
     * For a one-off: reacting to something that just happened, not building the environment. It is
     * not replayed, so a state rebuild leaves nothing of it behind.
     * @param source  the chunk text.
     * @param name    label used in an error message.
     * @return non-zero when the chunk ran; zero when there is no live state.
     */
    int(__cdecl* Execute)(const char* source, const char* name);

    /**
     * @brief Declares a console variable.
     *
     * A variable belongs to the engine's own registry rather than to the script state, so it is the
     * one place a setting outlives a rebuild without being replayed. Declaring the same name and
     * default twice is harmless.
     * @param name          the variable name.
     * @param defaultValue  the value it takes the first time it is declared.
     * @return non-zero when the variable exists after the call.
     */
    int(__cdecl* RegisterCVar)(const char* name, const char* defaultValue);
} WXL_FrameScriptApi;

#ifdef __cplusplus
}
#endif

#endif // WXL_FRAME_SCRIPT_API_H
