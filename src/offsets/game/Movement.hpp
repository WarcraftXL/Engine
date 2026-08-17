// Player movement, input-control, collision, and unit-animation bindings for build 12340.
// Copyright (C) 2026 WarcraftXL. GPLv3.

#pragma once

#include <cstddef>
#include <cstdint>

namespace wxl::offsets::game::movement
{
    constexpr uintptr_t kSetBoneSequence = 0x00735820;
    constexpr uintptr_t kInputControlPtr = 0x00C24954;
    constexpr uintptr_t kInputTimestamp = 0x00B499A4;
    constexpr uintptr_t kSetControlBit = 0x005FA170;
    constexpr uintptr_t kUnsetControlBit = 0x005FA450;
    constexpr uintptr_t kTraceLine = 0x007A3B70;

    constexpr size_t kUnitFacing = 0x7A8;
    constexpr size_t kUnitPitch = 0x7AC;
    constexpr size_t kUnitMovementFlags = 0x7CC;

    using SetBoneSequenceFn = void(__thiscall*)(
        void* unit, uintptr_t model, int boneSlot, int animationId,
        float sequenceTime, int unknown6, float speed,
        int unknown8, int unknown9, int unknown10);
    using SetControlBitFn = int(__thiscall*)(void* input, uint32_t bit, uint32_t timestamp);
    using UnsetControlBitFn =
        int(__thiscall*)(void* input, uint32_t bit, uint32_t timestamp, uint32_t unknown);
    using TraceLineFn =
        char(__cdecl*)(float* end, float* start, float* result,
                       float* distanceFraction, uint32_t flags, uint32_t unknown);
}
