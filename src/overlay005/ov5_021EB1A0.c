#include "overlay005/ov5_021EB1A0.h"

#include <nitro.h>
#include <string.h>

#include "generated/movement_actions.h"
#include "generated/movement_types.h"
#include "generated/object_events_gfx.h"

#include "struct_decls/map_object.h"

#include "constants/field/field_effect_renderer.h"

#include "overlay005/field_effect_manager.h"
#include "overlay005/object_event_gfx_data.h"
#include "overlay005/ov5_021ECA70.h"
#include "overlay005/ov5_021ECC20.h"
#include "overlay005/ov5_021ECE40.h"
#include "overlay005/ov5_021F10E8.h"
#include "overlay005/ov5_021F23D0.h"
#include "overlay005/struct_ov5_021ED01C.h"

#include "billboard.h"
#include "constants/field/map.h"
#include "constants/player_avatar.h"
#include "follower_mon.h"
#include "follower_mon_talk.h"
#include "map_object.h"
#include "map_object_move.h"
#include "map_tile_behavior.h"
#include "math_util.h"
#include "overworld_anim_manager.h"
#include "simple3d.h"
#include "player_avatar.h"

typedef struct {
    s16 unk_00;
    s8 unk_02;
    s8 unk_03;
    Billboard *unk_04;
    UnkStruct_ov5_021ED01C unk_08;
} UnkStruct_ov5_021EB2EC;

typedef struct {
    s8 unk_00;
    s8 unk_01;
    s8 unk_02;
    s8 unk_03;
    Billboard *unk_04;
    UnkStruct_ov5_021ED01C unk_08;
} UnkStruct_ov5_021EB7F8;

typedef struct {
    s8 unk_00;
    s8 unk_01;
    s8 unk_02;
    u8 unk_03_0 : 1;
    u8 unk_03_1 : 7;
    Billboard *unk_04;
    UnkStruct_ov5_021ED01C unk_08;
} UnkStruct_ov5_021EBA0C;

typedef struct {
    OverworldAnimManager *unk_00;
} UnkStruct_ov5_021EC760;

typedef struct {
    s16 unk_00;
    s8 unk_02;
    s8 unk_03;
    Billboard *unk_04;
    UnkStruct_ov5_021ED01C unk_08;
    u32 unk_10;
} UnkStruct_ov5_021EC804;

typedef struct {
    int unk_00;
    fx32 unk_04;
    Billboard *unk_08;
    UnkStruct_ov5_021ED01C unk_0C;
} UnkStruct_ov5_021EC938;

void ov5_021EB2EC(MapObject *mapObj);
void ov5_021EB314(MapObject *mapObj);
void ov5_021EB328(MapObject *mapObj);
void ov5_021EB354(MapObject *mapObj);
void ov5_021EB398(MapObject *mapObj);
void ov5_021EB40C(MapObject *mapObj);
static void ov5_021EB478(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EB2EC *param2, int param3);
static void ov5_021EB4AC(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EB2EC *param2, int param3);
static void ov5_021EB4E8(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EB2EC *param2, int param3);
static void ov5_021EB524(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EB2EC *param2, int param3);
static void ov5_021EB560(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EB2EC *param2, int param3);
static void ov5_021EB59C(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EB2EC *param2, int param3);
static void ov5_021EB620(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EB2EC *param2, int param3);
static void ov5_021EC068(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EBA0C *param2, int param3);
static void ov5_021EC0E4(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EBA0C *param2, int param3);
void ov5_021EC734(MapObject *mapObj);
void ov5_021EC75C(MapObject *mapObj);
void ov5_021EC760(MapObject *mapObj);
void ov5_021EC778(MapObject *mapObj);
void ov5_021EC790(MapObject *mapObj);
static void ov5_021EC638(Billboard *param0, int param1);
static void ov5_021EC668(Billboard *param0);
static void ov5_021EC674(Billboard *param0, int param1, int param2);
static void ov5_021EC69C(Billboard *param0, int param1);
static void ov5_021EC6C0(MapObject *mapObj, Billboard *param1);
static void ov5_021EC374(MapObject *mapObj, Billboard *param1, int param2);

void (*const Unk_ov5_021FF3D4[])(MapObject *, Billboard *, UnkStruct_ov5_021EB2EC *, int);
void (*const Unk_ov5_021FF420[])(MapObject *, Billboard *, UnkStruct_ov5_021EB2EC *, int);
void (*const Unk_ov5_021FF3F8[])(MapObject *, Billboard *, UnkStruct_ov5_021EBA0C *, int);
static void (*const Unk_ov5_021FF448[11])(MapObject *, Billboard *, UnkStruct_ov5_021EBA0C *, int);

// FOLLOWER ROTATION - REMOVED (2026-10-01, ENTRY P19): tried twice (ENTRY P17: separate FieldMap_Main
// update; ENTRY P18: moved directly into ov5_021EB398/ov5_021EB354 below, this exact call site). Both
// versions compiled, linked, and set a genuinely-varying rotation matrix via Billboard_SetRotMatrix - the
// user confirmed the walk animation started working (ENTRY P18's frame-range fix) but the follower stayed
// permanently facing north regardless. With the rotation math, the draw call, the live Billboard access,
// and the direction value itself all independently verified correct across two attempts, the remaining
// explanation is that BILLBOARD_MODEL_GENERIC_32x32 (and this whole Billboard system) most likely uses
// NDS hardware-level camera-facing billboarding for its polygon(s) - a real, documented DS GPU feature -
// which would make any caller-supplied rotation matrix a no-op for on-screen orientation regardless of
// what code sets it or when. This is also consistent with Billboard_SetRotMatrix being otherwise unused
// everywhere else in this project (grep found exactly one call site before ENTRY P17, and it was a
// pointer copy between two billboards during a resource rebuild, never a real orientation change) - every
// other billboard, including the 19 natively-working Pokemon species, has always relied on frame-swapping
// for direction, never rotation. See ENTRY P19 in Changelog.txt for the replacement: a corrected
// frame-swap table for the HG-sourced species' real 8-frame layout (4 directions x 2 frames each,
// determined by decoding the actual pixel content, not guessed).
Billboard *ov5_021EB1A0(MapObject *mapObj)
{
    int v0 = MapObject_GetGraphicsID(mapObj);

    switch (v0) {
    case OBJ_EVENT_GFX_PLAYER_M:
    case OBJ_EVENT_GFX_PLAYER_F:
    case OBJ_EVENT_GFX_PLAYER_M_BIKE:
    case OBJ_EVENT_GFX_PLAYER_F_BIKE:
    case OBJ_EVENT_GFX_PLAYER_M_HOLDING_POKE_BALL:
    case OBJ_EVENT_GFX_PLAYER_F_HOLDING_POKE_BALL:
    case OBJ_EVENT_GFX_PLAYER_M_SURF:
    case OBJ_EVENT_GFX_PLAYER_F_SURF:
    case OBJ_EVENT_GFX_PLAYER_M_SPRAYDUCK:
    case OBJ_EVENT_GFX_PLAYER_F_SPRAYDUCK:
    case OBJ_EVENT_GFX_PLAYER_M_SAVE:
    case OBJ_EVENT_GFX_PLAYER_F_SAVE:
    case OBJ_EVENT_GFX_PLAYER_M_CONTEST:
    case OBJ_EVENT_GFX_PLAYER_F_CONTEST:
    case OBJ_EVENT_GFX_PLAYER_M_FISHING:
    case OBJ_EVENT_GFX_PLAYER_F_FISHING:
    case OBJ_EVENT_GFX_PLAYER_M_POKETCH:
    case OBJ_EVENT_GFX_PLAYER_F_POKETCH:
    case OBJ_EVENT_GFX_PLAYER_M_POKECENTER_HEAL:
    case OBJ_EVENT_GFX_PLAYER_F_POKECENTER_HEAL:
    case OBJ_EVENT_GFX_PLAYER_M_HEARTHOME_GYM:
    case OBJ_EVENT_GFX_PLAYER_F_HEARTHOME_GYM:
    case OBJ_EVENT_GFX_DIST_WORLD_PLAYER_M:
    case OBJ_EVENT_GFX_DIST_WORLD_PLAYER_F:
    case OBJ_EVENT_GFX_DIST_WORLD_PLAYER_M_SAVE:
    case OBJ_EVENT_GFX_DIST_WORLD_PLAYER_F_SAVE:
    case OBJ_EVENT_GFX_DIST_WORLD_PLAYER_M_POKETCH:
    case OBJ_EVENT_GFX_DIST_WORLD_PLAYER_F_POKETCH:
    case OBJ_EVENT_GFX_PLAYER_M_SAVE_HEARTHOME_GYM:
    case OBJ_EVENT_GFX_PLAYER_F_SAVE_HEARTHOME_GYM:
    case OBJ_EVENT_GFX_PLAYER_M_POKETCH_HEARTHOME_GYM:
    case OBJ_EVENT_GFX_PLAYER_F_POKETCH_HEARTHOME_GYM: {
        UnkStruct_ov5_021EBA0C *v1;

        v1 = sub_02062AF0(mapObj);
        return v1->unk_04;
    }
    case OBJ_EVENT_GFX_MAGIKARP: {
        UnkStruct_ov5_021EB7F8 *v2;

        v2 = sub_02062AF0(mapObj);
        return v2->unk_04;
    }
    case OBJ_EVENT_GFX_UXIE:
    case OBJ_EVENT_GFX_MESPRIT:
    case OBJ_EVENT_GFX_AZELF: {
        return ov5_021ECB80(mapObj);
    }
    case OBJ_EVENT_GFX_DIST_WORLD_B1F_MESPRIT: {
        UnkStruct_ov5_021EC804 *v3;

        v3 = sub_02062AF0(mapObj);
        return v3->unk_04;
    }
    case OBJ_EVENT_GFX_BERRY_SOIL: {
        return NULL;
    }
    default: {
        const ObjectEventGfxRenderDetailsEntry *v4;

        v4 = ov5_021ECD04(mapObj);

        if (v4->modelType == 1) {
            UnkStruct_ov5_021EB2EC *v5;

            v5 = sub_02062AF0(mapObj);
            return v5->unk_04;
        }
    }
    }

    return NULL;
}

void ov5_021EB2EC(MapObject *mapObj)
{
    UnkStruct_ov5_021EB2EC *v0 = sub_02062ACC(mapObj, (sizeof(UnkStruct_ov5_021EB2EC)));
    v0->unk_02 = -1;

    ov5_021ECF04(mapObj, &v0->unk_04);

    if (v0->unk_04 != NULL) {
        sub_02062B68(mapObj);
    }
}

void ov5_021EB314(MapObject *mapObj)
{
    UnkStruct_ov5_021EB2EC *v0 = sub_02062AF0(mapObj);
    ov5_021ECFA4(mapObj, &v0->unk_04);
}

void ov5_021EB328(MapObject *mapObj)
{
    UnkStruct_ov5_021EB2EC *v0 = sub_02062AF0(mapObj);

    if (v0->unk_04 != NULL) {
        ov5_021ED01C(v0->unk_04, &v0->unk_08);
    }

    ov5_021ECFA4(mapObj, &v0->unk_04);
    MapObject_SetStatusFlagOn(mapObj, MAP_OBJ_STATUS_21);
}

void ov5_021EB354(MapObject *mapObj)
{
    int v0;
    UnkStruct_ov5_021EB2EC *v1 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    if (v1->unk_04 == NULL) {
        ov5_021ECF04(mapObj, &v1->unk_04);
    }

    if (v1->unk_04 != NULL) {
        ov5_021ED03C(v1->unk_04, &v1->unk_08);
        ov5_021EDEB4(mapObj, v1->unk_04);

        MapObject_SetStatusFlagOff(mapObj, MAP_OBJ_STATUS_21);
    }
}

// ENTRY P29 (2026-10-01): idle animation (ENTRY P20/P27/P28, three prior attempts) and visual follow-lag
// (ENTRY P28) both had ZERO visible effect from their previous call sites (ov5_021EB478's dispatch branch;
// separate FieldMap_Main-driven calls). Root cause for BOTH, found by finally reading the actual renderer
// vtable (Unk_ov5_021FAFD8 in src/overlay005/object_event_gfx_data.c) instead of continuing to guess at
// call-site timing: UnkStruct_ov5_021FB0F0 is a 5-slot {Create, Update, Free, Pause, Resume} lifecycle
// table, not a walking/idle split as an earlier entry's comment mistakenly assumed - EVERY billboard-
// rendered species that can be a follower (all 9 ENTRY P17 HG species and all 20 native species, confirmed
// by grepping every one of their table rows) uses Unk_ov5_021FAFD8, whose Update slot is this exact
// function, ov5_021EB398. There is no separate per-movement-state update function for this renderer - this
// one runs unconditionally, every single field frame, regardless of whether the object is walking or
// standing still, for every follower species without exception. That makes this the one genuinely reliable,
// always-runs call site - unlike FieldMap_Main (a separate, only-loosely-synchronized update pass) or
// ov5_021EB478 (one of nine per-speed-tier dispatch targets selected by sub_02062A14(mapObj), which may
// simply never be the one actually selected during real sustained idle).
// ENTRY P65: TRUE while the object is in a jump movement action (the follower's ledge leap).
static BOOL FollowerMon_IsJumping(MapObject *mapObj)
{
    enum MovementAction action = MapObject_GetMovementAction(mapObj);

    return (action >= MOVEMENT_ACTION_JUMP_ON_SPOT_SLOW_NORTH && action <= MOVEMENT_ACTION_JUMP_FAR_EAST)
        || (action >= MOVEMENT_ACTION_JUMP_NEAR_SLOW_WEST && action <= MOVEMENT_ACTION_JUMP_FARTHER_EAST)
        || (action >= MOVEMENT_ACTION_FOLLOWER_LEAP_NORMAL_NORTH && action <= MOVEMENT_ACTION_FOLLOWER_LEAP_FASTER_EAST); // ENTRY P66
}

// ENTRY P64: extra spacing for large followers (1 world unit = 1 pixel horizontally). ENTRY P69: 5 -> 6 at the
// user's request, same logic as ENTRY P64/P66 (always on, pushed behind the follower's facing).
// ENTRY P71: replaced by HeartGold's own values (asm/overlay_01_021F8D80.s ov01_021F8E70, 64x64 model): 10 units
// when the follower faces west/east, 1 unit when it faces north/south, always behind its facing.
// ENTRY P72: sideways 10 -> 7 at the user's request ("a happy medium between 6 and 10"); vertical stays 1.
#define FOLLOWER_TALL_EXTRA_DISTANCE_SIDEWAYS (FX32_ONE * 7)
#define FOLLOWER_TALL_EXTRA_DISTANCE_VERTICAL (FX32_ONE * 1)

