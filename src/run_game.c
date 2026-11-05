#include "run_game.h"
#include "field/field_system.h"
#include "field_task.h"
#include "field_transition.h"
#include "location.h"
#include "generated/map_headers.h"
#include "unk_020528D0.h"

#include <string.h>

#include "pokemon.h"
#include "run_randomizer.h"
#include "run_session.h"
#include "run_world.h"
#include "save_player.h"
#include "savedata/save_table.h"
#include "pc_boxes.h"
#include "bag.h"
#include "generated/items.h"
#include "generated/moves.h"
#include "generated/badges.h"
#include "constants/heap.h"
#include "message.h"
#include "string_gf.h"
#include "res/text/bank/run_engine.h"

static const FieldBattleDTO *sBattle;
static enum RunCapturePermission sPermission[2];
static u8 sOpportunity[2];
static u16 sSpecial[2];
static u16 sCaught[2];
static u8 sEnemyBattler[2];
static u8 sEnemySlot[2];
static BOOL sSuspended;
static BOOL sSyncing;
static BOOL sGiftContext;
static enum RunEncounterSource sGiftSource;
static u32 sGiftKey;
static BOOL sGiftAccepted;

#define RUN_MON_TRACKED 0x40
#define RUN_MON_DEAD    0x80

u16 RunGame_MonID(BoxPokemon *mon)
{
    if (!RunSession_Configured() || mon == NULL) return 0;
    u16 id = BoxPokemon_GetValue(mon, MON_DATA_UNUSED_114, NULL);
    RunMonRecord record;
    if (RunState_GetMon(RunSession_State(), id, &record)
        && record.personality == mon->personality
        && record.trainerID == BoxPokemon_GetValue(mon, MON_DATA_OT_ID, NULL)) return id;
    return 0;
}

u16 RunGame_Register(BoxPokemon *mon, enum RunEncounterSource source, u16 specialID)
{
    if (!RunSession_Configured() || mon == NULL) return 0;
    u16 id = RunGame_MonID(mon);
    if (id) { RunGame_Sync(mon); return id; }
    RunMonRecord record;
    memset(&record, 0, sizeof(record));
    record.species = record.originalSpecies = BoxPokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
    if (!RunSpecies_IsValid(record.species)) return 0;
    record.personality = mon->personality;
    record.trainerID = BoxPokemon_GetValue(mon, MON_DATA_OT_ID, NULL);
    record.originZone = RunSession_Zone();
    record.deathZone = 0xFFFF;
    record.specialID = specialID;
    record.flags = source;
    if (BoxPokemon_GetValue(mon, MON_DATA_IS_EGG, NULL)) record.flags |= RUN_MON_EGG;
    if (Pokemon_IsPersonalityShiny(record.trainerID, record.personality)) record.flags |= RUN_MON_SHINY;
    BoxPokemon_GetValue(mon, MON_DATA_NICKNAME, record.nickname);
    RunState_MarkOwned(RunSession_State(), record.species);
    id = RunState_AddMon(RunSession_State(), &record);
    u8 marker = BoxPokemon_GetValue(mon, MON_DATA_UNUSED_113, NULL) | RUN_MON_TRACKED;
    BoxPokemon_SetValue(mon, MON_DATA_UNUSED_113, &marker);
    if (id) BoxPokemon_SetValue(mon, MON_DATA_UNUSED_114, &id);
    return id;
}

void RunGame_Sync(BoxPokemon *mon)
{
    if (sSyncing || !RunSession_Configured()) return;
    sSyncing = TRUE;
    u16 id = RunGame_MonID(mon);
    RunMonRecord record;
    if (RunState_GetMon(RunSession_State(), id, &record)) {
        u16 species = BoxPokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
        if (RunSpecies_IsValid(species) && species != record.species) {
            record.species = species;
            if (record.evolutionCount < 3) record.evolutions[record.evolutionCount++] = species;
        }
        BoxPokemon_GetValue(mon, MON_DATA_NICKNAME, record.nickname);
        if (!BoxPokemon_GetValue(mon, MON_DATA_IS_EGG, NULL)) record.flags &= ~RUN_MON_EGG;
        RunState_PutMon(RunSession_State(), id, &record);
    }
    sSyncing = FALSE;
}

