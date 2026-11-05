#ifndef POKEPLATINUM_RUN_GAME_H
#define POKEPLATINUM_RUN_GAME_H

#include "field_battle_data_transfer.h"
#include "location.h"
#include "field/field_system_decl.h"
#include "run_state.h"

u16 RunGame_MonID(BoxPokemon *mon);
u16 RunGame_Register(BoxPokemon *mon, enum RunEncounterSource source, u16 specialID);
void RunGame_Sync(BoxPokemon *mon);
BOOL RunGame_Exhausted(BoxPokemon *mon);
void RunGame_Exhaust(Pokemon *mon);
u16 RunGame_FilterHP(Pokemon *mon, u16 hp);
u32 RunGame_Experience(BoxPokemon *mon, u32 current, u32 award);
u8 RunGame_LevelCap(void);
BOOL RunGame_CanUseCandy(Pokemon *mon);
void RunGame_BeginBattle(FieldBattleDTO *dto);
void RunGame_EndBattle(const FieldBattleDTO *dto);
enum RunCapturePermission RunGame_CapturePermission(u8 battler, u8 enemySlot);
void RunGame_Caught(Pokemon *mon, u8 battler, u8 enemySlot);
// Returns the optional detailed record ID; use GiftResult for acceptance.
u16 RunGame_Gift(Pokemon *mon, enum RunEncounterSource source, u32 eventKey);
BOOL RunGame_GiftUsed(enum RunEncounterSource source, u32 eventKey);
u16 RunGame_RandomSpecies(enum RunEncounterSource source, u32 key, u32 slot, u16 species);
u16 RunGame_TrainerSpecies(u32 trainerID, u32 slot, u16 species);
u8 RunGame_TrainerLevel(u32 trainerID, u8 baseLevel);
BOOL RunGame_DayCareEggs(void);
void RunGame_AdjustTrainerParty(Party *party, u32 trainerID, u32 battler, u32 battleType);
void RunGame_StoreExhausted(SaveData *save);
void RunGame_SetGiftContext(enum RunEncounterSource source, u32 eventKey);
void RunGame_ClearGiftContext(void);
BOOL RunGame_AcceptScriptGift(Pokemon *mon);
void RunGame_SetGiftResult(BOOL accepted);
BOOL RunGame_GiftResult(void);
BOOL RunGame_Over(void);
void RunGame_ConfineLocation(Location *location);
void RunGame_PollEnd(FieldSystem *fieldSystem);
void RunGame_ObserveParty(Party *party);
int RunGame_FieldMoveSlot(Party *party, u16 move);
void RunGame_ClearIdentity(BoxPokemon *mon);
void RunGame_DerivedIdentity(Pokemon *mon, Pokemon *parent);

#endif // POKEPLATINUM_RUN_GAME_H
