#include "run_state.h"

#include <string.h>

#include "run_randomizer.h"
#include "run_world.h"

#define STATE_MAGIC 0x31534E52u // RNS1
#define CONFIG_OFFSET 32
#define FAMILY_OFFSET 68
#define ZONE_OFFSET 132
#define ZONE_SIZE 28
#define SPECIAL_OFFSET (ZONE_OFFSET + RUN_ZONE_COUNT * ZONE_SIZE)
#define SPECIAL_SIZE 20
#define MON_OFFSET (SPECIAL_OFFSET + RUN_SPECIAL_CAPACITY * SPECIAL_SIZE)
#define MON_SIZE 48

typedef char RunStateFits[(MON_OFFSET + RUN_MON_CAPACITY * MON_SIZE <= RUN_STATE_SIZE) ? 1 : -1];

static u16 Get16(const u8 *p)
{
    return p[0] | ((u16)p[1] << 8);
}

static u32 Get32(const u8 *p)
{
    return Get16(p) | ((u32)Get16(p + 2) << 16);
}

static void Put16(u8 *p, u16 value)
{
    p[0] = value;
    p[1] = value >> 8;
}

static void Put32(u8 *p, u32 value)
{
    Put16(p, value);
    Put16(p + 2, value >> 16);
}

static u32 Checksum(const RunState *state)
{
    u32 crc = 0xFFFFFFFFu;
    for (u32 i = 0; i < RUN_STATE_SIZE; i++) {
        if (i >= 28 && i < 32) {
            continue;
        }
        crc ^= state->bytes[i];
        for (u32 bit = 0; bit < 8; bit++) {
            crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320u : 0);
        }
    }
    return crc ^ 0xFFFFFFFFu;
}

BOOL RunState_Init(RunState *state, const RunConfig *config, u32 nonce)
{
    if (state == NULL || !RunConfig_IsValid(config) || nonce == 0) {
        return FALSE;
    }
    memset(state, 0, sizeof(*state));
    Put32(state->bytes, STATE_MAGIC);
    Put16(state->bytes + 4, RUN_STATE_SCHEMA);
    Put16(state->bytes + 6, RUN_STATE_SIZE / 4);
    state->bytes[8] = RUN_STATUS_ACTIVE;
    Put32(state->bytes + 12, nonce);
    Put32(state->bytes + 24, RunWorld_DataVersion());
    RunConfig_Encode(config, state->bytes + CONFIG_OFFSET, RUN_CONFIG_WIRE_SIZE);
    RunState_Seal(state, 0);
    return TRUE;
}

BOOL RunState_Config(const RunState *state, RunConfig *config)
{
    return state != NULL && RunConfig_Decode(config, state->bytes + CONFIG_OFFSET, RUN_CONFIG_WIRE_SIZE) == RUN_CONFIG_READ_OK;
}

enum RunStatus RunState_Status(const RunState *state)
{
    return state != NULL && Get32(state->bytes) == STATE_MAGIC ? state->bytes[8] : RUN_STATUS_UNCONFIGURED;
}

void RunState_SetGameOver(RunState *state)
{
    if (RunState_Status(state) == RUN_STATUS_ACTIVE) {
        state->bytes[8] = RUN_STATUS_GAME_OVER;
    }
}

BOOL RunState_CaptureActivated(const RunState *state)
{
    return state != NULL && state->bytes[9] == 1;
}

void RunState_ActivateCapture(RunState *state)
{
    state->bytes[9] = 1;
}

void RunState_Visit(RunState *state, u16 zone)
{
    if (zone < RUN_ZONE_COUNT) {
        state->bytes[ZONE_OFFSET + zone * ZONE_SIZE] = 1;
    }
}

BOOL RunState_Visited(const RunState *state, u16 zone)
{
    return zone < RUN_ZONE_COUNT && state->bytes[ZONE_OFFSET + zone * ZONE_SIZE] == 1;
}