static void FollowerMon_ApplyVisualLag(MapObject *mapObj)
{
    // Every other user of the shared Unk_ov5_021FAFD8 renderer (ordinary trainers/NPCs, Prof. Rowan, gym
    // leaders, etc. - see object_event_gfx_data.c) must render at its true position, unmodified - only an
    // actual follower should ever get an offset.
    if (MapObject_GetMovementType(mapObj) != MOVEMENT_TYPE_FOLLOW_PLAYER) {
        return;
    }

    FieldSystem *fieldSystem = MapObject_FieldSystem(mapObj);
    int actionSpeed = PlayerAvatar_GetMovementActionSpeed(fieldSystem->playerAvatar);
    fx32 magnitude;

    // ENTRY P28 gated this on MapObject_IsMoving(playerObj) first and only read actionSpeed inside that
    // gate - but MapObject_IsMoving reads MAP_OBJ_STATUS_1, which src/map_object.c's own setters
    // (sub_02062D04/sub_02062D10) show is only ever touched from src/unk_0206450C.c and src/unk_02069BE0.c,
    // the generic NPC/follower movement-ACTION processor - never from src/overlay009/ov9_02249960.c, the
    // player's own separate, input-driven movement code. The player's MapObject almost certainly never gets
    // MAP_OBJ_STATUS_1 set at all, meaning that gate was likely FALSE 100% of the time, silently forcing
    // magnitude to 0 every frame regardless of the player's real movement - a much simpler explanation for
    // "zero visible effect" than a call-site timing race. PlayerAvatar_GetMovementActionSpeed alone (reading
    // playerAvatar->movementActionSpeed directly, no MapObject flag involved) is used exactly this way,
    // unconditionally, by the follower's own native catch-up movement code (src/unk_02069BE0.c:690's
    // sub_0206A3BC, confirmed by direct read) - proven reliable for detecting real player movement/speed,
    // so this drops the redundant, likely-broken MapObject_IsMoving gate entirely.
    // ENTRY P30 (2026-10-01): user confirmed the P29 distance mechanism works, but reported it as roughly
    // 2x too far at both tiers - halved both (MAP_OBJECT_TILE_SIZE/4 -> /8 walking, /2 -> /4 running).
    // ENTRY P46 (2026-10-01): ENTRY P45 added a baseline directly here, but that affects jumpOffset, which
    // the shadow (src/overlay005/ov5_021F134C.c's ApplyFollowerShadowLag) also reads - the user clarified
    // the shadow must stay exactly as it was (0/1/3px) and only the SPRITE should get the extra baseline
    // distance. Reverted this function back to exactly its pre-P45 form; the mon-only baseline now lives in
    // FollowerMon_ApplyMonOnlyOffsets below, via spritePosOffset (a field the shadow never reads).
    if (actionSpeed == PLAYER_ACTION_SPEED_NONE || actionSpeed == PLAYER_ACTION_SPEED_NOT_MOVING) {
        magnitude = 0;
    } else if (actionSpeed == PLAYER_ACTION_SPEED_FAST || actionSpeed == PLAYER_ACTION_SPEED_FASTER) {
        magnitude = MAP_OBJECT_TILE_SIZE / 4;
    } else {
        magnitude = MAP_OBJECT_TILE_SIZE / 8;
    }

    // ENTRY P64: large (64x64, ENTRY P61) followers keep FOLLOWER_TALL_EXTRA_DISTANCE more distance from the player
    // at all times. jumpOffset is also what the shadow reads (ApplyFollowerShadowLag), so the shadow moves with it.
    if (FollowerMon_IsTallGraphicsID(MapObject_GetGraphicsID(mapObj))) {
        int facing = MapObject_GetFacingDir(mapObj);

        magnitude += (facing == DIR_WEST || facing == DIR_EAST) ? FOLLOWER_TALL_EXTRA_DISTANCE_SIDEWAYS : FOLLOWER_TALL_EXTRA_DISTANCE_VERTICAL;
    }

    int behindDir = Direction_GetOpposite(MapObject_GetFacingDir(mapObj));
    VecFx32 offset = { 0, 0, 0 };
    VecFx32_StepDirection(behindDir, &offset, magnitude);

    // ENTRY P65: during a real jump (the follower's ledge leap, src/unk_02069BE0.c) the movement code drives
    // jumpOffset.y with the jump arc every frame just before this runs (MapObjectTask_Move moves, then draws) - keep
    // it instead of flattening it to 0.
    if (FollowerMon_IsJumping(mapObj)) {
        VecFx32 current;
        MapObject_GetSpriteJumpOffset(mapObj, &current);
        offset.y = current.y;
    }

    MapObject_SetSpriteJumpOffset(mapObj, &offset);
}

// FOLLOWER BOUNCE (ENTRY P38, 2026-10-01, ground-up rewrite): every previous attempt (ENTRY P33-P37) built
// its vertical shift on MapObject_SetSpriteJumpOffset - the same field/API FollowerMon_ApplyVisualLag above
// uses for the confirmed-correct horizontal lag - through 5 different code structures (a multi-step curve, a
// hard toggle, a phase-desynced toggle, a standalone get-modify-set version), all reported showing the same
// visual defect. This entry deliberately does NOT touch jumpOffset at all. It uses
// MapObject_SetSpritePosOffset instead - a separate MapObject field, summed into the exact same final render
// position by ov5_021ECDA0 (src/overlay005/ov5_021ECC20.c:134-146: objectPosition + jumpOffset + posOffset +
// terrainSpriteOffset, all three offset fields added in on equal footing), but written and read through
// entirely different code than the lag uses. This is the field the Distortion World's own Uxie/Mesprit/Azelf
// vertical hover animation is built on (src/overlay009/ov9_02249960.c, e.g. uxieSpritePosOffset.y +=
// ascendYDelta; MapObject_SetSpritePosOffset(...) - real, shipped, presumably-correct vertical motion on a
// Pokemon-species billboard, going through the same ov5_021ECDA0/Billboard_SetPos pipeline this follower
// uses) - a genuine precedent for vertical motion specifically, not a borrowed function, not shared state,
// just the same general-purpose MapObject field every other vertical effect in this game is already built on.
// ENTRY P41 (2026-10-01): user confirmed 10px (ENTRY P39) renders CLEANLY - a genuinely important result.
// The spritePosOffset mechanism itself (ENTRY P38) is correct; 1 world unit (FX32_ONE) is apparently too
// small to resolve as a clean, consistently-rounded whole pixel on this camera-facing billboard's actual
// screen projection, producing sub-pixel rounding jitter that reads as horizontal-looking garbage - MAP_
// OBJECT_TILE_SIZE's "16*FX32_ONE = one 16px tile" convention (used for the horizontal lag's much larger
// 2-4 unit magnitudes, which never hit this problem) apparently does not hold precisely enough at this small
// a scale for this vertical case. Trying 2px as a middle point between confirmed-broken (1) and
// confirmed-clean (10).
// ENTRY P42 (2026-10-01): user's detailed per-direction report at 2px was the real breakthrough - this
// function never reads facing direction at all (it always writes the same {0, height, 0} regardless of
// which way the follower faces), so the reported per-direction difference (correct for west/north/south,
// visibly wrong specifically when facing east - "moves up and forward, almost a 45 degree angle") cannot be
// coming from this function's own logic. It has to be the camera-facing billboard's own orientation
// recompute reacting differently depending on which way the object (nominally) faces relative to the fixed
// camera angle - worse for east specifically, and shrinking as a proportion of the total shift as magnitude
// grows (1px: wrong everywhere; 2px: only wrong facing east; 10px: reported fine overall). Trying 4px to see
// if east clears up too without needing to go all the way to 10.
// ENTRY P43 (2026-10-01): 4px confirmed too large (a size complaint, not distortion - "4 is too many").
// User also confirmed the horizontal lag (FollowerMon_ApplyVisualLag, same MapObject, same general idea)
// renders perfectly cleanly at similarly small real screen-pixel counts - walking ~1px, running ~3px, no
// pixelation/border artifacts at all - which rules out "small numbers are inherently imprecise on this
// hardware" as a general explanation; whatever is happening is specific to the vertical axis, not magnitude
// in general. User described the actual visual defect precisely for the first time: "it changes the actual
// pixelation of the sprite, sometimes it has an extra pixel on its border" - texture-edge/sampling artifacts
// from a non-pixel-aligned screen position, not a geometric warp. Trying 3px - between the still-imperfect
// 2px and the oversized 4px, and matching the user's own "~3px" reference point for what a small, clean,
// on-screen pixel-scale shift looks like on this exact rendering pipeline (via the working lag).
// ENTRY P44 (2026-10-01): user reported bouncing going long stretches with no bounce while walking, and
// asked for the ENTRY P35/P37 idle-animation-desync resync to be removed, confirming by their own testing it
// wasn't preventing any actual problem. Removed - this function is now a plain, unconditional free-running
// toggle again, with no dependency on or awareness of the walk-animation function at all.
// ENTRY P45 (2026-10-01): researched the sibling HeartGold project directly per the user's request - it has
// no equivalent to MapObject_SetSpriteJumpOffset/SetSpritePosOffset at all (grepped its map_object.h/.c,
// zero hits), and its overworld player/follower characters are true 3D models with real skeletal/vertex
// walk-cycle animation (not 2D billboards), so HG's clean "upper body moves up 1px and back down while
// walking" is very likely baked directly into the walk animation's own keyframe data, not a separate,
// portable position-offset trick this project could copy into Platinum's billboard system. One concrete,
// relevant finding from HG's own engine code though: src/map_object.c's world-position conversion uses
// `x * (16*FX32_ONE)` and `z * (16*FX32_ONE)` for the horizontal axes but `y * (8*FX32_ONE)` for vertical -
// HALF the granularity. Cross-referencing every bounce magnitude tested so far against this possibility -
// that 1 real screen pixel of Y motion might need 2 world units here, not 1, same as HG's own 2x ratio -
// lines up with the results far better than pure trial and error would suggest: 1px(odd)=garbage,
// 2px(even)="correct except east", 3px(odd)="still not clean", 4px(even)=clean-but-big, 10px(even)=clean.
// Every EVEN value tested has looked substantially better than either ODD value. Reverting to 2px - the best
// single data point so far - rather than continuing to guess new odd values.
// ENTRY P46 (2026-10-01): renamed from FollowerMon_ApplyBounce (the vertical bounce itself moved out to its
// own function again, below, on ENTRY P47's findings) - this carries only the mon-only extra lag baseline
// now (ENTRY P45's original intent, corrected: the shadow must NOT get this, only the sprite - see
// FollowerMon_ApplyVisualLag's own updated comment). FOLLOWER_LAG_BASELINE is applied unconditionally, to
// all 29 follower species (matching FollowerMon_ApplyVisualLag's own scope) via spritePosOffset.x/z - a
// field the shadow (ApplyFollowerShadowLag, src/overlay005/ov5_021F134C.c) never reads, so the shadow stays
// exactly at whatever FollowerMon_ApplyVisualLag alone produces (0/1/3px).
#define FOLLOWER_LAG_BASELINE (MAP_OBJECT_TILE_SIZE / 8)

// ENTRY P59 (2026-10-03): FOLLOWER DEPTH ORDER vs. THE PLAYER. Every billboard sprite (generic_32x32.nsbmd's
// only node carries SBC opcode 0x07, NNS_G3D_SBC_BB - a FULL camera-facing billboard, verified by decoding
// the model's render byte code) is a flat quad perpendicular to the camera's view direction, so its depth is
// just dot(origin, viewDir). The field camera (field_camera.c sCameraTypes, default pitch 59.0515 degrees,
// yaw 0, roll 0) looks north and down: viewDir = (0, -sin(p), -cos(p)) = (0, -0.85763, -0.51427). The
// follower's bounce used to be a pure +Y offset, which moves a sprite toward the camera by sin(p)*b - up to
// 0.86*2 = 1.7 units at the peak. Player and follower side by side (same Z) are at EXACTLY equal depth when
// the bounce is at 0 (the player, drawn first, wins the tie - the "mon behind the player" the user saw at the
// bottom of the bounce) and the follower is up to 1.7 units CLOSER at the peak (mon in front of the player).
// Fix, two parts, both in spritePosOffset so the shadow (which only reads jumpOffset) never sees them:
//  1. Depth-neutral bounce: the same on-screen motion can be made by moving along the screen-up vector
//     u = (0, cos(p), -sin(p)) instead of straight up. A 1.03px screen shift S = cos(p)*b along u is
//     (dy, dz) = (cos(p)^2 * b, -sin(p)*cos(p) * b) = (0.26447b, -0.44105b) - identical screen amplitude to
//     before (the user-confirmed ~1px bounce), zero change in depth. ApplyBounce now writes the scaled Y into
//     jumpOffset (still the ENTRY P47/P48 jumpOffset bounce, not the Uxie-style posOffset one) and stashes the
//     matching -Z in sFollowerBounceDepthCompZ for this function to add to posOffset.
//  2. A small constant depth bias away from the camera along viewDir (FOLLOWER_DEPTH_BIAS = 0.5 units, i.e.
//     dy = -0.42882, dz = -0.25713; screen position unchanged under the field camera's 8 degree FOV). Covers
//     rounding and the other camera types' slightly different pitch (zoomed/Pastoria/interior leave at most
//     0.17 units of residual depth change from part 1) and makes the tie-break deterministic instead of
//     depending on which billboard happens to be first in the draw list. Costs at most ~0.8px of the quad's
//     bottom edge under the ground plane, inside every frame's 2px of transparent padding.
// NOT changed: when the player walks NORTH the follower is one tile SOUTH of the player, i.e. 16 units closer
// to the camera (8.2 units of depth) - it draws in front of the player there, as it did before the bounce
// existed. Forcing it behind would need an ~8.2 unit bias, which buries ~14px of the sprite in the ground.
#define FOLLOWER_DEPTH_BIAS_Y FX32_CONST(0.42882)
#define FOLLOWER_DEPTH_BIAS_Z FX32_CONST(0.25713)
#define FOLLOWER_BOUNCE_Y_SCALE FX32_CONST(0.26447)
#define FOLLOWER_BOUNCE_Z_SCALE FX32_CONST(0.44105)

static fx32 sFollowerBounceDepthCompZ = 0;

// ENTRY P82: LARGE FOLLOWER IN FRONT OF THE PLAYER BESIDE IT / WALKING THROUGH IT, EVEN NEXT TO AN NPC.
// User (photos on ENTRY P81, an NPC standing one row in front of Rayquaza and a tile or two to the side): "sometimes
// when I am next to the mon and walking through it I get this awful clipping ... after enough frames, the issue goes
// away". Cause: an NPC on the follower's row or the row in front switches the upper-body lift to its careful 20 px
// start (FollowerMon_UpperBodyNeedsCarefulStart, ENTRY P74) - below that line only the plain cards compete and ENTRY
// P59's bias puts the PLAYER 0.5 unit in front, above it the follower wins: a hard horizontal cut through the player's
// hat. It went away when the wandering NPC walked out of range. ENTRY P81 did not touch the careful mode (and its
// overlap rule also fired mid-way through vertical swaps), so it was reverted.
// Fix: whenever the follower should be on top of the player - large follower facing west/east (ENTRY P79/P80), on the
// player's row and within about a tile and a half (beside it, or walking through it) - P59's 0.5 unit AWAY from the
// camera is replaced by FOLLOWER_OVER_PLAYER_DEPTH_BIAS TOWARD the camera, along the camera's own back axis (camera
// matrix column 2), so the sprite does not move on screen. The whole follower is then ~2.5 units in front of the
// player's card at every height, in either lift mode, while an NPC one row in front (8.2 units nearer than the
// follower's tile) stays ~5.7 units in front of it, and a tree/fence face at the south edge of the follower's tile stays
// in front too. Not when the follower stands on overlay ground (its own tall grass etc. is only 2 units nearer).
#define FOLLOWER_OVER_PLAYER_DEPTH_BIAS FX32_CONST(2.5)
#define FOLLOWER_OVER_PLAYER_MAX_DZ     FX32_CONST(4)
#define FOLLOWER_OVER_PLAYER_MAX_DX     FX32_CONST(24)

