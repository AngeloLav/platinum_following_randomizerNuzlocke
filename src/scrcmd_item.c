#include "scrcmd_item.h"

#include <nitro.h>
#include <string.h>

#include "constants/heap.h"

#include "field/field_system.h"

#include "bag.h"
#include "field_script_context.h"
#include "inlines.h"
#include "item.h"
#include "script_manager.h"
#include "run_extras.h"
#include "run_session.h"
#include "unk_0205DFC4.h"

BOOL ScrCmd_AddItem(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 item = ScriptContext_GetVar(ctx);
    u16 count = ScriptContext_GetVar(ctx);
    u16 *destVar = ScriptContext_GetVarPointer(ctx);

    *destVar = Bag_TryAddItem(SaveData_GetBag(fieldSystem->saveData), item, count, HEAP_ID_FIELD1);
    return FALSE;
}

BOOL ScrCmd_RemoveItem(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 item = ScriptContext_GetVar(ctx);
    u16 count = ScriptContext_GetVar(ctx);
    u16 *destVar = ScriptContext_GetVarPointer(ctx);

    *destVar = Bag_TryRemoveItem(SaveData_GetBag(fieldSystem->saveData), item, count, HEAP_ID_FIELD1);
    return FALSE;
}

BOOL ScrCmd_CanFitItem(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 item = ScriptContext_GetVar(ctx);
    u16 count = ScriptContext_GetVar(ctx);
    u16 *destVar = ScriptContext_GetVarPointer(ctx);

    u16 scriptID = *(u16 *)FieldSystem_GetScriptMemberPtr(fieldSystem, SCRIPT_MANAGER_SCRIPT_ID);
    if (scriptID >= SCRIPT_ID_OFFSET_VISIBLE_ITEMS && scriptID < SCRIPT_ID_OFFSET_HIDDEN_ITEMS) {
        u16 localID = *(u16 *)FieldSystem_GetScriptMemberPtr(fieldSystem, SCRIPT_DATA_TARGET_OBJECT_ID);
        item = RunExtras_FieldItem(RunSession_Config(),
            (((u32)fieldSystem->location->mapHeaderID << 16) | scriptID) ^ (localID * 0x9E3779B9u), item);
        *FieldSystem_GetVarPointer(fieldSystem, 0x8004) = item;
    }

    *destVar = Bag_CanFitItem(SaveData_GetBag(fieldSystem->saveData), item, count, HEAP_ID_FIELD1);
    return FALSE;
}

BOOL ScrCmd_CheckItem(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 item = ScriptContext_GetVar(ctx);
    u16 count = ScriptContext_GetVar(ctx);
    u16 *destVar = ScriptContext_GetVarPointer(ctx);

    *destVar = Bag_CanRemoveItem(SaveData_GetBag(fieldSystem->saveData), item, count, HEAP_ID_FIELD2);
    return FALSE;
}

BOOL ScrCmd_GetItemQuantity(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 item = ScriptContext_GetVar(ctx);
    u16 *destVar = ScriptContext_GetVarPointer(ctx);

    *destVar = Bag_GetItemQuantity(SaveData_GetBag(fieldSystem->saveData), item, HEAP_ID_FIELD2);
    return FALSE;
}

BOOL ScrCmd_IsItemTMHM(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 item = ScriptContext_GetVar(ctx);
    u16 *destVar = ScriptContext_GetVarPointer(ctx);

    *destVar = Item_IsTMHM(item);
    return FALSE;
}

BOOL ScrCmd_GetItemPocket(ScriptContext *ctx)
{
    FieldSystem *fieldSystem = ctx->fieldSystem;
    u16 item = ScriptContext_GetVar(ctx);
    u16 *pocket = ScriptContext_GetVarPointer(ctx);

    *pocket = (u16)Item_LoadParam(item, ITEM_PARAM_FIELD_POCKET, HEAP_ID_FIELD2);
    return FALSE;
}

BOOL ScrCmd_Dummy081(ScriptContext *ctx)
{
    return FALSE;
}

BOOL ScrCmd_Dummy082(ScriptContext *ctx)
{
    return FALSE;
}