u16 RunState_DuplicatesSeen(const RunState *state, u16 zone)
{
    return zone < RUN_ZONE_COUNT ? Get16(state->bytes + ZONE_OFFSET + zone * ZONE_SIZE + 2) : 0;
}

u8 RunState_Remaining(const RunState *state, u16 zone)
{
    RunConfig config;
    if (zone >= RUN_ZONE_COUNT || !RunState_Config(state, &config)) {
        return 0;
    }
    u8 used = state->bytes[ZONE_OFFSET + zone * ZONE_SIZE + 1];
    return used < config.captureAttempts ? config.captureAttempts - used : 0;
}

BOOL RunState_GetOpportunity(const RunState *state, u16 zone, u8 slot, RunOpportunity *opportunity)
{
    if (zone >= RUN_ZONE_COUNT || slot >= 3 || opportunity == NULL) {
        return FALSE;
    }
    const u8 *p = state->bytes + ZONE_OFFSET + zone * ZONE_SIZE + 4 + slot * 8;
    opportunity->species = Get16(p);
    opportunity->monID = Get16(p + 2);
    opportunity->outcome = p[4];
    opportunity->source = p[5];
    return TRUE;
}

BOOL RunState_OwnedFamily(const RunState *state, u16 species)
{
    u16 family = RunSpecies_EvolutionFamily(species);
    return family != 0 && (state->bytes[FAMILY_OFFSET + family / 8] & (1u << (family % 8))) != 0;
}

void RunState_MarkOwned(RunState *state, u16 species)
{
    u16 family = RunSpecies_EvolutionFamily(species);
    if (family != 0) {
        state->bytes[FAMILY_OFFSET + family / 8] |= 1u << (family % 8);
    }
}

enum RunCapturePermission RunState_Reserve(RunState *state, u16 zone, u16 species, enum RunEncounterSource source, BOOL shiny, u8 *slot)
{
    RunConfig config;
    if (slot == NULL || !RunState_Config(state, &config) || zone >= RUN_ZONE_COUNT || !RunSpecies_IsValid(species)
        || (u32)source > RUN_SOURCE_SHINY) {
        return RUN_CAPTURE_INVALID;
    }
    *slot = 0xFF;
    RunState_Visit(state, zone);
    if (!RunState_CaptureActivated(state) || !RunConfig_CaptureLimitsEnabled(&config)) {
        return RUN_CAPTURE_INACTIVE;
    }
    // Static exemption is unconditional and takes priority over dupes/history limits.
    if (source == RUN_SOURCE_STATIC || (shiny && RunConfig_ShinyClauseEnabled(&config))
        || (source == RUN_SOURCE_GIFT && (config.flags & RUN_SEPARATE_GIFTS))
        || (source == RUN_SOURCE_EGG && (config.flags & RUN_SEPARATE_EGGS))) {
        return RUN_CAPTURE_ALLOWED;
    }
    u8 *zoneData = state->bytes + ZONE_OFFSET + zone * ZONE_SIZE;
    if (source == RUN_SOURCE_WILD && RunConfig_DupesEnabled(&config) && RunState_OwnedFamily(state, species)) {
        u16 count = Get16(zoneData + 2);
        if (count != 0xFFFF) {
            Put16(zoneData + 2, count + 1);
        }
        return RUN_CAPTURE_DUPLICATE;
    }
    if (RunState_Remaining(state, zone) == 0) {
        return RUN_CAPTURE_EXHAUSTED;
    }
    if (RunState_MonCount(state) == RUN_MON_CAPACITY) {
        return RUN_CAPTURE_HISTORY_FULL;
    }
    *slot = zoneData[1]++;
    u8 *p = zoneData + 4 + *slot * 8;
    Put16(p, species);
    p[4] = RUN_OUTCOME_PENDING;
    p[5] = source;
    return RUN_CAPTURE_ALLOWED;
}

