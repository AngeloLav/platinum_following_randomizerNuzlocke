#ifndef POKEPLATINUM_RUN_CONFIG_H
#define POKEPLATINUM_RUN_CONFIG_H

#include <nitro/types.h>

enum RunPreset {
    RUN_PRESET_CLASSIC = 0,
    RUN_PRESET_RANDOMIZER,
    RUN_PRESET_NUZLOCKE,
    RUN_PRESET_HARDCORE_RANDOMIZER,
    RUN_PRESET_CUSTOM,
    RUN_PRESET_COUNT
};

enum RunFlag {
    RUN_RANDOMIZE_WILD = 1 << 0,
    RUN_RANDOMIZE_TRAINERS = 1 << 1,
    RUN_RANDOMIZE_STATICS = 1 << 2,
    RUN_RANDOMIZE_GIFTS = 1 << 3,
    RUN_RANDOMIZE_EGGS = 1 << 4,
    RUN_LEGENDARY_POOLS = 1 << 5,
    RUN_DUPES_CLAUSE = 1 << 6,
    RUN_SHINY_CLAUSE = 1 << 7,
    RUN_SEPARATE_GIFTS = 1 << 8,
    RUN_SEPARATE_EGGS = 1 << 9,
    RUN_PERMADEATH = 1 << 10,
    RUN_LEVEL_CAPS = 1 << 11,
    RUN_TRAINER_SCALING = 1 << 12,
    RUN_AREA_NOTIFICATIONS = 1 << 13,
    RUN_DAY_CARE_EGGS = 1 << 14,
    RUN_SCALE_BOSSES = 1 << 15,
    RUN_END_ON_PARTY_WIPE = 1 << 16,
    RUN_RANDOMIZE_FIELD_ITEMS = 1 << 17
};

enum RunSeedMode {
    RUN_SEED_AUTO = 0,
    RUN_SEED_MANUAL
};

enum RunLevelCapMode {
    RUN_CAP_EXP_LOCK = 0,
    RUN_CAP_OBEDIENCE = 1
};

enum RunConfigReadResult {
    RUN_CONFIG_READ_OK = 0,
    RUN_CONFIG_READ_INVALID,
    RUN_CONFIG_READ_INCOMPATIBLE
};

#define RUN_CONFIG_SCHEMA_VERSION 3
#define RUN_RULES_VERSION         3
#define RUN_CONFIG_WIRE_SIZE      36

// This is a runtime value, NOT a native-layout save structure. Use the codec.
typedef struct RunConfig {
    u32 seed;
    u32 speciesDataVersion;
    u32 flags;
    u8 bstTolerance;
    u8 bstWeight;
    u8 captureAttempts;
    u8 preset;
    u8 seedMode;
    u8 levelCapAllowance;
    u8 trainerLevelsPerBadge;
    u8 shinyRate; // 0..4: 1/8192, 1/4096, 1/2048, 1/1024, 1/512; new Pokemon only.
    u8 rulesVersion; // 2 retains published mappings; 3 uses the Gaussian mixture.
    u8 levelCapMode; // Zero is EXP Lock for all existing runs.
} RunConfig;

BOOL RunConfig_Init(RunConfig *config, enum RunPreset preset, u32 seed);
BOOL RunConfig_IsValid(const RunConfig *config);
u8 RunConfig_AdjustValue(u8 value, u8 low, u8 high, u8 step, BOOL decrease);
enum RunPreset RunConfig_EffectivePreset(const RunConfig *config);
BOOL RunConfig_CaptureLimitsEnabled(const RunConfig *config);
BOOL RunConfig_DupesEnabled(const RunConfig *config);
BOOL RunConfig_ShinyClauseEnabled(const RunConfig *config);
BOOL RunConfig_EndOnPartyWipe(const RunConfig *config);
BOOL RunConfig_DayCareEggsEnabled(const RunConfig *config);
BOOL RunConfig_ExpLockEnabled(const RunConfig *config);
BOOL RunConfig_ObedienceCapEnabled(const RunConfig *config);
BOOL RunConfig_Encode(const RunConfig *config, u8 *bytes, u32 size);
enum RunConfigReadResult RunConfig_Decode(RunConfig *config, const u8 *bytes, u32 size);
u32 RunConfig_SpeciesDataVersion(void);

#endif // POKEPLATINUM_RUN_CONFIG_H
