#ifndef POKEPLATINUM_RUN_WORLD_H
#define POKEPLATINUM_RUN_WORLD_H

#include "run_config.h"

u16 RunWorld_Zone(u32 mapHeaderID);
BOOL RunWorld_ZoneKnown(u16 zone);
u32 RunWorld_DataVersion(void);
BOOL RunWorld_IsBoss(u32 trainerID);
u8 RunWorld_BaseCap(u8 badgeCount);
u8 RunWorld_LevelCap(const RunConfig *config, u8 badgeCount);
u8 RunWorld_TrainerLevel(const RunConfig *config, u8 badgeCount, u8 baseLevel, BOOL boss);
u32 RunWorld_ClampExp(u32 current, u32 award, u32 capExp);

#endif // POKEPLATINUM_RUN_WORLD_H
