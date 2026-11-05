#include "applications/poketch/unused/4/graphics.h"

#include "constants/heap.h"
#include "constants/charcode.h"
#include "constants/string.h"

#include "applications/poketch/poketch_graphics.h"
#include "applications/poketch/poketch_task.h"

#include "bg_window.h"
#include "graphics.h"
#include "heap.h"
#include "font.h"
#include "text.h"
#include "run_session.h"
#include "run_world.h"
#include "save_player.h"
#include "trainer_info.h"
#include "res/text/bank/run_engine.h"
#include "sys_task_manager.h"

#include "res/graphics/poketch/poketch.naix"

static void EndTask(PoketchTaskManager *taskMan);
static void Task_DrawBackground(SysTask *task, void *taskMan);
static void Task_FreeBackground(SysTask *task, void *taskMan);

BOOL PoketchUnusedApp4Graphics_New(UnusedApp4Graphics **dest, const Unused4Data *data, BgConfig *bgConfig)
{
    UnusedApp4Graphics *graphics = Heap_Alloc(HEAP_ID_POKETCH_APP, sizeof(UnusedApp4Graphics));

    if (graphics != NULL) {
        MI_CpuClear8(graphics, sizeof(*graphics));
        PoketchTask_InitActiveTaskList(graphics->activeTasks, 8);
        graphics->data = data;
        graphics->bgConfig = BgConfig_New(HEAP_ID_POKETCH_APP);
        graphics->messages = MessageLoader_Init(MSG_LOADER_LOAD_ON_DEMAND, NARC_INDEX_MSGDATA__PL_MSG, TEXT_BANK_RUN_ENGINE, HEAP_ID_POKETCH_APP);
        graphics->locations = MessageLoader_Init(MSG_LOADER_LOAD_ON_DEMAND, NARC_INDEX_MSGDATA__PL_MSG, TEXT_BANK_LOCATION_NAMES, HEAP_ID_POKETCH_APP);
        graphics->line = String_Init(128, HEAP_ID_POKETCH_APP);
        graphics->value = String_Init(64, HEAP_ID_POKETCH_APP);

        if (graphics->bgConfig && graphics->messages && graphics->locations && graphics->line && graphics->value) {
            *dest = graphics;
            return TRUE;
        }
        PoketchUnusedApp4Graphics_Free(graphics);
    }

    return FALSE;
}

void PoketchUnusedApp4Graphics_Free(UnusedApp4Graphics *graphics)
{
    if (graphics != NULL) {
        if (graphics->messages) MessageLoader_Free(graphics->messages);
        if (graphics->locations) MessageLoader_Free(graphics->locations);
        if (graphics->line) String_Free(graphics->line);
        if (graphics->value) String_Free(graphics->value);
        if (graphics->bgConfig != NULL) {
            Heap_Free(graphics->bgConfig);
        }

        Heap_Free(graphics);
    }
}

static const PoketchTask sUnusedApp4Tasks[] = {
    { 0, Task_DrawBackground, 0 },
    { 1, Task_FreeBackground, 0 },
    { 0 }
};

void PoketchUnusedApp4Graphics_StartTask(UnusedApp4Graphics *graphics, u32 taskID)
{
    PoketchTask_Start(sUnusedApp4Tasks, taskID, graphics, graphics->data, graphics->activeTasks, 2, HEAP_ID_POKETCH_APP);
}

BOOL PoketchUnusedApp4Graphics_TaskIsNotActive(UnusedApp4Graphics *graphics, u32 taskID)
{
    return PoketchTask_TaskIsNotActive(graphics->activeTasks, taskID);
}

BOOL PoketchUnusedApp4Graphics_NoActiveTasks(UnusedApp4Graphics *graphics)
{
    return PoketchTask_NoActiveTasks(graphics->activeTasks);
}

static void EndTask(PoketchTaskManager *taskMan)
{
    UnusedApp4Graphics *graphics = PoketchTask_GetTaskData(taskMan);

    PoketchTask_EndTask(graphics->activeTasks, taskMan);
}