static BOOL FollowerMon_IsOverlayTerrain(u32 tileBehavior);

static BOOL FollowerMon_DrawsOverPlayer(MapObject *followerObj)
{
    int facing = MapObject_GetFacingDir(followerObj);

    if ((facing != DIR_WEST && facing != DIR_EAST) || !FollowerMon_IsTallGraphicsID(MapObject_GetGraphicsID(followerObj))
        || FollowerMon_IsOverlayTerrain(MapObject_GetCurrTileBehavior(followerObj))) {
        return FALSE;
    }

    FieldSystem *fieldSystem = MapObject_FieldSystem(followerObj);

    if (fieldSystem == NULL || fieldSystem->playerAvatar == NULL) {
        return FALSE;
    }

    MapObject *playerObj = PlayerAvatar_GetMapObject(fieldSystem->playerAvatar);

    if (playerObj == NULL) {
        return FALSE;
    }

    VecFx32 followerPos, playerPos;

    MapObject_GetPosPtr(followerObj, &followerPos);
    MapObject_GetPosPtr(playerObj, &playerPos);

    fx32 dx = followerPos.x - playerPos.x;
    fx32 dz = followerPos.z - playerPos.z;

    return dz > -FOLLOWER_OVER_PLAYER_MAX_DZ && dz < FOLLOWER_OVER_PLAYER_MAX_DZ
        && dx > -FOLLOWER_OVER_PLAYER_MAX_DX && dx < FOLLOWER_OVER_PLAYER_MAX_DX;
}

static void FollowerMon_ApplyMonOnlyOffsets(MapObject *mapObj)
{
    if (MapObject_GetMovementType(mapObj) != MOVEMENT_TYPE_FOLLOW_PLAYER) {
        return;
    }

    int behindDir = Direction_GetOpposite(MapObject_GetFacingDir(mapObj));
    VecFx32 offset = { 0, 0, 0 };
    VecFx32_StepDirection(behindDir, &offset, FOLLOWER_LAG_BASELINE);

    offset.z += sFollowerBounceDepthCompZ;

    if (FollowerMon_DrawsOverPlayer(mapObj)) {
        // ENTRY P82: toward the camera along its back axis (camera matrix column 2 = back, in world space).
        const MtxFx43 *camera = NNS_G3dGlbGetCameraMtx();

        offset.x += FX_Mul(camera->_02, FOLLOWER_OVER_PLAYER_DEPTH_BIAS);
        offset.y += FX_Mul(camera->_12, FOLLOWER_OVER_PLAYER_DEPTH_BIAS);
        offset.z += FX_Mul(camera->_22, FOLLOWER_OVER_PLAYER_DEPTH_BIAS);
    } else {
        offset.y -= FOLLOWER_DEPTH_BIAS_Y;
        offset.z -= FOLLOWER_DEPTH_BIAS_Z;
    }

    MapObject_SetSpritePosOffset(mapObj, &offset);
}

// ENTRY P49 (2026-10-01) - DIAGNOSTIC, NOT A FEATURE: user asked to rule out whether rendering ANY nonzero
// vertical offset on this billboard causes distortion, independent of animation entirely - screenshots
// afterward showed a static +1px lift STILL distorted specifically when facing east. Direct verification
// (texture-mirror pixel comparison, camera-angle config check) subsequently ruled out both bad source art and
// an asymmetric camera as the cause, without ever pinning down a definitive culprit.
// ENTRY P51 (2026-10-01): user tested on a different emulator and it looked much better - ENTRY P49's
// distortion was most likely a DeSmuME-specific rendering quirk, not a real engine bug. Restored ENTRY P48's
// confirmed-clean bounce (MapObject_SetSpriteJumpOffset + sFollowerBounceTable, the official-HG-jump-curve
// mechanism from ENTRY P47/P48 - NOT the MapObject_SetSpritePosOffset mechanism ENTRY P38-P46 built to mirror
// Uxie/Mesprit/Azelf's hover; those are two different fields/mechanisms, and the user specifically asked for
// this older jumpOffset one, not the posOffset one) verbatim from
// backups/2026-09-29_platinum-followerbounce17-v0_pre-fix, then slowed its cadence by 10% (see
// FOLLOWER_BOUNCE_ADVANCE_NUM/DEN below) per the user's request.
// ENTRY P53 (2026-10-01): user reported the bounce spends much longer "up" than "down" (~0.3s vs ~0.1s) -
// this table (ported verbatim from HG's/Platinum's official sJumpHeights_Low in ENTRY P47) is genuinely NOT
// symmetric: the peak (2) holds for 3 entries (idx 5-7), but the descent ALSO lingers at 1.66667 for 2
// entries (idx 8-9) where the ascent only passes through 1.66667 once (idx 4) - so 6 of 16 entries sit at or
// near the top (idx 4-9) versus only 4 near the bottom (idx 13-15 plus the idx-0 wrap), a real ~3:2 skew that
// matches the user's reported ~3:1 perceived skew closely enough to be the actual cause. Rebuilt from scratch
// as a genuinely symmetric curve instead of continuing to chase the source table's exact (asymmetric) shape:
// 3 entries flat at the bottom (idx 0-2), a uniform 5-step linear ramp up (idx 3-7, constant 0.33333 delta -
// smooth/multi-step, preserving ENTRY P47's ramp-not-toggle finding), 3 entries flat at the peak (idx 8-10),
// then the same 5-step ramp back down (idx 11-15, mirrored exactly) before wrapping to the bottom hold again.
// Top and bottom now hold for the exact same 3 entries each, and both ramps are the same length with the same
// per-step delta, so time spent "up" and "down" is now genuinely equal, not just close. Table stays 16 entries
// long and FOLLOWER_BOUNCE_ADVANCE_NUM/DEN below is untouched, so the overall cycle rate ("bounces per
// minute") is unchanged - only how those 16 slots are distributed within one cycle changed.
static const fx32 sFollowerBounceTable[16] = {
    FX32_CONST(0), FX32_CONST(0), FX32_CONST(0), FX32_CONST(0.33333),
    FX32_CONST(0.66667), FX32_CONST(1), FX32_CONST(1.33333), FX32_CONST(1.66667),
    FX32_CONST(2), FX32_CONST(2), FX32_CONST(2), FX32_CONST(1.66667),
    FX32_CONST(1.33333), FX32_CONST(1), FX32_CONST(0.66667), FX32_CONST(0.33333),
};

// ENTRY P51: advances sBounceIndex 9 times every 10 calls instead of every single call, via a small integer
// accumulator - an exact 90% rate (10% slower than ENTRY P47/P48's original 1-step-per-call cadence), evenly
// spaced rather than bursty. A plain integer ratio rather than a fixed-point multiply, matching this file's
// existing FOLLOWER_IDLE_ANIM_INTERVAL-style accumulate-and-flush counter convention below.
// ENTRY P52 (2026-10-01): user asked to speed the bounce up 80% from wherever ENTRY P51 left it - 0.9 * 1.8 =
// 1.62 steps/call, i.e. 81/50 (already lowest terms: 81 = 3^4, 50 = 2*5^2, no common factor). Since NUM now
// exceeds DEN, a single call can need to advance sBounceIndex by more than 1 step to stay caught up - the
// flush below is a while loop instead of ENTRY P51's if, otherwise the exact same accumulate-and-flush shape.
// ENTRY P54 (2026-10-01): user asked for "about 10% more" on top of ENTRY P52's 1.62 steps/call - 1.62 * 1.1 =
// 1.782 exactly, which has no clean small fraction (891/500). Kept the existing DEN=50 (consistent with
// ENTRY P52, keeps sBounceAccum's u8 range comfortably safe) and scaled NUM by the same 1.1, rounded to the
// nearest whole number: round(81 * 1.1) = round(89.1) = 89. 89/50 = 1.78 steps/call, a 9.88% increase over
// 81/50 - matches "about 10%" as asked, not claimed as an exact 10.00%.
#define FOLLOWER_BOUNCE_ADVANCE_NUM 89
#define FOLLOWER_BOUNCE_ADVANCE_DEN 50

// ENTRY P68/P70: "too tall" (64x64) followers. User report from classic HGSS: their bob and walk-frame change happen
// on identical frames. ENTRY P68 slowed the bob to half speed but stepped the frame once per cycle when the smooth
// bob table returned to 0 - and with a smooth ramp the sprite's visible 1px move lands somewhere mid-ramp
// (wherever the sub-pixel position rounds), never reliably on a chosen table entry, so they could not line up.
// ENTRY P70: in HG the follower's bob is part of its walk frames (ENTRY P45), so for tall followers the bob is a
// two-state step - down for FOLLOWER_TALL_BOB_HALF_CYCLE frames, up (the same peak as the regular table) for the
// same - and the walk frame is set from that state every frame (down = first frame, up = second): one frame
// change on each bob, on the identical frame, and a turn's animation reset can never put them out of phase.
// 2 x 9 frames = the same 18-frame cycle as ENTRY P68's half-speed bob, so the bob speed is unchanged and the walk
// frame now changes twice as often (every 9 frames instead of every 18).
#define FOLLOWER_TALL_BOB_HALF_CYCLE 9
#define FOLLOWER_TALL_BOB_HEIGHT     FX32_CONST(2)

static BOOL sTallBobUp = FALSE;

static void FollowerMon_ApplyBounce(MapObject *mapObj)
{
    static u8 sBounceIndex = 0;
    static u8 sBounceAccum = 0;
    static u8 sTallBobCounter = 0;

    sFollowerBounceDepthCompZ = 0;

    if (MapObject_GetMovementType(mapObj) != MOVEMENT_TYPE_FOLLOW_PLAYER) {
        return;
    }

    // ENTRY P63: while the player is talking to the follower (src/follower_mon_talk.c) the idle bounce is held
    // and HG's talk animation drives the sprite instead, for every species: x/z = HG's per-frame position moves
    // (added to jumpOffset like the object itself moving, so the shadow follows, as in HG), height through the
    // same depth-neutral path as the bounce (ENTRY P59) so it never changes draw order with the player.
    VecFx32 talkOffset;

    if (FollowerMonTalk_GetRenderOffset(&talkOffset)) {
        VecFx32 offset;
        MapObject_GetSpriteJumpOffset(mapObj, &offset);
        offset.x += talkOffset.x;
        offset.z += talkOffset.z;
        offset.y = FX_Mul(talkOffset.y, FOLLOWER_BOUNCE_Y_SCALE);
        MapObject_SetSpriteJumpOffset(mapObj, &offset);
        sFollowerBounceDepthCompZ = -FX_Mul(talkOffset.y, FOLLOWER_BOUNCE_Z_SCALE);
        return;
    }

    if (FollowerMon_GetHGPristinePalette(MapObject_GetGraphicsID(mapObj)) == NULL) {
        return;
    }

    // ENTRY P65: no idle bounce on top of a real jump arc (the follower's ledge leap).
    if (FollowerMon_IsJumping(mapObj)) {
        return;
    }

    // Get-modify-set, not a fresh {0,0,0} vector - FollowerMon_ApplyVisualLag (above, runs first in
    // ov5_021EB398) already wrote this same jumpOffset's x/z for the horizontal lag; this must preserve
    // that, only ever touching y.
    // ENTRY P59: the table value b is still the bounce amplitude the user tuned, but it is applied along the
    // screen-up vector instead of straight up (see FOLLOWER_DEPTH_BIAS_Y's comment) so the follower's depth
    // no longer changes during the bounce: Y scaled by cos(p)^2 here, the matching -Z handed to
    // FollowerMon_ApplyMonOnlyOffsets via sFollowerBounceDepthCompZ (posOffset, which the shadow never reads).
    BOOL tall = FollowerMon_IsTallGraphicsID(MapObject_GetGraphicsID(mapObj));
    fx32 bounce;

    if (tall) {
        // ENTRY P70: two-state bob; FollowerMon_ApplyWalkAnimation shows the matching walk frame this same call.
        if (++sTallBobCounter >= FOLLOWER_TALL_BOB_HALF_CYCLE) {
            sTallBobCounter = 0;
            sTallBobUp = !sTallBobUp;
        }

        bounce = sTallBobUp ? FOLLOWER_TALL_BOB_HEIGHT : 0;
    } else {
        bounce = sFollowerBounceTable[sBounceIndex];
    }

    VecFx32 offset;
    MapObject_GetSpriteJumpOffset(mapObj, &offset);
    offset.y = FX_Mul(bounce, FOLLOWER_BOUNCE_Y_SCALE);
    MapObject_SetSpriteJumpOffset(mapObj, &offset);
    sFollowerBounceDepthCompZ = -FX_Mul(bounce, FOLLOWER_BOUNCE_Z_SCALE);

    if (tall) {
        return;
    }

    sBounceAccum += FOLLOWER_BOUNCE_ADVANCE_NUM;
    while (sBounceAccum >= FOLLOWER_BOUNCE_ADVANCE_DEN) {
        sBounceAccum -= FOLLOWER_BOUNCE_ADVANCE_DEN;
        sBounceIndex = (sBounceIndex + 1) & 0xF;
    }
}

// ENTRY P30 (2026-10-01): the smooth ENTRY P29 approach (Billboard_AdvanceAnim(billboard, FX32_ONE/2) every
// single call while idle) was confirmed STILL invisible in-game even from this proven-continuous call site -
// so the underlying accumulation itself (not just the call site) is suspect. Per the user's own direction
// ("use a workaround to play the frames cyclically every x frames go by"), replaced the smooth per-frame
// dribble with a blunt, impossible-to-lose counter: every FOLLOWER_IDLE_ANIM_INTERVAL calls, jump frameNum
// forward by one whole real HG texture step (FX32_ONE * 4 - the measured spacing between hg_tsure_poke.bin's
// 8 sequence slots, ENTRY P19) in one shot, instead of relying on 60+ tiny FX32_ONE/2 adds actually
// accumulating correctly.
// ENTRY P31 fixed the actual bug (ov5_021EB398's dispatch call was resetting the frame back to 0 every idle
// tick, fighting this counter) - with that gone, the interval now genuinely controls the on-screen hold
// time. ENTRY P32 (2026-10-01): user asked to double the animation speed once it was actually visible -
// halved 30 -> 15. ENTRY P33 (2026-10-01): another 50% speed increase requested - 15 / 1.5 = 10.
// ENTRY P44 (2026-10-01): this used to freeze whenever the follower was moving (Billboard_AdvanceAnim only
// ever got called while genuinely idle) - the walking/running dispatch states (1-6) drove animation the rest
// of the time, each at its own player-speed-scaled rate, which is why the walk cycle visibly sped up with
// player speed. User asked for a single flat rate regardless of movement - "always play at the default
// resting speed" - so this now runs unconditionally (ov5_021EB398 bypasses the speed-scaled dispatch
// entirely for HG species now, not just for state 0 - see its own updated comment), making this the sole
// animation driver at every movement speed, always at this same FOLLOWER_IDLE_ANIM_INTERVAL/STEP rate.
#define FOLLOWER_IDLE_ANIM_INTERVAL 10
#define FOLLOWER_IDLE_ANIM_STEP (FX32_ONE * 4)

