#ifndef POKEPLATINUM_RUN_SESSION_H
#define POKEPLATINUM_RUN_SESSION_H

#include "run_state.h"
#include "savedata.h"
#include "bag.h"

void RunSession_Reset(void);
BOOL RunSession_Confirm(SaveData *save, const RunConfig *config, u32 nonce);
BOOL RunSession_Load(SaveData *save);
BOOL RunSession_PrepareSave(SaveData *save, u8 targetSlot);
BOOL RunSession_CanSave(SaveData *save);
const RunConfig *RunSession_Config(void);
RunState *RunSession_State(void);
BOOL RunSession_Configured(void);
BOOL RunSession_Damaged(void);
void RunSession_SetZone(u16 zone);
u16 RunSession_Zone(void);
BOOL RunSession_NoticeZone(u16 zone);
void RunSession_ItemReceived(Bag *bag, u16 item);
void RunSession_CheckBag(Bag *bag);

#endif // POKEPLATINUM_RUN_SESSION_H