BOOL RunGame_Exhausted(BoxPokemon *mon)
{
    return RunSession_Configured() && !sSuspended && (RunSession_Config()->flags & RUN_PERMADEATH)
        && (BoxPokemon_GetValue(mon, MON_DATA_UNUSED_113, NULL) & RUN_MON_DEAD) != 0;
}

void RunGame_Exhaust(Pokemon *mon)
{
    if (!RunSession_Configured() || sSuspended || !(RunSession_Config()->flags & RUN_PERMADEATH)) return;
    BoxPokemon *box = Pokemon_GetBoxPokemon(mon);
    u8 marker = BoxPokemon_GetValue(box, MON_DATA_UNUSED_113, NULL);
    if (!(marker & RUN_MON_TRACKED) || BoxPokemon_GetValue(box, MON_DATA_IS_EGG, NULL)) return;
    marker |= RUN_MON_DEAD;
    BoxPokemon_SetValue(box, MON_DATA_UNUSED_113, &marker);
    RunState_ExhaustMon(RunSession_State(), RunGame_MonID(box), RunSession_Zone());
    u16 zero = 0;
    Pokemon_SetValue(mon, MON_DATA_HP, &zero);
}

u16 RunGame_FilterHP(Pokemon *mon, u16 hp)
{
    if (!RunSession_Configured() || sSuspended || !(RunSession_Config()->flags & RUN_PERMADEATH)) return hp;
    if (RunGame_Exhausted(Pokemon_GetBoxPokemon(mon))) return 0;
    if (hp == 0 && Pokemon_GetValue(mon, MON_DATA_HP, NULL) != 0) RunGame_Exhaust(mon);
    return hp;
}

u8 RunGame_LevelCap(void)
{
    return RunSession_Configured() ? RunWorld_LevelCap(RunSession_Config(),
        TrainerInfo_BadgeCount(SaveData_GetTrainerInfo(SaveData_Ptr()))) : 100;
}

u32 RunGame_Experience(BoxPokemon *mon, u32 current, u32 award)
{
    if (RunGame_Exhausted(mon)) return current;
    if (RunSession_Configured() && !sSuspended && (RunSession_Config()->flags & RUN_LEVEL_CAPS)
        && (BoxPokemon_GetValue(mon, MON_DATA_UNUSED_113, NULL) & RUN_MON_TRACKED)) {
        u16 species = BoxPokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
        return RunWorld_ClampExp(current, award, Pokemon_GetSpeciesBaseExpAt(species, RunGame_LevelCap()));
    }
    u32 maxExp = Pokemon_GetSpeciesBaseExpAt(BoxPokemon_GetValue(mon, MON_DATA_SPECIES, NULL), 100);
    return RunWorld_ClampExp(current, award, maxExp);
}

BOOL RunGame_CanUseCandy(Pokemon *mon)
{
    return !RunGame_Exhausted(Pokemon_GetBoxPokemon(mon))
        && (!RunSession_Configured() || !(RunSession_Config()->flags & RUN_LEVEL_CAPS)
            || Pokemon_GetValue(mon, MON_DATA_LEVEL, NULL) < RunGame_LevelCap());
}