static void Task_DrawBackground(SysTask *task, void *taskMan)
{
    static const BgTemplate bgTemplate = {
        .x = 0,
        .y = 0,
        .bufferSize = 0x800,
        .baseTile = 0,
        .screenSize = BG_SCREEN_SIZE_256x256,
        .colorMode = GX_BG_COLORMODE_16,
        .screenBase = GX_BG_SCRBASE_0x7000,
        .charBase = GX_BG_CHARBASE_0x00000,
        .bgExtPltt = GX_BG_EXTPLTT_01,
        .priority = 2,
        .areaOver = 0,
        .mosaic = FALSE,
    };
    UnusedApp4Graphics *graphics = PoketchTask_GetTaskData(taskMan);

    Bg_InitFromTemplate(graphics->bgConfig, BG_LAYER_SUB_2, &bgTemplate, BG_TYPE_STATIC);
    Graphics_LoadTilesToBgLayer(NARC_INDEX_GRAPHIC__POKETCH, unused_apps_NCGR, graphics->bgConfig, BG_LAYER_SUB_2, 0, 0, FALSE, HEAP_ID_POKETCH_APP);
    Bg_FillTilemapRect(graphics->bgConfig, BG_LAYER_SUB_2, 0, 0, 0, POKETCH_WIDTH_TILES, POKETCH_HEIGHT_TILES, 0);
    Graphics_LoadPalette(NARC_INDEX_GRAPHIC__POKETCH, unused_apps_NCLR, PAL_LOAD_SUB_BG, 0, 0x20, HEAP_ID_POKETCH_APP);

    PoketchGraphics_LoadActivePalette(13, 0);
    Window_Add(graphics->bgConfig, &graphics->window, BG_LAYER_SUB_2, 2, 2, 20, 16, 13, 32);
    graphics->refreshCounter = 0;
    PoketchUnusedApp4Graphics_Update(graphics);
    Bg_CopyTilemapBufferToVRAM(graphics->bgConfig, BG_LAYER_SUB_2);

    GXSDispCnt dispCnt = GXS_GetDispCnt();
    GXS_SetVisiblePlane(dispCnt.visiblePlane | GX_PLANEMASK_BG2);

    EndTask(taskMan);
}

static void Task_FreeBackground(SysTask *task, void *taskMan)
{
    UnusedApp4Graphics *graphics = PoketchTask_GetTaskData(taskMan);

    Window_Remove(&graphics->window);
    Bg_FreeTilemapBuffer(graphics->bgConfig, BG_LAYER_SUB_2);
    EndTask(taskMan);
}

void PoketchUnusedApp4Graphics_Update(UnusedApp4Graphics *graphics)
{
    if (graphics->refreshCounter++ % 30) return;
    Window_FillTilemap(&graphics->window, 0);
    u8 badges = TrainerInfo_BadgeCount(SaveData_GetTrainerInfo(SaveData_Ptr()));
    const RunConfig *config = RunSession_Config();
    static const u16 labels[] = { Run_Text_Tracker, Run_Text_Area, Run_Text_Remaining,
        Run_Text_Cap, Run_Text_Badges, Run_Text_CurrentBonus, Run_Text_Active };
    for (u8 row = 0; row < 7; row++) {
        MessageLoader_GetString(graphics->messages, labels[row], graphics->line);
        if (row == 1) MessageLoader_GetString(graphics->locations, RunSession_Zone(), graphics->line);
        else if (row == 6) MessageLoader_GetString(graphics->messages,
            RunState_Status(RunSession_State()) == RUN_STATUS_GAME_OVER ? Run_Text_GameOver : Run_Text_Active, graphics->line);
        else if (row >= 2) {
            u32 number = row == 2 ? RunState_Remaining(RunSession_State(), RunSession_Zone())
                : row == 3 ? RunWorld_LevelCap(config, badges) : row == 4 ? badges
                : (config->flags & RUN_TRAINER_SCALING) ? badges * config->trainerLevelsPerBadge : 0;
            // Label and value occupy different halves; long labels are cropped
            // rather than drawing across the native Poketch frame.
            while (graphics->line->size && Font_CalcStringWidth(FONT_SYSTEM, graphics->line, 0) > 132)
                graphics->line->data[--graphics->line->size] = CHAR_EOS;
            if (row == 2 && !config->captureAttempts) MessageLoader_GetString(graphics->messages, Run_Text_Off, graphics->value);
            else String_FormatU64(graphics->value, number, 3, PADDING_MODE_NONE, CHARSET_MODE_EN);
            Text_AddPrinterWithParamsAndColor(&graphics->window, FONT_SYSTEM, graphics->value,
                140, row * 18, TEXT_SPEED_NO_TRANSFER, TEXT_COLOR(1, 2, 0), NULL);
        }
        while (graphics->line->size && Font_CalcStringWidth(FONT_SYSTEM, graphics->line, 0) > 158)
            graphics->line->data[--graphics->line->size] = CHAR_EOS;
        Text_AddPrinterWithParamsAndColor(&graphics->window, FONT_SYSTEM, graphics->line,
            0, row * 18, TEXT_SPEED_NO_TRANSFER, TEXT_COLOR(1, 2, 0), NULL);
    }
    Window_CopyToVRAM(&graphics->window);
}