BOOL RunState_Finish(RunState *state, u16 zone, u8 slot, enum RunEncounterOutcome outcome, u16 monID)
{
    if (zone >= RUN_ZONE_COUNT || slot >= 3 || outcome < RUN_OUTCOME_CAUGHT || outcome > RUN_OUTCOME_INTERRUPTED
        || monID > RunState_MonCount(state)) {
        return FALSE;
    }
    u8 *p = state->bytes + ZONE_OFFSET + zone * ZONE_SIZE + 4 + slot * 8;
    if (p[4] != RUN_OUTCOME_PENDING) {
        return p[4] == outcome && Get16(p + 2) == monID;
    }
    p[4] = outcome;
    Put16(p + 2, monID);
    return TRUE;
}

u16 RunState_MonCount(const RunState *state)
{
    return Get16(state->bytes + 20);
}

BOOL RunState_GetMon(const RunState *state, u16 id, RunMonRecord *record)
{
    if (id == 0 || id > RunState_MonCount(state) || id > RUN_MON_CAPACITY || record == NULL) {
        return FALSE;
    }
    const u8 *p = state->bytes + MON_OFFSET + (id - 1) * MON_SIZE;
    record->personality = Get32(p);
    record->trainerID = Get32(p + 4);
    record->originalSpecies = Get16(p + 8);
    record->species = Get16(p + 10);
    record->originZone = Get16(p + 12);
    record->deathZone = Get16(p + 14);
    record->specialID = Get16(p + 16);
    for (u32 i = 0; i < 3; i++) {
        record->evolutions[i] = Get16(p + 18 + i * 2);
    }
    for (u32 i = 0; i < RUN_HISTORY_NAME_LEN; i++) {
        record->nickname[i] = Get16(p + 24 + i * 2);
    }
    record->flags = p[46];
    record->evolutionCount = p[47];
    return TRUE;
}

BOOL RunState_PutMon(RunState *state, u16 id, const RunMonRecord *record)
{
    if (id == 0 || id > RunState_MonCount(state) || id > RUN_MON_CAPACITY || record == NULL
        || !RunSpecies_IsValid(record->species) || !RunSpecies_IsValid(record->originalSpecies)
        || record->evolutionCount > 3 || record->originZone >= RUN_ZONE_COUNT
        || (record->deathZone != 0xFFFF && record->deathZone >= RUN_ZONE_COUNT)
        || record->specialID > RunState_SpecialCount(state)
        || (record->flags & RUN_MON_SOURCE_MASK) > RUN_SOURCE_SHINY) {
        return FALSE;
    }
    u8 *p = state->bytes + MON_OFFSET + (id - 1) * MON_SIZE;
    Put32(p, record->personality);
    Put32(p + 4, record->trainerID);
    Put16(p + 8, record->originalSpecies);
    Put16(p + 10, record->species);
    Put16(p + 12, record->originZone);
    Put16(p + 14, record->deathZone);
    Put16(p + 16, record->specialID);
    for (u32 i = 0; i < 3; i++) {
        Put16(p + 18 + i * 2, record->evolutions[i]);
    }
    for (u32 i = 0; i < RUN_HISTORY_NAME_LEN; i++) {
        Put16(p + 24 + i * 2, record->nickname[i]);
    }
    p[46] = record->flags;
    p[47] = record->evolutionCount;
    RunState_MarkOwned(state, record->species);
    return TRUE;
}

u16 RunState_AddMon(RunState *state, const RunMonRecord *record)
{
    u16 count = RunState_MonCount(state);
    if (count >= RUN_MON_CAPACITY) {
        return 0;
    }
    Put16(state->bytes + 20, count + 1);
    if (!RunState_PutMon(state, count + 1, record)) {
        Put16(state->bytes + 20, count);
        return 0;
    }
    return count + 1;
}