void RunGame_BeginBattle(FieldBattleDTO *dto)
{
    if (sBattle == dto) return;
    sBattle = dto;
    sSuspended = (dto->battleType & (BATTLE_TYPE_LINK | BATTLE_TYPE_FRONTIER | BATTLE_TYPE_DEBUG | BATTLE_TYPE_CATCH_TUTORIAL)) != 0;
    for (u8 i = 0; i < 2; i++) {
        sPermission[i] = RUN_CAPTURE_INACTIVE;
        sOpportunity[i] = 0xFF;
        sSpecial[i] = sCaught[i] = 0;
    }
    if (!RunSession_Configured() || sSuspended) return;
    Party *player = dto->parties[BATTLER_PLAYER_1];
    for (u8 i = 0; i < Party_GetCurrentCount(player); i++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(player, i);
        BoxPokemon *box = Pokemon_GetBoxPokemon(mon);
        if (!RunGame_MonID(box)) RunGame_Register(box, RUN_SOURCE_GIFT, 0);
        if (RunGame_Exhausted(box)) RunGame_Exhaust(mon);
    }
    if ((dto->battleType & BATTLE_TYPE_TRAINER) || !RunState_CaptureActivated(RunSession_State())) return;
    RunSession_SetZone(RunWorld_Zone(dto->mapHeaderID));
    for (u8 i = 0; i < 2; i++) {
        sEnemyBattler[i] = i == 0 || Party_GetCurrentCount(dto->parties[BATTLER_ENEMY_2]) == 0 ? BATTLER_ENEMY_1 : BATTLER_ENEMY_2;
        sEnemySlot[i] = i && sEnemyBattler[i] == BATTLER_ENEMY_1 ? 1 : 0;
        Party *enemy = dto->parties[sEnemyBattler[i]];
        if (sEnemySlot[i] >= Party_GetCurrentCount(enemy)) continue;
        Pokemon *mon = Party_GetPokemonBySlotIndex(enemy, sEnemySlot[i]);
        u16 species = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
        enum RunEncounterSource source = dto->runEncounterSource;
        if (dto->battleType & BATTLE_TYPE_PAL_PARK) source = RUN_SOURCE_GIFT;
        if (dto->battleType & BATTLE_TYPE_ROAMER) source = RUN_SOURCE_STATIC;
        BOOL shiny = Pokemon_IsShiny(mon);
        if (source == RUN_SOURCE_WILD && shiny && RunConfig_ShinyClauseEnabled(RunSession_Config())) source = RUN_SOURCE_SHINY;
        sPermission[i] = RunState_Reserve(RunSession_State(), RunSession_Zone(), species, source, shiny, &sOpportunity[i]);
        if (source != RUN_SOURCE_WILD) {
            u32 key = dto->runEventKey ? dto->runEventKey : (((u32)dto->mapHeaderID << 16) | species);
            // Simultaneous shiny opponents need separate pending records even
            // when their species match; repeated single encounters still aggregate.
            if (source == RUN_SOURCE_SHINY && i) key ^= 0x80000000u;
            sSpecial[i] = RunState_SpecialAttempt(RunSession_State(), key, RunSession_Zone(), species, source);
        }
    }
}

static u8 RunGame_EnemyIndex(u8 battler, u8 slot)
{
    // Native doubles may share one enemy party; partner wild encounters use
    // separate parties. Resolve both layouts to the reservation established at entry.
    if (sBattle != NULL && battler == BATTLER_ENEMY_2
        && Party_GetCurrentCount(sBattle->parties[BATTLER_ENEMY_2]) == 0) battler = BATTLER_ENEMY_1;
    for (u8 i = 0; i < 2; i++) if (sEnemyBattler[i] == battler && sEnemySlot[i] == slot) return i;
    return 2;
}

enum RunCapturePermission RunGame_CapturePermission(u8 battler, u8 enemySlot)
{
    u8 index = RunGame_EnemyIndex(battler, enemySlot);
    return sBattle != NULL && index < 2 ? sPermission[index] : RUN_CAPTURE_INACTIVE;
}

void RunGame_Caught(Pokemon *mon, u8 battler, u8 partySlot)
{
    u8 enemySlot = RunGame_EnemyIndex(battler, partySlot);
    if (!RunSession_Configured() || enemySlot >= 2 || sBattle == NULL || sSuspended || sCaught[enemySlot]) return;
    enum RunEncounterSource source = sBattle->runEncounterSource;
    if (sSpecial[enemySlot]) {
        RunSpecialRecord event;
        RunState_GetSpecial(RunSession_State(), sSpecial[enemySlot], &event);
        source = event.source;
    }
    sCaught[enemySlot] = RunGame_Register(Pokemon_GetBoxPokemon(mon), source, sSpecial[enemySlot]);
    if (sOpportunity[enemySlot] < 3) RunState_Finish(RunSession_State(), RunSession_Zone(), sOpportunity[enemySlot], RUN_OUTCOME_CAUGHT, sCaught[enemySlot]);
    if (sSpecial[enemySlot]) RunState_FinishSpecial(RunSession_State(), sSpecial[enemySlot], RUN_OUTCOME_CAUGHT, sCaught[enemySlot]);
}

