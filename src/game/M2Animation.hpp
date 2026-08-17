// M2 animation-resolution bindings: typed access to the Wrath client's model and AnimationData
// fallback paths. Version-specific addresses remain in offsets; extensions consume these wrappers.
// Copyright (C) 2026 WarcraftXL. GPLv3.

#pragma once

#include "game/Binding.hpp"
#include "offsets/game/DB2.hpp"
#include "offsets/game/M2.hpp"
#include "offsets/game/Unit.hpp"

#include <cstdint>

namespace wxl::game::m2animation
{
    namespace db2off = wxl::offsets::game::db2;
    namespace m2off = wxl::offsets::game::m2;
    namespace unitoff = wxl::offsets::game::unit;

    // AnimationData row 506 is CurrentOrNone in the 3.3.5 client. IDs above it are not passed
    // through the native fallback walker even when a modern M2 contains the requested sequence.
    inline constexpr int kCurrentOrNone = 506;

#pragma pack(push, 1)
    struct AnimationDataRow
    {
        uint32_t id;
        const char* name;
        uint32_t weaponFlags;
        uint32_t bodyFlags;
        uint32_t flags;
        uint32_t fallback;
        uint32_t behaviorId;
        uint32_t behaviorTier;
    };
#pragma pack(pop)

    inline void* UnitModel(void* unit) noexcept
    {
        return unit ? *reinterpret_cast<void**>(
            reinterpret_cast<uintptr_t>(unit) + unitoff::kUnitModelField) : nullptr;
    }

    inline void* ModelData(void* model) noexcept
    {
        if (!model) return nullptr;
        const uintptr_t shared = *reinterpret_cast<uintptr_t*>(
            reinterpret_cast<uintptr_t>(model) + m2off::kOffPlayableModelShared);
        if (shared < 0x10000) return nullptr;
        void* data = *reinterpret_cast<void**>(shared + m2off::kOffPlayableSharedData);
        return reinterpret_cast<uintptr_t>(data) >= 0x10000 ? data : nullptr;
    }

    inline bool ModelHasSequence(void* model, uint32_t animationId) noexcept
    {
        void* data = ModelData(model);
        if (!data) return false;
        return Native<m2off::M2_HasSequenceByIdFn>(m2off::kM2DataHasSequenceById)(
            data, animationId);
    }

    /** Returns the model's authored sequence duration in milliseconds, or zero. */
    inline uint32_t ModelSequenceDuration(void* model, uint32_t animationId) noexcept
    {
        void* data = ModelData(model);
        if (!data) return 0;
        const auto* header = static_cast<const m2off::M2FileHeader*>(data);
        if (!header->seqCount || header->seqCount > 10000 ||
            reinterpret_cast<uintptr_t>(header->seqPtr) < 0x10000)
            return 0;

        const auto* sequences = static_cast<const m2off::M2SequenceRec*>(header->seqPtr);
        for (uint32_t index = 0; index < header->seqCount; ++index)
            if (sequences[index].id == animationId)
                return sequences[index].durationMs;
        return 0;
    }

    inline const AnimationDataRow* Lookup(uint32_t animationId) noexcept
    {
        return static_cast<const AnimationDataRow*>(
            Native<db2off::ClientDbGetRowFn>(db2off::kClientDbGetRow)(
                reinterpret_cast<void*>(db2off::animationdata::kStorageObject), animationId));
    }
}
