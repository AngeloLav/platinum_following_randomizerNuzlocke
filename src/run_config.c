#include "run_config.h"

#include <string.h>

#include "run_species_data.h"

#define RUN_KNOWN_FLAGS ((1u << 18) - 1)
#define RUN_CONFIG_MAGIC 0x434E5552u // "RUNC", little endian

static u16 Read16(const u8 *p)
{
    return p[0] | ((u16)p[1] << 8);
}

static u32 Read32(const u8 *p)
{
    return Read16(p) | ((u32)Read16(p + 2) << 16);
}

static void Write16(u8 *p, u16 value)
{
    p[0] = value;
    p[1] = value >> 8;
}

static void Write32(u8 *p, u32 value)
{
    Write16(p, value);
    Write16(p + 2, value >> 16);
}

static u32 Checksum(const u8 *bytes, u32 size)
{
    u32 crc = 0xFFFFFFFFu;

    for (u32 i = 0; i < size; i++) {
        crc ^= bytes[i];
        for (u32 bit = 0; bit < 8; bit++) {
            crc = (crc >> 1) ^ ((crc & 1) ? 0xEDB88320u : 0);
        }
    }

    return crc ^ 0xFFFFFFFFu;
}

u32 RunConfig_SpeciesDataVersion(void)
{
    return RUN_SPECIES_DATA_VERSION;
}

BOOL RunConfig_Init(RunConfig *config, enum RunPreset preset, u32 seed)
{
    if (config == NULL || (u32)preset >= RUN_PRESET_COUNT) {
        return FALSE;
    }

    memset(config, 0, sizeof(*config));
    config->seed = seed;
    config->rulesVersion = RUN_RULES_VERSION;
    config->speciesDataVersion = RUN_SPECIES_DATA_VERSION;
    config->bstTolerance = 25;
    config->bstWeight = 75;
    config->preset = preset;
    config->seedMode = RUN_SEED_AUTO;
    config->levelCapAllowance = 0;
    config->trainerLevelsPerBadge = 1;
    config->flags = RUN_LEGENDARY_POOLS | RUN_SEPARATE_GIFTS | RUN_SEPARATE_EGGS
        | RUN_AREA_NOTIFICATIONS | RUN_DAY_CARE_EGGS | RUN_SCALE_BOSSES;

    if (preset == RUN_PRESET_RANDOMIZER || preset == RUN_PRESET_HARDCORE_RANDOMIZER) {
        config->flags |= RUN_RANDOMIZE_WILD | RUN_RANDOMIZE_TRAINERS;
    }

    if (preset == RUN_PRESET_NUZLOCKE || preset == RUN_PRESET_HARDCORE_RANDOMIZER) {
        config->captureAttempts = 1;
        config->flags |= RUN_DUPES_CLAUSE | RUN_SHINY_CLAUSE | RUN_PERMADEATH | RUN_END_ON_PARTY_WIPE;
    }

    if (preset == RUN_PRESET_HARDCORE_RANDOMIZER) {
        config->flags |= RUN_LEVEL_CAPS | RUN_TRAINER_SCALING;
        config->flags &= ~RUN_DAY_CARE_EGGS;
    }

    // Custom starts with the Classic configuration, then accepts individual edits.
    return TRUE;
}

BOOL RunConfig_IsValid(const RunConfig *config)
{
    return config != NULL
        && config->speciesDataVersion == RUN_SPECIES_DATA_VERSION
        && (config->flags & ~RUN_KNOWN_FLAGS) == 0
        && (config->rulesVersion == 2 || config->rulesVersion == RUN_RULES_VERSION)
        && config->shinyRate <= 3
        && config->levelCapMode <= RUN_CAP_OBEDIENCE
        && (config->rulesVersion != 2 || (config->shinyRate == 0 && config->levelCapMode == RUN_CAP_EXP_LOCK && !(config->flags & RUN_RANDOMIZE_FIELD_ITEMS)))
        && config->bstTolerance >= 10 && config->bstTolerance <= 75
        && config->bstTolerance % 5 == 0
        && config->bstWeight <= 100 && config->bstWeight % 5 == 0
        && config->captureAttempts <= 3
        && config->preset < RUN_PRESET_COUNT
        && config->seedMode <= RUN_SEED_MANUAL
        && config->levelCapAllowance <= 5
        && config->trainerLevelsPerBadge >= 1 && config->trainerLevelsPerBadge <= 3;
}

enum RunPreset RunConfig_EffectivePreset(const RunConfig *config)
{
    if (!RunConfig_IsValid(config) || config->preset == RUN_PRESET_CUSTOM) {
        return RUN_PRESET_CUSTOM;
    }

    RunConfig reference;
    RunConfig_Init(&reference, config->preset, config->seed);

    if (config->flags != reference.flags
        || config->bstTolerance != reference.bstTolerance
        || config->bstWeight != reference.bstWeight
        || config->captureAttempts != reference.captureAttempts
        || config->levelCapAllowance != reference.levelCapAllowance
        || config->trainerLevelsPerBadge != reference.trainerLevelsPerBadge
        || config->shinyRate != reference.shinyRate || config->levelCapMode != reference.levelCapMode) {
        return RUN_PRESET_CUSTOM;
    }