void RunGame_EndBattle(const FieldBattleDTO *dto)
{
    if (sBattle != dto) return;
    if (RunSession_Configured() && !sSuspended) {
        for (u8 i = 0; i < 2; i++) {
            if (sCaught[i]) continue;
            enum RunEncounterOutcome outcome = RUN_OUTCOME_FAILED;
            if (sEnemySlot[i] < Party_GetCurrentCount(dto->parties[sEnemyBattler[i]])
                && Pokemon_GetValue(Party_GetPokemonBySlotIndex(dto->parties[sEnemyBattler[i]], sEnemySlot[i]), MON_DATA_HP, NULL) == 0) outcome = RUN_OUTCOME_DEFEATED;
            else if (dto->resultMask == BATTLE_RESULT_PLAYER_FLED || dto->resultMask == BATTLE_RESULT_ENEMY_FLED) outcome = RUN_OUTCOME_FLED;
            if (sOpportunity[i] < 3) RunState_Finish(RunSession_State(), RunSession_Zone(), sOpportunity[i], outcome, 0);
            if (sSpecial[i]) RunState_FinishSpecial(RunSession_State(), sSpecial[i], outcome, 0);
        }
        Party *party = dto->parties[BATTLER_PLAYER_1];
        BOOL living = FALSE;
        for (u32 i = 0; i < Party_GetCurrentCount(party); i++) {
            Pokemon *mon = Party_GetPokemonBySlotIndex(party, i);
            if (Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL)) continue;
            if (Pokemon_GetValue(mon, MON_DATA_HP, NULL) == 0) RunGame_Exhaust(mon);
            if (!RunGame_Exhausted(Pokemon_GetBoxPokemon(mon))) living = TRUE;
            RunGame_Sync(Pokemon_GetBoxPokemon(mon));
        }
        if (!living && RunConfig_EndOnPartyWipe(RunSession_Config())) RunState_SetGameOver(RunSession_State());
    }
    sBattle = NULL;
    sSuspended = FALSE;
}

BOOL RunGame_GiftUsed(enum RunEncounterSource source, u32 eventKey)
{
    if (!RunSession_Configured()) return FALSE;
    for (u16 i = 1; i <= RunState_SpecialCount(RunSession_State()); i++) {
        RunSpecialRecord record;
        RunState_GetSpecial(RunSession_State(), i, &record);
        if (record.eventKey == eventKey && record.source == source && record.caught) return TRUE;
    }
    return FALSE;
}

u16 RunGame_Gift(Pokemon *mon, enum RunEncounterSource source, u32 eventKey)
{
    sGiftAccepted = FALSE;
    if (!RunSession_Configured()) return 0;
    u16 species = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
    u8 slot;
    enum RunCapturePermission permission = RunState_Reserve(RunSession_State(), RunSession_Zone(), species, source, Pokemon_IsShiny(mon), &slot);
    u16 special = RunState_SpecialAttempt(RunSession_State(), eventKey, RunSession_Zone(), species, source);
    if (permission != RUN_CAPTURE_ALLOWED && permission != RUN_CAPTURE_INACTIVE) {
        RunState_FinishSpecial(RunSession_State(), special, RUN_OUTCOME_FAILED, 0);
        return 0;
    }
    // Acceptance is independent of the optional detailed history ID. At the
    // archive limit, exempt gifts still exist in the party and aggregate event
    // log; ID zero must never turn an obtained gift into a rejected transaction.
    sGiftAccepted = TRUE;
    u16 id = RunGame_Register(Pokemon_GetBoxPokemon(mon), source, special);
    RunState_FinishSpecial(RunSession_State(), special, RUN_OUTCOME_CAUGHT, id);
    if (permission == RUN_CAPTURE_ALLOWED && slot < 3) {
        RunState_Finish(RunSession_State(), RunSession_Zone(), slot, RUN_OUTCOME_CAUGHT, id);
    }
    return id;
}

