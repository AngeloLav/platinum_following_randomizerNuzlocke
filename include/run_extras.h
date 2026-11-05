#ifndef POKEPLATINUM_RUN_EXTRAS_H
#define POKEPLATINUM_RUN_EXTRAS_H
#include "run_config.h"
// Schema-2 Route 201 GivePokemon opcode parameters began at byte 0x045C.
// Pin the published key so later script/UI edits cannot reroll legacy starters.
#define RUN_LEGACY_STARTER_EVENT_KEY 0x0156045Cu
u16 RunExtras_FieldItem(const RunConfig *config, u32 locationKey, u16 original);
BOOL RunExtras_SafeFieldItem(u16 item);
BOOL RunExtras_UsableBall(u16 item);
u16 RunExtras_IntroSpecies(const RunConfig *config);
u16 RunExtras_Starter(const RunConfig *config, u8 slot);
// constraint: 0 ordinary, 1 Unown form, 2 Wurmple evolution branch.
u32 RunExtras_ShinyPID(u32 personality, u32 trainerID, u8 rate, u8 constraint);
#endif
