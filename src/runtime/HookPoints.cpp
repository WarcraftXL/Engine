// Named hook points: the addresses behind WXL_Api::HookAttachByName, so an extension can attach to
// one without including an offsets/ header itself.
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

#include "runtime/HookPoints.hpp"

#include "common/Log.hpp"
#include "engine/hook/Hook.hpp"

#include "offsets/engine/Boot.hpp"
#include "offsets/engine/Camera.hpp"
#include "offsets/engine/Frame.hpp"
#include "offsets/engine/Gx.hpp"
#include "offsets/engine/Io.hpp"
#include "offsets/engine/Liquid.hpp"
#include "offsets/engine/Lua.hpp"
#include "offsets/engine/Mem.hpp"
#include "offsets/engine/Shader.hpp"
#include "offsets/engine/Sky.hpp"
#include "offsets/engine/Sound.hpp"
#include "offsets/game/ADT.hpp"
#include "offsets/game/DB2.hpp"
#include "offsets/game/Doodad.hpp"
#include "offsets/game/GroundEffect.hpp"
#include "offsets/game/M2.hpp"
#include "offsets/game/Network.hpp"
#include "offsets/game/Unit.hpp"
#include "offsets/game/Weather.hpp"
#include "offsets/game/WMO.hpp"
#include "offsets/game/World.hpp"
#include "offsets/game/WorldMap.hpp"
#include "offsets/game/WorldScene.hpp"

#include <cstring>

namespace wxl::runtime::hookpoints
{
    namespace
    {
        namespace adt    = wxl::offsets::game::adt;
        namespace boot   = wxl::offsets::engine::boot;
        namespace cam    = wxl::offsets::engine::camera;
        namespace db2    = wxl::offsets::game::db2;
        namespace dd     = wxl::offsets::game::doodad;
        namespace frm    = wxl::offsets::engine::frame;
        namespace grass  = wxl::offsets::game::groundeffect;
        namespace gxoff  = wxl::offsets::engine::gx;
        namespace io     = wxl::offsets::engine::io;
        namespace liq    = wxl::offsets::engine::liquid;
        namespace lua    = wxl::offsets::engine::lua;
        namespace mem    = wxl::offsets::engine::mem;
        namespace m2     = wxl::offsets::game::m2;
        namespace net    = wxl::offsets::game::network;
        namespace shoff  = wxl::offsets::engine::shader;
        namespace sky    = wxl::offsets::engine::sky;
        namespace snd    = wxl::offsets::engine::sound;
        namespace unit   = wxl::offsets::game::unit;
        namespace wld    = wxl::offsets::game::world;
        namespace wmo    = wxl::offsets::game::wmo;
        namespace wthr   = wxl::offsets::game::weather;
        namespace wmap   = wxl::offsets::game::worldmap;
        namespace wscene = wxl::offsets::game::worldscene;

        struct Point
        {
            const char* name;
            uintptr_t   address;
        };