static void FollowerMon_ApplyWalkAnimation(MapObject *mapObj, Billboard *billboard)
{
    static u32 sAnimCounter = 0;

    if (MapObject_GetMovementType(mapObj) != MOVEMENT_TYPE_FOLLOW_PLAYER) {
        return;
    }

    // HG-sourced species only (matches ENTRY P21/P23's precedent) - the 20 native species use Platinum's
    // own original walk-cycle convention, which the user hasn't asked to change.
    if (FollowerMon_GetHGPristinePalette(MapObject_GetGraphicsID(mapObj)) == NULL) {
        return;
    }

    // ENTRY P64: HG's field-move reaction pauses the follower's animation while it hops (PauseAnimation).
    if (FollowerMonTalk_IsAnimationPaused()) {
        return;
    }

    // ENTRY P70: tall followers show the walk frame that belongs to the current bob state (see sTallBobUp): first
    // frame while down, second while up - set, not advanced, so it always matches the bob on the same frame.
    if (FollowerMon_IsTallGraphicsID(MapObject_GetGraphicsID(mapObj))) {
        Billboard_SetAnimFrameNum(billboard, sTallBobUp ? FOLLOWER_IDLE_ANIM_STEP : 0);
        return;
    }

    sAnimCounter++;

    if (sAnimCounter >= FOLLOWER_IDLE_ANIM_INTERVAL) {
        sAnimCounter = 0;
        Billboard_AdvanceAnim(billboard, FOLLOWER_IDLE_ANIM_STEP);
    }
}

// ENTRY P73/P74: LARGE FOLLOWERS DRAW OVER THE SCENERY BEHIND AND BESIDE THEM (replaces ENTRY P71/P72).
// Every overworld sprite is a camera-facing card (NNS_G3D_SBC_BB). With the field camera looking down ~59 degrees a
// card leans back into the world: a point s px up the card is 0.86 * s units north of its feet, so a 64 px follower
// sinks into props behind it. Trainer Tips sign (fldeff model 74: board y 12-28, front face 1 unit north of its tile
// centre after its z -2 placement): one tile north of the follower it covers the card from ~24.5 to ~35 px; on the
// follower's own row, one tile to the side, it covers the card's side from ~10 to ~21 px.
// Depth only, no model change (ENTRY P73): right after the follower is drawn normally, BillboardList_Draw calls
// FollowerMon_DrawUpperBodyPasses, which draws the same billboard again with a projection matrix whose depth column is
// tilted: clip z' = z + a * (clipY - startY). Clip x, y and w are untouched, so the second pass covers exactly the same
// pixels with exactly the same texels and the sprite cannot look different; only its depth changes (the field already
// does a depth-only projection tweak for all sprites, fieldmap.c: _32 += _22 * 4). A point s px up the card gets the
// depth of an UPRIGHT creature at the same screen spot, tan(pitch) * (s - start) units nearer the camera; below the
// start line the pass is pushed back instead and adds nothing.
// ENTRY P74 (user: the sign beside the follower still covered its side; "photo 2 he clips into my hat"):
//  - The start line is now the follower's feet (FOLLOWER_UPPER_BODY_START_PX 0): the whole sprite gets the depth of an
//    upright creature standing at its card origin, 2 units north of its tile's south edge. Every signpost/mailbox
//    model's front face is at most 3.5 units south of its own tile centre (decoded fldeff models 0x45-0x4A), i.e. at
//    least 2.5 units behind that line on the follower's row, so they no longer cover it; trees, fences, roofs and
//    cards in front of that line still do.
//  - Lifting from the feet would also put the follower over FLAT things in front of its lower body: its own ground
//    overlay (tall grass, mud, deep snow, shallow water), the player's overlay when side by side (the player's pass
//    uses the same tilt, below), or an NPC card on the follower's row or the row in front of it. In those cases
//    (checked every update) the start goes back to ENTRY P73's 20 px (FOLLOWER_UPPER_BODY_START_PX_CAREFUL).
//  - The player: ENTRY P73 gave the player its own pass, measured from the player's own feet and gated by a flag, which
//    left only ENTRY P59's 0.5 unit between the two side by side - the follower ended up over the player's hat. Now the
//    follower's own post-draw callback draws the player's lifted pass right after its own, with exactly the same tilt
//    (same a, same start line on screen). Both cards get the same depth change at every pixel, so their order is
//    exactly the order of the plain cards: P59's player on top side by side, the player in front when it is one tile
//    south. Not drawn when the follower is south of the player (it is in front anyway, and the shared tilt would lift
//    the player's whole body). ENTRY P79: also not drawn on the player's own row - the follower stays on top there.
#define FOLLOWER_UPPER_BODY_START_PX         0  // ENTRY P74: lift from the feet (upright creature)
#define FOLLOWER_UPPER_BODY_START_PX_CAREFUL 20 // ENTRY P73's start: tall grass covers the lowest ~13 px, a card one tile south ~18 px
#define FOLLOWER_UPPER_BODY_MAX_LIFT_RATE    FX32_CONST(4) // guard for a near top-down camera (default camera: ~1.67)
#define FOLLOWER_UPPER_BODY_NPC_REACH_TILES  3  // half the 64 px sprite + 10 px of lag + half a 32 px sprite, in tiles
#define FOLLOWER_PLAYER_PASS_MIN_NORTH       FX32_CONST(8) // ENTRY P79: player pass only if the follower is >= half a tile north (facing W/E)
#define FOLLOWER_PLAYER_PASS_MAX_SOUTH       FX32_CONST(2) // ENTRY P75/P80: follower facing N/S, farther south than this: no player pass

static int sFollowerUpperBodyStartPx = FOLLOWER_UPPER_BODY_START_PX_CAREFUL;

static fx32 FollowerMon_ClipW(const VecFx32 *viewPos, const MtxFx44 *projection)
{
    return FX_Mul(viewPos->x, projection->_03) + FX_Mul(viewPos->y, projection->_13) + FX_Mul(viewPos->z, projection->_23) + projection->_33;
}

// The depth tilt of one frame, computed from the follower's card with the matrices it was just drawn with (camera
// history/delay included): a, and the start line as a screen (NDC) height.
static BOOL FollowerMon_ComputeLiftParams(const Billboard *billboard, int startPx, fx32 *outA, fx32 *outStartNdcY)
{
    const MtxFx43 *camera = NNS_G3dGlbGetCameraMtx();
    const MtxFx44 *projection = NNS_G3dGlbGetProjectionMtx();
    VecFx32 origin;

    // NNSi_G3dFuncSbc_BB gives the card the camera's axes: a point s units up it is origin + (0, s, 0) in view space.
    MTX_MultVec43(&billboard->pos, camera, &origin);

    // Camera matrix columns 1 and 2 are the view up and back axes; an upright object's point at the same screen spot
    // as card height s is s * back.y / up.y nearer the camera.
    if (camera->_11 <= 0 || camera->_12 <= 0) {
        return FALSE;
    }

    fx32 liftRate = FX_Div(camera->_12, camera->_11);

    if (liftRate > FOLLOWER_UPPER_BODY_MAX_LIFT_RATE) {
        liftRate = FOLLOWER_UPPER_BODY_MAX_LIFT_RATE;
    }

    fx32 startY = origin.y + FX_Mul(FX32_CONST(startPx), billboard->scale.y);
    fx32 w = FollowerMon_ClipW(&origin, projection);
    fx32 z = FX_Mul(origin.x, projection->_02) + FX_Mul(origin.y, projection->_12) + FX_Mul(origin.z, projection->_22) + projection->_32;
    fx32 clipYAtStart = FX_Mul(origin.x, projection->_01) + FX_Mul(startY, projection->_11) + FX_Mul(origin.z, projection->_21) + projection->_31;
    fx32 clipYPerUnit = FX_Mul(w, projection->_11);

    if (w <= 0 || clipYPerUnit <= 0) {
        return FALSE;
    }

    // Moving a point toward the camera changes its NDC depth by (_22 * w - z * _23) / w^2 per unit, and every vertex of
    // the card has the same w, so s - start = (clipY - clipYAtStart) / _11. Wanted: depth change = that slope *
    // liftRate * (s - start), i.e. clip z' = z + a * (clipY - clipYAtStart) with a = slope * liftRate / (w * _11).
    // a < 0: points above the start line get nearer, points below it farther.
    fx32 depthSlope = FX_Mul(projection->_22, w) - FX_Mul(z, projection->_23);

    *outA = FX_Div(FX_Mul(depthSlope, liftRate), clipYPerUnit);
    *outStartNdcY = FX_Div(clipYAtStart, w);

    return TRUE;
}

// The tilted projection for one card: clip z' = z + a * clipY - a * startNdcY * w, w being this card's (the same at all
// of its vertices). In NDC that is a * (y - startNdcY) for ANY card, so every card drawn with these a and startNdcY
// gets the same depth change at the same screen height. The constant goes straight into _32 (not as a multiple of w)
// and is formed from startNdcY * w first, so its fx32 rounding stays ~1/1000 of ENTRY P59's 0.5 unit margin.
static BOOL FollowerMon_BuildLiftedProjection(const Billboard *billboard, fx32 a, fx32 startNdcY, MtxFx44 *outProjection)
{
    const MtxFx43 *camera = NNS_G3dGlbGetCameraMtx();
    const MtxFx44 *projection = NNS_G3dGlbGetProjectionMtx();
    VecFx32 origin;

    MTX_MultVec43(&billboard->pos, camera, &origin);

    fx32 w = FollowerMon_ClipW(&origin, projection);

    if (w <= 0) {
        return FALSE;
    }

    *outProjection = *projection;
    outProjection->_02 += FX_Mul(a, projection->_01);
    outProjection->_12 += FX_Mul(a, projection->_11);
    outProjection->_22 += FX_Mul(a, projection->_21);
    outProjection->_32 += FX_Mul(a, projection->_31) - FX_Mul(a, FX_Mul(startNdcY, w));

    return TRUE;
}

static Billboard *FollowerMon_GetPlayerBillboard(MapObject *followerObj)
{
    FieldSystem *fieldSystem = MapObject_FieldSystem(followerObj);

    if (fieldSystem == NULL || fieldSystem->playerAvatar == NULL) {
        return NULL;
    }

    MapObject *playerObj = PlayerAvatar_GetMapObject(fieldSystem->playerAvatar);

    if (playerObj == NULL || ov5_021EDD94(playerObj) == 1) {
        return NULL;
    }

    return ov5_021EB1A0(playerObj);
}

// Post-draw callback of a large follower's billboard (param: the follower's MapObject).
static void FollowerMon_DrawUpperBodyPasses(Billboard *billboard, void *followerObj)
{
    fx32 a, startNdcY;
    MtxFx44 projection;

    if (!FollowerMon_ComputeLiftParams(billboard, sFollowerUpperBodyStartPx, &a, &startNdcY)) {
        return;
    }

    if (FollowerMon_BuildLiftedProjection(billboard, a, startNdcY, &projection)) {
        Billboard_RedrawWithProjection(billboard, &projection);
    }

    Billboard *playerBillboard = FollowerMon_GetPlayerBillboard(followerObj);

    // ENTRY P79: only while the follower is at least half a tile BEHIND (north of) the player - the walking-up/down
    // case the user is happy with. On the player's own row the user now wants the follower on top (photos, real DS
    // Lite / 3DS: "the corner of the hat clips through Rayquaza's mouth"; melonDS showed no clipping). With the
    // shared tilt the player's lifted pass sat only ENTRY P59's 0.5 unit from the follower's - close enough that real
    // hardware and melonDS resolve it differently. Without it, the follower's lifted pass beats the player's plain
    // card by its full lift (tens of units over the player's head) on any hardware.
    // ENTRY P80: the P79 rule only while the follower FACES WEST OR EAST (user: led left/right "it looks great", but
    // walking down then stepping left/right - follower beside the player, still facing down - its body now drew in
    // front of the player; "isolate your fix ... to only be active if the large mon faces left or faces right").
    // Facing north/south it is ENTRY P75's rule again: player pass unless the follower is more than 2 units south.
    int followerFacing = MapObject_GetFacingDir(followerObj);
    fx32 followerSouthOfPlayer = playerBillboard != NULL ? billboard->pos.z - playerBillboard->pos.z : 0;
    BOOL drawPlayerPass = (followerFacing == DIR_WEST || followerFacing == DIR_EAST)
        ? followerSouthOfPlayer <= -FOLLOWER_PLAYER_PASS_MIN_NORTH
        : followerSouthOfPlayer <= FOLLOWER_PLAYER_PASS_MAX_SOUTH;

    if (playerBillboard != NULL && drawPlayerPass
        && FollowerMon_BuildLiftedProjection(playerBillboard, a, startNdcY, &projection)) {
        Billboard_DrawOtherWithProjection(playerBillboard, billboard, &projection);
    }
}

// Ground the game draws an overlay on for objects standing in it, covering their lower body.
static BOOL FollowerMon_IsOverlayTerrain(u32 tileBehavior)
{
    u8 behavior = tileBehavior;

    return TileBehavior_IsTallGrass(behavior) || TileBehavior_IsVeryTallGrass(behavior)
        || TileBehavior_IsMud(behavior) || TileBehavior_IsDeepMud(behavior)
        || TileBehavior_IsMudWithGrass(behavior) || TileBehavior_IsDeepMudWithGrass(behavior)
        || TileBehavior_IsDeepSnow(behavior) || TileBehavior_IsDeeperSnow(behavior) || TileBehavior_IsDeepestSnow(behavior)
        || TileBehavior_IsShallowWater(behavior);
}

// Like ov5_021ECD04, but returns MODEL_TYPE_NONE instead of asserting for a graphics ID it does not know.
static u32 FollowerMon_GetModelType(u32 graphicsID)
{
    for (const ObjectEventGfxRenderDetailsEntry *entry = gObjectEventGfxRenderDetailsTable; entry->graphicsID != 0xffff; entry++) {
        if (entry->graphicsID == graphicsID) {
            return entry->modelType;
        }
    }

    return MODEL_TYPE_NONE;
}

// ENTRY P74: TRUE when lifting the follower from its feet would draw it over a flat card in front of its lower body
// (see the ENTRY P73/P74 comment above).
static BOOL FollowerMon_UpperBodyNeedsCarefulStart(MapObject *followerObj)
{
    FieldSystem *fieldSystem = MapObject_FieldSystem(followerObj);

    if (fieldSystem == NULL || fieldSystem->playerAvatar == NULL) {
        return TRUE;
    }

    if (FollowerMon_IsOverlayTerrain(MapObject_GetCurrTileBehavior(followerObj))) {
        return TRUE;
    }

    MapObject *playerObj = PlayerAvatar_GetMapObject(fieldSystem->playerAvatar);
    int followerX = MapObject_GetX(followerObj);
    int followerZ = MapObject_GetZ(followerObj);

    if (playerObj != NULL && MapObject_GetZ(playerObj) == followerZ && FollowerMon_IsOverlayTerrain(MapObject_GetCurrTileBehavior(playerObj))) {
        return TRUE;
    }

    MapObject *mapObj;
    int index = 0;

    while (MapObjectMan_FindObjectWithStatus(fieldSystem->mapObjMan, &mapObj, &index, MAP_OBJ_STATUS_0) == TRUE) {
        if (mapObj == followerObj || mapObj == playerObj || MapObject_CheckStatusFlag(mapObj, MAP_OBJ_STATUS_HIDE)) {
            continue;
        }

        int dx = MapObject_GetX(mapObj) - followerX;
        int dz = MapObject_GetZ(mapObj) - followerZ;

        if (dz < 0 || dz > 1 || dx < -FOLLOWER_UPPER_BODY_NPC_REACH_TILES || dx > FOLLOWER_UPPER_BODY_NPC_REACH_TILES) {
            continue;
        }

        // Signposts, mailboxes etc. are 3D scenery - the follower should be in front of those.
        if (FollowerMon_GetModelType(MapObject_GetGraphicsID(mapObj)) == MODEL_TYPE_3D) {
            continue;
        }

        return TRUE;
    }

    return FALSE;
}

