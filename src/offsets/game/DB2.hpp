// Data-table runtime storage offsets and definition-override targets.
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

// INTERNAL to the core. Data-table runtime offsets. The map-data override rewrites the in-memory map
// storage built by the engine; the definition overrides target the stock storage object + accessor of
// each replaced table. Modules never include this; they use wxl::game / wxl::events.
namespace wxl::offsets::game::db2
{
    // Generic row accessor shared by the client's data tables: yields the record for an id, or null
    // when the table carries no such row. ecx = the table's storage object.
    constexpr uintptr_t kClientDbGetRow = 0x0065C290;
    using ClientDbGetRowFn = void*(__thiscall*)(void* storage, uint32_t id);

    // Animation table: what an animation id means, and what to play instead when a model lacks it.
    namespace animationdata
    {
        constexpr uintptr_t kStorageObject = 0x00AD30C8;

        // Highest id the native resolver accepts. Its fallback walk marks visited ids in a
        // stack-allocated array indexed by the id itself, so the bound is that buffer's size rather
        // than anything about the table: an id above it is refused before the walk starts, even when
        // the model carries the sequence. Never hand the native entries an id past this.
        constexpr uint32_t kLastStockId = 505;

#pragma pack(push, 1)
        /** @brief One animation row: its identity, its classification bits, and its fallback id. */
        struct Row
        {
            uint32_t    id;
            const char* name;
            uint32_t    weaponFlags;
            uint32_t    bodyFlags;
            uint32_t    flags;
            uint32_t    fallback;      // id to play instead when the model lacks this one (0 = none)
            uint32_t    behaviorId;
            uint32_t    behaviorTier;
        };
#pragma pack(pop)
        static_assert(sizeof(Row) == 32, "AnimationData row stride");
    }

    // Item DBC. Several hot consumers read this ID table inline instead of calling the generic accessor.
    namespace item
    {
        constexpr uintptr_t kStorageObject = 0x00AD3D4C;
        constexpr uintptr_t kMaxId         = 0x00AD3D58;
        constexpr uintptr_t kMinId         = 0x00AD3D5C;
        constexpr uintptr_t kIdTable       = 0x00AD3D6C;
        constexpr size_t    kRecordSize    = 32;
    }

    // -------------------------------------------------------------------------
    // Map storage. The engine compacts each on-disk map row into a fixed-size in-memory record and
    // indexes it by (id - minId). The override merges modern map rows into this storage in place.
    // -------------------------------------------------------------------------
    namespace map
    {
        constexpr uintptr_t kStorageLoadedFlag = 0x00AD4164; // u32, set to 1 once the storage is built
        constexpr uintptr_t kRecordCount       = 0x00AD4168; // u32
        constexpr uintptr_t kMaxId             = 0x00AD416C; // i32
        constexpr uintptr_t kMinId             = 0x00AD4170; // i32
        constexpr uintptr_t kRecordData        = 0x00AD417C; // record array base
        constexpr uintptr_t kIdTable           = 0x00AD4180; // record* table, indexed by (id - minId)
    }

    // -------------------------------------------------------------------------
    // Stock data-table definition override targets. A definition declares the storage object + accessor
    // its table replaces; the install reads these. Fields not needed for a table are left 0.
    // -------------------------------------------------------------------------
    namespace mapdef
    {
        constexpr uintptr_t kStorageObject  = 0x00AD4160; // storage instance
        constexpr uintptr_t kRecordCount    = 0x00AD4168; // recordCount field
        constexpr uintptr_t kRecordData     = 0x00AD417C; // recordData pointer field
        constexpr uintptr_t kLoader         = 0x00648510; // on-disk loader (no accessor funnel exists)
        constexpr uintptr_t kPostLoadBuild  = 0x0065E020; // post-load builder (rebuilds min/max + idTable)
        constexpr uintptr_t kIdTable        = 0x00AD4180; // idTable consumers read inline
    }

    namespace charsectionsdef
    {
        constexpr uintptr_t kStorageObject  = 0x00AD332C; // storage instance
        constexpr uintptr_t kRecordCount    = 0x00AD3334; // recordCount field
        constexpr uintptr_t kRecordData     = 0x00AD3348; // recordData pointer field
        constexpr uintptr_t kRecordLookup   = 0x004F3BA0; // consumer accessor (hook point)
        constexpr uintptr_t kCacheBuilder   = 0x004F3DD0; // cache builder
        constexpr uintptr_t kCacheRoot      = 0x00B6B864; // consumer cache root
    }

