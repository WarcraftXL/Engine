// Animation-resolution bindings: what a model can actually play, and what to play instead.
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
#include "offsets/game/DB2.hpp"
#include "offsets/game/M2.hpp"

/**
 * @brief Typed access to the two halves of animation resolution: the table that says what an
 *        animation id means, and the model that says whether it can play it.
 *
 * The engine's own resolver stops at the last id its table defines, so an id past that never reaches
 * a model even when the model carries the sequence. A module that wants those ids to play asks the
 * model directly through here, and walks the table's fallbacks itself.
 */
namespace wxl::game::m2animation
{
    namespace db2off = wxl::offsets::game::db2;
    namespace m2off  = wxl::offsets::game::m2;

    /// One row of the animation table.
    using AnimationRow = db2off::animationdata::Row;

    /// Last id the table defines. Anything above it is ours to resolve.
    inline constexpr uint32_t kLastStockId = db2off::animationdata::kLastStockId;

    /**
     * @brief Reaches the parsed model data behind a displayed model.
     * @param model  Model as hung off a unit.
     * @return The parsed data, or null while the model is still loading.
     */
    inline void* ModelData(void* model) noexcept
    {
        if (!model) return nullptr;
        const auto* instance = static_cast<const m2off::M2Instance*>(model);
        const auto* shared = reinterpret_cast<const m2off::M2Model*>(instance->model);
        return shared ? shared->header : nullptr;
    }

    /**
     * @brief Asks the model itself whether it carries a sequence, bypassing the table's ceiling.
     * @param model        Model as hung off a unit.
     * @param animationId  Animation id, stock or beyond.
     * @return Whether the model can play it.
     */
    inline bool ModelHasSequence(void* model, uint32_t animationId) noexcept
    {
        void* data = ModelData(model);
        return data && Native<m2off::M2_HasSequenceByIdFn>(m2off::kM2DataHasSequenceById)(
            data, animationId);
    }

    /**
     * @brief Reads one row of the animation table.
     * @param animationId  Animation id.
     * @return The row, or null when the table defines no such id.
     */
    inline const AnimationRow* Lookup(uint32_t animationId) noexcept
    {
        return static_cast<const AnimationRow*>(
            Native<db2off::ClientDbGetRowFn>(db2off::kClientDbGetRow)(
                reinterpret_cast<void*>(db2off::animationdata::kStorageObject), animationId));
    }
}