BOOL RunState_ExhaustMon(RunState *state, u16 id, u16 zone)
{
    RunMonRecord record;
    if (zone >= RUN_ZONE_COUNT || !RunState_GetMon(state, id, &record)) {
        return FALSE;
    }
    if (!(record.flags & RUN_MON_EXHAUSTED)) {
        record.flags |= RUN_MON_EXHAUSTED;
        record.deathZone = zone;
        return RunState_PutMon(state, id, &record);
    }
    return TRUE;
}

u16 RunState_SpecialCount(const RunState *state)
{
    return Get16(state->bytes + 22);
}

BOOL RunState_GetSpecial(const RunState *state, u16 id, RunSpecialRecord *record)
{
    if (id == 0 || id > RunState_SpecialCount(state) || id > RUN_SPECIAL_CAPACITY || record == NULL) {
        return FALSE;
    }
    const u8 *p = state->bytes + SPECIAL_OFFSET + (id - 1) * SPECIAL_SIZE;
    record->eventKey = Get32(p);
    record->zone = Get16(p + 4);
    record->species = Get16(p + 6);
    record->lastMonID = Get16(p + 8);
    record->attempts = Get16(p + 10);
    record->caught = Get16(p + 12);
    record->failed = Get16(p + 14);
    record->source = p[16];
    record->outcome = p[17];
    return TRUE;
}

u16 RunState_SpecialAttempt(RunState *state, u32 eventKey, u16 zone, u16 species, enum RunEncounterSource source)
{
    if (zone >= RUN_ZONE_COUNT || !RunSpecies_IsValid(species) || source < RUN_SOURCE_STATIC || source > RUN_SOURCE_SHINY) {
        return 0;
    }
    u16 count = RunState_SpecialCount(state);
    u16 id;
    for (id = 1; id <= count; id++) {
        const u8 *p = state->bytes + SPECIAL_OFFSET + (id - 1) * SPECIAL_SIZE;
        if (Get32(p) == eventKey && p[16] == source) {
            // Only callers with an established new encounter can start another attempt.
            break;
        }
    }
    if (id > RUN_SPECIAL_CAPACITY) {
        return 0;
    }
    u8 *p = state->bytes + SPECIAL_OFFSET + (id - 1) * SPECIAL_SIZE;
    if (id > count) {
        Put16(state->bytes + 22, id);
        Put32(p, eventKey);
        Put16(p + 4, zone);
        Put16(p + 6, species);
        p[16] = source;
    }
    if (Get16(p + 10) != 0xFFFF) {
        Put16(p + 10, Get16(p + 10) + 1);
    }
    p[17] = RUN_OUTCOME_PENDING;
    return id;
}

BOOL RunState_FinishSpecial(RunState *state, u16 id, enum RunEncounterOutcome outcome, u16 monID)
{
    if (id == 0 || id > RunState_SpecialCount(state) || outcome < RUN_OUTCOME_CAUGHT || outcome > RUN_OUTCOME_INTERRUPTED
        || monID > RunState_MonCount(state)) {
        return FALSE;
    }
    u8 *p = state->bytes + SPECIAL_OFFSET + (id - 1) * SPECIAL_SIZE;
    if (p[17] != RUN_OUTCOME_PENDING) {
        return p[17] == outcome && Get16(p + 8) == monID;
    }
    u8 offset = outcome == RUN_OUTCOME_CAUGHT ? 12 : 14;
    if (Get16(p + offset) != 0xFFFF) {
        Put16(p + offset, Get16(p + offset) + 1);
    }
    p[17] = outcome;
    Put16(p + 8, monID);
    return TRUE;
}

u32 RunState_Nonce(const RunState *state)
{
    return Get32(state->bytes + 12);
}

void RunState_Seal(RunState *state, u32 normalCounter)
{
    Put32(state->bytes + 16, normalCounter);
    Put32(state->bytes + 28, Checksum(state));
}

