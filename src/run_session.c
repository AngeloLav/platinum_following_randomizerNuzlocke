#include "run_session.h"

static u16 sNoticeZone = 0xFFFF;

#include <string.h>

#include "heap.h"
#include "run_store.h"
#include "savedata/save_table.h"
#include "savedata_misc.h"
#include "generated/items.h"
#include "run_extras.h"

// Separate BSS, not FIELD1/FIELD2 or the fixed native SaveData allocation.
static RunState sRun;
static RunConfig sConfig;
static BOOL sConfigured;
static BOOL sDamaged;
static u16 sZone;
static Bag *sOwnedBag;

static BOOL Read(void *context, u32 address, void *data, u32 size)
{
    (void)context;
    return SaveData_CardLoad(address, data, size);
}

static BOOL Write(void *context, u32 address, const void *data, u32 size)
{
    (void)context;
    return SaveData_CardSave(address, (void *)data, size);
}

static BOOL LayoutIsSafe(const SaveData *save)
{
    // Check actual compiled sizes, including extra footers/mirror keys, before
    // ever touching the extension. A future native-data expansion fails closed.
    for (int i = 0; i < gExtraSaveTableSize; i++) {
        u32 end = gExtraSaveTable[i].blockID * SAVE_SECTOR_SIZE
            + gExtraSaveTable[i].sizeFunc() + sizeof(SaveCheckFooter) + 16;
        if (end > RunStore_Address(0)) {
            return FALSE;
        }
    }
    for (int i = 0; i < SAVE_BLOCK_ID_MAX; i++) {
        if (save->blockInfo[i].offset + save->blockInfo[i].size > SAVE_PAGE_MAX * SAVE_SECTOR_SIZE) {
            return FALSE;
        }
    }
    return RunStore_Address(1) + RUN_STATE_SIZE <= 128u * SAVE_SECTOR_SIZE;
}

void RunSession_Reset(void)
{
    sNoticeZone = 0xFFFF;
    memset(&sRun, 0, sizeof(sRun));
    RunConfig_Init(&sConfig, RUN_PRESET_CLASSIC, 0);
    sConfigured = FALSE;
    sDamaged = FALSE;
    sZone = 0;
    sOwnedBag = NULL;
}

BOOL RunSession_Confirm(SaveData *save, const RunConfig *config, u32 nonce)
{
    if (sConfigured || sDamaged || !LayoutIsSafe(save) || !RunState_Init(&sRun, config, nonce)) {
        return FALSE;
    }
    sConfig = *config;
    sConfigured = TRUE;
    sOwnedBag = SaveData_GetBag(save);
    // Previously unused native u32: no table sizes or offsets are changed.
    // A nonzero anchor requires a matching extension, never a silent Classic reset.
    SaveData_MiscSaveBlock(save)->unused = nonce;
    return TRUE;
}

BOOL RunSession_Load(SaveData *save)
{
    RunSession_Reset();
    u32 nonce = SaveData_MiscSaveBlock(save)->unused;
    if (nonce == 0) {
        return TRUE; // Unmodified retail/Following saves remain Classic.
    }
    sDamaged = TRUE;
    if (!LayoutIsSafe(save)) {
        return FALSE;
    }
    RunState *scratch = Heap_AllocAtEnd(HEAP_ID_APPLICATION, sizeof(RunState));
    if (scratch == NULL) {
        return FALSE;
    }
    RunStoreIO io = { save, Read, Write };
    BOOL valid = RunStore_Load(&io, &sRun, scratch, nonce, save->blockCounters[SAVE_BLOCK_ID_NORMAL]);
    Heap_Free(scratch);
    if (!valid || !RunState_Config(&sRun, &sConfig)) {
        return FALSE;
    }
    sConfigured = TRUE;
    sDamaged = FALSE;
    sOwnedBag = SaveData_GetBag(save);
    RunSession_CheckBag(sOwnedBag);
    return TRUE;
}

void RunSession_ItemReceived(Bag *bag, u16 item)
{
    if (RunSession_Configured() && bag == sOwnedBag && RunExtras_UsableBall(item)) {
        RunState_ActivateCapture(&sRun);
    }
}

void RunSession_CheckBag(Bag *bag)
{
    if (!RunSession_Configured() || bag != sOwnedBag || RunState_CaptureActivated(&sRun)) return;
    // The pocket is already categorized. Do not load item NARCs or allocate
    // memory from this defensive check, which also runs during field frames.
    for (u16 slot = 0; slot < POKEBALL_POCKET_SIZE; slot++) {
        if (bag->pokeballs[slot].quantity && RunExtras_UsableBall(bag->pokeballs[slot].item)) {
            RunSession_ItemReceived(bag, bag->pokeballs[slot].item);
            break;
        }
    }
}

BOOL RunSession_PrepareSave(SaveData *save, u8 targetSlot)
{
    if (sDamaged) {
        return FALSE;
    }
    if (!sConfigured) {
        return SaveData_MiscSaveBlock(save)->unused == 0;
    }
    if (!LayoutIsSafe(save) || SaveData_MiscSaveBlock(save)->unused != RunState_Nonce(&sRun)) {
        return FALSE;
    }
    RunStoreIO io = { save, Read, Write };
    return RunStore_Prepare(&io, &sRun, save->blockCounters[SAVE_BLOCK_ID_NORMAL], targetSlot);
}

BOOL RunSession_CanSave(SaveData *save)
{
    if (sDamaged) return FALSE;
    if (!sConfigured) return SaveData_MiscSaveBlock(save)->unused == 0;
    if (!LayoutIsSafe(save) || SaveData_MiscSaveBlock(save)->unused != RunState_Nonce(&sRun)) return FALSE;
    RunState_Seal(&sRun, save->blockCounters[SAVE_BLOCK_ID_NORMAL]);
    return RunState_Validate(&sRun, RunState_Nonce(&sRun), save->blockCounters[SAVE_BLOCK_ID_NORMAL]);
}

const RunConfig *RunSession_Config(void)
{
    return &sConfig;
}

RunState *RunSession_State(void)
{
    return &sRun;
}

BOOL RunSession_Configured(void)
{
    return sConfigured && !sDamaged;
}

BOOL RunSession_Damaged(void)
{
    return sDamaged;
}

void RunSession_SetZone(u16 zone)
{
    if (zone < RUN_ZONE_COUNT) {
        sZone = zone;
        if (RunSession_Configured()) {
            RunState_Visit(&sRun, zone);
        }
    }
}

u16 RunSession_Zone(void)
{
    return sZone;
}

BOOL RunSession_NoticeZone(u16 zone)
{
    BOOL changed = zone != sNoticeZone;
    sNoticeZone = zone;
    return changed;
}