// Called every frame from ov5_021EB398, after the card position is committed. This renderer is shared by most NPCs,
// so only the follower installs anything; small followers clear the callback again.
static void FollowerMon_UpdateUpperBodyPass(MapObject *mapObj, Billboard *billboard)
{
    if (MapObject_GetMovementType(mapObj) != MOVEMENT_TYPE_FOLLOW_PLAYER) {
        return;
    }

    if (!FollowerMon_IsTallGraphicsID(MapObject_GetGraphicsID(mapObj))) {
        Billboard_SetPostDrawCallback(billboard, NULL, NULL);
        return;
    }

    sFollowerUpperBodyStartPx = FollowerMon_UpperBodyNeedsCarefulStart(mapObj) ? FOLLOWER_UPPER_BODY_START_PX_CAREFUL : FOLLOWER_UPPER_BODY_START_PX;
    Billboard_SetPostDrawCallback(billboard, FollowerMon_DrawUpperBodyPasses, mapObj);
}

void ov5_021EB398(MapObject *mapObj)
{
    int v0, v1;
    UnkStruct_ov5_021EB2EC *v2 = sub_02062AF0(mapObj);
    Billboard *v3 = v2->unk_04;

    v2 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    if (v3 == NULL) {
        return;
    }

    v0 = MapObject_GetFacingDir(mapObj);
    v1 = sub_02062A14(mapObj);

    GF_ASSERT(v1 < (0x8 + 1));

    // ENTRY P31 (2026-10-01): found the actual reason ENTRY P29/P30's idle animation kept getting wiped out
    // - read every function ov5_021EB478 (the v1==0/idle dispatch target) calls, not just its own body.
    // When direction hasn't changed (the normal case while genuinely standing still), it calls
    // ov5_021EC638(billboard, 8) via ov5_021EC668, which does `floor(animFrameNum / 8) * 8` - and since
    // every one of this renderer's anims is exactly 8 raw frames wide (see sHGTsurePokeBillboardAnims,
    // object_event_gfx_data.c), that floor is ALWAYS 0. In other words: ov5_021EB478 forces animFrameNum
    // back to exactly 0 - EVERY single idle field frame, unconditionally - immediately before
    // FollowerMon_ApplyIdleAnimation below ever runs. Fix: skip the dispatch entirely (and therefore this
    // reset) for the exact case it's needed for - an HG follower species, facing the same direction as last
    // frame - and let FollowerMon_ApplyIdleAnimation be the sole owner of frameNum instead of racing against
    // a reset/speed-scaled-advance that runs first every time. A real direction change (v0 != v2->unk_02)
    // still dispatches normally, unaffected - that's the legitimate "snap to the new direction's first
    // frame" case, not the bug.
    // ENTRY P44 (2026-10-01): originally this only applied when v1==0 (idle) - user reported the follower's
    // walk-cycle speed still visibly changed with player speed (faster walking, faster still running) and
    // asked for it to be flat, always at the same "default resting" rate regardless of movement. Dropped the
    // v1==0 condition entirely - this now bypasses the dispatch's per-speed-tier Billboard_AdvanceAnim calls
    // (states 1-6, each with its own scaled rate) the same way it already bypassed state 0's reset, for
    // every movement state, not just idle.
    BOOL skipDispatchForFixedAnimSpeed = (v0 == v2->unk_02
        && MapObject_GetMovementType(mapObj) == MOVEMENT_TYPE_FOLLOW_PLAYER
        && FollowerMon_GetHGPristinePalette(MapObject_GetGraphicsID(mapObj)) != NULL);

    if (!skipDispatchForFixedAnimSpeed) {
        Unk_ov5_021FF3D4[v1](mapObj, v3, v2, v0);
    }

    v2->unk_02 = v0;
    v2->unk_03 = sub_02062A14(mapObj);

    // ENTRY P50 (2026-10-01): these three used to run AFTER ov5_021EDEB4 (which reads jumpOffset/posOffset
    // and commits them to the actual render position via Billboard_SetPos) - meaning whatever offset they
    // computed here, using THIS frame's just-updated v0/facing direction, never reached the screen until
    // ov5_021EB398's NEXT call. On an ordinary frame (facing unchanged) that 1-frame lag is invisible - the
    // offset from last frame and this frame are the same value anyway. But on the exact frame the follower's
    // facing direction changes (the dispatch just above already updated the BILLBOARD TEXTURE to the new
    // direction, this same frame), ov5_021EDEB4 was still committing the offset computed LAST frame - i.e.
    // for the OLD direction. Sprite shows the new direction's texture, but sits offset for the old one, for
    // exactly one frame, every single direction change - a real, provable one-frame desync (perceived as "a
    // brief flash", "off by 1 to 3 frames" per the user's own report - one genuine frame plus however long
    // it reads at a glance). Fix: moved ahead of ov5_021EDEB4/ov5_021EC6C0 so the offset these compute (from
    // the SAME v0 the dispatch above just used to pick the new texture) is committed to the render position
    // in the SAME frame the texture changes - no separate turn-then-shift step, both happen together.
    FollowerMon_ApplyVisualLag(mapObj);
    FollowerMon_ApplyBounce(mapObj);
    FollowerMon_ApplyMonOnlyOffsets(mapObj);

    ov5_021EDEB4(mapObj, v3);
    ov5_021EC6C0(mapObj, v3);

    FollowerMon_ApplyWalkAnimation(mapObj, v3);
    FollowerMon_UpdateUpperBodyPass(mapObj, v3);
}

void ov5_021EB40C(MapObject *mapObj)
{
    int v0, v1;
    UnkStruct_ov5_021EB2EC *v2;
    Billboard *v3;

    v2 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    v3 = v2->unk_04;

    if (v3 == NULL) {
        return;
    }

    ov5_021EDEB4(mapObj, v3);
    ov5_021EC6C0(mapObj, v3);
}

void ov5_021EB438(MapObject *mapObj)
{
    int v0, v1;
    UnkStruct_ov5_021EB2EC *v2;
    Billboard *v3;

    v2 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    v3 = v2->unk_04;

    if (v3 == NULL) {
        return;
    }

    if (ov5_021ECD38(mapObj) == 0) {
        Billboard_AdvanceAnim(v3, (FX32_ONE));
    }

    ov5_021EDEB4(mapObj, v3);
    ov5_021EC6C0(mapObj, v3);
}

// ENTRY P21 (2026-10-01) added FollowerMon_IsHGImportedSpecies here (a scope check for the ENTRY P20
// idle-animation/bounce additions). REMOVED in ENTRY P25 along with its last caller - see ov5_021EB478
// above for why (real HGSS followers freeze when idle, same as every other species already did).

static void ov5_021EB478(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EB2EC *param2, int param3)
{
    if (param3 != param2->unk_02) {
        int v0 = ov5_021EDF18(param3);

        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
    } else {
        ov5_021EC668(param1);
    }

    // ENTRY P20/P21 (2026-10-01) added a HG-species-only exception here to keep animating while idle
    // instead of freezing - REVERTED in ENTRY P25: the user clarified real HGSS followers DO freeze when
    // idle, same as every other species already correctly does by default ("the HG ss sprites are not
    // idly cycling through their frames. That is what they are suppose to do."). Restored to plain,
    // unconditional stock behavior - no species-specific branching here at all anymore.
    Billboard_AdvanceAnim(param1, 0);
}

static void ov5_021EB4AC(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EB2EC *param2, int param3)
{
    if (param3 != param2->unk_02) {
        int v0 = ov5_021EDF18(param3);

        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
    }

    if (ov5_021ECD38(mapObj) == 0) {
        Billboard_AdvanceAnim(param1, ((FX32_ONE) / 2));
    }
}

static void ov5_021EB4E8(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EB2EC *param2, int param3)
{
    if (param3 != param2->unk_02) {
        int v0 = ov5_021EDF18(param3);

        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
    }

    if (ov5_021ECD38(mapObj) == 0) {
        Billboard_AdvanceAnim(param1, (FX32_ONE));
    }
}

static void ov5_021EB524(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EB2EC *param2, int param3)
{
    if (param3 != param2->unk_02) {
        int v0 = ov5_021EDF18(param3);

        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
    }

    if (ov5_021ECD38(mapObj) == 0) {
        Billboard_AdvanceAnim(param1, ((FX32_ONE) * 2));
    }
}

static void ov5_021EB560(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EB2EC *param2, int param3)
{
    if (param3 != param2->unk_02) {
        int v0 = ov5_021EDF18(param3);

        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
    }

    if (ov5_021ECD38(mapObj) == 0) {
        Billboard_AdvanceAnim(param1, ((FX32_ONE) * 4));
    }
}

static void ov5_021EB59C(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EB2EC *param2, int param3)
{
    if (param3 != param2->unk_02) {
        int v0 = ov5_021EDF18(param3);

        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);

        param2->unk_00 = 0;
    }

    if (param2->unk_03 != 0x6) {
        param2->unk_00 = 0;
    }

    if (ov5_021ECD38(mapObj) == 0) {
        fx32 v1[] = {
            (FX32_ONE),
            (FX32_ONE),
            ((FX32_ONE) * 2),
            (FX32_ONE),
            (FX32_ONE),
            ((FX32_ONE) * 2),
            0
        };

        Billboard_AdvanceAnim(param1, v1[param2->unk_00]);
        param2->unk_00++;

        if (v1[param2->unk_00] == 0) {
            param2->unk_00 = 0;
        }
    }
}

static void ov5_021EB620(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EB2EC *param2, int param3)
{
    if (param3 != param2->unk_02) {
        int v0 = ov5_021EDF18(param3);

        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);

        param2->unk_00 = 0;
    }

    if (param2->unk_03 != 0x7) {
        param2->unk_00 = 0;
    }

    if (ov5_021ECD38(mapObj) == 0) {
        fx32 v1[] = {
            ((FX32_ONE) * 3),
            ((FX32_ONE) * 2),
            ((FX32_ONE) * 3),
            0
        };

        Billboard_AdvanceAnim(param1, v1[param2->unk_00]);
        param2->unk_00++;

        if (v1[param2->unk_00] == 0) {
            param2->unk_00 = 0;
        }
    }
}

static void ov5_021EB69C(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EB2EC *param2, int param3)
{
    if (param3 != param2->unk_02) {
        int v0 = ov5_021EDF18(param3);

        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);

        param2->unk_00 = 0;
    }

    if (param2->unk_03 != 0x8) {
        param2->unk_00 = 0;
    }

    if (ov5_021ECD38(mapObj) == 0) {
        fx32 v1[] = {
            ((FX32_ONE) * 2),
            (FX32_ONE),
            (FX32_ONE),
            (FX32_ONE),
            (FX32_ONE),
            (FX32_ONE),
            (FX32_ONE),
            0
        };

        Billboard_AdvanceAnim(param1, v1[param2->unk_00]);

        param2->unk_00++;
        if (v1[param2->unk_00] == 0) {
            param2->unk_00 = 0;
        }
    }
}

static void (*const Unk_ov5_021FF3D4[])(
    MapObject *, Billboard *, UnkStruct_ov5_021EB2EC *, int)
    = {
          ov5_021EB478,
          ov5_021EB4AC,
          ov5_021EB4AC,
          ov5_021EB4E8,
          ov5_021EB524,
          ov5_021EB560,
          ov5_021EB59C,
          ov5_021EB620,
          ov5_021EB69C
      };

void ov5_021EB720(MapObject *mapObj)
{
    int v0, v1;
    UnkStruct_ov5_021EB2EC *v2 = sub_02062AF0(mapObj);
    Billboard *v3 = v2->unk_04;

    v2 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    if (v3 == NULL) {
        return;
    }

    v0 = MapObject_GetFacingDir(mapObj);
    v1 = sub_02062A14(mapObj);

    GF_ASSERT(v1 < (0x9 + 1));

    Unk_ov5_021FF420[v1](mapObj, v3, v2, v0);

    v2->unk_02 = v0;
    v2->unk_03 = sub_02062A14(mapObj);

    ov5_021EDEB4(mapObj, v3);
    ov5_021EC6C0(mapObj, v3);
}

static void ov5_021EB794(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EB2EC *param2, int param3)
{
    if ((param3 != param2->unk_02) || (param2->unk_03 == 0x9)) {
        int v0 = ov5_021EDF18(param3);

        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
    } else {
        ov5_021EC668(param1);
    }

    Billboard_AdvanceAnim(param1, 0);
}

static void ov5_021EB7D0(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EB2EC *param2, int param3)
{
    if (param2->unk_03 != 0x9) {
        Billboard_SetAnimNum(param1, 4);
        Billboard_SetAnimFrameNum(param1, 0);
    }

    Billboard_AdvanceAnim(param1, (FX32_ONE));
}

static void (*const Unk_ov5_021FF420[])(MapObject *, Billboard *, UnkStruct_ov5_021EB2EC *, int) = {
    ov5_021EB794,
    ov5_021EB4AC,
    ov5_021EB4AC,
    ov5_021EB4E8,
    ov5_021EB524,
    ov5_021EB560,
    ov5_021EB59C,
    ov5_021EB620,
    ov5_021EB69C,
    ov5_021EB7D0
};

void ov5_021EB7F8(MapObject *mapObj)
{
    UnkStruct_ov5_021EB7F8 *v0 = sub_02062ACC(mapObj, (sizeof(UnkStruct_ov5_021EB7F8)));

    v0->unk_00 = -1;
    v0->unk_02 = LCRNG_Next() % 16;

    ov5_021ECF04(mapObj, &v0->unk_04);

    if (v0->unk_04 != NULL) {
        sub_02062B68(mapObj);
    }
}

void ov5_021EB834(MapObject *mapObj)
{
    UnkStruct_ov5_021EB7F8 *v0 = sub_02062AF0(mapObj);
    ov5_021ECFA4(mapObj, &v0->unk_04);
}

void ov5_021EB848(MapObject *mapObj)
{
    UnkStruct_ov5_021EB7F8 *v0 = sub_02062AF0(mapObj);

    if (v0->unk_04 != NULL) {
        ov5_021ED01C(v0->unk_04, &v0->unk_08);
    }

    ov5_021ECFA4(mapObj, &v0->unk_04);
    MapObject_SetStatusFlagOn(mapObj, MAP_OBJ_STATUS_21);
}

void ov5_021EB874(MapObject *mapObj)
{
    int v0;
    UnkStruct_ov5_021EB7F8 *v1 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    if (v1->unk_04 == NULL) {
        ov5_021ECF04(mapObj, &v1->unk_04);
    }

    if (v1->unk_04 != NULL) {
        ov5_021ED03C(v1->unk_04, &v1->unk_08);
        ov5_021EDEB4(mapObj, v1->unk_04);
        MapObject_SetStatusFlagOff(mapObj, MAP_OBJ_STATUS_21);
    }
}

