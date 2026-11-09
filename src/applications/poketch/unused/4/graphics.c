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
#include "run_game.h"
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
    PoketchGraphics_LoadActivePalette(0, 0);

    PoketchGraphics_LoadActivePalette(13, 0);
    Window_Add(graphics->bgConfig, &graphics->window, BG_LAYER_SUB_2, 2, 2, 24, 20, 13, 32);
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

// LCD panels use the active native palette, not a white message rectangle.
static void TrackerText(UnusedApp4Graphics *graphics, u16 id, int x, int y, BOOL centered)
{
    MessageLoader_GetString(graphics->messages, id, graphics->line);
    while (graphics->line->size && Font_CalcStringWidth(FONT_SYSTEM, graphics->line, 0) > 184)
        graphics->line->data[--graphics->line->size] = CHAR_EOS;
    if (centered) x = (192 - Font_CalcStringWidth(FONT_SYSTEM, graphics->line, 0)) / 2;
    Text_AddPrinterWithParamsAndColor(&graphics->window, FONT_SYSTEM, graphics->line,
        x, y, TEXT_SPEED_NO_TRANSFER, TEXT_COLOR(1, 8, 4), NULL);
}

static void TrackerNumber(UnusedApp4Graphics *graphics, u32 value, int x, int y)
{
    String_FormatU64(graphics->value, value, 3, PADDING_MODE_NONE, CHARSET_MODE_EN);
    int width = Font_CalcStringWidth(FONT_SYSTEM, graphics->value, 0);
    Text_AddPrinterWithParamsAndColor(&graphics->window, FONT_SYSTEM, graphics->value,
        x - width, y, TEXT_SPEED_NO_TRANSFER, TEXT_COLOR(1, 8, 4), NULL);
}

static void TrackerDigit(Window *window, u8 digit, int x, int y)
{
    static const u8 segments[] = { 0x3F, 0x06, 0x5B, 0x4F };
    static const u8 bars[7][4] = {
        { 3, 0, 10, 3 }, { 13, 3, 3, 8 }, { 13, 13, 3, 8 },
        { 3, 21, 10, 3 }, { 0, 13, 3, 8 }, { 0, 3, 3, 8 }, { 3, 10, 10, 3 }
    };
    for (u8 bar = 0; bar < 7; bar++) if (segments[digit <= 3 ? digit : 0] & (1 << bar))
        Window_FillRectWithColor(window, 1, x + bars[bar][0], y + bars[bar][1], bars[bar][2], bars[bar][3]);
}

