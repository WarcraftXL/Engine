// Reading a script call's arguments and answering it.
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

#include <cstddef>

#include "game/Binding.hpp"
#include "offsets/engine/Lua.hpp"

/**
 * @brief The arguments a script call carries, and what it hands back.
 *
 * Indices are one-based. A method invoked as frame:Method(a, b) puts the frame at 1 and its first
 * argument at 2 -- read the frame with wxl::game::glue::MethodSelf() rather than from the stack.
 *
 * A return value is pushed, and the count of pushes returned from the function.
 */
namespace wxl::game::script
{
    namespace off = wxl::offsets::engine::lua;

    /// A script function, as the method tables and the global registry hold it.
    using Function = off::LuaCFunction;

    // The engine checks every script callback against the bounds of its own image before invoking it,
    // and a function compiled into an extension lies outside them by construction -- the call is
    // refused with a fatal "Invalid function pointer", which is what makes this seam the gate any
    // script function added from outside has to pass.
    //
    // Detour it and return, without calling the original, for the pointers you registered yourself.
    // Anything else must still reach the original: the check is doing its job for the calls it was
    // written for, and widening it wholesale would give that up for all of them.

    /// Entry the engine validates a script callback through.
    constexpr uintptr_t kValidateCallbackSeam = off::kValidateFunctionPointer;

    /// Its signature, for a detour and the matching trampoline.
    using ValidateCallbackFn = off::ValidateFunctionPointerFn;

    /**
     * @brief Reads the live script state.
     * @return The state, or null before the script engine has been brought up.
     */
    inline void* CurrentState()
    { return Native<off::FrameScriptGetContextFn>(off::kFrameScriptGetContext)(); }

    /**
     * @brief Loads and runs a chunk on the live state.
     * @param source     Chunk text.
     * @param chunkName  What the chunk is called if it raises an error.
     * @param taintName  Owner recorded for the duration of the call; null runs it as the engine's own.
     */
    inline void Execute(const char* source, const char* chunkName, const char* taintName = nullptr)
    {
        Native<off::FrameScriptExecuteFn>(off::kFrameScriptExecute)(source, chunkName, taintName);
    }

    /**
     * @brief Declares a console variable, so a setting survives the script state being rebuilt.
     * @param name          Variable name, as the console and the script side see it.
     * @param defaultValue  Value it takes the first time it is declared.
     * @param archive       Whether the value is written back to the saved configuration on exit.
     * @return The variable, or null if @p name or @p defaultValue was empty.
     */
    inline void* RegisterCVar(const char* name, const char* defaultValue, bool archive = true)
    {
        constexpr uint32_t kNoFlags = 0;
        constexpr int      kDeclaredByCode = 1;
        constexpr uint32_t kNoUserData = 0;
        return Native<off::CVarRegisterFn>(off::kCVarRegister)(
            name, nullptr, kNoFlags, defaultValue, nullptr, off::kCVarCategoryDefault,
            kDeclaredByCode, kNoUserData, archive ? 1 : 0);
    }

    /**
     * @brief Counts the values passed to the call.
     * @param state  Script state the call arrived on.
     * @return How many, including the frame for a method call.
     */
    inline int ArgCount(void* state)
    { return Native<off::LuaGetTopFn>(off::kLuaGetTop)(state); }

    /**
     * @brief Reports whether an argument is a number, or a string convertible to one.
     * @param state  Script state.
     * @param index  One-based argument index.
     */
    inline bool IsNumber(void* state, int index)
    { return Native<off::LuaIsNumberFn>(off::kLuaIsNumber)(state, index) != 0; }

    /**
     * @brief Reports whether an argument is a string, or a number convertible to one.
     * @param state  Script state.
     * @param index  One-based argument index.
     */
    inline bool IsString(void* state, int index)
    { return Native<off::LuaIsStringFn>(off::kLuaIsString)(state, index) != 0; }

    /**
     * @brief Reads an argument as a number.
     * @param state  Script state.
     * @param index  One-based argument index.
     * @return The value, or 0 when it is not convertible.
     */
    inline double ToNumber(void* state, int index)
    { return Native<off::LuaToNumberFn>(off::kLuaToNumber)(state, index); }

    /**
     * @brief Reads an argument as a string.
     * @param state  Script state.
     * @param index  One-based argument index.
     * @return The text, or null when it is not convertible. It belongs to the script state and is only
     *         valid until the call returns -- copy anything kept.
     */
    inline const char* ToString(void* state, int index)
    { return Native<off::LuaToStringFn>(off::kLuaToString)(state, index, nullptr); }

    /**
     * @brief Pushes a number as a return value.
     * @param state  Script state.
     * @param value  Value to return.
     */
    inline void PushNumber(void* state, double value)
    { Native<off::LuaPushNumberFn>(off::kLuaPushNumber)(state, value); }

    /**
     * @brief Pushes a string as a return value.
     * @param state  Script state.
     * @param value  Text to return; it is copied.
     */
    inline void PushString(void* state, const char* value)
    { Native<off::LuaPushStringFn>(off::kLuaPushString)(state, value); }

    /**
     * @brief Pushes a boolean as a return value.
     * @param state  Script state.
     * @param value  Value to return.
     */
    inline void PushBoolean(void* state, bool value)
    { Native<off::LuaPushBooleanFn>(off::kLuaPushBoolean)(state, value ? 1 : 0); }

    /** Pushes nil as a return value. */
    inline void PushNil(void* state)
    { Native<off::LuaPushNilFn>(off::kLuaPushNil)(state); }

    /**
     * @brief Registers a global script function.
     * @param name      Name it is called by.
     * @param function  Function invoked.
     *
     * For a method on a frame type, see wxl::game::glue::AddMethods instead.
     */
    inline void Register(const char* name, Function function)
    { Native<off::FrameScriptRegisterFunctionFn>(off::kFrameScriptRegisterFunction)(name, function); }
}