void ov5_021EB8B8(MapObject *mapObj)
{
    int v0, v1;
    UnkStruct_ov5_021EB7F8 *v2 = sub_02062AF0(mapObj);
    Billboard *v3 = v2->unk_04;

    v2 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    if (v3 == NULL) {
        return;
    }

    v0 = MapObject_GetFacingDir(mapObj);
    v1 = sub_02062A14(mapObj);

    if (v2->unk_02) {
        v2->unk_02--;
        Billboard_AdvanceAnim(v3, 0);
    } else {
        Billboard_AdvanceAnim(v3, (FX32_ONE));
        v2->unk_01++;

        if (v2->unk_01 >= 32) {
            v2->unk_01 = 0;
            v2->unk_02 = LCRNG_Next() % 16;
        }
    }

    v2->unk_00 = v0;
    v2->unk_03 = sub_02062A14(mapObj);

    ov5_021EDEB4(mapObj, v3);
    ov5_021EC6C0(mapObj, v3);
}

void ov5_021EB944(MapObject *mapObj)
{
    int v0, v1;
    UnkStruct_ov5_021EB2EC *v2 = sub_02062AF0(mapObj);
    Billboard *v3 = v2->unk_04;

    v2 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    if (v3 == NULL) {
        return;
    }

    v0 = MapObject_GetFacingDir(mapObj);
    v1 = sub_02062A14(mapObj);

    switch (v1) {
    case 0x0: {
        int v4;

        if ((v0 == 0) || (v0 == 1)) {
            v4 = 0;
        } else {
            v4 = 1;
        }

        if ((Billboard_GetAnimNum(v3) != v4) || (v2->unk_03 == 0x0)) {
            Billboard_SetAnimNum(v3, v4);
            Billboard_SetAnimFrameNum(v3, 0);
            Billboard_AdvanceAnim(v3, 0);
        }

        Billboard_AdvanceAnim(v3, 0);
    } break;
    default: {
        int v5;
        fx32 v6 = 0;

        if ((v0 == 0) || (v0 == 1)) {
            v5 = 0;
        } else {
            v5 = 1;
        }

        if (Billboard_GetAnimNum(v3) != v5) {
            Billboard_SetAnimNum(v3, v5);
            Billboard_SetAnimFrameNum(v3, 0);
        }

        Billboard_AdvanceAnim(v3, FX32_ONE );
    } break;
    }

    v2->unk_02 = v0;
    v2->unk_03 = sub_02062A14(mapObj);

    ov5_021EDEB4(mapObj, v3);
    ov5_021EC6C0(mapObj, v3);
}

void ov5_021EBA0C(MapObject *mapObj)
{
    UnkStruct_ov5_021EBA0C *v0 = sub_02062ACC(mapObj, (sizeof(UnkStruct_ov5_021EBA0C)));
    v0->unk_00 = -1;

    ov5_021ECF04(mapObj, &v0->unk_04);

    if (v0->unk_04 != NULL) {
        sub_02062B68(mapObj);
    }
}

void ov5_021EBA34(MapObject *mapObj)
{
    UnkStruct_ov5_021EBA0C *v0;
    VecFx32 v1 = { 0, 0, 0 };

    v0 = sub_02062AF0(mapObj);

    ov5_021ECFA4(mapObj, &v0->unk_04);
    MapObject_SetSpriteJumpOffset(mapObj, &v1);
}

void ov5_021EBA60(MapObject *mapObj)
{
    UnkStruct_ov5_021EBA0C *v0 = sub_02062AF0(mapObj);

    if (v0->unk_04 != NULL) {
        ov5_021ED01C(v0->unk_04, &v0->unk_08);
    }

    ov5_021ECFA4(mapObj, &v0->unk_04);
    MapObject_SetStatusFlagOn(mapObj, MAP_OBJ_STATUS_21);
}

void ov5_021EBA8C(MapObject *mapObj)
{
    int v0;
    UnkStruct_ov5_021EBA0C *v1 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    if (v1->unk_04 == NULL) {
        ov5_021ECF04(mapObj, &v1->unk_04);
    }

    if (v1->unk_04 != NULL) {
        ov5_021ED03C(v1->unk_04, &v1->unk_08);
        ov5_021EDEB4(mapObj, v1->unk_04);
        MapObject_SetStatusFlagOff(mapObj, MAP_OBJ_STATUS_21);
    }
}

void ov5_021EBAD0(MapObject *mapObj)
{
    int v0, v1;
    UnkStruct_ov5_021EBA0C *v2 = sub_02062AF0(mapObj);
    Billboard *v3 = v2->unk_04;

    v2 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    if (v3 == NULL) {
        return;
    }

    v0 = MapObject_GetFacingDir(mapObj);
    v1 = sub_02062A14(mapObj);

    GF_ASSERT(v1 < ((0x8 + 1) + 1));
    Unk_ov5_021FF3F8[v1](mapObj, v3, v2, v0);

    v2->unk_00 = v0;
    v2->unk_02 = sub_02062A14(mapObj);

    ov5_021EDEB4(mapObj, v3);
    ov5_021EC6C0(mapObj, v3);
}

static void ov5_021EBB44(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EBA0C *param2, int param3)
{
    if (param3 != param2->unk_00) {
        int v0 = ov5_021EDF18(param3);

        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
        param2->unk_03_0 = 0;
    } else if (param2->unk_02 == (0x8 + 1)) {
        param2->unk_03_0 = 1;
    } else if (param2->unk_03_0 == 1) {
        ov5_021EC69C(param1, ov5_021EDF18(param3));
        param2->unk_03_0 = 0;
    } else {
        ov5_021EC668(param1);
    }
}

static void ov5_021EBBB4(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EBA0C *param2, int param3)
{
    int v0;

    if (param3 != param2->unk_00) {
        v0 = ov5_021EDF18(param3);
        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
    } else if (param2->unk_02 == (0x8 + 1)) {
        param2->unk_03_0 = 1;
        return;
    } else if (param2->unk_03_0 == 1) {
        ov5_021EC69C(param1, ov5_021EDF18(param3));
        param2->unk_03_0 = 0;
    }

    if (ov5_021ECD38(mapObj) == 0) {
        Billboard_AdvanceAnim(param1, ((FX32_ONE) / 2));
    }
}

static void ov5_021EBC28(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EBA0C *param2, int param3)
{
    int v0;

    if (param3 != param2->unk_00) {
        v0 = ov5_021EDF18(param3);
        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
    } else if (param2->unk_02 == (0x8 + 1)) {
        ov5_021EC69C(param1, ov5_021EDF18(param3));
    }

    if (ov5_021ECD38(mapObj) == 0) {
        Billboard_AdvanceAnim(param1, (FX32_ONE));
    }
}

static void ov5_021EBC7C(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EBA0C *param2, int param3)
{
    if (param3 != param2->unk_00) {
        int v0 = ov5_021EDF18(param3);

        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
    } else if (param2->unk_02 == (0x8 + 1)) {
        ov5_021EC69C(param1, ov5_021EDF18(param3));
    }

    if (ov5_021ECD38(mapObj) == 0) {
        Billboard_AdvanceAnim(param1, ((FX32_ONE) * 2));
    }
}

static void ov5_021EBCD0(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EBA0C *param2, int param3)
{
    if (param3 != param2->unk_00) {
        int v0 = ov5_021EDF18(param3);

        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
    } else if (param2->unk_02 == (0x8 + 1)) {
        ov5_021EC69C(param1, ov5_021EDF18(param3));
    }

    if (ov5_021ECD38(mapObj) == 0) {
        Billboard_AdvanceAnim(param1, ((FX32_ONE) * 4));
    }
}

static void ov5_021EBD24(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EBA0C *param2, int param3)
{
    if (param3 != param2->unk_00) {
        int v0 = ov5_021EDF18(param3);

        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
        param2->unk_01 = 0;
    } else if (param2->unk_02 == (0x8 + 1)) {
        ov5_021EC69C(param1, ov5_021EDF18(param3));
    }

    if (param2->unk_02 != 0x6) {
        param2->unk_01 = 0;
    }

    if (ov5_021ECD38(mapObj) == 0) {
        fx32 v1[] = {
            (FX32_ONE),
            (FX32_ONE),
            ((FX32_ONE) * 2),
            (FX32_ONE),
            (FX32_ONE),
            ((FX32_ONE) * 2),
            0
        };

        Billboard_AdvanceAnim(param1, v1[param2->unk_01]);
        param2->unk_01++;

        if (v1[param2->unk_01] == 0) {
            param2->unk_01 = 0;
        }
    }
}

static void ov5_021EBDC4(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EBA0C *param2, int param3)
{
    if (param3 != param2->unk_00) {
        int v0 = ov5_021EDF18(param3);

        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
        param2->unk_01 = 0;
    } else if (param2->unk_02 == (0x8 + 1)) {
        ov5_021EC69C(param1, ov5_021EDF18(param3));
    }

    if (param2->unk_02 != 0x7) {
        param2->unk_01 = 0;
    }

    if (ov5_021ECD38(mapObj) == 0) {
        fx32 v1[] = {
            ((FX32_ONE) * 3),
            ((FX32_ONE) * 2),
            ((FX32_ONE) * 3),
            0
        };

        Billboard_AdvanceAnim(param1, v1[param2->unk_01]);
        param2->unk_01++;

        if (v1[param2->unk_01] == 0) {
            param2->unk_01 = 0;
        }
    }
}

static void ov5_021EBE5C(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EBA0C *param2, int param3)
{
    if (param3 != param2->unk_00) {
        int v0 = ov5_021EDF18(param3);

        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
        param2->unk_01 = 0;
    } else if (param2->unk_02 == (0x8 + 1)) {
        ov5_021EC69C(param1, ov5_021EDF18(param3));
    }

    if (param2->unk_02 != 0x8) {
        param2->unk_01 = 0;
    }

    if (ov5_021ECD38(mapObj) == 0) {
        fx32 v1[] = {
            ((FX32_ONE) * 2),
            (FX32_ONE),
            (FX32_ONE),
            (FX32_ONE),
            (FX32_ONE),
            (FX32_ONE),
            (FX32_ONE),
            0
        };

        Billboard_AdvanceAnim(param1, v1[param2->unk_01]);
        param2->unk_01++;

        if (v1[param2->unk_01] == 0) {
            param2->unk_01 = 0;
        }
    }
}

static void ov5_021EBEFC(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EBA0C *param2, int param3)
{
    int v0;

    if (param3 != param2->unk_00) {
        v0 = ov5_021EDF24(param3);
        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
    } else if (param2->unk_02 != (0x8 + 1)) {
        ov5_021EC674(
            param1, ov5_021EDF24(param3), 4);
    }

    if (ov5_021ECD38(mapObj) == 0) {
        Billboard_AdvanceAnim(param1, (FX32_ONE));
    }
}

static void (*const Unk_ov5_021FF3F8[])(
    MapObject *, Billboard *, UnkStruct_ov5_021EBA0C *, int)
    = {
          ov5_021EBB44,
          ov5_021EBBB4,
          ov5_021EBBB4,
          ov5_021EBC28,
          ov5_021EBC7C,
          ov5_021EBCD0,
          ov5_021EBD24,
          ov5_021EBDC4,
          ov5_021EBE5C,
          ov5_021EBEFC
      };

void ov5_021EBF50(MapObject *mapObj)
{
    int v0, v1;
    UnkStruct_ov5_021EBA0C *v2 = sub_02062AF0(mapObj);
    Billboard *v3 = v2->unk_04;

    v2 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    if (v3 == NULL) {
        return;
    }

    v0 = MapObject_GetFacingDir(mapObj);
    v1 = sub_02062A14(mapObj);

    GF_ASSERT(v1 < (((0x8 + 1) + 1) + 1));

    if ((v2->unk_02 == 0x0) && (v1 != v2->unk_02) && (v2->unk_03_0 == 0)) {
        v2->unk_00 = -1;
    }

    Unk_ov5_021FF448[v1](mapObj, v3, v2, v0);

    v2->unk_00 = v0;
    v2->unk_02 = v1;

    ov5_021EDEB4(mapObj, v3);
    ov5_021EC6C0(mapObj, v3);
}

static void ov5_021EBFDC(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EBA0C *param2, int param3)
{
    int v0[4] = { 6, 7, 8, 9 };
    int v1 = v0[param3];

    if (param3 != param2->unk_00) {
        Billboard_SetAnimNum(param1, v1);
        Billboard_SetAnimFrameNum(param1, 0);
        param2->unk_03_0 = 0;
    } else if (param2->unk_02 != 0x0) {
        param2->unk_03_0 = 1;
    } else if (param2->unk_03_0 == 1) {
        Billboard_SetAnimNum(param1, v1);
        Billboard_SetAnimFrameNum(param1, 0);
        param2->unk_03_0 = 0;
    } else {
        Billboard_AdvanceAnim(param1, (FX32_ONE));
    }
}

static void ov5_021EC068(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EBA0C *param2, int param3)
{
    int v0 = ov5_021EDF30(param3);

    if ((param3 != param2->unk_00) || (param2->unk_02 != 0x0)) {
        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
    } else if (param2->unk_02 != (0x8 + 1)) {
        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
    } else if (Billboard_GetAnimNum(param1) != v0) {
        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
    }

    if (ov5_021ECD38(mapObj) == 0) {
        Billboard_AdvanceAnim(param1, ((FX32_ONE) / 2));
    }
}

static void ov5_021EC0E4(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EBA0C *param2, int param3)
{
    int v0 = ov5_021EDF30(param3);

    if (param3 != param2->unk_00) {
        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
    } else if (param2->unk_02 != ((0x8 + 1) + 1)) {
        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
    } else if (Billboard_GetAnimNum(param1) != v0) {
        Billboard_SetAnimNum(param1, v0);
        Billboard_SetAnimFrameNum(param1, 0);
    }

    if (ov5_021ECD38(mapObj) == 0) {
        Billboard_AdvanceAnim(param1, (FX32_ONE));
    }
}

static void (*const Unk_ov5_021FF448[11])(
    MapObject *, Billboard *, UnkStruct_ov5_021EBA0C *, int)
    = {
          ov5_021EBFDC,
          ov5_021EBBB4,
          ov5_021EBBB4,
          ov5_021EBC28,
          ov5_021EBC7C,
          ov5_021EBCD0,
          ov5_021EBD24,
          ov5_021EBDC4,
          ov5_021EBE5C,
          ov5_021EC068,
          ov5_021EC0E4
      };

void (*const Unk_ov5_021FF31C[])(MapObject *, Billboard *, UnkStruct_ov5_021EBA0C *, int);

static const int Unk_ov5_021FF34C[] = {
    0x0,
    0x1,
    0x2,
    0x3
};

void ov5_021EC15C(MapObject *mapObj)
{
    int v0, v1;
    UnkStruct_ov5_021EBA0C *v2 = sub_02062AF0(mapObj);
    Billboard *v3 = v2->unk_04;

    v2 = sub_02062AF0(mapObj);

    if ((v3 == NULL) || (ov5_021EDD94(mapObj) == 1)) {
        return;
    }

    v0 = MapObject_GetFacingDir(mapObj);
    v1 = sub_02062A14(mapObj);

    GF_ASSERT(v1 < (0x3 + 1));

    Unk_ov5_021FF31C[v1](mapObj, v3, v2, v0);
    ov5_021EC374(mapObj, v3, v0);

    v2->unk_00 = v0;
    v2->unk_02 = sub_02062A14(mapObj);

    ov5_021EDEB4(mapObj, v3);
    ov5_021EC6C0(mapObj, v3);
}