void PoketchUnusedApp4Graphics_Update(UnusedApp4Graphics *graphics)
{
    if (graphics->refreshCounter++ % 15) return;
    const RunConfig *config = RunSession_Config();
    u16 zone = RunSession_Zone();
    u8 badges = TrainerInfo_BadgeCount(SaveData_GetTrainerInfo(SaveData_Ptr()));
    u8 remaining = RunState_Remaining(RunSession_State(), zone);
    u8 eligible = RunGame_EligibleCount(SaveData_GetParty(SaveData_Ptr()));
    u16 stateText = !RunSession_Configured() ? Run_Text_TrackerInactive
        : !config->captureAttempts ? Run_Text_TrackerOff
        : !RunState_CaptureActivated(RunSession_State()) ? Run_Text_TrackerInactive
        : !RunWorld_ZoneHasWild(zone) ? Run_Text_TrackerNoWild
        : remaining ? Run_Text_TrackerAvailable : Run_Text_TrackerExhausted;
    u32 state = remaining | (badges << 3) | (eligible << 7)
        | ((u32)stateText << 10) | ((u32)RunGame_Over() << 30);
    if (graphics->displayReady && graphics->lastZone == zone && graphics->lastState == state) return;
    graphics->displayReady = TRUE;
    graphics->lastZone = zone;
    graphics->lastState = state;
    Window *window = &graphics->window;
    Window_FillTilemap(window, 4);
    // Thin LCD rules and stepped corner marks echo native Poketch tools.
    for (int y = 20; y <= 104; y += 84) Window_FillRectWithColor(window, 1, 4, y, 184, 1);
    Window_FillRectWithColor(window, 1, 4, 138, 184, 1);
    Window_FillRectWithColor(window, 1, 0, 0, 3, 160);
    Window_FillRectWithColor(window, 1, 189, 0, 3, 160);
    TrackerText(graphics, Run_Text_TrackerTitle, 0, 1, TRUE);
    MessageLoader_GetString(graphics->locations, zone, graphics->line);
    while (graphics->line->size && Font_CalcStringWidth(FONT_SYSTEM, graphics->line, 0) > 184)
        graphics->line->data[--graphics->line->size] = CHAR_EOS;
    Text_AddPrinterWithParamsAndColor(window, FONT_SYSTEM, graphics->line,
        (192 - Font_CalcStringWidth(FONT_SYSTEM, graphics->line, 0)) / 2, 23,
        TEXT_SPEED_NO_TRANSFER, TEXT_COLOR(1, 8, 4), NULL);
    TrackerText(graphics, Run_Text_TrackerCaptures, 0, 43, TRUE);
    // Three native-looking ball opportunity icons: hollow when spent, filled
    // while available. OFF, inactive and non-wild areas never show fake slots.
    if (stateText == Run_Text_TrackerAvailable || stateText == Run_Text_TrackerExhausted) {
        for (int i = 0; i < config->captureAttempts; i++) {
            int x = 10 + i * 14;
            Window_FillRectWithColor(window, 1, x + 2, 62, 10, 12);
            Window_FillRectWithColor(window, i < remaining ? 8 : 4, x + 3, 63, 8, 10);
            for (u8 corner = 0; corner < 4; corner++)
                Window_FillRectWithColor(window, 4, x + 2 + (corner & 1) * 8, 62 + (corner >> 1) * 10, 2, 2);
            Window_FillRectWithColor(window, 1, x + 2, 67, 10, 2);
            Window_FillRectWithColor(window, 4, x + 6, 67, 2, 2);
        }
        TrackerDigit(window, remaining, 96, 60);
        TrackerText(graphics, Run_Text_TrackerSlash, 130, 65, FALSE);
        TrackerDigit(window, config->captureAttempts, 150, 60);
    }
    BOOL ordinary = stateText == Run_Text_TrackerAvailable || stateText == Run_Text_TrackerExhausted;
    TrackerText(graphics, stateText, 0, ordinary ? 84 : 65, TRUE);
    if (!ordinary) TrackerText(graphics, Run_Text_TrackerDetails, 0, 84, TRUE);
    // Compact two-column lower panel; all labels measured against 120px.
    TrackerText(graphics, Run_Text_TrackerCap, 8, 106, FALSE);
    if (config->flags & RUN_LEVEL_CAPS) TrackerNumber(graphics, RunWorld_LevelCap(config, badges), 183, 106);
    else TrackerText(graphics, Run_Text_Off, 128, 106, FALSE);
    TrackerText(graphics, Run_Text_TrackerBadges, 8, 122, FALSE);
    TrackerNumber(graphics, badges, 85, 122);
    TrackerText(graphics, Run_Text_TrackerBonus, 109, 122, FALSE);
    TrackerNumber(graphics, (config->flags & RUN_TRAINER_SCALING) ? badges * config->trainerLevelsPerBadge : 0, 183, 122);
    TrackerText(graphics, RunGame_Over() ? Run_Text_GameOver
        : RunConfig_ObedienceCapEnabled(config) && eligible == 1 ? Run_Text_TrackerOneReady : Run_Text_Active, 0, 142, TRUE);
    if (RunConfig_ObedienceCapEnabled(config) && eligible == 1) {
        Window_FillRectWithColor(window, 1, 7, 145, 3, 8);
        Window_FillRectWithColor(window, 1, 7, 155, 3, 2);
    }
    Window_CopyToVRAM(window);
}