    // -------------------------------------------------------------------------
    // CreatureModelData DBC. Where a displayed model's FILE lives: one string per row and nothing
    // else that names an asset, which is what makes repointing a model a single pointer write rather
    // than a row rebuild. The client asserts the shape at load and refuses the file otherwise, so a
    // replacement array must keep both numbers.
    // -------------------------------------------------------------------------
    namespace creaturemodeldata
    {
        constexpr uintptr_t kStorageObject = 0x00AD3500; // storage instance (g_creatureModelDataDB)
        constexpr uintptr_t kRecordCount   = 0x00AD3508; // storage +0x08
        constexpr uintptr_t kRecordData    = 0x00AD351C; // storage +0x1C, record array base
        constexpr uint32_t  kColumnCount   = 0x1C;       // 28, asserted by Load
        constexpr uint32_t  kRowSize       = 0x70;       // 112, asserted by Load
        // The row's ONLY string: every other column is a raw int. Read relocates just this one by the
        // string block base, and points it at an empty-string constant when there is no block.
        constexpr size_t    kOffModelName  = 0x08;       // char* model file path
    }

    // -------------------------------------------------------------------------
    // CreatureDisplayInfo DBC. The indirection between a display id and the model row above it.
    // -------------------------------------------------------------------------
    namespace creaturedisplayinfo
    {
        constexpr uintptr_t kStorageObject = 0x00AD34B8; // storage instance (g_creatureDisplayInfoDB)
        constexpr uintptr_t kRecordCount   = 0x00AD34C0; // storage +0x08
        constexpr uintptr_t kRecordData    = 0x00AD34D4; // storage +0x1C, record array base
        constexpr uint32_t  kColumnCount   = 0x10;       // 16, asserted by Load
        constexpr uint32_t  kRowSize       = 0x40;       // 64, asserted by Load
        constexpr size_t    kOffModelId    = 0x04;       // u32 CreatureModelData id
    }

    // -------------------------------------------------------------------------
    // ChrRaces DBC. Compacted storage indexed by (id - minId). The ClientPrefix field at +0x18 is
    // a 4-char race code (e.g. "Hum", "Orc") used to build model paths.
    // -------------------------------------------------------------------------
    namespace chrraces
    {
        constexpr uintptr_t kMinId          = 0x00AD3438; // i32 minimum race id in the table
        constexpr uintptr_t kMaxId          = 0x00AD3434; // i32 maximum race id
        constexpr uintptr_t kIdTable        = 0x00AD3448; // record* table, indexed by (id - minId)
        constexpr size_t    kOffRecordPrefix= 0x18;       // char[4] race client prefix (e.g. "Hum")
    }

    // -------------------------------------------------------------------------
    // ItemDisplayInfo DBC. Looked up by display_id via the funnel accessor at kLookup. The record
    // contains model/texture names, the icon2 extension string, and the particle color id.
    // -------------------------------------------------------------------------
    namespace itemdisplayinfo
    {
        constexpr uintptr_t kStorageObject  = 0x00AD3DDC; // storage instance
        constexpr uintptr_t kMaxId          = 0x00AD3DE8;
        constexpr uintptr_t kMinId          = 0x00AD3DEC;
        constexpr uintptr_t kIdTable        = 0x00AD3DFC;
        // kLookup: thiscall(ecx=storageObj, displayId, outBuf); fills outBuf with the 256-byte
        // record copy (field pointers point into the live DBC string block); returns non-zero if found.
        constexpr uintptr_t kLookup         = 0x004CFD90;
        using LookupFn = uint32_t (__fastcall*)(void* storageObj, void* edx, uint32_t displayId, void* outBuf);

        // Field offsets within the resolved record pointer (byte offsets from the record base).
        constexpr size_t kOffModel1     = 0x04; // char* primary model filename (no path, no extension)
        constexpr size_t kOffModel2     = 0x08; // char* secondary model filename (left/right variant)
        constexpr size_t kOffTex1       = 0x0C; // char* primary texture name
        constexpr size_t kOffTex2       = 0x10; // char* secondary texture name
        constexpr size_t kOffIcon2      = 0x18; // char* icon2 string (consumer-defined convention; raw pointer only)
        constexpr size_t kOffParticleId = 0x60; // uint32 particle color id
        constexpr size_t kRecordSize    = 256;  // byte stride between records in the storage array
    }

    // -------------------------------------------------------------------------
    // Gender strings table. A flat array of char* pointers indexed by gender id (0 = Male, 1 = Female).
    // Used to build gender-specific model paths (e.g. "HumanMale" vs "HumanFemale").
    // -------------------------------------------------------------------------
    namespace genderstrings
    {
        constexpr uintptr_t kTable = 0x00AC46A0; // char*[2] (index 0 = "Male", index 1 = "Female")
    }
}
