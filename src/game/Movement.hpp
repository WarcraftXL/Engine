// Movement bindings: the unit's own heading and state, the control input, and a world probe.
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

#include "game/Binding.hpp"
#include "offsets/game/Movement.hpp"
#include "offsets/game/Unit.hpp"

/**
 * @brief Typed accessors for what a movement controller reads and writes on a unit, plus the two
 *        calls it needs beyond the unit itself: the control input, and a segment cast.
 *
 * Every accessor here aliases live engine memory. Writing one changes what the engine does on its
 * next tick, so a controller that stops writing leaves the unit where it stood rather than snapping
 * it back.
 */
namespace wxl::game::movement
{
    namespace off     = wxl::offsets::game::movement;
    namespace unitoff = wxl::offsets::game::unit;

    /// Applies a sequence to every bone slot rather than one.
    inline constexpr int kAllBoneSlots = -1;

    /// @brief Reads the unit's heading, in radians.
    inline float Facing(void* unit)
    { return static_cast<unitoff::UnitObject*>(unit)->facing; }

    /// @brief Writes the unit's heading, in radians.
    inline void SetFacing(void* unit, float radians)
    { static_cast<unitoff::UnitObject*>(unit)->facing = radians; }

    /// @brief Reads the unit's pitch, in radians, positive nose-up.
    inline float Pitch(void* unit)
    { return static_cast<unitoff::UnitObject*>(unit)->pitch; }

    /// @brief Writes the unit's pitch, in radians, positive nose-up.
    inline void SetPitch(void* unit, float radians)
    { static_cast<unitoff::UnitObject*>(unit)->pitch = radians; }

    /// @brief Reads the unit's movement-state bits.
    inline uint32_t MoveFlags(void* unit)
    { return static_cast<unitoff::UnitObject*>(unit)->moveFlags; }

    /// @brief Writes the unit's movement-state bits.
    inline void SetMoveFlags(void* unit, uint32_t flags)
    { static_cast<unitoff::UnitObject*>(unit)->moveFlags = flags; }

    /**
     * @brief Reads the unit's world position.
     * @return Pointer to the live x, y, z, so a controller can write it in place.
     */
    inline float* Position(void* unit)
    { return static_cast<unitoff::UnitObject*>(unit)->position; }

    /**
     * @brief Casts a segment against the collidable world.
     * @param start             Segment start, world x, y, z.
     * @param end               Segment end, world x, y, z.
     * @param impact            Receives the impact point on a hit.
     * @param distanceFraction  Receives how far along the segment the hit landed, 0..1.
     * @param flags             What the cast collides with.
     * @return Non-zero on a hit.
     */
    inline int TraceLine(const float start[3], const float end[3], float impact[3],
                         float* distanceFraction, uint32_t flags)
    {
        return Native<off::TraceLineFn>(off::kTraceLine)(
            const_cast<float*>(end), const_cast<float*>(start), impact, distanceFraction, flags, 0);
    }

    /**
     * @brief Plays an animation on a unit's model, carrying it to whatever is attached to it.
     * @param unit            Unit the animation is driven from.
     * @param model           Model the sequence is set on.
     * @param animationId     Animation to play.
     * @param speed           Playback rate, 1.0 for authored speed.
     * @param variationIndex  Which variation of the animation, or kVariationAuto to let the engine
     *                        choose. An index the animation does not have is treated as automatic.
     */
    inline void SetBoneSequence(void* unit, void* model, int animationId, float speed = 1.0f,
                                uint32_t variationIndex = off::kVariationAuto)
    {
        Native<off::SetBoneSequenceFn>(off::kSetBoneSequence)(
            unit, reinterpret_cast<uintptr_t>(model), kAllBoneSlots, animationId, variationIndex,
            0, speed, 0, 1, 0);
    }

    /**
     * @brief Holds or releases one movement control, as if it were being pressed.
     *
     * The change carries the engine's current tick stamp, so it takes effect on the frame it is made
     * rather than the next one.
     * @param bit      Which control.
     * @param pressed  Whether it is held.
     */
    inline void SetControl(uint32_t bit, bool pressed)
    {
        void* input = *reinterpret_cast<void**>(off::kInputControlPtr);
        if (!input) return;
        const uint32_t stamp = *reinterpret_cast<uint32_t*>(off::kInputTimestamp);
        if (pressed)
            Native<off::SetControlBitFn>(off::kSetControlBit)(input, bit, stamp);
        else
            Native<off::UnsetControlBitFn>(off::kUnsetControlBit)(input, bit, stamp, 0);
    }
}