static void ov5_021EC1D8(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EBA0C *param2, int param3)
{
    fx32 v0;

    if ((param3 != param2->unk_00) || (param2->unk_02 != 0x0)) {
        Billboard_SetAnimNum(param1, Unk_ov5_021FF34C[param3]);
        Billboard_SetAnimFrameNum(param1, (FX32_ONE * 15));
    }

    v0 = Billboard_GetAnimFrameNum(param1);
    v0 -= (FX32_ONE);

    if (v0 < 0) {
        v0 = 0;
    }

    Billboard_SetAnimFrameNum(param1, v0);
    Billboard_AdvanceAnim(param1, 0);
}

static void ov5_021EC228(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EBA0C *param2, int param3)
{
    fx32 v0;

    if ((param3 != param2->unk_00) || (param2->unk_02 != 0x1)) {
        Billboard_SetAnimNum(param1, Unk_ov5_021FF34C[param3]);
        Billboard_SetAnimFrameNum(param1, 0);
    }

    Billboard_AdvanceAnim(param1, (FX32_ONE));
}

static void ov5_021EC260(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EBA0C *param2, int param3)
{
    fx32 v0;

    if ((param3 != param2->unk_00) || (param2->unk_02 != 0x2)) {
        Billboard_SetAnimNum(param1, Unk_ov5_021FF34C[param3]);
        Billboard_SetAnimFrameNum(param1, (FX32_ONE * 15));
        param2->unk_03_1 = 0;
    }

    v0 = Billboard_GetAnimFrameNum(param1);

    switch (param2->unk_03_1) {
    case 0:
        v0 -= (FX32_ONE);

        if (v0 <= (FX32_ONE * 10)) {
            param2->unk_03_1++;
        }
        break;
    case 1:
        v0 += (FX32_ONE);

        if (v0 >= (FX32_ONE * 15)) {
            param2->unk_03_1++;
            param2->unk_01 = 0;
        }
        break;
    case 2:
        param2->unk_01++;

        if (param2->unk_01 >= 30) {
            param2->unk_01 = 0;
            param2->unk_03_1 = 0;
        }
    }

    Billboard_SetAnimFrameNum(param1, v0);
    Billboard_AdvanceAnim(param1, 0);
}

static void ov5_021EC324(MapObject *mapObj, Billboard *param1, UnkStruct_ov5_021EBA0C *param2, int param3)
{
    fx32 v0;

    if ((param3 != param2->unk_00) || (param2->unk_02 != 0x3)) {
        Billboard_SetAnimNum(param1, Unk_ov5_021FF34C[param3]);
        Billboard_SetAnimFrameNum(param1, (FX32_ONE * 15));
    }

    v0 = Billboard_GetAnimFrameNum(param1) - (FX32_ONE);

    if (v0 < 0) {
        v0 = 0;
    }

    Billboard_SetAnimFrameNum(param1, v0);
    Billboard_AdvanceAnim(param1, 0);
}

static void ov5_021EC374(MapObject *mapObj, Billboard *param1, int param2)
{
    VecFx32 v0 = { 0, 0, 0 };
    int v1 = (Billboard_GetFrameNum(param1) / FX32_ONE);

    switch (param2) {
    case 0:
        break;
    case 1:
        if (v1 >= 24) {
            v0.z = (FX32_ONE * 6);
        }
        break;
    case 2:
        if (v1 < 40) {
            v0.x = (FX32_ONE * 1);
        } else {
            v0.x = (FX32_ONE * -5);
        }
        break;
    case 3:
        if (v1 < 56) {
            v0.x = (FX32_ONE * -1);
        } else {
            v0.x = (FX32_ONE * 5);
        }
        break;
    }

    MapObject_SetSpriteJumpOffset(mapObj, &v0);
}

static void (*const Unk_ov5_021FF31C[])(MapObject *, Billboard *, UnkStruct_ov5_021EBA0C *, int) = {
    ov5_021EC1D8,
    ov5_021EC228,
    ov5_021EC260,
    ov5_021EC324
};

void ov5_021EC3F0(MapObject *mapObj)
{
    int v0;
    UnkStruct_ov5_021EBA0C *v1 = sub_02062AF0(mapObj);
    Billboard *v2 = v1->unk_04;

    v1 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    if (v2 == NULL) {
        return;
    }

    v0 = MapObject_GetFacingDir(mapObj);

    v1->unk_00 = v0;
    v1->unk_02 = sub_02062A14(mapObj);

    {
        int v3 = ov5_021EDF18(v0);

        Billboard_SetAnimNum(v2, v3);
        Billboard_SetAnimFrameNum(v2, 0);
        Billboard_AdvanceAnim(v2, 0);
    }

    ov5_021EDEB4(mapObj, v2);
    ov5_021EC6C0(mapObj, v2);
}

void ov5_021EC454(MapObject *mapObj)
{
    int v0;
    UnkStruct_ov5_021EBA0C *v1 = sub_02062AF0(mapObj);
    Billboard *v2 = v1->unk_04;

    v1 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    if (v2 == NULL) {
        return;
    }

    v0 = MapObject_GetFacingDir(mapObj);

    v1->unk_00 = v0;
    v1->unk_02 = sub_02062A14(mapObj);

    if (Billboard_GetAnimNum(v2) != 0) {
        Billboard_SetAnimNum(v2, 0);
        Billboard_SetAnimFrameNum(v2, 0);
    }

    Billboard_AdvanceAnim(v2, (FX32_ONE));

    ov5_021EDEB4(mapObj, v2);
    ov5_021EC6C0(mapObj, v2);
}

void ov5_021EC4BC(MapObject *mapObj)
{
    int v0, v1;
    UnkStruct_ov5_021EBA0C *v2 = sub_02062AF0(mapObj);
    Billboard *v3 = v2->unk_04;

    v2 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    if (v3 == NULL) {
        return;
    }

    v0 = MapObject_GetFacingDir(mapObj);
    v1 = sub_02062A14(mapObj);

    switch (v1) {
    case 0x0:
        if (Billboard_GetAnimNum(v3) != 1) {
            Billboard_SetAnimNum(v3, 1);
            Billboard_SetAnimFrameNum(v3, 0);
        }

        Billboard_AdvanceAnim(v3, (FX32_ONE));
        break;
    default:
        if (Billboard_GetAnimNum(v3) != 0) {
            Billboard_SetAnimNum(v3, 0);
            Billboard_SetAnimFrameNum(v3, 0);
        }

        Billboard_AdvanceAnim(v3, (FX32_ONE));
    }

    v2->unk_00 = v0;
    v2->unk_02 = sub_02062A14(mapObj);

    ov5_021EDEB4(mapObj, v3);
    ov5_021EC6C0(mapObj, v3);
}

void ov5_021EC554(MapObject *mapObj)
{
    int v0, v1;
    UnkStruct_ov5_021EBA0C *v2 = sub_02062AF0(mapObj);
    Billboard *v3 = v2->unk_04;

    v2 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    if (v3 == NULL) {
        return;
    }

    v0 = MapObject_GetFacingDir(mapObj);

    if (v0 != v2->unk_00) {
        v1 = ov5_021EDF18(v0);
        Billboard_SetAnimNum(v3, v1);
        Billboard_SetAnimFrameNum(v3, 0);
    }

    Billboard_AdvanceAnim(v3, (FX32_ONE));
    ov5_021EDEB4(mapObj, v3);
    ov5_021EC6C0(mapObj, v3);

    v2->unk_00 = v0;
    v2->unk_02 = sub_02062A14(mapObj);
}

void ov5_021EC5C0(MapObject *mapObj)
{
    int v0, v1;
    UnkStruct_ov5_021EBA0C *v2 = sub_02062AF0(mapObj);
    Billboard *v3 = v2->unk_04;

    v2 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    if (v3 == NULL) {
        return;
    }

    v0 = MapObject_GetFacingDir(mapObj);
    v1 = sub_02062A14(mapObj);

    if (v1 != v2->unk_02) {
        if (v1 == 0x0) {
            Billboard_SetAnimNum(v3, 0);
        } else {
            Billboard_SetAnimNum(v3, 1);
        }

        Billboard_SetAnimFrameNum(v3, 0);
    }

    Billboard_AdvanceAnim(v3, (FX32_ONE));

    v2->unk_00 = v0;
    v2->unk_02 = v1;

    ov5_021EDEB4(mapObj, v3);
    ov5_021EC6C0(mapObj, v3);
}

static void ov5_021EC638(Billboard *param0, int param1)
{
    fx32 v0, v1;

    v0 = Billboard_GetAnimFrameNum(param0);
    v0 /= FX32_ONE;
    v1 = v0 % param1;
    v0 -= v1;
    v0 *= FX32_ONE;

    Billboard_SetAnimFrameNum(param0, v0);
    Billboard_AdvanceAnim(param0, 0);
}

static void ov5_021EC668(Billboard *param0)
{
    ov5_021EC638(param0, 8);
}

static void ov5_021EC674(Billboard *param0, int param1, int param2)
{
    fx32 v0;

    ov5_021EC638(param0, param2);
    v0 = Billboard_GetAnimFrameNum(param0);

    Billboard_SetAnimNum(param0, param1);
    Billboard_SetAnimFrameNum(param0, v0);
}

static void ov5_021EC69C(Billboard *param0, int param1)
{
    fx32 v0;

    ov5_021EC668(param0);
    v0 = Billboard_GetAnimFrameNum(param0);

    Billboard_SetAnimNum(param0, param1);
    Billboard_SetAnimFrameNum(param0, v0);
}

static void ov5_021EC6C0(MapObject *mapObj, Billboard *param1)
{
    int v0 = 1;

    if (MapObject_CheckStatusFlag(mapObj, MAP_OBJ_STATUS_HIDE) == 1) {
        v0 = 0;
    }

    if (MapObject_CheckStatusFlag(mapObj, MAP_OBJ_STATUS_12) == 1) {
        if (!MapObject_CheckStatusFlag(mapObj, MAP_OBJ_STATUS_13)) {
            v0 = 0;
        }
    }

    Billboard_SetDrawFlag(param1, v0);
}

static void ov5_021EC700(u32 param0, VecFx32 *param1)
{
    const ObjectEventGfxFieldAnimsInitialPosEntry *v0 = gObjectEventGfxOverworldAnimsInitialPosTable;

    while (v0->graphicsID != 0xffff) {
        if (v0->graphicsID == param0) {
            *param1 = v0->initialPos;
            return;
        }

        v0++;
    }

    GF_ASSERT(FALSE);
}

void ov5_021EC734(MapObject *mapObj)
{
    VecFx32 v0;
    UnkStruct_ov5_021EC760 *v1 = sub_02062ACC(mapObj, (sizeof(UnkStruct_ov5_021EC760)));
    ov5_021EC700(MapObject_GetEffectiveGraphicsID(mapObj), &v0);
    v1->unk_00 = ov5_021F121C(mapObj, &v0);
}

void ov5_021EC75C(MapObject *mapObj)
{
    return;
}

void ov5_021EC760(MapObject *mapObj)
{
    UnkStruct_ov5_021EC760 *v0 = sub_02062AF0(mapObj);

    if (v0->unk_00 != NULL) {
        FieldEffectManager_FinishAnimManager(v0->unk_00);
        v0->unk_00 = NULL;
    }
}

void ov5_021EC778(MapObject *mapObj)
{
    UnkStruct_ov5_021EC760 *v0 = sub_02062AF0(mapObj);

    if (v0->unk_00 != NULL) {
        FieldEffectManager_FinishAnimManager(v0->unk_00);
        v0->unk_00 = NULL;
    }
}

void ov5_021EC790(MapObject *mapObj)
{
    VecFx32 v0;
    UnkStruct_ov5_021EC760 *v1 = sub_02062AF0(mapObj);
    ov5_021EC700(MapObject_GetEffectiveGraphicsID(mapObj), &v0);
    v1->unk_00 = ov5_021F121C(mapObj, &v0);
}

void ov5_021EC7B8(MapObject *mapObj)
{
    UnkStruct_ov5_021EC760 *v0 = sub_02062ACC(mapObj, (sizeof(UnkStruct_ov5_021EC760)));
    v0->unk_00 = ov5_021F2438(mapObj);
}

void ov5_021EC7D0(MapObject *mapObj)
{
    UnkStruct_ov5_021EC760 *v0 = sub_02062AF0(mapObj);
    GF_ASSERT(v0->unk_00 == NULL);
    v0->unk_00 = ov5_021F2438(mapObj);
}

void ov5_021EC7F0(MapObject *mapObj)
{
    UnkStruct_ov5_021EC760 *v0 = sub_02062AF0(mapObj);
    ov5_021F247C(v0->unk_00);
    v0->unk_00 = NULL;
}

static void ov5_021EC804(Billboard *param0, void *param1)
{
    UnkStruct_ov5_021EC804 *v0 = param1;
    NNSG3dResMdl *v1 = Billboard_GetModel(param0);

    NNS_G3dMdlUseMdlAlpha(v1);
    NNS_G3dMdlSetMdlAlphaAll(v1, v0->unk_10);
}

void ov5_021EC824(MapObject *mapObj)
{
    UnkStruct_ov5_021EC804 *v0 = sub_02062ACC(mapObj, sizeof(UnkStruct_ov5_021EC804));
    v0->unk_02 = -1;

    ov5_021ECF04(mapObj, &v0->unk_04);

    if (v0->unk_04 != NULL) {
        Billboard_SetCallback(v0->unk_04, ov5_021EC804, v0);
        sub_02062B68(mapObj);
    }
}

void ov5_021EC858(MapObject *mapObj)
{
    UnkStruct_ov5_021EC804 *v0 = sub_02062AF0(mapObj);
    ov5_021ECFA4(mapObj, &v0->unk_04);
}

void ov5_021EC86C(MapObject *mapObj)
{
    UnkStruct_ov5_021EC804 *v0 = sub_02062AF0(mapObj);

    if (v0->unk_04 != NULL) {
        ov5_021ED01C(v0->unk_04, &v0->unk_08);
    }

    ov5_021ECFA4(mapObj, &v0->unk_04);
    MapObject_SetStatusFlagOn(mapObj, MAP_OBJ_STATUS_21);
}

void ov5_021EC898(MapObject *mapObj)
{
    int v0;
    UnkStruct_ov5_021EB2EC *v1 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    if (v1->unk_04 == NULL) {
        ov5_021ECF04(mapObj, &v1->unk_04);
    }

    if (v1->unk_04 != NULL) {
        ov5_021ED03C(v1->unk_04, &v1->unk_08);
        ov5_021EDEB4(mapObj, v1->unk_04);
        MapObject_SetStatusFlagOff(mapObj, MAP_OBJ_STATUS_21);
        Billboard_SetCallback(v1->unk_04, ov5_021EC804, v1);
    }
}

void ov5_021EC8EC(MapObject *mapObj)
{
    int v0, v1;
    UnkStruct_ov5_021EC804 *v2;
    Billboard *v3;

    v2 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    v3 = v2->unk_04;

    if (v3 == NULL) {
        return;
    }

    if (v2->unk_10 < 31) {
        v2->unk_10++;
    }

    if (ov5_021ECD38(mapObj) == 0) {
        Billboard_AdvanceAnim(v3, (FX32_ONE));
    }

    ov5_021EDEB4(mapObj, v3);
    ov5_021EC6C0(mapObj, v3);
}