        // Every hook point an extension may attach to by name. Add an entry here (and give it a
        // stable name) rather than have the extension include an offsets/ header of its own.
        //
        // A name is part of the ABI: it may be added, never renamed or dropped. Two names may resolve
        // to one address -- the chain is keyed by the address, so both detours land in the same chain.
        // Only function entries belong here; the patch sites, return-address anchors and data globals
        // that also live in offsets/ are not detour targets and are deliberately absent.
        constexpr Point kPoints[] = {
            // --- startup and per-frame anchors -------------------------------------------------------
            { "Boot.EngineInit",                           boot::kEngineInit },
            { "Frame.Pump",                                frm::kFramePump },
            { "Camera.BuildMatrices",                      cam::kBuildCameraMatrices },
            { "Camera.GetActive",                          cam::kGetActiveCamera },
            // Camera and view
            { "Camera.WorldProjectionSetup",               cam::kWorldProjectionSetup },
            { "Camera.ScreenRayBuild",                     cam::kScreenRayBuild },
            { "Camera.UnderwaterCheck",                    cam::kUnderwaterCheck },
            { "Camera.ViewSet",                            cam::kViewSet },
            { "Camera.UpdateCallback",                     cam::kUpdateCallback },

            // --- archive and file I/O ----------------------------------------------------------------
            { "Io.FileOpen",                               io::kFileOpen },
            { "Io.FileOpenAlt",                            io::kFileOpen2 },
            { "Io.FileSize",                               io::kFileSize },
            { "Io.FileRead",                               io::kFileRead },
            { "Io.FileSeek",                               io::kFileSeek },
            { "Io.FileClose",                              io::kFileClose },
            { "Io.FileExists",                             wld::kFileExists },
            { "Io.ArchiveMount",                           io::kArchiveMount },
            { "Io.ArchiveOpen",                            io::kMopaqOpenArchive },
            { "Io.InitializeWowConfig",                    io::kInitializeWowConfig },

            // --- script surface ----------------------------------------------------------------------
            { "Lua.GetContext",                            lua::kFrameScriptGetContext },
            { "Lua.RegisterFunction",                      lua::kFrameScriptRegisterFunction },
            { "Lua.Execute",                               lua::kFrameScriptExecute },
            { "Lua.FillScriptMethodTable",                 lua::kFillScriptMethodTable },
            { "Lua.ValidateFunctionPointer",               lua::kValidateFunctionPointer },
            { "Lua.GetObjectThis",                         lua::kGetObjectThis },

            // --- allocator and caches ----------------------------------------------------------------
            { "Mem.Alloc",                                 mem::kAlloc },
            { "Mem.Free",                                  mem::kFree },
            { "Mem.M2CacheGarbageCollect",                 mem::kM2CacheGarbageCollect },
            { "Mem.M2SceneAdvanceTime",                    mem::kM2SceneAdvanceTime },
            { "Mem.TextureCacheUpdate",                    mem::kTextureCacheUpdate },

            // --- graphics device and textures --------------------------------------------------------
            { "Gx.SetDefaultWindow",                       gxoff::kDeviceSetDefWindow },
            { "Gx.SceneClear",                             gxoff::kGxSceneClear },
            { "Gx.ShaderUpdateProjMatrix",                 gxoff::kShaderUpdateProjMatrix },
            { "Gx.WorldOnRender",                          gxoff::kWorldOnRender },
            { "Gx.WorldRenderFinalize",                    gxoff::kWorldRenderFinalize },
            { "Gx.GlueModelRender",                        gxoff::kSimpleModelFFXRender },
            { "Gx.GlueModelRegisterMethods",               gxoff::kSimpleModelRegisterMethods },
            { "Gx.TextureCreate",                          gxoff::kTextureCreate },
            { "Gx.TextureUpdate",                          gxoff::kTextureUpdate },
            { "Gx.TextureResolve",                         adt::kTexResolve },
            { "Gx.TextureRelease",                         adt::kTextureRelease },
            { "Gx.TexSetWrap",                             gxoff::kGxTexSetWrap },
            { "Gx.StateSet",                               shoff::kGxStateSet },
            { "Gx.StateDirty",                             shoff::kGxStateDirty },
            { "Gx.RenderStateSet",                         adt::kGxRsSetInt },
            { "Gx.LiquidRenderPass",                       gxoff::kLiquidRenderPass },

            // --- programmable shader path ------------------------------------------------------------
            { "Shader.EffectActivate",                     shoff::kEffectActivate },
            { "Shader.EffectBind",                         shoff::kEffectBind },
            { "Shader.OutdoorIndexDriver",                 shoff::kOutdoorIndexDriver },
            { "Shader.PixelIndexDriver",                   shoff::kPixelIndexDriver },
            { "Shader.NativeBlsLoad",                      shoff::kNativeBlsLoad },
            { "Shader.CreateVertex",                       shoff::kShaderCreateVertex },
            { "Shader.ConstantsSet",                       shoff::kShaderConstantsSet },
            { "Shader.TerrainPermutationIndex",            shoff::kTerrainShaderPermutationIndex },
            { "Shader.TerrainSelect",                      shoff::kTerrainShaderSelect },
            { "Shader.ShadowTierGet",                      adt::kShadowTierGetter },

            // --- sky, day-night, fog and outdoor light -----------------------------------------------
            { "Sky.CloudsGenerate",                        sky::kCloudsGenerate },
            { "Sky.DayNightGetInfo",                       sky::kDayNightGetInfo },
            { "Sky.SetOverrideFog",                        sky::kSetOverrideFog },
            { "Sky.ClearOverrideFog",                      sky::kClearOverrideFog },
            // Day-night, fog and outdoor light
            { "Sky.Update",                                sky::kUpdate },
            { "Sky.FogRateCompute",                        sky::kFogRateCompute },
            { "Sky.FogUpdate",                             sky::kFogUpdate },
            { "Sky.Initialize",                            sky::kInitialize },
            { "Sky.SkyOverrideResolve",                    sky::kSkyOverrideResolve },
            { "Sky.ColorsResolve",                         sky::kColorsResolve },
            { "Sky.LightingUpdate",                        sky::kLightingUpdate },
            // Map lifecycle: load / unload / enter / leave
            { "Sky.MapUnload",                             sky::kMapUnload },

            // --- sound -------------------------------------------------------------------------------
            { "Sound.SetMasterVolume",                     snd::kSetMasterVolume },
            { "Sound.PlaySound",                           snd::kPlaySound },
            { "Sound.PlaySoundKit",                        snd::kPlaySoundKit },

            // --- liquid ------------------------------------------------------------------------------
            { "Liquid.MaterialWaterRender",                liq::kMaterialWaterRender },
            { "Liquid.MaterialWaterNoSpecRender",          liq::kMaterialWaterNoSpecRender },
            { "Liquid.MaterialBankGetMaterial",            liq::kMaterialBankGetMaterial },
            { "Liquid.ChunkGeomGetBuffers",                liq::kChunkGeomGetBuffers },
            { "Liquid.MeshGeomGetBuffers",                 liq::kMeshGeomGetBuffers },
            { "Liquid.MapChunkBufAlloc",                   liq::kMapChunkBufAlloc },
            { "Liquid.ChunkVertexEmit",                    liq::kChunkVertexEmit },
            { "Liquid.ConstantUpload",                     liq::kConstantUpload },
            { "Liquid.SettingsAnimTexture",                liq::kSettingsAnimTexture },
            { "Liquid.GxPoolCreate",                       liq::kGxPoolCreate },
            { "Liquid.GxBufCreate",                        liq::kGxBufCreate },
            { "Liquid.GxBufSizeSet",                       liq::kGxBufSizeSet },
            { "Liquid.MapGetHeightTerrain",                liq::kMapGetHeightTerrain },

            // --- world tick, map lifecycle, streaming and picking ------------------------------------
            { "World.Tick",                                wld::kTick },
            { "World.Enter",                               wld::kEnter },
            { "World.MapEnter",                            wld::kMapEnter },
            { "World.PrepareUpdate",                       wld::kPrepareUpdate },
            { "World.Render",                              wld::kRender },
            { "World.StreamingTick",                       wld::kStreamingTick },
            { "World.PurgeAllTiles",                       wld::kPurgeAllTiles },
            { "World.TileFactory",                         wld::kTileFactory },
            { "World.TileLoader",                          wld::kTileLoader },
            { "World.TileUnload",                          wld::kTileUnload },
            { "World.TileDestroy",                         wld::kTileDestroy },
            { "World.SetFarClip",                          wld::kSetFarClip },
            { "World.ValidateFarClip",                     wld::kValidateFarClip },
            { "World.GetScreenCoordinates",                wld::kGetScreenCoordinates },
            { "World.PickAtScreen",                        wld::kPickAtScreen },
            { "World.ScreenToRay",                         wld::kScreenToRay },
            { "World.IntersectWrapper",                    wld::kIntersectWrapper },
            { "World.Intersect",                           wld::kWorldIntersect },
            { "World.AsyncFileReadInitialize",             wld::kAsyncFileReadInitialize },
            { "World.AsyncFileReadObject",                 wld::kAsyncFileReadObject },
            { "World.AsyncFileReadAllocObject",            wld::kAsyncFileReadAllocObject },
            { "World.AsyncFileReadLinkObject",             wld::kAsyncFileReadLinkObject },
            { "World.AsyncFileReadWait",                   wld::kAsyncFileReadWait },
            { "World.AsyncWaitAll",                        wld::kAsyncWaitAll },
            { "World.AsyncPending",                        wld::kAsyncPending },
            { "World.AsyncServiceQueues",                  wld::kAsyncServiceQueues },
            { "World.AsyncDestroy",                        wld::kAsyncDestroy },
            { "World.AsyncQueueAlloc",                     wld::kAsyncQueueAlloc },
            { "World.AsyncThreadWrap",                     wld::kAsyncThreadWrap },
            // Area / zone resolution
            { "World.OutdoorsQuery",                       wld::kOutdoorsQuery },
            { "World.MapAreaIdQuery",                      wld::kMapAreaIdQuery },
            // Async file-read queues
            { "World.AsyncStatusHandlerAdd",               wld::kAsyncStatusHandlerAdd },
            { "World.AsyncRequestCancel",                  wld::kAsyncRequestCancel },
            { "World.AsyncShutdown",                       wld::kAsyncShutdown },
            // Camera and view
            { "World.FrameCameraPositionGet",              wld::kFrameCameraPositionGet },
            { "World.FrameCameraTargetSet",                wld::kFrameCameraTargetSet },
            // Cursor picking, ray intersection and collision queries
            { "World.FrameTerrainTrack",                   wld::kFrameTerrainTrack },
            { "World.FrameMouseDown",                      wld::kFrameMouseDown },
            { "World.FrameSelectionLegalTest",             wld::kFrameSelectionLegalTest },
            { "World.FrameDefaultActionPerform",           wld::kFrameDefaultActionPerform },
            { "World.FrameClosestModelFind",               wld::kFrameClosestModelFind },
            { "World.FrameHitTestEnumProc",                wld::kFrameHitTestEnumProc },
            { "World.MapLiquidSoundQuery",                 wld::kMapLiquidSoundQuery },
            { "World.MapRayIntersect",                     wld::kMapRayIntersect },
            { "World.MapFacetQuery",                       wld::kMapFacetQuery },
            // Loading gate and loading screen
            { "World.LoadingScreenDisable",                wld::kLoadingScreenDisable },
            { "World.LoadingScreenEnable",                 wld::kLoadingScreenEnable },
            { "World.LoadProgressCallbackSet",             wld::kLoadProgressCallbackSet },
            // Map lifecycle: load / unload / enter / leave
            { "World.Initialize",                          wld::kInitialize },
            { "World.MapLoadBegin",                        wld::kMapLoadBegin },
            { "World.MapUnload",                           wld::kMapUnload },
            { "World.Shutdown",                            wld::kShutdown },
            { "World.MapInitialize",                       wld::kMapInitialize },
            { "World.MapDestroy",                          wld::kMapDestroy },
            { "World.MapPreload",                          wld::kMapPreload },
            { "World.MapWdtParse",                         wld::kMapWdtParse },
            { "World.MapUnloadTiles",                      wld::kMapUnloadTiles },
            // Terrain tile streaming and the tile grid
            { "World.MapFileOpen",                         wld::kMapFileOpen },
            { "World.MapFileRead",                         wld::kMapFileRead },
            { "World.MapTileAlloc",                        wld::kMapTileAlloc },
            // World frame update chain
            { "World.FrameDayNightUpdate",                 wld::kFrameDayNightUpdate },
            { "World.FramePerFrameTick",                   wld::kFramePerFrameTick },
            { "World.FrameObjectUpdate",                   wld::kFrameObjectUpdate },
            { "World.FrameLayerUpdate",                    wld::kFrameLayerUpdate },
            { "World.FrameWorldUpdate",                    wld::kFrameWorldUpdate },
            { "World.FrameFrameRender",                    wld::kFrameFrameRender },
            // World object residency
            { "World.ObjectDestroy",                       wld::kObjectDestroy },
            // World render parameters and screen effects
            { "World.FrameScreenEffectSet",                wld::kFrameScreenEffectSet },
            { "World.FrameScreenEffectUpdate",             wld::kFrameScreenEffectUpdate },
            { "World.DetailDoodadDistSet",                 wld::kDetailDoodadDistSet },
            { "World.ParamDefaultsApply",                  wld::kParamDefaultsApply },
            { "World.ParamInitialize",                     wld::kParamInitialize },
            // World scene and view setup
            { "World.NdcClip",                             wld::kNdcClip },
            // World tick and per-frame update
            { "World.AreaOfInterestPrepare",               wld::kAreaOfInterestPrepare },
            { "World.Update",                              wld::kUpdate },
            { "World.MapEntityUpdate",                     wld::kMapEntityUpdate },

            // --- terrain -----------------------------------------------------------------------------
            { "Adt.GetChunk",                              adt::kGetChunk },
            { "Adt.TileAreaLoad",                          adt::kTileAreaLoad },
            { "Adt.TileAreaCreate",                        adt::kTileAreaCreate },
            { "Adt.TileAreaAsyncLoadCallback",             adt::kTileAreaAsyncLoadCallback },
            { "Adt.TileAreaDestroy",                       adt::kTileAreaDestroy },
            { "Adt.PrepareChunk",                          adt::kPrepareChunk },
            { "Adt.ProcessIffChunks",                      adt::kChunkProcessIffChunks },
            { "Adt.ChunkBuild",                            adt::kChunkBuild },
            { "Adt.ChunkFrustumCull",                      adt::kChunkFrustumCull },
            { "Adt.NearObjectCount",                       adt::kNearObjectCount },
            { "Adt.AreaUpdate",                            adt::kAreaUpdate },
            { "Adt.AllocRawAreaData",                      adt::kAllocRawAreaData },
            { "Adt.FreeRawAreaData",                       adt::kFreeRawAreaData },
            { "Adt.LoadWdl",                               adt::kLoadWdl },
            { "Adt.AllocAreaLow",                          adt::kAllocAreaLow },
            { "Adt.FreeAreaLow",                           adt::kFreeAreaLow },
            { "Adt.AreaLoadTextures",                      adt::kAreaLoadTextures },
            { "Adt.LoadTerrainTexture",                    adt::kLazyLoadTexSlot },
            { "Adt.MapLoadTexture",                        adt::kMapLoadTexture },
            { "Adt.AllocTerrainTexture",                   adt::kAllocTerrainTexture },
            { "Adt.BuildLayerAlpha",                       adt::kBuildLayerAlpha },
            { "Adt.SurfaceChunkDraw",                      adt::kSurfaceChunkDraw },
            { "Adt.SurfaceChunkDrawShader",                adt::kSurfaceChunkDrawShader },
            { "Adt.BuildTerrainConstants",                 adt::kBuildTerrainConstants },
            // Chunk visibility and the low-detail (WDL) horizon
            { "Adt.UpdateViewerLiquid",                    adt::kUpdateViewerLiquid },
            { "Adt.CullHorizon",                           adt::kCullHorizon },
            { "Adt.CullLiquidChunks",                      adt::kCullLiquidChunks },
            { "Adt.CullChunks",                            adt::kCullChunks },
            // Ground effects / detail doodads
            { "Adt.RenderDetailDoodads",                   adt::kRenderDetailDoodads },
            { "Adt.CreateDetailDoodadAlphaRamp",           adt::kCreateDetailDoodadAlphaRamp },
            { "Adt.DestroyDetailDoodadModels",             adt::kDestroyDetailDoodadModels },
            { "Adt.InitDetailDoodads",                     adt::kInitDetailDoodads },
            { "Adt.UpdateDetailDoodadPools",               adt::kUpdateDetailDoodadPools },
            { "Adt.SetupDetailDoodadRenderState",          adt::kSetupDetailDoodadRenderState },
            { "Adt.LoadDetailDoodadData",                  adt::kLoadDetailDoodadData },
            { "Adt.AddDetailDoodadInstance",               adt::kAddDetailDoodadInstance },
            { "Adt.ResolveDetailDoodadModel",              adt::kResolveDetailDoodadModel },
            { "Adt.LoadChunkDetailDoodadModels",           adt::kLoadChunkDetailDoodadModels },
            { "Adt.BuildChunkDetailDoodads",               adt::kBuildChunkDetailDoodads },
            { "Adt.EnsureChunkDetailDoodadInst",           adt::kEnsureChunkDetailDoodadInst },
            // Per-chunk terrain draw and the terrain render passes
            { "Adt.RenderChunksSolid",                     adt::kRenderChunksSolid },
            { "Adt.RenderChunksZoneDebug",                 adt::kRenderChunksZoneDebug },
            { "Adt.RenderChunksSinglePass",                adt::kRenderChunksSinglePass },
            { "Adt.RenderChunks",                          adt::kRenderChunks },
            { "Adt.InitTerrainVertexShaderConstants",      adt::kInitTerrainVertexShaderConstants },
            { "Adt.PrepareChunkBuffers",                   adt::kPrepareChunkBuffers },
            { "Adt.UseStreamingChunkBuffers",              adt::kUseStreamingChunkBuffers },
            { "Adt.SetupChunkRender",                      adt::kSetupChunkRender },
            { "Adt.DrawChunkSolid",                        adt::kDrawChunkSolid },
            { "Adt.DrawChunkSolidShader",                  adt::kDrawChunkSolidShader },
            { "Adt.PrepareChunkRender",                    adt::kPrepareChunkRender },
            // Terrain CVar callbacks (callback-slot targets, no CALL xrefs by design)
            { "Adt.TerrainLodCallback",                    adt::kTerrainLodCallback },
            { "Adt.TerrainShadowsCallback",                adt::kTerrainShadowsCallback },
            { "Adt.TerrainAlphaBitDepthCallback",          adt::kTerrainAlphaBitDepthCallback },
            { "Adt.GroundEffectDensityCallback",           adt::kGroundEffectDensityCallback },
            { "Adt.GroundEffectDistCallback",              adt::kGroundEffectDistCallback },
            // Terrain chunk build (MCVT / MCNR / MCLY / MCRF / MCSE / index+vertex buffers)
            { "Adt.PurgeChunk",                            adt::kPurgeChunk },
            { "Adt.BuildChunkIndices",                     adt::kBuildChunkIndices },
            { "Adt.InitChunkVertexTable",                  adt::kInitChunkVertexTable },
            { "Adt.InitTerrainChunkSystem",                adt::kInitTerrainChunkSystem },
            { "Adt.BuildChunkVerticesWorldHigh",           adt::kBuildChunkVerticesWorldHigh },
            { "Adt.BuildChunkVerticesWorldLow",            adt::kBuildChunkVerticesWorldLow },
            { "Adt.BuildChunkVerticesLocalHigh",           adt::kBuildChunkVerticesLocalHigh },
            { "Adt.BuildChunkVerticesLocalLow",            adt::kBuildChunkVerticesLocalLow },
            { "Adt.BuildChunkIndexRange",                  adt::kBuildChunkIndexRange },
            { "Adt.BuildChunkBounds",                      adt::kBuildChunkBounds },
            { "Adt.BuildChunkVertices",                    adt::kBuildChunkVertices },
            { "Adt.BuildChunkLiquids",                     adt::kBuildChunkLiquids },
            { "Adt.ConstructChunk",                        adt::kConstructChunk },
            { "Adt.DestroyChunk",                          adt::kDestroyChunk },
            { "Adt.BuildChunkSoundEmitters",               adt::kBuildChunkSoundEmitters },
            { "Adt.BuildChunkRefs",                        adt::kBuildChunkRefs },
            { "Adt.UpdateChunkLights",                     adt::kUpdateChunkLights },
            // Terrain liquid geometry (MCLQ / MH2O instances)
            { "Adt.UpdateChunkLiquidPurge",                adt::kUpdateChunkLiquidPurge },
            { "Adt.GetChunkLiquidBounds",                  adt::kGetChunkLiquidBounds },
            { "Adt.BuildChunkLiquidVertexGrid",            adt::kBuildChunkLiquidVertexGrid },
            { "Adt.GetChunkLiquidSurfaceHeight",           adt::kGetChunkLiquidSurfaceHeight },
            { "Adt.ChunkLiquidTileExists",                 adt::kChunkLiquidTileExists },
            { "Adt.GetChunkLiquidTris",                    adt::kGetChunkLiquidTris },
            { "Adt.ConstructChunkLiquid",                  adt::kConstructChunkLiquid },
            { "Adt.BatchChunkLiquid",                      adt::kBatchChunkLiquid },
            { "Adt.FreeChunkBuffer",                       adt::kFreeChunkBuffer },
            { "Adt.PrepareChunkLiquidRender",              adt::kPrepareChunkLiquidRender },
            // Terrain queries: area id, ground type, shadow, liquid, height and collision
            { "Adt.QueryChunkLiquidSounds",                adt::kQueryChunkLiquidSounds },
            { "Adt.QueryChunkAreaId",                      adt::kQueryChunkAreaId },
            { "Adt.QueryGroundType",                       adt::kQueryGroundType },
            { "Adt.QueryTerrainShadow",                    adt::kQueryTerrainShadow },
            { "Adt.QueryTerrainLiquid",                    adt::kQueryTerrainLiquid },
            { "Adt.QueryLiquidStatus",                     adt::kQueryLiquidStatus },
            { "Adt.IntersectTerrain",                      adt::kIntersectTerrain },
            { "Adt.GetSubChunkTri",                        adt::kGetSubChunkTri },
            { "Adt.GetChunkTris",                          adt::kGetChunkTris },
            { "Adt.GetTerrainTris",                        adt::kGetTerrainTris },
            { "Adt.SnapObjectToSubChunk",                  adt::kSnapObjectToSubChunk },
            { "Adt.LinkEntityGetChunk",                    adt::kLinkEntityGetChunk },
            { "Adt.IntersectChunkBox",                     adt::kIntersectChunkBox },
            { "Adt.IntersectChunkRay",                     adt::kIntersectChunkRay },
            { "Adt.IntersectChunkVolume",                  adt::kIntersectChunkVolume },
            // Terrain shadow map bind
            { "Adt.BindTerrainShadowMap",                  adt::kBindTerrainShadowMap },
            // Texture layers, alpha maps and terrain shadow maps (render-chunk side)
            { "Adt.FreeChunkLayers",                       adt::kFreeChunkLayers },
            { "Adt.UpdateChunkLayersLoaded",               adt::kUpdateChunkLayersLoaded },
            { "Adt.SelectChunkLights",                     adt::kSelectChunkLights },
            { "Adt.UnpackLayerAlphaShadowBits",            adt::kUnpackLayerAlphaShadowBits },
            { "Adt.UnpackLayerAlphaBits",                  adt::kUnpackLayerAlphaBits },
            { "Adt.CreateChunkLayer",                      adt::kCreateChunkLayer },
            { "Adt.ConstructRenderChunk",                  adt::kConstructRenderChunk },
            { "Adt.CreateChunkLayers",                     adt::kCreateChunkLayers },
            { "Adt.FreeRenderChunkBuf",                    adt::kFreeRenderChunkBuf },
            { "Adt.CreateChunkLayerTexture",               adt::kCreateChunkLayerTexture },
            { "Adt.UnpackChunkShadowBits",                 adt::kUnpackChunkShadowBits },
            { "Adt.CreateChunkShaderTexture",              adt::kCreateChunkShaderTexture },
            { "Adt.DestroyRenderChunk",                    adt::kDestroyRenderChunk },
            { "Adt.AllocChunkShadowTexture",               adt::kAllocChunkShadowTexture },
            { "Adt.AllocChunkShaderTexture",               adt::kAllocChunkShaderTexture },
            { "Adt.AllocChunkLayerTextures",               adt::kAllocChunkLayerTextures },
            { "Adt.InitRenderChunkSystem",                 adt::kInitRenderChunkSystem },
            { "Adt.DestroyRenderChunkSystem",              adt::kDestroyRenderChunkSystem },
            { "Adt.UpdateRenderChunkPools",                adt::kUpdateRenderChunkPools },
            // Tile-area lifecycle, pooling and the per-frame terrain update lists
            { "Adt.ClearChunkDetailDoodads",               adt::kClearChunkDetailDoodads },
            { "Adt.ClearChunkBufs",                        adt::kClearChunkBufs },
            { "Adt.UpdateTileArea",                        adt::kUpdateTileArea },
            { "Adt.ProcessChunkLiquidUpdates",             adt::kProcessChunkLiquidUpdates },
            { "Adt.ProcessDetailDoodadUpdates",            adt::kProcessDetailDoodadUpdates },
            { "Adt.ProcessRenderChunkUpdates",             adt::kProcessRenderChunkUpdates },
            { "Adt.FreeChunk",                             adt::kFreeChunk },
            { "Adt.FreeChunkLiquid",                       adt::kFreeChunkLiquid },
            { "Adt.ConstructAreaLow",                      adt::kConstructAreaLow },
            { "Adt.AllocChunk",                            adt::kAllocChunk },
            { "Adt.AllocChunkLiquid",                      adt::kAllocChunkLiquid },
            { "Adt.CancelTileAreaLoad",                    adt::kCancelTileAreaLoad },
            { "Adt.PurgeTileAreaChunks",                   adt::kPurgeTileAreaChunks },
            { "Adt.ConstructTileArea",                     adt::kConstructTileArea },

            // --- placed doodads and grass ------------------------------------------------------------
            { "Doodad.SpawnFromMddf",                      dd::kSpawnFromMDDF },
            { "Doodad.Purge",                              dd::kDoodadPurge },
            { "Grass.ChunkConstantUpload",                 grass::kChunkConstantUpload },
            { "Grass.InitShaderConstants",                 grass::kInitShaderConstants },

            // --- objects and units -------------------------------------------------------------------
            { "Unit.GetObjectByGuid",                      unit::kGetObjectByGuid },
            { "Unit.EnumObjects",                          unit::kEnumObjects },
            { "Unit.GetActivePlayerGuid",                  unit::kActivePlayerGuid },
            { "Unit.Reaction",                             unit::kUnitReaction },
            { "Unit.ObjectUpdate",                         unit::kObjectUpdateHandler },
            { "Unit.ObjectDestroy",                        unit::kObjectDestroyHandler },
            { "Unit.TargetSet",                            unit::kTargetSet },
            { "Unit.ResolveModelAnimation",                unit::kResolveModelAnimation },
            { "Network.ProcessMessage",                    net::kProcessMessage },

            // --- client data tables ------------------------------------------------------------------
            { "Db2.MapLoad",                               db2::mapdef::kLoader },
            { "Db2.MapPostLoadBuild",                      db2::mapdef::kPostLoadBuild },
            { "Db2.CharSectionsLookup",                    db2::charsectionsdef::kRecordLookup },
            { "Db2.CharSectionsCacheBuild",                db2::charsectionsdef::kCacheBuilder },
            { "Db2.ItemDisplayInfoLookup",                 db2::itemdisplayinfo::kLookup },

            // --- M2 models ---------------------------------------------------------------------------
            { "M2.Init",                                   m2::kInit },
            { "M2.SharedInitialize",                       m2::kSharedInitialize },
            { "M2.InitializeLoaded",                       m2::kInitializeLoaded },
            { "M2.ReadVertices",                           m2::kReadVertices },
            { "M2.ReadByteArray",                          m2::kReadByteArray },
            { "M2.ReadVector3",                            m2::kReadVector3 },
            { "M2.ReadInt32Array",                         m2::kReadInt32Array },
            { "M2.ReadInt16Array",                         m2::kReadInt16Array },
            { "M2.ReadAnimations",                         m2::kReadAnimations },
            { "M2.ReadTextures",                           m2::kReadTextures },
            { "M2.ReadEvents",                             m2::kReadEvents },
            { "M2.ReadColors",                             m2::kReadColors },
            { "M2.ReadTransparency",                       m2::kReadTransparency },
            { "M2.ReadBones",                              m2::kReadBones },
            { "M2.ReadUVAnimation",                        m2::kReadUVAnimation },
            { "M2.ReadAttachments",                        m2::kReadAttachments },
            { "M2.ReadLights",                             m2::kReadLights },
            { "M2.ReadCameras",                            m2::kReadCameras },
            { "M2.ReadParticleEmitters",                   m2::kReadParticleEmitters },
            { "M2.RibbonDeRelocate",                       m2::kRibbonDeRelocate },
            { "M2.BuildSkinPath",                          m2::kBuildSkinPath },
            { "M2.LoadSkinProfile",                        m2::kLoadSkinProfile },
            { "M2.FinishLoadingSkinProfile",               m2::kFinishLoadingSkinProfile },
            { "M2.FinalizeSkin",                           m2::kFinalizeSkin },
            { "M2.BuildBatchMaterial",                     m2::kBuildBatchMaterial },
            { "M2.BuildAnimPath",                          m2::kBuildAnimPath },
            { "M2.SequenceLoad",                           m2::kSequenceLoad },
            { "M2.PerSeqDeReloc",                          m2::kPerSeqDeReloc },
            { "M2.AnimLoadComplete",                       m2::kAnimLoadComplete },
            { "M2.BufferAlloc",                            m2::kBufferAlloc },
            { "M2.BufferFree",                             m2::kBufferFree },
            { "M2.CreateSceneModel",                       m2::kCreateSceneModel },
            { "M2.AttachToScene",                          m2::kAttachToScene },
            { "M2.DetachSlot",                             m2::kDetachSlot },
            { "M2.ReleaseRenderCtx",                       m2::kReleaseRenderCtx },
            { "M2.BindTexSlot",                            m2::kBindTexSlot },
            { "M2.SetSequenceCallback",                    m2::kSetSequenceCallback },
            { "M2.SetEventCallback",                       m2::kSetEventCallback },
            { "M2.SetBoneSequence",                        m2::kSetBoneSequence },
            { "M2.CharModelSlotDispatch",                  m2::kCharModelSlotDispatch },
            { "M2.CharModelSlotClear",                     m2::kCharModelSlotClear },
            { "M2.ModelLightingCallback",                  m2::kModelLightingCallback },
            { "M2.PerFrameUpdate",                         m2::kM2PerFrameUpdate },
            { "M2.HasPlayableAnimation",                   m2::kModelHasPlayableAnimation },
            { "M2.FindPlayableAnimation",                  m2::kModelFindPlayableAnimation },
            { "M2.CacheBeginThread",                       m2::kCacheBeginThread },
            { "M2.CacheWaitThread",                        m2::kCacheWaitThread },
            { "M2.TrackEvalVec3",                          m2::kTrackEvalVec3 },
            { "M2.TrackEvalQuat",                          m2::kTrackEvalQuat },
            { "M2.BuildBonePalette",                       m2::kBuildBonePalette },
            { "M2.BuildBonePaletteSimple",                 m2::kBuildBonePaletteSimple },
            { "M2.IsDrawable",                             m2::kIsDrawable },
            { "M2.IsBatchDoodadCompatible",                m2::kIsBatchDoodadCompatible },
            { "M2.SetupMaterial",                          m2::kSetupMaterial },
            { "M2.PushAlphaRef",                           m2::kPushAlphaRef },
            { "M2.ShaderConstUnlock",                      m2::kShaderConstUnlock },
            { "M2.SortOpaqueGeoBatches",                   m2::kSortOpaqueGeoBatches },
            { "M2.DrawBatch",                              gxoff::kDrawTriangleBatch },
            { "M2.DrawBatchDoodad",                        gxoff::kDrawBatchDoodad },
            { "M2.ProjectedDecalDraw",                     m2::kProjectedDecalDraw },
            { "M2.ShadowMapBatches",                       m2::kShadowMapBatches },
            { "M2.RenderBatchShadowMap",                   m2::kRenderBatchShadowMap },
            { "M2.RibbonDraw",                             m2::kRibbonDraw },
            { "M2.SceneTriangleHitTest",                   m2::kSceneTriangleHitTest },
            { "M2.AnimateParticlesMT",                     m2::kAnimateParticlesMT },
            { "M2.EmitNewParticles",                       m2::kEmitNewParticles },
            { "M2.EmitterSync",                            m2::kEmitterSync },
            { "M2.EmitterSyncAllocation",                  m2::kEmitterSyncAllocation },
            { "M2.ParticleSetZSource",                     m2::kSetZsource },
            // character component: geosets, item slots, baked textures
            { "M2.CharValidateComponentData",              m2::kCharValidateComponentData },
            { "M2.CharLoadBaseVariation",                  m2::kCharLoadBaseVariation },
            { "M2.CharSetSkinColor",                       m2::kCharSetSkinColor },
            { "M2.CharAddAttachmentLink",                  m2::kCharAddAttachmentLink },
            { "M2.CharAddHandItem",                        m2::kCharAddHandItem },
            { "M2.CharApplyGuildColor",                    m2::kCharApplyGuildColor },
            { "M2.CharGeosetRenderPrep",                   m2::kCharGeosetRenderPrep },
            { "M2.CharRemoveItem",                         m2::kCharRemoveItem },
            { "M2.CharCreateBaseTexture",                  m2::kCharCreateBaseTexture },
            { "M2.CharAllocComponent",                     m2::kCharAllocComponent },
            { "M2.CharRenderPrep",                         m2::kCharRenderPrep },
            { "M2.CharFreeComponent",                      m2::kCharFreeComponent },
            { "M2.CharReplaceMonsterSkin",                 m2::kCharReplaceMonsterSkin },
            { "M2.CharInit",                               m2::kCharInit },
            { "M2.CharAddItemBySlot",                      m2::kCharAddItemBySlot },
            // header array de-relocation
            { "M2.ReadSkinTextureUnits",                   m2::kReadSkinTextureUnits },
            // model cache and its worker thread
            { "M2.CacheInitialize",                        m2::kCacheInitialize },
            { "M2.CacheCreateShared",                      m2::kCacheCreateShared },
            { "M2.CacheUpdateShared",                      m2::kCacheUpdateShared },
            // other model entries
            { "M2.LinkLight",                              m2::kLinkLight },
            { "M2.AddDiffuseLight",                        m2::kAddDiffuseLight },
            { "M2.AddLight",                               m2::kAddLight },
            { "M2.LightingToCameraSpace",                  m2::kLightingToCameraSpace },
            { "M2.SetupSunlight",                          m2::kSetupSunlight },
            { "M2.SetupGxLights",                          m2::kSetupGxLights },
            { "M2.GetSunLight",                            m2::kGetSunLight },
            { "M2.SetupGxFog",                             m2::kSetupGxFog },
            { "M2.ParticleSystemFlush",                    m2::kParticleSystemFlush },
            // particle emitters
            { "M2.ParticleMoveOne",                        m2::kParticleMoveOne },
            { "M2.ParticleInterpolateTracks",              m2::kParticleInterpolateTracks },
            { "M2.ParticleRenderPrep",                     m2::kParticleRenderPrep },
            { "M2.ParticleEmitVertices",                   m2::kParticleEmitVertices },
            { "M2.ParticleRenderSingle",                   m2::kParticleRenderSingle },
            { "M2.ParticleSetColors",                      m2::kParticleSetColors },
            { "M2.ParticleInternalUpdate",                 m2::kParticleInternalUpdate },
            { "M2.ParticleChangeFrameOfReference",         m2::kParticleChangeFrameOfReference },
            { "M2.ParticleBuildVertex",                    m2::kParticleBuildVertex },
            { "M2.ParticleDetermineIfSimple",              m2::kParticleDetermineIfSimple },
            { "M2.ParticleStepUpdate",                     m2::kParticleStepUpdate },
            { "M2.ParticleBuildVertexBuffer",              m2::kParticleBuildVertexBuffer },
            { "M2.ParticleRenderBatch",                    m2::kParticleRenderBatch },
            { "M2.ParticlePlaceModels",                    m2::kParticlePlaceModels },
            { "M2.ParticleEmitterUpdate",                  m2::kParticleEmitterUpdate },
            // per-instance model state
            { "M2.GetBoundingBox",                         m2::kGetBoundingBox },
            { "M2.CanUpdateNonReadyTextures",              m2::kCanUpdateNonReadyTextures },
            { "M2.WaitForLoad",                            m2::kWaitForLoad },
            { "M2.SetAnimating",                           m2::kSetAnimating },
            { "M2.GetCameraByIndex",                       m2::kGetCameraByIndex },
            { "M2.SetRibbonsEnabled",                      m2::kSetRibbonsEnabled },
            { "M2.ChangeFrameOfReference",                 m2::kChangeFrameOfReference },
            { "M2.ComputeLoadProgress",                    m2::kComputeLoadProgress },
            { "M2.FreeExternalResources",                  m2::kFreeExternalResources },
            { "M2.IsLoaded",                               m2::kIsLoaded },
            { "M2.UpdateLoadedState",                      m2::kUpdateLoadedState },
            { "M2.SetWorldTransformSimple",                m2::kSetWorldTransformSimple },
            { "M2.ReplaceParticleColor",                   m2::kReplaceParticleColor },
            { "M2.GetCurrentBoundingBox",                  m2::kGetCurrentBoundingBox },
            { "M2.GetSplitBodyBoundingBox",                m2::kGetSplitBodyBoundingBox },
            { "M2.UnoptimizeVisibleGeometry",              m2::kUnoptimizeVisibleGeometry },
            { "M2.HasSequence",                            m2::kHasSequence },
            { "M2.ResolveSequenceFallback",                m2::kResolveSequenceFallback },
            { "M2.SetBoneFlags",                           m2::kSetBoneFlags },
            { "M2.GetBoneSequenceInfo",                    m2::kGetBoneSequenceInfo },
            { "M2.GetBoneSequenceId",                      m2::kGetBoneSequenceId },
            { "M2.OnSequenceInterrupted",                  m2::kOnSequenceInterrupted },
            { "M2.SetupBoneSequence",                      m2::kSetupBoneSequence },
            { "M2.SetPrimaryBoneSequence",                 m2::kSetPrimaryBoneSequence },
            { "M2.SetSecondaryBoneSequence",               m2::kSetSecondaryBoneSequence },
            { "M2.SetBoneSequenceTime",                    m2::kSetBoneSequenceTime },
            { "M2.LoadSequenceOnDemand",                   m2::kLoadSequenceOnDemand },
            { "M2.SetBoneProceduralTransform",             m2::kSetBoneProceduralTransform },
            { "M2.HasAttachment",                          m2::kHasAttachment },
            { "M2.GetAttachmentPivot",                     m2::kGetAttachmentPivot },
            { "M2.DetachFromParent",                       m2::kDetachFromParent },
            { "M2.HasEvent",                               m2::kHasEvent },
            { "M2.SetEmittersEnabled",                     m2::kSetEmittersEnabled },
            { "M2.CountGeometryBatches",                   m2::kCountGeometryBatches },
            { "M2.FindTrackKey",                           m2::kFindTrackKey },
            { "M2.SetModelIndices",                        m2::kSetModelIndices },
            { "M2.SetModelVertices",                       m2::kSetModelVertices },
            { "M2.GetRegionBounds",                        m2::kGetRegionBounds },
            { "M2.RenderBatchListShadowMap",               m2::kRenderBatchListShadowMap },
            { "M2.TransformVertices",                      m2::kTransformVertices },
            { "M2.ModelConstruct",                         m2::kModelConstruct },
            { "M2.FreeInternalResources",                  m2::kFreeInternalResources },
            { "M2.GetCurrentBoundingSphere",               m2::kGetCurrentBoundingSphere },
            { "M2.SetGeometryVisible",                     m2::kSetGeometryVisible },
            { "M2.OptimizeVisibleGeometry",                m2::kOptimizeVisibleGeometry },
            { "M2.GetSequenceInfo",                        m2::kGetSequenceInfo },
            { "M2.CountBatches",                           m2::kCountBatches },
            { "M2.AnimateTextureTransforms",               m2::kAnimateTextureTransforms },
            { "M2.SetModelVerticesMultiSample",            m2::kSetModelVerticesMultiSample },
            { "M2.SetWorldTransformFull",                  m2::kSetWorldTransformFull },
            { "M2.AnimateAttachments",                     m2::kAnimateAttachments },
            { "M2.ProcessSequenceCallback",                m2::kProcessSequenceCallback },
            { "M2.GetCollisionFacets",                     m2::kGetCollisionFacets },
            { "M2.AnimateParticlesST",                     m2::kAnimateParticlesST },
            { "M2.ModelAnimate",                           m2::kModelAnimate },
            { "M2.DispatchEventCallbacks",                 m2::kDispatchEventCallbacks },
            { "M2.GetAttachmentPosition",                  m2::kGetAttachmentPosition },
            { "M2.GetAttachmentWorldTransform",            m2::kGetAttachmentWorldTransform },
            { "M2.GetEventPosition",                       m2::kGetEventPosition },
            { "M2.AnimateShadowModel",                     m2::kAnimateShadowModel },
            { "M2.ModelSetupLighting",                     m2::kModelSetupLighting },
            { "M2.SetBoneSequenceDeferred",                m2::kSetBoneSequenceDeferred },
            { "M2.DispatchSequenceCallback",               m2::kDispatchSequenceCallback },
            { "M2.ProcessCallbackList",                    m2::kProcessCallbackList },
            { "M2.ModelDestruct",                          m2::kModelDestruct },
            { "M2.UnsetBoneSequence",                      m2::kUnsetBoneSequence },
            { "M2.AttachModelToScene",                     m2::kAttachModelToScene },
            { "M2.ModelInitialize",                        m2::kModelInitialize },
            // ribbon emitters
            { "M2.RibbonSetPos",                           m2::kRibbonSetPos },
            { "M2.RibbonReplaceTexture",                   m2::kRibbonReplaceTexture },
            { "M2.RibbonChangeFrameOfReference",           m2::kRibbonChangeFrameOfReference },
            { "M2.RibbonUpdate",                           m2::kRibbonUpdate },
            { "M2.RibbonInitialize",                       m2::kRibbonInitialize },
            // scene graph, hit testing and collision
            { "M2.AllocateHitList",                        m2::kAllocateHitList },
            { "M2.SphereTestModels",                       m2::kSphereTestModels },
            { "M2.HitTestGeometry",                        m2::kHitTestGeometry },
            { "M2.HitTestCollisionMesh",                   m2::kHitTestCollisionMesh },
            { "M2.EndHitTest",                             m2::kEndHitTest },
            { "M2.EndHitTestCollisionWorld",               m2::kEndHitTestCollisionWorld },
            { "M2.SceneSelectLights",                      m2::kSceneSelectLights },
            { "M2.SortOpaqueRibbonElements",               m2::kSortOpaqueRibbonElements },
            { "M2.SortOpaqueParticleElements",             m2::kSortOpaqueParticleElements },
            { "M2.SortOpaqueElements",                     m2::kSortOpaqueElements },
            { "M2.SortTransparentElements",                m2::kSortTransparentElements },
            { "M2.SceneComputeElementShaders",             m2::kSceneComputeElementShaders },
            { "M2.SetupBatchTextures",                     m2::kSetupBatchTextures },
            { "M2.UploadBatchVertices",                    m2::kUploadBatchVertices },
            { "M2.SceneDuplicateModel",                    m2::kSceneDuplicateModel },
            { "M2.SetupBatchLighting",                     m2::kSetupBatchLighting },
            { "M2.DrawBatchProjected",                     m2::kDrawBatchProjected },
            { "M2.DrawRibbonBatch",                        m2::kDrawRibbonBatch },
            { "M2.DrawBatchedParticles",                   m2::kDrawBatchedParticles },
            { "M2.DrawParticleBatch",                      m2::kDrawParticleBatch },
            { "M2.SceneAnimate",                           m2::kSceneAnimate },
            { "M2.SceneRenderDraw",                        m2::kSceneRenderDraw },
            { "M2.SceneDraw",                              m2::kSceneDraw },
            // shared-model load, build and teardown
            { "M2.SharedAddRef",                           m2::kSharedAddRef },
            { "M2.SharedCallbackWhenLoaded",               m2::kSharedCallbackWhenLoaded },
            { "M2.SharedSetIndices",                       m2::kSharedSetIndices },
            { "M2.SharedSetVertices",                      m2::kSharedSetVertices },
            { "M2.SharedDestroyBuffers",                   m2::kSharedDestroyBuffers },
            { "M2.SharedAllocInstances",                   m2::kSharedAllocInstances },
            { "M2.SharedConvertTextureCombos",             m2::kSharedConvertTextureCombos },
            { "M2.SharedAssignBatchTextureCombos",         m2::kSharedAssignBatchTextureCombos },
            { "M2.SharedSubstituteSpecializedShaders",     m2::kSharedSubstituteSpecializedShaders },
            { "M2.SharedFinishLowPrioritySequence",        m2::kSharedFinishLowPrioritySequence },
            { "M2.SharedLoad",                             m2::kSharedLoad },
            { "M2.SharedDestroy",                          m2::kSharedDestroy },
            { "M2.SharedRelease",                          m2::kSharedRelease },

            // --- WMO map objects ---------------------------------------------------------------------
            { "Wmo.SpawnFromModf",                         wmo::kSpawnFromModf },
            { "Wmo.RootWalk",                              wmo::kRootWalk },
            { "Wmo.RootCreateData",                        wmo::kRootCreateData },
            { "Wmo.RootComplete",                          wmo::kRootComplete },
            { "Wmo.GroupParse",                            wmo::kGroupParse },
            { "Wmo.GroupWalk",                             wmo::kGroupWalk },
            { "Wmo.GroupWalkOptional",                     wmo::kGroupWalkOptional },
            { "Wmo.GroupComplete",                         wmo::kGroupComplete },
            { "Wmo.CreateMaterial",                        wmo::kResolveMaterialTexture },
            { "Wmo.CreateMaterials",                       wmo::kCreateMaterials },
            { "Wmo.UpdateMaterials",                       wmo::kUpdateMaterials },
            { "Wmo.FixColorVertexAlpha",                   wmo::kFixColorVertexAlpha },
            { "Wmo.BspInit",                               wmo::kBspInit },
            { "Wmo.BspRaycastRefine",                      wmo::kBspRaycastRefine },
            { "Wmo.WaitLoad",                              wmo::kWaitLoad },
            { "Wmo.WaitLoadGroup",                         wmo::kWaitLoadGroup },
            { "Wmo.FreeMapObj",                            wmo::kFreeMapObj },
            { "Wmo.FreeMapObjGroup",                       wmo::kFreeMapObjGroup },
            { "Wmo.CullBatch",                             wmo::kCullBatch },
            { "Wmo.CullSortTable",                         wmo::kCullSortTable },
            { "Wmo.LocateViewerMapObjs",                   wmo::kLocateViewerMapObjs },
            { "Wmo.PortalTraverse",                        wmo::kPortalTraverse },
            { "Wmo.PortalVisibility",                      wmo::kPortalVisibility },
            { "Wmo.PortalRectAccum",                       wmo::kPortalRectAccum },
            { "Wmo.GroupResidentAccessor",                 wmo::kGroupResidentAccessor },
            { "Wmo.CameraInGroupTest",                     wmo::kCameraInGroupTest },
            { "Wmo.HorizonAabbTest",                       wmo::kHorizonAabbTest },
            { "Wmo.CreateOccluders",                       wmo::kCreateOccluders },
            { "Wmo.AllocOccluder",                         wmo::kAllocOccluder },
            { "Wmo.AddOccluderEdge",                       wmo::kAddOccluderEdge },
            { "Wmo.ExtRender",                             wmo::kExtRender },
            { "Wmo.IntRender",                             wmo::kIntRender },
            { "Wmo.CompositeEffectBind",                   shoff::kEffectBind },
            // Allocators
            { "Wmo.AllocRoot",                             wmo::kAllocRoot },
            { "Wmo.AllocGroup",                            wmo::kAllocGroup },
            { "Wmo.FreeInstanceGroup",                     wmo::kFreeInstanceGroup },
            { "Wmo.AllocInstance",                         wmo::kAllocInstance },
            { "Wmo.FreeInstance",                          wmo::kFreeInstance },
            { "Wmo.AllocInstanceGroup",                    wmo::kAllocInstanceGroup },
            // Area, zone and minimap queries
            { "Wmo.EntityResolveGroup",                    wmo::kEntityResolveGroup },
            { "Wmo.QueryZoneName",                         wmo::kQueryZoneName },
            { "Wmo.QueryAreaTable",                        wmo::kQueryAreaTable },
            { "Wmo.QueryFileName",                         wmo::kQueryFileName },
            { "Wmo.QueryMinimapInfo",                      wmo::kQueryMinimapInfo },
            { "Wmo.QueryIds",                              wmo::kQueryIds },
            { "Wmo.QueryMatrix",                           wmo::kQueryMatrix },
            { "Wmo.GroupMinimapQuery",                     wmo::kGroupMinimapQuery },
            { "Wmo.RootMinimapQuery",                      wmo::kRootMinimapQuery },
            { "Wmo.SetupInstanceMatrices",                 wmo::kSetupInstanceMatrices },
            { "Wmo.AreaTableRecordRead",                   wmo::kAreaTableRecordRead },
            { "Wmo.AreaTableLookup",                       wmo::kAreaTableLookup },
            // BSP and collision
            { "Wmo.BspConstruct",                          wmo::kBspConstruct },
            { "Wmo.BspFree",                               wmo::kBspFree },
            { "Wmo.BspDigestCacheReset",                   wmo::kBspDigestCacheReset },
            { "Wmo.BspClear",                              wmo::kBspClear },
            { "Wmo.InstanceGetFacets",                     wmo::kInstanceGetFacets },
            { "Wmo.QueryTrisForInstances",                 wmo::kQueryTrisForInstances },
            { "Wmo.RootVectorIntersect",                   wmo::kRootVectorIntersect },
            { "Wmo.RootCollectTris",                       wmo::kRootCollectTris },
            { "Wmo.RootCollectTrisAlt",                    wmo::kRootCollectTrisAlt },
            { "Wmo.RootIntersect",                         wmo::kRootIntersect },
            { "Wmo.BspFrustumVolumeTest",                  wmo::kBspFrustumVolumeTest },
            { "Wmo.GroupTrisFromQuery",                    wmo::kGroupTrisFromQuery },
            { "Wmo.BspQueryFaceIndices",                   wmo::kBspQueryFaceIndices },
            { "Wmo.GroupCollectTris",                      wmo::kGroupCollectTris },
            { "Wmo.GroupFacesForLinking",                  wmo::kGroupFacesForLinking },
            { "Wmo.GroupIntersect",                        wmo::kGroupIntersect },
            { "Wmo.GroupCollectTrisAlt",                   wmo::kGroupCollectTrisAlt },
            // Instance placement (MODF) and per-instance state
            { "Wmo.ResolveDoodadSet",                      wmo::kResolveDoodadSet },
            { "Wmo.InstanceGetGroundType",                 wmo::kInstanceGetGroundType },
            { "Wmo.InstanceGroupUpdateLights",             wmo::kInstanceGroupUpdateLights },
            { "Wmo.InstanceGroupUpdate",                   wmo::kInstanceGroupUpdate },
            { "Wmo.InstanceGroupSetSequence",              wmo::kInstanceGroupSetSequence },
            { "Wmo.InstanceGroupSetEventCallback",         wmo::kInstanceGroupSetEventCallback },
            { "Wmo.InstanceConstruct",                     wmo::kInstanceConstruct },
            { "Wmo.InstanceSetSequence",                   wmo::kInstanceSetSequence },
            { "Wmo.InstanceSetEventCallback",              wmo::kInstanceSetEventCallback },
            { "Wmo.PrepareInstance",                       wmo::kPrepareInstance },
            { "Wmo.PrepareInstances",                      wmo::kPrepareInstances },
            { "Wmo.InstanceUpdateMoved",                   wmo::kInstanceUpdateMoved },
            { "Wmo.InstanceUpdatePos",                     wmo::kInstanceUpdatePos },
            { "Wmo.InstanceUpdateMatrix",                  wmo::kInstanceUpdateMatrix },
            { "Wmo.LinkDoodadToInstance",                  wmo::kLinkDoodadToInstance },
            { "Wmo.MoveInstanceDoodads",                   wmo::kMoveInstanceDoodads },
            { "Wmo.SetDoodadEmitterDistanceIgnore",        wmo::kSetDoodadEmitterDistanceIgnore },
            { "Wmo.LinkDoodadToInstances",                 wmo::kLinkDoodadToInstances },
            { "Wmo.SetDoodadsEnabled",                     wmo::kSetDoodadsEnabled },
            { "Wmo.CreateInstanceGroups",                  wmo::kCreateInstanceGroups },
            { "Wmo.SpawnInstanceExplicit",                 wmo::kSpawnInstanceExplicit },
            { "Wmo.SpawnGroupDoodads",                     wmo::kSpawnGroupDoodads },
            { "Wmo.LinkIntersectInstanceGroup",            wmo::kLinkIntersectInstanceGroup },
            { "Wmo.LinkObjectToInstanceGroup",             wmo::kLinkObjectToInstanceGroup },
            { "Wmo.LinkIntersectInstance",                 wmo::kLinkIntersectInstance },
            { "Wmo.LinkIntersectInstances",                wmo::kLinkIntersectInstances },
            { "Wmo.PurgeInstanceGroup",                    wmo::kPurgeInstanceGroup },
            { "Wmo.PurgeInstance",                         wmo::kPurgeInstance },
            // Interior lighting and fog
            { "Wmo.RootQueryLightingAt",                   wmo::kRootQueryLightingAt },
            { "Wmo.RootQueryLighting",                     wmo::kRootQueryLighting },
            { "Wmo.GroupQueryLighting",                    wmo::kGroupQueryLighting },
            { "Wmo.LinkLightToInstances",                  wmo::kLinkLightToInstances },
            // Load, parse and lifecycle
            { "Wmo.RootInitPointers",                      wmo::kRootInitPointers },
            { "Wmo.RootInitFields",                        wmo::kRootInitFields },
            { "Wmo.RootClear",                             wmo::kRootClear },
            { "Wmo.SubsystemInit",                         wmo::kSubsystemInit },
            { "Wmo.CachePurge",                            wmo::kCachePurge },
            { "Wmo.SubsystemDestroy",                      wmo::kSubsystemDestroy },
            { "Wmo.RootCreate",                            wmo::kRootCreate },
            { "Wmo.GroupInitPointers",                     wmo::kGroupInitPointers },
            { "Wmo.GroupInitFields",                       wmo::kGroupInitFields },
            { "Wmo.GroupClear",                            wmo::kGroupClear },
            { "Wmo.RootRead",                              wmo::kRootRead },
            { "Wmo.GroupRead",                             wmo::kGroupRead },
            // Portals, bounds and visibility
            { "Wmo.CreateIgnoreFlags",                     wmo::kCreateIgnoreFlags },
            { "Wmo.IsGroupLoaded",                         wmo::kIsGroupLoaded },
            { "Wmo.IsGroupLoading",                        wmo::kIsGroupLoading },
            { "Wmo.GetRootBounds",                         wmo::kGetRootBounds },
            { "Wmo.GetGroupBounds",                        wmo::kGetGroupBounds },
            { "Wmo.GetGroupFlags",                         wmo::kGetGroupFlags },
            { "Wmo.TestConvexVolume",                      wmo::kTestConvexVolume },
            { "Wmo.GetGroupName",                          wmo::kGetGroupName },
            { "Wmo.GetGroupInfo",                          wmo::kGetGroupInfo },
            { "Wmo.IsRootLoaded",                          wmo::kIsRootLoaded },
            { "Wmo.IsRootDrawable",                        wmo::kIsRootDrawable },
            { "Wmo.DistanceToExteriorPortal",              wmo::kDistanceToExteriorPortal },
            { "Wmo.ExteriorPortalDistanceQuery",           wmo::kExteriorPortalDistanceQuery },
            // Rendering
            { "Wmo.SceneRenderInstanceGroups",             wmo::kSceneRenderInstanceGroups },
            { "Wmo.SceneCullInstanceGroups",               wmo::kSceneCullInstanceGroups },
            { "Wmo.RenderCollidableFaces",                 wmo::kRenderCollidableFaces },
            { "Wmo.RenderPortalGeometry",                  wmo::kRenderPortalGeometry },
            { "Wmo.RenderShadowMapGroups",                 wmo::kRenderShadowMapGroups },
            { "Wmo.RenderGroup",                           wmo::kRenderGroup },
            { "Wmo.RenderThroughPortals",                  wmo::kRenderThroughPortals },
            { "Wmo.PrepareUpdate",                         wmo::kPrepareUpdate },
            { "Wmo.GroupFillVertexBuffer",                 wmo::kGroupFillVertexBuffer },
            { "Wmo.GroupSetVertexBuffer",                  wmo::kGroupSetVertexBuffer },
            { "Wmo.GroupSetIndexBuffer",                   wmo::kGroupSetIndexBuffer },
            { "Wmo.GroupAllocVertexArray",                 wmo::kGroupAllocVertexArray },
            { "Wmo.GroupAllocVertexBuffer",                wmo::kGroupAllocVertexBuffer },
            { "Wmo.GroupFreeVertexBuffer",                 wmo::kGroupFreeVertexBuffer },
            { "Wmo.GroupAllocLiquidBuffer",                wmo::kGroupAllocLiquidBuffer },
            { "Wmo.GroupFreeLiquidBuffer",                 wmo::kGroupFreeLiquidBuffer },
            { "Wmo.AttenuateTransitionVerts",              wmo::kAttenuateTransitionVerts },
            // WMO liquids
            { "Wmo.GroupQueryLiquidSounds",                wmo::kGroupQueryLiquidSounds },
            { "Wmo.RootQueryLiquidSounds",                 wmo::kRootQueryLiquidSounds },
            { "Wmo.MapQueryLiquidStatusInstances",         wmo::kMapQueryLiquidStatusInstances },
            { "Wmo.EntityUpdateLiquid",                    wmo::kEntityUpdateLiquid },
            { "Wmo.RootQueryLiquidStatus",                 wmo::kRootQueryLiquidStatus },
            { "Wmo.GroupQueryLiquid",                      wmo::kGroupQueryLiquid },
            { "Wmo.GroupSharedTileCount",                  wmo::kGroupSharedTileCount },
            { "Wmo.GroupGenLiquidVerts",                   wmo::kGroupGenLiquidVerts },
            { "Wmo.IdentifyLegacyLiquidType",              wmo::kIdentifyLegacyLiquidType },
            { "Wmo.GroupLiquidTileIntersect",              wmo::kGroupLiquidTileIntersect },
            { "Wmo.GroupLiquidTris",                       wmo::kGroupLiquidTris },
            { "Wmo.GroupLiquidVectorIntersect",            wmo::kGroupLiquidVectorIntersect },
            { "Wmo.GroupLiquidTrisAlt",                    wmo::kGroupLiquidTrisAlt },
            { "Wmo.MapLegacyLiquidId",                     wmo::kMapLegacyLiquidId },

            // --- world scene -------------------------------------------------------------------------
            { "Scene.CullMapObjDefGroupFromExterior",      wscene::kCullMapObjDefGroupFromExterior },
            { "Scene.ClipBufferStampPolyline",             wscene::kClipBufferStampPolyline },
            // World scene and view setup
            { "Scene.FadeDistanceScale",                   wscene::kFadeDistanceScale },
            { "Scene.ViewerLocate",                        wscene::kViewerLocate },
            { "Scene.Destroy",                             wscene::kDestroy },
            { "Scene.Initialize",                          wscene::kInitialize },
            { "Scene.Render",                              wscene::kRender },

            // --- world-map screen --------------------------------------------------------------------
            // Area / zone resolution
            { "WorldMap.AreaFromPosition",                 wmap::kAreaFromPosition },
            { "WorldMap.ZoneSync",                         wmap::kZoneSync },
            // Map lifecycle: load / unload / enter / leave
            { "WorldMap.EnterWorld",                       wmap::kEnterWorld },
            { "WorldMap.LeaveWorld",                       wmap::kLeaveWorld },

            // --- weather -----------------------------------------------------------------------------
            // Weather
            { "Weather.StormIntensitySet",                 wthr::kStormIntensitySet },
            { "Weather.Render",                            wthr::kRender },
            { "Weather.Clear",                             wthr::kClear },
            { "Weather.Update",                            wthr::kUpdate },
        };

        const Point* Find(const char* name)
        {
            if (!name) return nullptr;
            for (const Point& p : kPoints)
                if (std::strcmp(p.name, name) == 0) return &p;
            return nullptr;
        }
    }

    int AttachByName(const char* pointName, void* detour, void** original, int priority)
    {
        const Point* p = Find(pointName);
        if (!p)
        {
            WLOG_ERROR("hookpoints: '%s' is not a registered hook point", pointName ? pointName : "(null)");
            return 0;
        }
        return wxl::hook::Install(pointName, p->address, detour, original, priority) ? 1 : 0;
    }
}