u16 RunGame_RandomSpecies(enum RunEncounterSource source, u32 key, u32 slot, u16 species)
{
    if (!RunSession_Configured()) return species;
    enum RunRandomizerDomain domain = RUN_DOMAIN_WILD;
    if (source == RUN_SOURCE_STATIC) domain = RUN_DOMAIN_STATIC;
    if (source == RUN_SOURCE_GIFT) domain = RUN_DOMAIN_GIFT;
    if (source == RUN_SOURCE_EGG) domain = RUN_DOMAIN_GIFT_EGG;
    return RunRandomizer_SelectSpecies(RunSession_Config(), domain, key, slot, species);
}

u16 RunGame_TrainerSpecies(u32 trainerID, u32 slot, u16 species)
{
    return RunSession_Configured() ? RunRandomizer_SelectSpecies(RunSession_Config(), RUN_DOMAIN_TRAINER, trainerID, slot, species) : species;
}

u8 RunGame_TrainerLevel(u32 trainerID, u8 baseLevel)
{
    return RunSession_Configured() ? RunWorld_TrainerLevel(RunSession_Config(),
        TrainerInfo_BadgeCount(SaveData_GetTrainerInfo(SaveData_Ptr())), baseLevel, RunWorld_IsBoss(trainerID)) : baseLevel;
}

BOOL RunGame_DayCareEggs(void)
{
    return !RunSession_Configured() || RunConfig_DayCareEggsEnabled(RunSession_Config());
}

void RunGame_SetGiftContext(enum RunEncounterSource source, u32 eventKey)
{
    sGiftContext = TRUE;
    sGiftSource = source;
    sGiftKey = eventKey;
}

void RunGame_ClearGiftContext(void)
{
    sGiftContext = FALSE;
}

BOOL RunGame_AcceptScriptGift(Pokemon *mon)
{
    if (!sGiftContext || !RunSession_Configured()) return TRUE;
    // Fossil revivals are repeatable; the native pending-revival variable and
    // consumed fossil establish each transaction. One-time gifts stay guarded.
    if ((sGiftKey >> 16) != MAP_HEADER_MINING_MUSEUM && RunGame_GiftUsed(sGiftSource, sGiftKey)) return FALSE;
    RunGame_Gift(mon, sGiftSource, sGiftKey);
    return sGiftAccepted;
}

void RunGame_SetGiftResult(BOOL accepted) { sGiftAccepted = accepted; }
BOOL RunGame_GiftResult(void) { return sGiftAccepted; }

void RunGame_AdjustTrainerParty(Party *party, u32 trainerID, u32 battler, u32 battleType)
{
    if (!RunSession_Configured() || !(battler & 1) || (battleType & (BATTLE_TYPE_LINK | BATTLE_TYPE_FRONTIER))) return;
    for (u32 slot = 0; slot < Party_GetCurrentCount(party); slot++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(party, slot);
        u16 original = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
        u16 species = RunGame_TrainerSpecies(trainerID, slot, original);
        u8 baseLevel = Pokemon_GetValue(mon, MON_DATA_LEVEL, NULL);
        u8 level = RunGame_TrainerLevel(trainerID, baseLevel);
        if (species != original) {
            u32 personality = Pokemon_GetValue(mon, MON_DATA_PERSONALITY, NULL);
            u32 ivs = Pokemon_GetValue(mon, MON_DATA_COMBINED_IVS, NULL);
            u16 item = Pokemon_GetValue(mon, MON_DATA_HELD_ITEM, NULL);
            u8 sealID = Pokemon_GetValue(mon, MON_DATA_BALL_CAPSULE_ID, NULL);
            BallCapsule capsule;
            Pokemon_GetValue(mon, MON_DATA_BALL_CAPSULE, &capsule);
            Pokemon_InitWith(mon, species, level, 0, TRUE, personality, OTID_NOT_SHINY, 0);
            Pokemon_SetValue(mon, MON_DATA_COMBINED_IVS, &ivs);
            Pokemon_SetValue(mon, MON_DATA_HELD_ITEM, &item);
            Pokemon_SetValue(mon, MON_DATA_BALL_CAPSULE_ID, &sealID);
            Pokemon_SetValue(mon, MON_DATA_BALL_CAPSULE, &capsule);
            Pokemon_CalcLevelAndStats(mon);
        } else if (level != baseLevel) {
            u32 exp = Pokemon_GetSpeciesBaseExpAt(species, level);
            Pokemon_SetValue(mon, MON_DATA_EXPERIENCE, &exp);
            Pokemon_CalcLevelAndStats(mon);
        }
    }
}

