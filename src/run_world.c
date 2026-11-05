#include "run_world.h"

#define RUN_INCLUDE_WORLD_TABLES
#include "run_world_data.h"

u16 RunWorld_Zone(u32 mapHeaderID)
{
    return mapHeaderID < RUN_MAP_COUNT ? sRunMapZones[mapHeaderID] : 0xFFFF;
}

u32 RunWorld_DataVersion(void)
{
    return RUN_WORLD_DATA_VERSION;
}

BOOL RunWorld_ZoneKnown(u16 zone)
{
    return zone < 256 && sRunKnownZones[zone] != 0;
}

BOOL RunWorld_IsBoss(u32 trainerID)
{
    return trainerID < RUN_TRAINER_COUNT && sRunTrainerBoss[trainerID] != 0;
}

u8 RunWorld_BaseCap(u8 badgeCount)
{
    return sRunBaseCaps[badgeCount < 8 ? badgeCount : 8];
}

u8 RunWorld_LevelCap(const RunConfig *config, u8 badgeCount)
{
    if (!RunConfig_IsValid(config) || !(config->flags & RUN_LEVEL_CAPS)) {
        return 100;
    }
    u32 cap = RunWorld_BaseCap(badgeCount) + config->levelCapAllowance;
    return cap > 100 ? 100 : cap;
}

u8 RunWorld_TrainerLevel(const RunConfig *config, u8 badgeCount, u8 baseLevel, BOOL boss)
{
    if (!RunConfig_IsValid(config) || !(config->flags & RUN_TRAINER_SCALING)
        || (boss && !(config->flags & RUN_SCALE_BOSSES))) {
        return baseLevel;
    }
    u32 level = baseLevel + (badgeCount > 8 ? 8 : badgeCount) * config->trainerLevelsPerBadge;
    return level > 100 ? 100 : level;
}

u32 RunWorld_ClampExp(u32 current, u32 award, u32 capExp)
{
    if (current >= capExp) {
        return current;
    }
    return award >= capExp - current ? capExp : current + award;
}
