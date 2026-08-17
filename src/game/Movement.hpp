// Typed movement bindings used by optional movement-controller extensions.
// Copyright (C) 2026 WarcraftXL. GPLv3.

#pragma once

#include "game/Binding.hpp"
#include "offsets/game/Movement.hpp"
#include "offsets/game/Unit.hpp"

#include <cstdint>

namespace wxl::game::movement
{
    namespace off = wxl::offsets::game::movement;
    namespace unitoff = wxl::offsets::game::unit;

    inline void* UnitModel(void* unit) noexcept
    {
        return unit ? *reinterpret_cast<void**>(
            reinterpret_cast<uintptr_t>(unit) + unitoff::kUnitModelField) : nullptr;
    }

    inline void* ModelParent(void* model) noexcept
    {
        return model ? *reinterpret_cast<void**>(
            reinterpret_cast<uintptr_t>(model) + unitoff::kModelParentField) : nullptr;
    }

    inline uint32_t& Flags(void* unit)
    {
        return *reinterpret_cast<uint32_t*>(
            reinterpret_cast<uintptr_t>(unit) + off::kUnitMovementFlags);
    }

    inline float& Facing(void* unit)
    {
        return *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(unit) + off::kUnitFacing);
    }

    inline float& Pitch(void* unit)
    {
        return *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(unit) + off::kUnitPitch);
    }

    inline float* Position(void* unit)
    {
        return reinterpret_cast<float*>(
            reinterpret_cast<uintptr_t>(unit) + unitoff::kUnitPositionField);
    }

    inline int TraceLine(float* end, float* start, float* result,
                         float* distanceFraction, uint32_t flags)
    {
        return static_cast<int>(Native<off::TraceLineFn>(off::kTraceLine)(
            end, start, result, distanceFraction, flags, 0));
    }

    inline void SetBoneSequence(void* unit, void* model, int animationId,
                                float sequenceTime, float speed)
    {
        Native<off::SetBoneSequenceFn>(off::kSetBoneSequence)(
            unit, reinterpret_cast<uintptr_t>(model), -1, animationId,
            sequenceTime, 0, speed, 0, 1, 0);
    }

    inline void SetForwardControl(bool enabled, uint32_t controlBit)
    {
        void* input = *reinterpret_cast<void**>(off::kInputControlPtr);
        if (!input) return;
        const uint32_t timestamp = *reinterpret_cast<uint32_t*>(off::kInputTimestamp);
        if (enabled)
            Native<off::SetControlBitFn>(off::kSetControlBit)(input, controlBit, timestamp);
        else
            Native<off::UnsetControlBitFn>(off::kUnsetControlBit)(
                input, controlBit, timestamp, 0);
    }
}