    return config->preset;
}

BOOL RunConfig_CaptureLimitsEnabled(const RunConfig *config)
{
    return RunConfig_IsValid(config) && config->captureAttempts != 0;
}

BOOL RunConfig_DupesEnabled(const RunConfig *config)
{
    return RunConfig_CaptureLimitsEnabled(config) && (config->flags & RUN_DUPES_CLAUSE) != 0;
}

BOOL RunConfig_ShinyClauseEnabled(const RunConfig *config)
{
    return RunConfig_CaptureLimitsEnabled(config) && (config->flags & RUN_SHINY_CLAUSE) != 0;
}

BOOL RunConfig_EndOnPartyWipe(const RunConfig *config)
{
    return RunConfig_IsValid(config) && (config->flags & RUN_PERMADEATH) != 0
        && (config->flags & RUN_END_ON_PARTY_WIPE) != 0;
}

BOOL RunConfig_DayCareEggsEnabled(const RunConfig *config)
{
    return RunConfig_IsValid(config) && (config->flags & RUN_DAY_CARE_EGGS) != 0;
}

BOOL RunConfig_ExpLockEnabled(const RunConfig *config)
{
    return RunConfig_IsValid(config) && (config->flags & RUN_LEVEL_CAPS) && config->levelCapMode == RUN_CAP_EXP_LOCK;
}

BOOL RunConfig_ObedienceCapEnabled(const RunConfig *config)
{
    return RunConfig_IsValid(config) && (config->flags & RUN_LEVEL_CAPS) && config->levelCapMode == RUN_CAP_OBEDIENCE;
}

BOOL RunConfig_Encode(const RunConfig *config, u8 *bytes, u32 size)
{
    if (!RunConfig_IsValid(config) || bytes == NULL || size != RUN_CONFIG_WIRE_SIZE) {
        return FALSE;
    }

    memset(bytes, 0, size);
    Write32(bytes, RUN_CONFIG_MAGIC);
    Write16(bytes + 4, config->rulesVersion == 2 ? 2 : RUN_CONFIG_SCHEMA_VERSION);
    Write16(bytes + 6, RUN_CONFIG_WIRE_SIZE);
    Write16(bytes + 8, config->rulesVersion);
    bytes[10] = config->seedMode;
    Write32(bytes + 12, config->speciesDataVersion);
    Write32(bytes + 16, config->seed);
    Write32(bytes + 20, config->flags);
    bytes[24] = config->bstTolerance;
    bytes[25] = config->bstWeight;
    bytes[26] = config->captureAttempts;
    bytes[27] = config->preset;
    bytes[28] = config->levelCapAllowance;
    bytes[29] = config->trainerLevelsPerBadge;
    bytes[30] = config->shinyRate;
    bytes[31] = config->levelCapMode;
    Write32(bytes + 32, Checksum(bytes, 32));
    return TRUE;
}

enum RunConfigReadResult RunConfig_Decode(RunConfig *config, const u8 *bytes, u32 size)
{
    if (config == NULL || bytes == NULL || size != RUN_CONFIG_WIRE_SIZE
        || Read32(bytes) != RUN_CONFIG_MAGIC || Read16(bytes + 6) != RUN_CONFIG_WIRE_SIZE
        || Read32(bytes + 32) != Checksum(bytes, 32)) {
        return RUN_CONFIG_READ_INVALID;
    }

    BOOL legacy = Read16(bytes + 4) == 2 && Read16(bytes + 8) == 2;
    if ((!legacy && (Read16(bytes + 4) != RUN_CONFIG_SCHEMA_VERSION || Read16(bytes + 8) != RUN_RULES_VERSION))
        || Read32(bytes + 12) != RUN_SPECIES_DATA_VERSION) {
        return RUN_CONFIG_READ_INCOMPATIBLE;
    }

    if (bytes[11] != 0 || (legacy && (bytes[30] != 0 || bytes[31] != 0))) {
        return RUN_CONFIG_READ_INVALID;
    }

    RunConfig decoded;
    memset(&decoded, 0, sizeof(decoded));
    decoded.seed = Read32(bytes + 16);
    decoded.speciesDataVersion = Read32(bytes + 12);
    decoded.flags = Read32(bytes + 20);
    decoded.bstTolerance = bytes[24];
    decoded.bstWeight = bytes[25];
    decoded.captureAttempts = bytes[26];
    decoded.preset = bytes[27];
    decoded.seedMode = bytes[10];
    decoded.levelCapAllowance = bytes[28];
    decoded.trainerLevelsPerBadge = bytes[29];
    decoded.shinyRate = bytes[30];
    decoded.rulesVersion = Read16(bytes + 8);
    decoded.levelCapMode = bytes[31];

    if (!RunConfig_IsValid(&decoded)) {
        return RUN_CONFIG_READ_INVALID;
    }

    *config = decoded;
    return RUN_CONFIG_READ_OK;
}