BOOL RunState_Validate(const RunState *state, u32 nonce, u32 normalCounter)
{
    RunConfig config;
    if (state == NULL || Get32(state->bytes) != STATE_MAGIC || Get16(state->bytes + 4) != RUN_STATE_SCHEMA
        || Get16(state->bytes + 6) != RUN_STATE_SIZE / 4 || nonce == 0 || RunState_Nonce(state) != nonce
        || Get32(state->bytes + 16) != normalCounter || !RunState_Config(state, &config)
        || Get32(state->bytes + 24) != RunWorld_DataVersion()
        || (state->bytes[8] != RUN_STATUS_ACTIVE && state->bytes[8] != RUN_STATUS_GAME_OVER)
        || state->bytes[9] > 1 || RunState_MonCount(state) > RUN_MON_CAPACITY
        || RunState_SpecialCount(state) > RUN_SPECIAL_CAPACITY || Get32(state->bytes + 28) != Checksum(state)) {
        return FALSE;
    }
    for (u16 zone = 0; zone < RUN_ZONE_COUNT; zone++) {
        const u8 *p = state->bytes + ZONE_OFFSET + zone * ZONE_SIZE;
        if (p[0] > 1 || p[1] > config.captureAttempts) {
            return FALSE;
        }
        for (u8 slot = 0; slot < 3; slot++) {
            RunOpportunity opportunity;
            RunState_GetOpportunity(state, zone, slot, &opportunity);
            if (opportunity.outcome > RUN_OUTCOME_INTERRUPTED || opportunity.monID > RunState_MonCount(state)
                || opportunity.source > RUN_SOURCE_SHINY
                || (slot < p[1] && (!RunSpecies_IsValid(opportunity.species) || opportunity.outcome == RUN_OUTCOME_NONE))
                || (slot >= p[1] && opportunity.outcome != RUN_OUTCOME_NONE)) {
                return FALSE;
            }
        }
    }
    for (u16 id = 1; id <= RunState_MonCount(state); id++) {
        RunMonRecord record;
        RunState_GetMon(state, id, &record);
        if (!RunSpecies_IsValid(record.species) || !RunSpecies_IsValid(record.originalSpecies)
            || record.originZone >= RUN_ZONE_COUNT || record.evolutionCount > 3
            || record.specialID > RunState_SpecialCount(state)
            || (record.deathZone != 0xFFFF && record.deathZone >= RUN_ZONE_COUNT)
            || (record.flags & RUN_MON_SOURCE_MASK) > RUN_SOURCE_SHINY) {
            return FALSE;
        }
        for (u8 i = 0; i < record.evolutionCount; i++) {
            if (!RunSpecies_IsValid(record.evolutions[i])) {
                return FALSE;
            }
        }
    }
    for (u16 id = 1; id <= RunState_SpecialCount(state); id++) {
        RunSpecialRecord record;
        RunState_GetSpecial(state, id, &record);
        if (record.zone >= RUN_ZONE_COUNT || !RunSpecies_IsValid(record.species)
            || record.lastMonID > RunState_MonCount(state) || record.source < RUN_SOURCE_STATIC
            || record.source > RUN_SOURCE_SHINY || record.outcome == RUN_OUTCOME_NONE
            || record.outcome > RUN_OUTCOME_INTERRUPTED) {
            return FALSE;
        }
    }
    return TRUE;
}

void RunState_RecoverInterrupted(RunState *state)
{
    for (u16 zone = 0; zone < RUN_ZONE_COUNT; zone++) {
        for (u8 slot = 0; slot < 3; slot++) {
            RunOpportunity opportunity;
            RunState_GetOpportunity(state, zone, slot, &opportunity);
            if (opportunity.outcome == RUN_OUTCOME_PENDING) {
                RunState_Finish(state, zone, slot, RUN_OUTCOME_INTERRUPTED, 0);
            }
        }
    }
    for (u16 id = 1; id <= RunState_SpecialCount(state); id++) {
        RunSpecialRecord record;
        RunState_GetSpecial(state, id, &record);
        if (record.outcome == RUN_OUTCOME_PENDING) {
            RunState_FinishSpecial(state, id, RUN_OUTCOME_INTERRUPTED, 0);
        }
    }
}
