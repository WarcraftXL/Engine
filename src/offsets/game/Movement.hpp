// Movement input, collision probing, and the unit-driven animation entry.
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

// INTERNAL to the core. The movement-side entries the wxl::game::movement bindings wrap. Unit field
// offsets are NOT here: facing, pitch and the movement flags live on the unit object, so they belong
// with the rest of it in offsets/game/Unit.hpp. Modules never include this.
namespace wxl::offsets::game::movement
{
    // Drives one unit's displayed animation, then walks its attached units so a mount and its rider
    // stay in step. ecx = the unit; the model is the one the sequence is set on.
    constexpr uintptr_t kSetBoneSequence = 0x00735820;

    // The keyboard/mouse control state the movement tick reads, and the tick stamp a control change
    // has to carry to be accepted on the frame it is made.
    constexpr uintptr_t kInputControlPtr = 0x00C24954;
    constexpr uintptr_t kInputTimestamp  = 0x00B499A4;
    constexpr uintptr_t kSetControlBit   = 0x005FA170;
    constexpr uintptr_t kUnsetControlBit = 0x005FA450;

    // Segment cast against the collidable world: terrain, map objects and their doodads, selected by
    // the flags word. Writes the impact point and how far along the segment it landed.
    constexpr uintptr_t kTraceLine = 0x007A3B70;

    /// Passed as the variation index to let the client pick the variation itself.
    constexpr uint32_t kVariationAuto = 0xFFFFFFFF;

    using SetBoneSequenceFn = void(__thiscall*)(
        void* unit, uintptr_t model, int boneSlot, int animationId, uint32_t variationIndex,
        int unk6, float speed, int unk8, int unk9, int unk10);

    using SetControlBitFn   = int(__thiscall*)(void* input, uint32_t bit, uint32_t timestamp);
    using UnsetControlBitFn = int(__thiscall*)(void* input, uint32_t bit, uint32_t timestamp,
                                               uint32_t unk);
    using TraceLineFn       = char(__cdecl*)(float* end, float* start, float* impact,
                                             float* distanceFraction, uint32_t flags, uint32_t unk);
}