void RunGame_StoreExhausted(SaveData *save)
{
    if (!RunSession_Configured() || !(RunSession_Config()->flags & RUN_PERMADEATH)) return;
    Party *party = SaveData_GetParty(save);
    PCBoxes *boxes = SaveData_GetPCBoxes(save);
    for (int slot = Party_GetCurrentCount(party) - 1; slot >= 0; slot--) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(party, slot);
        // Retain one inaccessible exhausted party member on a wipe to preserve
        // native field/script assumptions. Never delete a mon if every box is full.
        if (!RunGame_Exhausted(Pokemon_GetBoxPokemon(mon))) continue;
        RunGame_Exhaust(mon);
        if (Party_GetCurrentCount(party) <= 1) continue;
        BOOL memorialEmpty = PCBoxes_CountMonsInBox(boxes, MAX_PC_BOXES - 1) == 0;
        BOOL stored = PCBoxes_TryStoreBoxMonInBox(boxes, MAX_PC_BOXES - 1, Pokemon_GetBoxPokemon(mon));
        if (stored && memorialEmpty) {
            MessageLoader *messages = MessageLoader_Init(MSG_LOADER_LOAD_ON_DEMAND, NARC_INDEX_MSGDATA__PL_MSG, TEXT_BANK_RUN_ENGINE, HEAP_ID_FIELD2);
            if (messages != NULL) {
                String *name = MessageLoader_GetNewString(messages, Run_Text_ExhaustedBox);
                if (name != NULL) { PCBoxes_RenameBox(boxes, MAX_PC_BOXES - 1, name); String_Free(name); }
                MessageLoader_Free(messages);
            }
        }
        if (stored || PCBoxes_TryStoreBoxMon(boxes, Pokemon_GetBoxPokemon(mon))) {
            Party_RemovePokemonBySlotIndex(party, slot);
            SaveData_SetFullSaveRequired();
        }
    }
}

BOOL RunGame_Over(void)
{
    return RunSession_Configured() && RunState_Status(RunSession_State()) == RUN_STATUS_GAME_OVER;
}

void RunGame_ConfineLocation(Location *location)
{
    if (!RunGame_Over()) return;
    location->mapHeaderID = MAP_HEADER_SANDGEM_TOWN_POKECENTER_1F;
    location->warpId = WARP_ID_NONE;
    location->x = 8;
    location->z = 6;
    location->faceDirection = FACE_UP;
}

static BOOL RunGame_EndTask(FieldTask *task)
{
    int *state = FieldTask_GetState(task);
    if (*state == 0) {
        (*state)++;
        if (FieldSystem_IsRunningFieldMap(FieldTask_GetFieldSystem(task))) FieldTransition_FinishMap(task);
    } else if (*state == 1) {
        (*state)++;
        FieldTask_StartBlackOutFromBattle(task);
    } else return TRUE;
    return FALSE;
}

void RunGame_PollEnd(FieldSystem *fieldSystem)
{
    // Script/battle tasks own their cleanup. Wait until their entire call stack
    // has unwound before moving maps; never abandon a task's heap environment.
    if (RunGame_Over() && fieldSystem->location->mapHeaderID != MAP_HEADER_SANDGEM_TOWN_POKECENTER_1F
        && !fieldSystem->task && !fieldSystem->processManager->child) {
        FieldSystem_CreateTask(fieldSystem, RunGame_EndTask, NULL);
    }
}