void ov5_021EC938(MapObject *mapObj)
{
    UnkStruct_ov5_021EC938 *v0 = sub_02062ACC(mapObj, (sizeof(UnkStruct_ov5_021EC938)));
    ov5_021ECF04(mapObj, &v0->unk_08);

    if (v0->unk_08 != NULL) {
        sub_02062B68(mapObj);
    }
}

void ov5_021EC95C(MapObject *mapObj)
{
    UnkStruct_ov5_021EC938 *v0 = sub_02062AF0(mapObj);
    ov5_021ECFA4(mapObj, &v0->unk_08);
}

void ov5_021EC970(MapObject *mapObj)
{
    UnkStruct_ov5_021EC938 *v0 = sub_02062AF0(mapObj);

    if (v0->unk_08 != NULL) {
        ov5_021ED01C(v0->unk_08, &v0->unk_0C);
    }

    ov5_021ECFA4(mapObj, &v0->unk_08);
    MapObject_SetStatusFlagOn(mapObj, MAP_OBJ_STATUS_21);
}

void ov5_021EC9A0(MapObject *mapObj)
{
    int v0;
    UnkStruct_ov5_021EC938 *v1 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    if (v1->unk_08 == NULL) {
        ov5_021ECF04(mapObj, &v1->unk_08);
    }

    if (v1->unk_08 != NULL) {
        ov5_021ED03C(v1->unk_08, &v1->unk_0C);
        ov5_021EDEB4(mapObj, v1->unk_08);
        MapObject_SetStatusFlagOff(mapObj, MAP_OBJ_STATUS_21);
    }
}

void ov5_021EC9E8(MapObject *mapObj)
{
    int v0, v1;
    UnkStruct_ov5_021EC938 *v2;
    Billboard *v3;

    v2 = sub_02062AF0(mapObj);

    if (ov5_021EDD94(mapObj) == 1) {
        return;
    }

    v3 = v2->unk_08;

    if (v3 == NULL) {
        return;
    }

    if (ov5_021ECD38(mapObj) == 0) {
        Billboard_AdvanceAnim(v3, (FX32_ONE));
    }

    {
        VecFx32 v4;
        fx32 v5[16] = {
            0x0,
            0x800,
            0x1000,
            0x2000,
            0x4000,
            0x5800,
            0x7000,
            0x8000,
            0x7800,
            0x6800,
            0x5000,
            0x3000,
            0x2000,
            0x1000,
            0x800,
            0x0
        };

        MapObject_GetSpriteJumpOffset(mapObj, &v4);

        if (v4.y == v2->unk_04) {
            v4.y = v5[v2->unk_00 >> 1];
            v2->unk_00 = (v2->unk_00 + 1) & 0x1f;
            v2->unk_04 = v4.y;

            MapObject_SetSpriteJumpOffset(mapObj, &v4);
        }
    }

    ov5_021EDEB4(mapObj, v3);
    ov5_021EC6C0(mapObj, v3);
}

// ENTRY P87: HeartGold's "the follower comes out of its Poke Ball" effect - HG asm/overlay_01_022031C0.s, variant 0
// (template ov01_022094DC: init ov01_0220335C, tick ov01_022033E4, draw ov01_022034B8; resources ov01_022031F8).
// HG's two models and the burst's texture animation were appended unchanged to Platinum's fldeff.narc:
//   217 = HG fldeff 129 "monsterball" (the small ball), 218 = HG 104 "mb_out" (the opening burst),
//   219 = HG 164, the burst's SRT animation.
// HG: the ball is drawn for 2 frames; then it is swapped for the burst, the follower is shown (sub_02069DC8(obj, 0))
// and the burst animation plays once at 1 frame per frame; the effect ends with the animation. The effect sits at the
// follower's position + 6 on z, like HG's ov01_0220329C.
#define FOLLOWER_BALL_NARC_BALL  217
#define FOLLOWER_BALL_NARC_BURST 218
#define FOLLOWER_BALL_NARC_ANIM  219
#define FOLLOWER_BALL_FRAMES     2

// ENTRY P89: the render objects and the burst animation live HERE (heap, created once per map with the renderer), not
// in the effect: an OverworldAnimManager's context is a fixed OVERWORLD_ANIM_MANAGER_FUNCS_CONTEXT_SIZE (160) bytes
// inside the manager, and ENTRY P87 kept two Simple3DRenderObj (84 bytes each) + an animation there (~232 bytes) - it
// overwrote the manager's own tick function and the next manager, crashing (Data Abort, PC FFFFFFFC in
// OverworldAnimManager_CallTickFunc) the first time the effect ran, i.e. on every door. Only one follower exists, so
// one effect at a time shares these; a second request while one is running just shows the follower.
typedef struct FollowerBallRenderer {
    FieldEffectManager *fieldEffMan;
    Simple3DModel ballModel;
    Simple3DModel burstModel;
    Simple3DAnimation burstAnimSet;
    Simple3DAnimation burstAnim;
    Simple3DRenderObj ballRenderObj;
    Simple3DRenderObj burstRenderObj;
    BOOL busy;
} FollowerBallRenderer;

// ENTRY P96: two kinds of effect share the renderer. RELEASE = out of the ball (HG variant 0, above). RETURN = back into
// the ball, HG variant 3 (template ov01_022094C8: init ov01_02203820, tick ov01_02203890, draw ov01_022037E8): the
// follower shrinks for 4 frames (scale 1/1, 1/2, 1/3, 1/4 on x and y), then it is hidden and the small ball stays on its
// spot for 16 frames.
enum FollowerBallMode {
    FOLLOWER_BALL_MODE_RELEASE = 0,
    FOLLOWER_BALL_MODE_RETURN,
};

#define FOLLOWER_BALL_RETURN_SHRINK_FRAMES 4
#define FOLLOWER_BALL_RETURN_BALL_FRAMES   16

typedef struct FollowerBallEffectParams {
    FollowerBallRenderer *renderer;
    MapObject *mapObj;
    int mode;
} FollowerBallEffectParams;

typedef struct FollowerBallEffect {
    FollowerBallRenderer *renderer;
    MapObject *mapObj;
    int mode;
    int state;
    int timer;
    int localID;
    int mapHeaderID;
} FollowerBallEffect;

// Compile-time guard: the effect context must fit in the anim manager's fixed context buffer.
typedef char FollowerBallEffectFitsContext[(sizeof(FollowerBallEffect) <= OVERWORLD_ANIM_MANAGER_FUNCS_CONTEXT_SIZE) ? 1 : -1];

void *FollowerBallRenderer_New(FieldEffectManager *fieldEffMan)
{
    FollowerBallRenderer *renderer = FieldEffectManager_HeapAllocInit(fieldEffMan, sizeof(FollowerBallRenderer), FALSE, 0);

    renderer->fieldEffMan = fieldEffMan;
    renderer->busy = FALSE;

    FieldEffectManager_LoadModel(fieldEffMan, &renderer->ballModel, 0, FOLLOWER_BALL_NARC_BALL, FALSE);
    FieldEffectManager_LoadModel(fieldEffMan, &renderer->burstModel, 0, FOLLOWER_BALL_NARC_BURST, FALSE);
    FieldEffectManager_LoadAnimation(fieldEffMan, &renderer->burstAnimSet, 0, FOLLOWER_BALL_NARC_ANIM, FALSE);
    FieldEffectManager_ApplyAnimCopyToModel(fieldEffMan, &renderer->burstAnim, &renderer->burstModel, &renderer->burstAnimSet, 0);

    Simple3D_CreateRenderObject(&renderer->ballRenderObj, &renderer->ballModel);
    Simple3D_CreateRenderObjectWithAnim(&renderer->burstRenderObj, &renderer->burstModel, &renderer->burstAnim);

    return renderer;
}

void FollowerBallRenderer_Free(void *context)
{
    FollowerBallRenderer *renderer = context;

    Simple3D_FreeAnimation(&renderer->burstAnim);
    Simple3D_FreeModel(&renderer->ballModel);
    Simple3D_FreeModel(&renderer->burstModel);
    Simple3D_FreeAnimation(&renderer->burstAnimSet);
    FieldEffectManager_HeapFree(renderer);
}

static void FollowerBall_SetFollowerScale(MapObject *mapObj, fx32 scale)
{
    Billboard *billboard = ov5_021EB1A0(mapObj);

    if (billboard != NULL) {
        VecFx32 vec = { scale, scale, FX32_ONE };
        Billboard_SetScale(billboard, &vec);
    }
}

static BOOL FollowerBallEffect_Init(OverworldAnimManager *animMan, void *context)
{
    FollowerBallEffect *effect = context;
    const FollowerBallEffectParams *params = OverworldAnimManager_GetUserData(animMan);

    effect->renderer = params->renderer;
    effect->mapObj = params->mapObj;
    effect->mode = params->mode;
    effect->state = 0;
    effect->timer = 0;
    effect->localID = MapObject_GetLocalID(params->mapObj);
    effect->mapHeaderID = MapObject_GetMapHeaderID(params->mapObj);

    effect->renderer->busy = TRUE;
    Simple3D_SetAnimFrame(&effect->renderer->burstAnim, 0);
    Simple3D_UpdateAnim(&effect->renderer->burstAnim, 0, FALSE);

    return TRUE;
}

static void FollowerBallEffect_Exit(OverworldAnimManager *animMan, void *context)
{
    FollowerBallEffect *effect = context;

    effect->renderer->busy = FALSE;

    // ENTRY P102: the return effect gives the follower its normal size back only now, while it is hidden (see
    // FollowerBallEffect_TickReturn) - if the follower still exists.
    if (effect->mode == FOLLOWER_BALL_MODE_RETURN && sub_02062764(effect->mapObj, effect->localID, effect->mapHeaderID) == TRUE) {
        FollowerBall_SetFollowerScale(effect->mapObj, FX32_ONE);
    }

    FollowerMon_SetWhiteFlash(FALSE); // ENTRY P101: never left white
}

static void FollowerBallEffect_TickRelease(OverworldAnimManager *animMan, FollowerBallEffect *effect)
{
    if (effect->state == 0) {
        if (++effect->timer < FOLLOWER_BALL_FRAMES) {
            return;
        }

        MapObject_SetHidden(effect->mapObj, FALSE);
        effect->state = 1;
    }

    if (Simple3D_UpdateAnim(&effect->renderer->burstAnim, FX32_ONE, FALSE) == TRUE) {
        FieldEffectManager_FinishAnimManager(animMan);
    }
}

static void FollowerBallEffect_TickReturn(OverworldAnimManager *animMan, FollowerBallEffect *effect)
{
    if (effect->state == 0) {
        effect->timer++;
        FollowerBall_SetFollowerScale(effect->mapObj, FX_Div(FX32_ONE, effect->timer << FX32_SHIFT));

        if (effect->timer >= FOLLOWER_BALL_RETURN_SHRINK_FRAMES) {
            // ENTRY P102 (user, Rayquaza: "an additional flash of the full size rayquaza sprite in regular colors toward
            // the end of the anim"): the hidden flag only reaches the billboard on the follower's next render update, so
            // resetting the scale and the white palette here showed one full-size, normal-coloured frame. The billboard
            // is switched off right now instead, and the scale / colours are restored in FollowerBallEffect_Exit.
            Billboard *billboard = ov5_021EB1A0(effect->mapObj);

            MapObject_SetHidden(effect->mapObj, TRUE);

            if (billboard != NULL) {
                Billboard_SetDrawFlag(billboard, FALSE);
            }

            effect->state = 1;
            effect->timer = 0;
        }

        return;
    }

    if (++effect->timer >= FOLLOWER_BALL_RETURN_BALL_FRAMES) {
        FieldEffectManager_FinishAnimManager(animMan);
    }
}

static void FollowerBallEffect_Tick(OverworldAnimManager *animMan, void *context)
{
    FollowerBallEffect *effect = context;

    // The follower was deleted (map change, lead change to a species without a sprite, ...): just stop.
    if (sub_02062764(effect->mapObj, effect->localID, effect->mapHeaderID) == FALSE) {
        FieldEffectManager_FinishAnimManager(animMan);
        return;
    }

    if (effect->mode == FOLLOWER_BALL_MODE_RETURN) {
        FollowerBallEffect_TickReturn(animMan, effect);
    } else {
        FollowerBallEffect_TickRelease(animMan, effect);
    }
}

static void FollowerBallEffect_Render(OverworldAnimManager *animMan, void *context)
{
    FollowerBallEffect *effect = context;
    VecFx32 pos;

    OverworldAnimManager_GetPosition(animMan, &pos);

    if (effect->mode == FOLLOWER_BALL_MODE_RETURN) {
        if (effect->state == 1) {
            Simple3D_DrawRenderObjWithPos(&effect->renderer->ballRenderObj, &pos);
        }
    } else if (effect->state == 0) {
        Simple3D_DrawRenderObjWithPos(&effect->renderer->ballRenderObj, &pos);
    } else {
        Simple3D_DrawRenderObjWithPos(&effect->renderer->burstRenderObj, &pos);
    }
}

static const OverworldAnimManagerFuncs sFollowerBallEffectFuncs = {
    sizeof(FollowerBallEffect),
    FollowerBallEffect_Init,
    FollowerBallEffect_Exit,
    FollowerBallEffect_Tick,
    FollowerBallEffect_Render
};

// Returns FALSE if the effect could not start (no renderer on this map type, e.g. the Underground, or one already
// running); the caller then shows/hides the follower directly.
static BOOL FollowerBall_Start(MapObject *mapObj, int mode)
{
    FieldEffectManager *fieldEffMan = MapObject_GetFieldEffectManager(mapObj);

    if (fieldEffMan == NULL || FieldEffectManager_HasRenderer(fieldEffMan, FIELD_EFFECT_RENDERER_FOLLOWER_BALL) == FALSE) {
        return FALSE;
    }

    FollowerBallEffectParams params;
    VecFx32 pos;

    params.renderer = FieldEffectManager_GetRendererContext(fieldEffMan, FIELD_EFFECT_RENDERER_FOLLOWER_BALL);
    params.mapObj = mapObj;
    params.mode = mode;

    if (params.renderer == NULL || params.renderer->busy) {
        return FALSE;
    }

    MapObject_GetPosPtr(mapObj, &pos);
    pos.z += FX32_ONE * 6;

    return FieldEffectManager_InitAnimManager(fieldEffMan, &sFollowerBallEffectFuncs, &pos, 0, &params, MapObject_CalculateTaskPriority(mapObj, 2)) != NULL;
}

void FollowerMon_StartBallEffect(MapObject *mapObj)
{
    FollowerMon_SetWhiteFlash(FALSE); // ENTRY P101
    FollowerBall_SetFollowerScale(mapObj, FX32_ONE); // ENTRY P102: full size even if a return effect was cut short

    if (FollowerBall_Start(mapObj, FOLLOWER_BALL_MODE_RELEASE) == FALSE) {
        MapObject_SetHidden(mapObj, FALSE);
    }
}

void FollowerMon_StartBallReturnEffect(MapObject *mapObj)
{
    if (FollowerBall_Start(mapObj, FOLLOWER_BALL_MODE_RETURN) == FALSE) {
        MapObject_SetHidden(mapObj, TRUE);
        return;
    }

    // ENTRY P101: HG - the Pokemon turns pure white the moment it starts going back into its ball (FollowerMon_RefreshPalette).
    FollowerMon_SetWhiteFlash(TRUE);
}
