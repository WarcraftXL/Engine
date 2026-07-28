// Game-logic detours that publish the non-render events (model load, doodad spawn, world lifecycle...).
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

namespace wxl::runtime::game
{
    /**
     * @brief Installs the disk-queue worker-count extension; must run before the client's own startup
     *        proceeds (call from the DLL entry, on the loader thread), same timing as the archive-mount
     *        guard. Enables its own hooks immediately.
     */
    void InstallEarly();

    /**
     * @brief Reserves the large-M2 virtual address arena before world loading fragments the client VA space.
     */
    void ReserveM2Memory();

    /**
     * @brief Installs the function-entry detours that republish game-logic events.
     *
     * Emits OnModelLoad, OnDoodadSpawn, OnWorldEnter, OnWorldLeave, OnTextureUpload and
     * OnAdtChunkBuild. The caller runs hook::EnableAll() once after every installer.
     */
    void Install();

    /**
     * @brief Calls the native (un-hooked) ItemDisplayInfo row-by-id lookup directly, bypassing the
     *        OnItemDisplayLookup event entirely.
     *
     * db2::itemdisplayinfo::kLookup is live-detoured once Install() has run (see hkItemDisplayLookup
     * in GameHooks.cpp), so any caller that still resolves and calls that address directly would
     * recurse into its own OnItemDisplayLookup subscriber and see its own already-rewritten fields
     * on the next lookup for the same id. Scripts that need the row's true native contents -- e.g.
     * to decide what an override for a displayId should even contain -- must call this instead.
     * Returns 0 before Install() has run (no trampoline captured yet).
     * @param displayId  ItemDisplayInfo row id to look up.
     * @param outBuf     destination buffer, db2::itemdisplayinfo::kRecordSize bytes.
     * @return the native lookup's own return value (nonzero on success).
     */
    uint32_t ItemDisplayInfoLookupNative(uint32_t displayId, void* outBuf);
}