void RunGame_ObserveParty(Party *party)
{
    if (!RunSession_Configured() || sBattle != NULL) return;
    for (u32 slot = 0; slot < Party_GetCurrentCount(party); slot++) {
        Pokemon *partyMon = Party_GetPokemonBySlotIndex(party, slot);
        BoxPokemon *mon = Pokemon_GetBoxPokemon(partyMon);
        if (!(BoxPokemon_GetValue(mon, MON_DATA_UNUSED_113, NULL) & RUN_MON_TRACKED)) RunGame_Register(mon, RUN_SOURCE_GIFT, 0);
        if (RunGame_Exhausted(mon) && Pokemon_GetValue(partyMon, MON_DATA_HP, NULL)) RunGame_Exhaust(partyMon);
    }
}

void RunGame_ClearIdentity(BoxPokemon *mon)
{
    if (!RunSession_Configured()) return;
    u8 marker = BoxPokemon_GetValue(mon, MON_DATA_UNUSED_113, NULL) & ~(RUN_MON_TRACKED | RUN_MON_DEAD);
    u16 id = 0;
    BoxPokemon_SetValue(mon, MON_DATA_UNUSED_113, &marker);
    BoxPokemon_SetValue(mon, MON_DATA_UNUSED_114, &id);
}

void RunGame_DerivedIdentity(Pokemon *mon, Pokemon *parent)
{
    if (!RunSession_Configured()) return;
    RunMonRecord record;
    if (!RunState_GetMon(RunSession_State(), RunGame_MonID(Pokemon_GetBoxPokemon(parent)), &record)) {
        RunGame_Register(Pokemon_GetBoxPokemon(mon), RUN_SOURCE_GIFT, 0);
        return;
    }
    record.species = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
    Pokemon_GetValue(mon, MON_DATA_NICKNAME, record.nickname);
    if (record.evolutionCount) record.evolutions[record.evolutionCount - 1] = record.species;
    else { record.evolutions[0] = record.species; record.evolutionCount = 1; }
    u16 id = RunState_AddMon(RunSession_State(), &record);
    u8 marker = BoxPokemon_GetValue(Pokemon_GetBoxPokemon(mon), MON_DATA_UNUSED_113, NULL) | RUN_MON_TRACKED;
    BoxPokemon_SetValue(Pokemon_GetBoxPokemon(mon), MON_DATA_UNUSED_113, &marker);
    BoxPokemon_SetValue(Pokemon_GetBoxPokemon(mon), MON_DATA_UNUSED_114, &id);
}

int RunGame_FieldMoveSlot(Party *party, u16 move)
{
    if (!RunSession_Configured() || RunGame_Over()
        || !(RunSession_Config()->captureAttempts || (RunSession_Config()->flags & (RUN_RANDOMIZE_WILD | RUN_RANDOMIZE_GIFTS | RUN_RANDOMIZE_EGGS)))) return MAX_PARTY_SIZE;
    static const u16 moves[] = { MOVE_CUT, MOVE_FLY, MOVE_SURF, MOVE_STRENGTH, MOVE_DEFOG, MOVE_ROCK_SMASH, MOVE_WATERFALL, MOVE_ROCK_CLIMB };
    static const u8 badges[] = { BADGE_ID_FOREST, BADGE_ID_COBBLE, BADGE_ID_FEN, BADGE_ID_MINE, BADGE_ID_RELIC, BADGE_ID_COAL, BADGE_ID_BEACON, BADGE_ID_ICICLE };
    u32 index;
    for (index = 0; index < NELEMS(moves); index++) if (moves[index] == move) break;
    if (index == NELEMS(moves)) return MAX_PARTY_SIZE;
    SaveData *save = SaveData_Ptr();
    if (!TrainerInfo_HasBadge(SaveData_GetTrainerInfo(save), badges[index])
        || !Bag_CanRemoveItem(SaveData_GetBag(save), ITEM_HM01 + index, 1, HEAP_ID_FIELD2)) return MAX_PARTY_SIZE;
    // Field-only HM assistance avoids randomizer/capture-limit softlocks. The
    // real disc and badge are mandatory; learnsets and battle moves are untouched.
    for (u32 slot = 0; slot < Party_GetCurrentCount(party); slot++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(party, slot);
        if (!Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL) && Pokemon_GetValue(mon, MON_DATA_HP, NULL)
            && !RunGame_Exhausted(Pokemon_GetBoxPokemon(mon))) return slot;
    }
    return MAX_PARTY_SIZE;
}
