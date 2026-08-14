// The one seam a movement feature gets on resolved animation, published by wxl-modern-m2.
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

#ifndef WXL_M2_ANIMATION_API_H
#define WXL_M2_ANIMATION_API_H

#include <stdint.h>

// Published as wxl.m2-animation. wxl-modern-m2 already answers "what can this model actually play",
// which is the question the engine's own table stops short of: a modern model carries sequences the
// table has no id for, and the stock resolver never asks the model about them.
//
// What it deliberately does not answer is "what SHOULD be playing", because that is a movement
// decision -- a glide, a fall, a mount -- and it belongs to whatever owns the movement, not to the
// model reader. This is the seam that owner arms: it sees what resolution produced and may replace
// it. One slot, because two features each replacing the other's answer is not a merge, it is a race.
//
// Plain C, __cdecl spelled out, POD only -- see PluginApi.h for why.

#ifdef __cplusplus
extern "C" {
#endif

#define WXL_M2_ANIMATION_API_VERSION 1

/**
 * @brief Offered the resolved animation, may name a different one.
 * @param unit                the unit being animated.
 * @param requestedAnimation  the animation asked for, before resolution.
 * @param model               the model it resolved against.
 * @param resolvedAnimation   what resolution settled on, negative when it found nothing.
 * @return an animation id to play instead, or a negative value to keep @p resolvedAnimation.
 */
typedef int32_t(__cdecl* WXL_M2AnimationResolveOverrideFn)(
    void* unit, int32_t requestedAnimation, void* model, int32_t resolvedAnimation);

typedef struct WXL_M2AnimationApi
{
    uint32_t structSize;
    uint32_t apiVersion;

    /**
     * @brief Arms the single override, replacing whatever was armed before.
     *
     * Registration is for the process lifetime unless replaced. Pass NULL to disarm.
     * @param callback  invoked on every resolution, on the thread that animates.
     */
    void(__cdecl* SetResolveOverride)(WXL_M2AnimationResolveOverrideFn callback);
} WXL_M2AnimationApi;

#ifdef __cplusplus
}
#endif

#endif // WXL_M2_ANIMATION_API_H
