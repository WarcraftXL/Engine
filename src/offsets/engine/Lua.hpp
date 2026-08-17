// Lua/FrameScript landmarks for the target client (335).
// Copyright (C) 2026 WarcraftXL

#pragma once

#include <cstddef>
#include <cstdint>

namespace wxl::offsets::engine::lua
{
    using LuaCFunction = int(__cdecl*)(void* state);

    // Returns the active FrameScript Lua state, or null before script initialization.
    constexpr uintptr_t kFrameScriptGetContext = 0x00817DB0;
    using FrameScriptGetContextFn = void*(__cdecl*)();

    // Adds a global Lua function to the active FrameScript context.
    constexpr uintptr_t kFrameScriptRegisterFunction = 0x00817F90;
    using FrameScriptRegisterFunctionFn = void(__cdecl*)(const char* name, LuaCFunction function);

    // Lua 5.1 number push; WoW's lua_Number is double.
    constexpr uintptr_t kLuaPushNumber = 0x0084E2A0;
    using LuaPushNumberFn = void(__cdecl*)(void* state, double value);

    constexpr uintptr_t kLuaGetTop = 0x0084DBD0;
    using LuaGetTopFn = int(__cdecl*)(void* state);

    constexpr uintptr_t kLuaIsNumber = 0x0084DF20;
    using LuaIsNumberFn = int(__cdecl*)(void* state, int index);

    constexpr uintptr_t kLuaIsString = 0x0084DF60;
    using LuaIsStringFn = int(__cdecl*)(void* state, int index);

    constexpr uintptr_t kLuaToNumber = 0x0084E030;
    using LuaToNumberFn = double(__cdecl*)(void* state, int index);

    constexpr uintptr_t kLuaToString = 0x0084E0E0;
    using LuaToStringFn = const char*(__cdecl*)(void* state, int index, size_t* length);

    constexpr uintptr_t kLuaPushString = 0x0084E350;
    using LuaPushStringFn = void(__cdecl*)(void* state, const char* value);

    constexpr uintptr_t kLuaPushNil = 0x0084E280;
    using LuaPushNilFn = void(__cdecl*)(void* state);

    constexpr uintptr_t kLuaPushBoolean = 0x0084E4D0;
    using LuaPushBooleanFn = void(__cdecl*)(void* state, int value);

    constexpr uintptr_t kFrameScriptExecute = 0x00819210;
    // Executes a chunk in the active FrameScript context. The client supplies the state internally;
    // the second and third arguments identify the diagnostic chunk and taint owner.
    using FrameScriptExecuteFn = void(__cdecl*)(
        const char* source, const char* chunkName, const char* taintName);

    // Registers a native engine CVar. The final flag archives the value to Config.wtf.
    constexpr uintptr_t kCVarRegister = 0x00767FC0;
    using CVarRegisterFn = void*(__cdecl*)(const char* name, const char* description,
        uint32_t flags, const char* defaultValue, void* callback, uint32_t category,
        int codeRegistered, uint32_t userData, int archive);

    // Verifies that an indirect callback lies in Wow.exe's .text section before Lua invokes it.
    constexpr uintptr_t kValidateFunctionPointer = 0x0086B5A0;
    using ValidateFunctionPointerFn = void(__cdecl*)(uintptr_t function);

    // --- script methods on frame objects ---
    // A method as the tables hold it: the name Lua calls, then the function.
    struct ScriptMethod { const char* name; LuaCFunction function; };

    // Adds an array of methods to a metatable under construction. Every frame class registers through a
    // callback that ends in a call to this, which is what makes such a callback the place to add more:
    // the stock methods land first, ours after, on that class alone.
    constexpr uintptr_t kFillScriptMethodTable = 0x008167E0;
    using FillScriptMethodTableFn = void(__cdecl*)(void* target, const ScriptMethod* methods, int count);

    // The object a script method was invoked on, for a class's type id.
    constexpr uintptr_t kGetObjectThis = 0x004A81B0;
    using GetObjectThisFn = void*(__cdecl*)(int typeId);

    // Type ids are handed out lazily from this counter: a class's slot stays zero until one of its
    // script methods runs and claims the next id. A method added to a class has to claim it the same
    // way and into the same slot -- claiming its own would resolve a different object, or none.
    constexpr uintptr_t kObjectTypeCounter = 0x00D3F778;
}
