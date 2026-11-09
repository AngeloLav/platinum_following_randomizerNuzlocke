#include "applications/options_menu.h"

#include <nitro.h>
#include <stddef.h>
#include <string.h>

#include "constants/heap.h"
#include "constants/narc.h"
#include "constants/string.h"

#include "bg_window.h"
#include "font.h"
#include "game_options.h"
#include "graphics.h"
#include "gx_layers.h"
#include "heap.h"
#include "menu.h"
#include "message.h"
#include "narc.h"
#include "network_icon.h"
#include "overlay_manager.h"
#include "palette.h"
#include "render_text.h"
#include "render_window.h"
#include "screen_fade.h"
#include "sound.h"
#include "sound_playback.h"
#include "sprite_system.h"
#include "string_gf.h"
#include "system.h"
#include "text.h"
#include "vram_transfer.h"
#include "constants/charcode.h"
#include "main.h"
#include "math_util.h"
#include "run_menu.h"
#include "run_game.h"
#include "run_session.h"
#include "run_randomizer.h"
#include "generated/trainers.h"
#include "run_world.h"
#include "save_player.h"
#include "trainer_info.h"
#include "applications/title_screen.h"
#include "game_overlay.h"
#include "game_start.h"
#include "res/text/bank/run_engine.h"

#include "res/graphics/options_menu/config_gra.naix"
#include "res/text/bank/options_menu.h"

FS_EXTERN_OVERLAY(game_opening);

#define MENU_TITLE_BASE_TILE      10
#define MENU_TITLE_WIDTH          12
#define MENU_TITLE_HEIGHT         2
#define ENTRIES_BASE_TILE         (MENU_TITLE_BASE_TILE + MENU_TITLE_WIDTH * MENU_TITLE_HEIGHT)
#define ENTRIES_WIDTH             30
#define ENTRIES_HEIGHT            14
#define DESCRIPTION_BASE_TILE     (ENTRIES_BASE_TILE + ENTRIES_WIDTH * ENTRIES_HEIGHT)
#define DESCRIPTION_WIDTH         27
#define DESCRIPTION_HEIGHT        4
#define STANDARD_WINDOW_BASE_TILE (DESCRIPTION_BASE_TILE + DESCRIPTION_WIDTH * DESCRIPTION_HEIGHT)
#define MESSAGE_BOX_BASE_TILE     (STANDARD_WINDOW_BASE_TILE + 9)
#define CONFIRMATION_BASE_TILE    (MESSAGE_BOX_BASE_TILE + 30)

#define CHOICES_X     (12 * 8 + 4)
#define CHOICES_WIDTH (48 * 8)

#define SINGLE_ENTRY_HEIGHT 16
#define FIRST_ENTRY_OFFSET  24

#define NUM_WINDOWS      3
#define NUM_BG_TEMPLATES 5

#define HEAP_ALLOCATION_SIZE 0x10000

enum OptionsMenuEntryID {
    ENTRY_TEXT_SPEED = 0,
    ENTRY_SOUND_MODE,
    ENTRY_BATTLE_SCENE,
    ENTRY_BATTLE_STYLE,
    ENTRY_BUTTON_MODE,
    ENTRY_MESSAGE_BOX_FRAME,
    ENTRY_CLOSE,

    MAX_ENTRIES,
};

typedef struct OptionsMenuEntry {
    u16 numChoices;
    u16 selected;
    String *choices[20];
} OptionsMenuEntry;

typedef struct OptionsMenuData {
    enum HeapID heapID;
    int state;
    int subState;
    int dummy0C;
    u32 saveSelections : 2;
    u32 cursor : 3;
    u32 dummy10_5 : 16;
    u32 redrawMessageBox : 1;
    u32 dummy10_22 : 10;
    BgConfig *bgConfig;
    OptionsMenu options;
    Options *saveOptions;
    MessageLoader *msgLoader;
    void *nscrBuffer;
    NNSG2dScreenData *tilemapData;

    union {
        Window asArray[NUM_WINDOWS];
        struct {
            Window title;
            Window entries;
            Window description;
        };
    } windows;

    union {
        OptionsMenuEntry asArray[MAX_ENTRIES];
        struct {
            OptionsMenuEntry textSpeed;
            OptionsMenuEntry soundMode;
            OptionsMenuEntry battleScene;
            OptionsMenuEntry battleStyle;
            OptionsMenuEntry buttonMode;
            OptionsMenuEntry messageBoxStyle;
            OptionsMenuEntry close;
        };
    } entries;

    Menu *yesNoChoice;
    u32 textPrinter;
    void *dummy2B0;
    void *dummy2B4;
    void *dummy2B8;
} OptionsMenuData;

static void OptionsMenuVBlank(void *data);

static int SetupMenuVisuals(OptionsMenuData *menuData);
static void SetVRAMBanks(void);
static void SetupBgs(OptionsMenuData *menuData);
static void LoadBgTiles(OptionsMenuData *menuData);
static void SetupWindows(OptionsMenuData *menuData);

static void PrintTitleAndEntries(OptionsMenuData *param0);
static void PrintEntryChoices(OptionsMenuData *menuData, u16 entry);
static void PrintEntryDescription(OptionsMenuData *menuData, u16 entry, BOOL scheduleVRAMCopy);
static void PrintBankEntryAsDescription(OptionsMenuData *menuData, u16 entry, BOOL scheduleVRAMCopy);
static BOOL IsTextPrinterDone(const OptionsMenuData *menuData);
static void LoadAllEntryChoices(OptionsMenuData *menuData);

static BOOL ChangesWereMade(OptionsMenuData *menuData);
static void DrawConfirmationPrompt(OptionsMenuData *menuData);

static void ProcessMainInput(OptionsMenuData *menuData);
static u32 ProcessConfirmationInput(OptionsMenuData *menuData);

static int TeardownMenuData(OptionsMenuData *menuData);
static void TeardownBgs(OptionsMenuData *menuData);
static void TeardownTilemaps(OptionsMenuData *menuData);
static void TeardownWindows(OptionsMenuData *menuData);

BOOL OptionsMenu_Init(ApplicationManager *appMan, int *state)
{
    OptionsMenuData *menuData = NULL;
    Options *options = ApplicationManager_Args(appMan);

    Heap_Create(HEAP_ID_APPLICATION, HEAP_ID_OPTIONS_MENU, HEAP_ALLOCATION_SIZE);

    menuData = ApplicationManager_NewData(appMan, sizeof(OptionsMenuData), HEAP_ID_OPTIONS_MENU);
    memset(menuData, 0, sizeof(OptionsMenuData));

    menuData->options.textSpeed = Options_TextSpeed(options);
    menuData->options.battleScene = Options_BattleScene(options);
    menuData->options.battleStyle = Options_BattleStyle(options);
    menuData->options.soundMode = Options_SoundMode(options);
    menuData->options.buttonMode = Options_ButtonMode(options);
    menuData->options.messageBoxStyle = Options_Frame(options);
    menuData->heapID = HEAP_ID_OPTIONS_MENU;
    menuData->saveOptions = options;

    RenderControlFlags_SetCanABSpeedUpPrint(FALSE);

    return TRUE;
}

BOOL OptionsMenu_Exit(ApplicationManager *appMan, int *state)
{
    OptionsMenuData *menuData = ApplicationManager_Data(appMan);

    if (menuData->saveSelections == 1) {
        menuData->options.textSpeed = menuData->entries.textSpeed.selected;
        menuData->options.battleScene = menuData->entries.battleScene.selected;
        menuData->options.battleStyle = menuData->entries.battleStyle.selected;
        menuData->options.soundMode = menuData->entries.soundMode.selected;
        menuData->options.buttonMode = menuData->entries.buttonMode.selected;
        menuData->options.messageBoxStyle = menuData->entries.messageBoxStyle.selected;
    }

    Options_SetTextSpeed(menuData->saveOptions, menuData->options.textSpeed);
    Options_SetBattleScene(menuData->saveOptions, menuData->options.battleScene);
    Options_SetBattleStyle(menuData->saveOptions, menuData->options.battleStyle);
    Options_SetSoundMode(menuData->saveOptions, menuData->options.soundMode);
    Options_SetButtonMode(menuData->saveOptions, menuData->options.buttonMode);
    Options_SetFrame(menuData->saveOptions, menuData->options.messageBoxStyle);
    Sound_SetPlaybackMode(menuData->options.soundMode);
    Options_SetSystemButtonMode(NULL, menuData->options.buttonMode);

    RenderControlFlags_SetCanABSpeedUpPrint(TRUE);

    ApplicationManager_FreeData(appMan);
    Heap_Destroy(menuData->heapID);

    return TRUE;
}

enum OptonsMenuState {
    STATE_SETUP_MENU_VISUALS = 0,
    STATE_WAIT_FOR_FADE_IN,
    STATE_HANDLE_INPUT,
    STATE_CONFIRM_NEW_SETTINGS,
    STATE_WAIT_CONFIRM_NEW_SETTINGS,
    STATE_HANDLE_YES_NO_INPUT,
    STATE_START_VISUAL_TEARDOWN,
    STATE_WAIT_FOR_FADE_OUT,
    STATE_TEARDOWN,
};

BOOL OptionsMenu_Main(ApplicationManager *appMan, int *state)
{
    OptionsMenuData *menuData = ApplicationManager_Data(appMan);
    u32 choiceYesNo;

    switch (menuData->state) {
    case STATE_SETUP_MENU_VISUALS:
        if (!SetupMenuVisuals(menuData)) {
            return FALSE;
        }

        StartScreenFade(FADE_MAIN_ONLY, FADE_TYPE_BRIGHTNESS_IN, FADE_TYPE_BRIGHTNESS_IN, COLOR_BLACK, 6, 1, menuData->heapID);
        break;

    case STATE_WAIT_FOR_FADE_IN:
        if (!IsScreenFadeDone()) {
            return FALSE;
        }
        break;

    case STATE_HANDLE_INPUT:
        if ((JOY_NEW(PAD_BUTTON_A) && menuData->cursor == ENTRY_CLOSE) || JOY_NEW(PAD_BUTTON_B)) {
            if (ChangesWereMade(menuData) == TRUE) {
                menuData->state = STATE_CONFIRM_NEW_SETTINGS;
            } else {
                Sound_PlayEffect(SE_CONFIRM_sseq_3);
                menuData->saveSelections = 2;
                menuData->state = STATE_START_VISUAL_TEARDOWN;
            }

            return FALSE;
        }

        ProcessMainInput(menuData);
        return FALSE;

    case STATE_CONFIRM_NEW_SETTINGS:
        RenderControlFlags_SetCanABSpeedUpPrint(TRUE);
        PrintBankEntryAsDescription(menuData, OptionsMenu_Text_ConfirmDialog, FALSE);
        break;

    case STATE_WAIT_CONFIRM_NEW_SETTINGS:
        if (IsTextPrinterDone(menuData)) {
            RenderControlFlags_SetCanABSpeedUpPrint(FALSE);
            DrawConfirmationPrompt(menuData);
            menuData->state = STATE_HANDLE_YES_NO_INPUT;
        }

        return FALSE;

    case STATE_HANDLE_YES_NO_INPUT:
        choiceYesNo = ProcessConfirmationInput(menuData);

        if (choiceYesNo != MENU_NOTHING_CHOSEN) {
            if (choiceYesNo == 0) {
                Sound_StopEffect(SE_CONFIRM_sseq_3, 0);
                Sound_PlayEffect(SEQ_SE_DP_SAVE_sseq);
                menuData->saveSelections = 1;
            } else {
                menuData->saveSelections = 2;
            }

            menuData->state = STATE_START_VISUAL_TEARDOWN;
        }

        return FALSE;

    case STATE_START_VISUAL_TEARDOWN:
        if (Text_IsPrinterActive(menuData->textPrinter)) {
            Text_RemovePrinter(menuData->textPrinter);
        }

        StartScreenFade(FADE_MAIN_ONLY, FADE_TYPE_BRIGHTNESS_OUT, FADE_TYPE_BRIGHTNESS_OUT, COLOR_BLACK, 6, 1, menuData->heapID);
        break;

    case STATE_WAIT_FOR_FADE_OUT:
        if (!IsScreenFadeDone()) {
            return FALSE;
        }
        break;

    case STATE_TEARDOWN:
        if (!TeardownMenuData(menuData)) {
            return FALSE;
        }

        return TRUE;
    }

    menuData->state++;
    return FALSE;
}

static void SetVRAMBanks()
{
    GXBanks banks = {
        GX_VRAM_BG_128_A,
        GX_VRAM_BGEXTPLTT_NONE,
        GX_VRAM_SUB_BG_128_C,
        GX_VRAM_SUB_BGEXTPLTT_NONE,
        GX_VRAM_OBJ_16_G,
        GX_VRAM_OBJEXTPLTT_NONE,
        GX_VRAM_SUB_OBJ_16_I,
        GX_VRAM_SUB_OBJEXTPLTT_NONE,
        GX_VRAM_TEX_NONE,
        GX_VRAM_TEXPLTT_NONE,
    };

    GXLayers_SetBanks(&banks);
}

static void OptionsMenuVBlank(void *data)
{
    OptionsMenuData *menuData = data;

    if (menuData->redrawMessageBox) {
        LoadMessageBoxGraphics(menuData->bgConfig,
            BG_LAYER_MAIN_1,
            MESSAGE_BOX_BASE_TILE,
            15,
            menuData->entries.messageBoxStyle.selected,
            menuData->heapID);
        menuData->redrawMessageBox = FALSE;
    }

    SpriteSystem_TransferOam();
    NNS_GfdDoVramTransfer();
    Bg_RunScheduledUpdates(menuData->bgConfig);
    OS_SetIrqCheckFlag(OS_IE_V_BLANK);
}

static int SetupMenuVisuals(OptionsMenuData *menuData)
{
    switch (menuData->subState) {
    case 0:
        SetVBlankCallback(NULL, NULL);
        DisableHBlank();
        GXLayers_DisableEngineALayers();
        GXLayers_DisableEngineBLayers();

        GX_SetVisiblePlane(0);
        GXS_SetVisiblePlane(0);

        SetVRAMBanks();
        ResetVisibleHardwareWindows(DS_SCREEN_MAIN);
        ResetVisibleHardwareWindows(DS_SCREEN_SUB);
        SetupBgs(menuData);
        break;

    case 1:
        LoadBgTiles(menuData);
        menuData->msgLoader = MessageLoader_Init(MSG_LOADER_LOAD_ON_DEMAND,
            NARC_INDEX_MSGDATA__PL_MSG,
            TEXT_BANK_OPTIONS_MENU,
            menuData->heapID);
        LoadAllEntryChoices(menuData);
        break;

    case 2:
        SetupWindows(menuData);
        PrintTitleAndEntries(menuData);
        VramTransfer_New(32, menuData->heapID);
        GXLayers_EngineAToggleLayers(GX_PLANEMASK_OBJ, TRUE);
        NetworkIcon_InitIfConnected();
        SetVBlankCallback(OptionsMenuVBlank, menuData);
        menuData->subState = 0;
        return TRUE;
    }

    menuData->subState++;
    return FALSE;
}

static int TeardownMenuData(OptionsMenuData *menuData)
{
    int v0 = 0, v1 = 0;

    switch (menuData->subState) {
    case 0:
        VramTransfer_Free();
        TeardownWindows(menuData);

        for (v0 = 0; v0 < MAX_ENTRIES; v0++) {
            for (v1 = 0; v1 < menuData->entries.asArray[v0].numChoices; v1++) {
                String_Free(menuData->entries.asArray[v0].choices[v1]);
            }
        }

        MessageLoader_Free(menuData->msgLoader);
        TeardownTilemaps(menuData);
        TeardownBgs(menuData);
        break;

    case 1:
        SetVBlankCallback(NULL, NULL);
        DisableHBlank();
        GXLayers_DisableEngineALayers();
        GXLayers_DisableEngineBLayers();

        GX_SetVisiblePlane(0);
        GXS_SetVisiblePlane(0);
        menuData->subState = 0;
        return TRUE;
    }

    menuData->subState++;
    return FALSE;
}

static void SetupBgs(OptionsMenuData *menuData)
{
    menuData->bgConfig = BgConfig_New(menuData->heapID);

    GraphicsModes graphicsModes = {
        GX_DISPMODE_GRAPHICS,
        GX_BGMODE_0,
        GX_BGMODE_0,
        GX_BG0_AS_2D
    };

    SetAllGraphicsModes(&graphicsModes);

    BgTemplate bgTemplates[NUM_BG_TEMPLATES] = {
        {
            .x = 0,
            .y = 0,
            .bufferSize = 0x800,
            .baseTile = 0,
            .screenSize = BG_SCREEN_SIZE_256x256,
            .colorMode = GX_BG_COLORMODE_16,
            .screenBase = GX_BG_SCRBASE_0xf800,
            .charBase = GX_BG_CHARBASE_0x00000,
            .bgExtPltt = GX_BG_EXTPLTT_01,
            .priority = 1,
            .areaOver = 0,
            .mosaic = FALSE,
        },
        {
            .x = 0,
            .y = 0,
            .bufferSize = 0x800,
            .baseTile = 0,
            .screenSize = BG_SCREEN_SIZE_256x256,
            .colorMode = GX_BG_COLORMODE_16,
            .screenBase = GX_BG_SCRBASE_0xf000,
            .charBase = GX_BG_CHARBASE_0x00000,
            .bgExtPltt = GX_BG_EXTPLTT_01,
            .priority = 2,
            .areaOver = 0,
            .mosaic = FALSE,
        },
        {
            .x = 0,
            .y = 0,
            .bufferSize = 0x800,
            .baseTile = 0,
            .screenSize = BG_SCREEN_SIZE_256x256,
            .colorMode = GX_BG_COLORMODE_16,
            .screenBase = GX_BG_SCRBASE_0xe800,
            .charBase = GX_BG_CHARBASE_0x00000,
            .bgExtPltt = GX_BG_EXTPLTT_01,
            .priority = 3,
            .areaOver = 0,
            .mosaic = FALSE,
        },
        {
            .x = 0,
            .y = 0,
            .bufferSize = 0x800,
            .baseTile = 0,
            .screenSize = BG_SCREEN_SIZE_256x256,
            .colorMode = GX_BG_COLORMODE_16,
            .screenBase = GX_BG_SCRBASE_0xe000,
            .charBase = GX_BG_CHARBASE_0x00000,
            .bgExtPltt = GX_BG_EXTPLTT_01,
            .priority = 0,
            .areaOver = 0,
            .mosaic = FALSE,
        },
        {
            .x = 0,
            .y = 0,
            .bufferSize = 0x800,
            .baseTile = 0,
            .screenSize = BG_SCREEN_SIZE_256x256,
            .colorMode = GX_BG_COLORMODE_16,
            .screenBase = GX_BG_SCRBASE_0xf800,
            .charBase = GX_BG_CHARBASE_0x00000,
            .bgExtPltt = GX_BG_EXTPLTT_01,
            .priority = 0,
            .areaOver = 0,
            .mosaic = FALSE,
        },
    };

    for (int i = 0; i < NUM_BG_TEMPLATES; i++) {
        static const int sBgLayers[NUM_BG_TEMPLATES] = {
            // Must declared inline to match.
            BG_LAYER_MAIN_0,
            BG_LAYER_MAIN_1,
            BG_LAYER_MAIN_2,
            BG_LAYER_MAIN_3,
            BG_LAYER_SUB_0,
        };

        Bg_InitFromTemplate(menuData->bgConfig, sBgLayers[i], &(bgTemplates[i]), BG_TYPE_STATIC);
        Bg_ClearTilemap(menuData->bgConfig, sBgLayers[i]);
    }

    Bg_ClearTilesRange(BG_LAYER_MAIN_0, 32, 0, menuData->heapID);
    Bg_ClearTilesRange(BG_LAYER_SUB_0, 32, 0, menuData->heapID);
}

static void TeardownBgs(OptionsMenuData *menuData)
{
    Bg_FreeTilemapBuffer(menuData->bgConfig, BG_LAYER_SUB_0);
    Bg_FreeTilemapBuffer(menuData->bgConfig, BG_LAYER_MAIN_3);
    Bg_FreeTilemapBuffer(menuData->bgConfig, BG_LAYER_MAIN_2);
    Bg_FreeTilemapBuffer(menuData->bgConfig, BG_LAYER_MAIN_1);
    Bg_FreeTilemapBuffer(menuData->bgConfig, BG_LAYER_MAIN_0);
    Heap_Free(menuData->bgConfig);
}

static void LoadBgTiles(OptionsMenuData *menuData)
{
    NNSG2dCharacterData *cursorTiles;
    NNSG2dPaletteData *cursorPalette;

    NARC *narc = NARC_ctor(NARC_INDEX_GRAPHIC__CONFIG_GRA, menuData->heapID);

    u32 memberSize = NARC_GetMemberSize(narc, tiles_NCGR);
    void *memberBuffer = Heap_AllocAtEnd(menuData->heapID, memberSize);
    NARC_ReadWholeMember(narc, tiles_NCGR, memberBuffer);
    NNS_G2dGetUnpackedCharacterData(memberBuffer, &cursorTiles);
    Bg_LoadTiles(menuData->bgConfig, BG_LAYER_MAIN_0, cursorTiles->pRawData, cursorTiles->szByte, 0);
    Bg_LoadTiles(menuData->bgConfig, BG_LAYER_SUB_0, cursorTiles->pRawData, cursorTiles->szByte, 0);
    Heap_Free(memberBuffer);

    memberSize = NARC_GetMemberSize(narc, tiles_NCLR);
    memberBuffer = Heap_AllocAtEnd(menuData->heapID, memberSize);
    NARC_ReadWholeMember(narc, tiles_NCLR, memberBuffer);
    NNS_G2dGetUnpackedPaletteData(memberBuffer, &cursorPalette);
    Bg_LoadPalette(BG_LAYER_MAIN_0, cursorPalette->pRawData, PALETTE_SIZE_BYTES, 0);
    Bg_LoadPalette(BG_LAYER_SUB_0, cursorPalette->pRawData, PALETTE_SIZE_BYTES, 0);
    Heap_Free(memberBuffer);

    memberSize = NARC_GetMemberSize(narc, tilemap_bin);
    menuData->nscrBuffer = Heap_Alloc(menuData->heapID, memberSize);
    NARC_ReadWholeMember(narc, tilemap_bin, menuData->nscrBuffer);
    NNS_G2dGetUnpackedScreenData(menuData->nscrBuffer, &(menuData->tilemapData));

    NARC_dtor(narc);

    Bg_FillTilemapRect(menuData->bgConfig,
        BG_LAYER_MAIN_2,
        1,
        0,
        0,
        32,
        32,
        TILEMAP_FILL_VAL_INCLUDES_PALETTE);
    Bg_FillTilemapRect(menuData->bgConfig,
        BG_LAYER_SUB_0,
        1,
        0,
        0,
        32,
        32,
        TILEMAP_FILL_VAL_INCLUDES_PALETTE);

    Bg_CopyRectToTilemapRect(menuData->bgConfig,
        BG_LAYER_MAIN_0,
        0,
        0,
        32,
        2,
        menuData->tilemapData->rawData,
        0,
        0,
        menuData->tilemapData->screenWidth / 8,
        menuData->tilemapData->screenHeight / 8);

    Bg_SetOffset(menuData->bgConfig, BG_LAYER_MAIN_0, BG_OFFSET_UPDATE_SET_Y, -FIRST_ENTRY_OFFSET);
    Bg_ScheduleTilemapTransfer(menuData->bgConfig, BG_LAYER_MAIN_2);
    Bg_ScheduleTilemapTransfer(menuData->bgConfig, BG_LAYER_MAIN_0);
    Bg_ScheduleTilemapTransfer(menuData->bgConfig, BG_LAYER_SUB_0);
}

static void TeardownTilemaps(OptionsMenuData *menuData)
{
    Heap_Free(menuData->nscrBuffer);
}

static void SetupWindows(OptionsMenuData *menuData)
{
    Window_Add(menuData->bgConfig,
        &menuData->windows.title,
        BG_LAYER_MAIN_1,
        1,
        0,
        MENU_TITLE_WIDTH,
        MENU_TITLE_HEIGHT,
        13,
        MENU_TITLE_BASE_TILE);
    Window_Add(menuData->bgConfig,
        &menuData->windows.entries,
        BG_LAYER_MAIN_1,
        1,
        3,
        ENTRIES_WIDTH,
        ENTRIES_HEIGHT,
        13,
        ENTRIES_BASE_TILE);
    Window_Add(menuData->bgConfig,
        &menuData->windows.description,
        BG_LAYER_MAIN_1,
        2,
        19,
        DESCRIPTION_WIDTH,
        DESCRIPTION_HEIGHT,
        12,
        DESCRIPTION_BASE_TILE);
    LoadStandardWindowGraphics(menuData->bgConfig,
        BG_LAYER_MAIN_1,
        STANDARD_WINDOW_BASE_TILE,
        14,
        STANDARD_WINDOW_SYSTEM,
        menuData->heapID);
    LoadMessageBoxGraphics(menuData->bgConfig,
        BG_LAYER_MAIN_1,
        MESSAGE_BOX_BASE_TILE,
        15,
        menuData->options.messageBoxStyle,
        menuData->heapID);

    Font_LoadTextPalette(PAL_LOAD_MAIN_BG, PLTT_OFFSET(13), menuData->heapID);
    Font_LoadTextPalette(PAL_LOAD_SUB_BG, PLTT_OFFSET(13), menuData->heapID);
    Font_LoadScreenIndicatorsPalette(PAL_LOAD_MAIN_BG, PLTT_OFFSET(12), menuData->heapID);
    Font_LoadScreenIndicatorsPalette(PAL_LOAD_SUB_BG, PLTT_OFFSET(12), menuData->heapID);

    Window_FillTilemap(&menuData->windows.title, PIXEL_FILL(0));
    Window_FillTilemap(&menuData->windows.entries, PIXEL_FILL(15));
    Window_FillTilemap(&menuData->windows.description, PIXEL_FILL(15));
    Window_ClearTilemap(&menuData->windows.description);
    Window_ClearTilemap(&menuData->windows.entries);
    Window_ClearTilemap(&menuData->windows.title);

    Window_DrawStandardFrame(&menuData->windows.entries, TRUE, STANDARD_WINDOW_BASE_TILE, 14);
    Window_DrawMessageBoxWithScrollCursor(&menuData->windows.description, TRUE, MESSAGE_BOX_BASE_TILE, 15);
}

static void TeardownWindows(OptionsMenuData *menuData)
{
    Window_EraseStandardFrame(&menuData->windows.entries, FALSE);
    Window_EraseMessageBox(&menuData->windows.description, FALSE);

    for (u16 i = 0; i < 3; i++) {
        Window_ClearAndCopyToVRAM(&menuData->windows.asArray[i]);
        Window_FillTilemap(&menuData->windows.asArray[i], 0);
        Window_ClearTilemap(&menuData->windows.asArray[i]);
        Window_Remove(&menuData->windows.asArray[i]);
    }
}

static const u8 sEntryLabels[MAX_ENTRIES] = {
    OptionsMenu_Text_TextSpeedLabel,
    OptionsMenu_Text_SoundModeLabel,
    OptionsMenu_Text_BattleSceneLabel,
    OptionsMenu_Text_BattleStyleLabel,
    OptionsMenu_Text_ButtonModeLabel,
    OptionsMenu_Text_MessageBoxStyleLabel,
    OptionsMenu_Text_CloseLabel,
};

static void PrintTitleAndEntries(OptionsMenuData *menuData)
{
    u16 i; // Must forward-declare to match
    TextColor transparentBg = TEXT_COLOR(1, 2, 0);
    TextColor whiteBg = TEXT_COLOR(1, 2, 15);

    String *string = String_Init(256, menuData->heapID);
    MessageLoader_GetString(menuData->msgLoader, OptionsMenu_Text_Title, string);
    Text_AddPrinterWithParamsAndColor(&menuData->windows.title,
        FONT_SYSTEM,
        string,
        2,
        2,
        TEXT_SPEED_INSTANT,
        transparentBg,
        NULL);

    for (i = 0; i < MAX_ENTRIES; i++) {
        String_Clear(string);
        MessageLoader_GetString(menuData->msgLoader, sEntryLabels[i], string);
        Text_AddPrinterWithParamsAndColor(&menuData->windows.entries,
            FONT_SYSTEM,
            string,
            4,
            16 * i,
            TEXT_SPEED_NO_TRANSFER,
            whiteBg,
            NULL);
    }

    for (i = 0; i < MAX_ENTRIES; i++) {
        PrintEntryChoices(menuData, i);
    }

    PrintEntryDescription(menuData, ENTRY_TEXT_SPEED, TRUE);
    Window_CopyToVRAM(&menuData->windows.title);
    Window_CopyToVRAM(&menuData->windows.entries);
    String_Free(string);
}

static const int sNumChoicesPerEntry[MAX_ENTRIES] = {
    3,
    2,
    2,
    2,
    3,
    20,
    0,
};

static const u8 sFirstChoicePerEntry[MAX_ENTRIES] = {
    OptionsMenu_Text_TextSpeedSlow,
    OptionsMenu_Text_SoundModeStereo,
    OptionsMenu_Text_BattleSceneOn,
    OptionsMenu_Text_BattleStyleShift,
    OptionsMenu_Text_ButtonModeNormal,
    OptionsMenu_Text_MessageBoxStyle_01,
    NULL,
};

static void LoadAllEntryChoices(OptionsMenuData *menuData)
{
    u16 i, j;
    for (i = 0; i < MAX_ENTRIES; i++) {
        menuData->entries.asArray[i].numChoices = sNumChoicesPerEntry[i];
        for (j = 0; j < sNumChoicesPerEntry[i]; j++) {
            menuData->entries.asArray[i].choices[j] = MessageLoader_GetNewString(menuData->msgLoader, sFirstChoicePerEntry[i] + j);
        }
    }

    menuData->entries.textSpeed.selected = menuData->options.textSpeed;
    menuData->entries.battleScene.selected = menuData->options.battleScene;
    menuData->entries.battleStyle.selected = menuData->options.battleStyle;
    menuData->entries.soundMode.selected = menuData->options.soundMode;
    menuData->entries.buttonMode.selected = menuData->options.buttonMode;
    menuData->entries.messageBoxStyle.selected = menuData->options.messageBoxStyle;
}

static const s8 sEntryXOffsets[] = { 0, 0, 0, 0, 0, 0, 0 };

static void PrintEntryChoices(OptionsMenuData *menuData, u16 entry)
{
    TextColor darkGray, red, color;
    u16 i;
    u8 textSpeed;
    s8 xOffset = 0;

    darkGray = TEXT_COLOR(1, 2, 15);
    red = TEXT_COLOR(3, 4, 15);

    Window_FillRectWithColor(&menuData->windows.entries, PIXEL_FILL(15), CHOICES_X + sEntryXOffsets[entry], entry * SINGLE_ENTRY_HEIGHT, CHOICES_WIDTH, SINGLE_ENTRY_HEIGHT);
    if (entry == ENTRY_MESSAGE_BOX_FRAME) {
        Text_AddPrinterWithParamsAndColor(&menuData->windows.entries,
            FONT_SYSTEM,
            menuData->entries.asArray[entry].choices[menuData->entries.asArray[entry].selected],
            48 + CHOICES_X,
            entry * SINGLE_ENTRY_HEIGHT,
            TEXT_SPEED_NO_TRANSFER,
            red,
            NULL);
        Window_CopyToVRAM(&menuData->windows.entries);
        menuData->redrawMessageBox = TRUE;
        return;
    }

    if (entry == ENTRY_SOUND_MODE) {
        Sound_SetPlaybackMode(menuData->entries.asArray[entry].selected);
    } else if (entry == ENTRY_BUTTON_MODE) {
        Options_SetSystemButtonMode(NULL, menuData->entries.asArray[entry].selected);
    } else if (entry == ENTRY_TEXT_SPEED) {
        Options_SetTextSpeed(menuData->saveOptions, menuData->entries.asArray[entry].selected);
        PrintEntryDescription(menuData, entry, FALSE);
    }

    xOffset = 0;
    for (i = 0; i < menuData->entries.asArray[entry].numChoices; i++) {
        if (i == menuData->entries.asArray[entry].selected) {
            color = red;
        } else {
            color = darkGray;
        }

        if (i == menuData->entries.asArray[entry].numChoices - 1) {
            textSpeed = TEXT_SPEED_NO_TRANSFER;
        } else {
            textSpeed = TEXT_SPEED_NO_TRANSFER;
        }

        if (entry == ENTRY_BUTTON_MODE) {
            Text_AddPrinterWithParamsAndColor(&menuData->windows.entries,
                FONT_SYSTEM,
                menuData->entries.asArray[entry].choices[i],
                xOffset + CHOICES_X,
                entry * SINGLE_ENTRY_HEIGHT,
                textSpeed,
                color,
                NULL);
            xOffset += Font_CalcStringWidth(FONT_SYSTEM, menuData->entries.asArray[entry].choices[i], 0) + 12;
        } else {
            Text_AddPrinterWithParamsAndColor(&menuData->windows.entries,
                FONT_SYSTEM,
                menuData->entries.asArray[entry].choices[i],
                i * 48 + CHOICES_X + sEntryXOffsets[entry],
                entry * SINGLE_ENTRY_HEIGHT,
                textSpeed,
                color,
                NULL);
        }
    }

    Window_CopyToVRAM(&menuData->windows.asArray[1]);
}

static void PrintBankEntryAsDescription(OptionsMenuData *menuData, u16 entry, BOOL scheduleVRAMCopy)
{
    // Kill any active printer
    if (IsTextPrinterDone(menuData) == FALSE) {
        Text_RemovePrinter(menuData->textPrinter);
    }

    u8 renderDelay = Options_TextFrameDelay(menuData->saveOptions);

    Window_FillTilemap(&menuData->windows.description, 15);

    TextColor color = TEXT_COLOR(1, 2, 15);
    String *string = String_Init(256, menuData->heapID);
    MessageLoader_GetString(menuData->msgLoader, entry, string);

    if (scheduleVRAMCopy == FALSE) {
        menuData->textPrinter = Text_AddPrinterWithParamsAndColor(&menuData->windows.description,
            FONT_MESSAGE,
            string,
            4,
            0,
            renderDelay,
            color,
            NULL);
    } else {
        Text_AddPrinterWithParamsAndColor(&menuData->windows.description,
            FONT_MESSAGE,
            string,
            4,
            0,
            TEXT_SPEED_NO_TRANSFER,
            color,
            NULL);
        Window_ScheduleCopyToVRAM(&menuData->windows.description);
    }

    String_Free(string);
}

static BOOL IsTextPrinterDone(const OptionsMenuData *menuData)
{
    return Text_IsPrinterActive(menuData->textPrinter) == FALSE;
}

static void ProcessMainInput(OptionsMenuData *menuData)
{
    OptionsMenuEntry *entry = &menuData->entries.asArray[menuData->cursor];

    if (menuData->cursor != ENTRY_CLOSE) {
        if (JOY_NEW(PAD_KEY_RIGHT)) {
            entry->selected = (entry->selected + 1) % entry->numChoices;
            PrintEntryChoices(menuData, menuData->cursor);
            Sound_PlayEffect(SE_CONFIRM_sseq_3);
        } else if (JOY_NEW(PAD_KEY_LEFT)) {
            entry->selected = (entry->selected + entry->numChoices - 1) % entry->numChoices;
            PrintEntryChoices(menuData, menuData->cursor);
            Sound_PlayEffect(SE_CONFIRM_sseq_3);
        }
    }

    if (JOY_NEW(PAD_KEY_UP)) {
        menuData->cursor = (menuData->cursor + 7 - 1) % 7;
        Bg_ScheduleScroll(menuData->bgConfig,
            BG_LAYER_MAIN_0,
            BG_OFFSET_UPDATE_SET_Y,
            -(menuData->cursor * SINGLE_ENTRY_HEIGHT + FIRST_ENTRY_OFFSET));

        PrintEntryDescription(menuData, menuData->cursor, TRUE);
        Sound_PlayEffect(SE_CONFIRM_sseq_3);
    } else if (JOY_NEW(PAD_KEY_DOWN)) {
        menuData->cursor = (menuData->cursor + 1) % 7;
        Bg_ScheduleScroll(menuData->bgConfig,
            BG_LAYER_MAIN_0,
            BG_OFFSET_UPDATE_SET_Y,
            -(menuData->cursor * SINGLE_ENTRY_HEIGHT + FIRST_ENTRY_OFFSET));

        PrintEntryDescription(menuData, menuData->cursor, TRUE);
        Sound_PlayEffect(SE_CONFIRM_sseq_3);
    }
}

static BOOL ChangesWereMade(OptionsMenuData *menuData)
{
    return menuData->options.textSpeed != menuData->entries.textSpeed.selected
        || menuData->options.battleScene != menuData->entries.battleScene.selected
        || menuData->options.battleStyle != menuData->entries.battleStyle.selected
        || menuData->options.soundMode != menuData->entries.soundMode.selected
        || menuData->options.buttonMode != menuData->entries.buttonMode.selected
        || menuData->options.messageBoxStyle != menuData->entries.messageBoxStyle.selected;
}

static const WindowTemplate sConfirmationWindowTemplate = {
    .bgLayer = BG_LAYER_MAIN_3,
    .tilemapLeft = 25,
    .tilemapTop = 13,
    .width = 6,
    .height = 4,
    .palette = 13,
    .baseTile = CONFIRMATION_BASE_TILE,
};

static void DrawConfirmationPrompt(OptionsMenuData *menuData)
{
    menuData->yesNoChoice = Menu_MakeYesNoChoice(menuData->bgConfig,
        &sConfirmationWindowTemplate,
        STANDARD_WINDOW_BASE_TILE,
        14,
        menuData->heapID);
}

static u32 ProcessConfirmationInput(OptionsMenuData *menuData)
{
    return Menu_ProcessInputAndHandleExit(menuData->yesNoChoice, menuData->heapID);
}

static const u8 sEntryDescriptions[MAX_ENTRIES] = {
    OptionsMenu_Text_TextSpeedDescription,
    OptionsMenu_Text_SoundModeDescription,
    OptionsMenu_Text_BattleSceneDescription,
    OptionsMenu_Text_BattleStyleDescription,
    OptionsMenu_Text_ButtonModeDescription,
    OptionsMenu_Text_MessageBoxStyleDescription,
    OptionsMenu_Text_CloseDescription,
};

static void PrintEntryDescription(OptionsMenuData *menuData, u16 entry, BOOL scheduleVRAMCopy)
{
    PrintBankEntryAsDescription(menuData, sEntryDescriptions[entry], scheduleVRAMCopy);
}

// Run menus deliberately share the retail Options screen's graphics, windows,
// palettes and teardown. The lower-screen help uses one native text window;
// there are no new graphics assets, sprites or persistent graphics heaps.
#define RUN_VISIBLE_ROWS 6
#define RUN_ROW_HEIGHT 18
typedef struct RunMenuOption {
    u16 label;
    u8 page;
    u8 offset;
    u32 flag;
    u8 low;
    u8 high;
    u8 step;
} RunMenuOption;

#define RUN_BOOL(label, page, flag) { label, page, 0, flag, 0, 1, 1 }
#define RUN_NUMBER(label, page, field, low, high, step) { label, page, offsetof(RunConfig, field), 0, low, high, step }
static const RunMenuOption sRunOptions[] = {
    RUN_BOOL(Run_Text_Wild, 1, RUN_RANDOMIZE_WILD),
    RUN_BOOL(Run_Text_Trainers, 1, RUN_RANDOMIZE_TRAINERS),
    RUN_NUMBER(Run_Text_Tolerance, 1, bstTolerance, 10, 75, 5),
    RUN_NUMBER(Run_Text_Weight, 1, bstWeight, 0, 100, 5),
    RUN_BOOL(Run_Text_Legendaries, 1, RUN_LEGENDARY_POOLS),
    RUN_BOOL(Run_Text_Static, 1, RUN_RANDOMIZE_STATICS),
    RUN_BOOL(Run_Text_Gifts, 1, RUN_RANDOMIZE_GIFTS),
    RUN_BOOL(Run_Text_Eggs, 1, RUN_RANDOMIZE_EGGS),
    RUN_BOOL(Run_Text_FieldItems, 1, RUN_RANDOMIZE_FIELD_ITEMS),
    RUN_NUMBER(Run_Text_SeedMode, 1, seedMode, 0, 1, 1),
    RUN_NUMBER(Run_Text_Attempts, 2, captureAttempts, 0, 3, 1),
    RUN_BOOL(Run_Text_Dupes, 2, RUN_DUPES_CLAUSE),
    RUN_BOOL(Run_Text_Shiny, 2, RUN_SHINY_CLAUSE),
    RUN_NUMBER(Run_Text_ShinyRate, 2, shinyRate, 0, 4, 1),
    RUN_BOOL(Run_Text_SeparateGifts, 2, RUN_SEPARATE_GIFTS),
    RUN_BOOL(Run_Text_SeparateEggs, 2, RUN_SEPARATE_EGGS),
    RUN_BOOL(Run_Text_Permadeath, 3, RUN_PERMADEATH),
    RUN_BOOL(Run_Text_DayCare, 3, RUN_DAY_CARE_EGGS),
    RUN_BOOL(Run_Text_Cap, 4, RUN_LEVEL_CAPS),
    RUN_NUMBER(Run_Text_CapMode, 4, levelCapMode, 0, 1, 1),
    RUN_NUMBER(Run_Text_Allowance, 4, levelCapAllowance, 0, 5, 1),
    RUN_BOOL(Run_Text_Scaling, 4, RUN_TRAINER_SCALING),
    RUN_NUMBER(Run_Text_Bonus, 4, trainerLevelsPerBadge, 1, 3, 1),
    RUN_BOOL(Run_Text_Bosses, 4, RUN_SCALE_BOSSES),
    RUN_BOOL(Run_Text_Notifications, 5, RUN_AREA_NOTIFICATIONS),
};
#undef RUN_BOOL
#undef RUN_NUMBER

static const u16 sRunShinyRates[] = { Run_Text_RateOriginal, Run_Text_RateIncreased,
    Run_Text_RateHigh, Run_Text_RateVeryHigh, Run_Text_RateUltraHigh };

typedef struct RunMenuData {
    OptionsMenuData native;
    SaveData *save;
    RunConfig draft;
    MessageLoader *messages;
    MessageLoader *locations;
    MessageLoader *species;
    String *line;
    String *value;
    String *digits;
    BOOL newRun;
    BOOL damaged;
    BOOL confirmed;
    BOOL details;
    BOOL resetPrompt;
    BOOL seedEditing;
    u32 oldSeed;
    u8 seedDigit;
    u8 page;
    u16 cursor;
    u16 scroll;
    u16 detailID;
    u16 listCursor;
    BOOL confirmFailed;
    BOOL specialList;
    u8 specialCategory;
    BOOL reviewing;
    BOOL settingsView;
    BOOL helpReady;
    Window helpWindow;
    u16 pageCursor[5];
    u16 pageScroll[5];
    u16 zoneOrder[RUN_ZONE_COUNT];
    u16 zoneCount;
    u32 summary[7];
    BOOL currentAreaDetails;
    BOOL modalHelp;
    u8 helpPage;
    BOOL helpMore;
    BOOL restarting;
    BOOL restartErased;
    BOOL restartFailed;
    u16 restartWait;
} RunMenuData;

static u8 RunSpecialCategory(const RunSpecialRecord *event)
{
    if (event->source == RUN_SOURCE_GIFT) return 2;
    if (event->source == RUN_SOURCE_EGG) return 3;
    if (event->source == RUN_SOURCE_SHINY) return 4;
    u16 original = event->eventKey & 0xFFFF;
    if (!RunSpecies_IsValid(original)) original = event->species;
    return RunSpecies_Class(original) == RUN_SPECIES_ORDINARY ? 0 : 1;
}

static BOOL RunMythicalRecord(const RunSpecialRecord *event)
{
    return event->source == RUN_SOURCE_STATIC
        && (event->eventKey == RunGame_EventKey(0) || event->eventKey == RunGame_EventKey(1));
}

static u16 RunSpecialCount(u8 category)
{
    // Two virtual entries expose native event availability before the first
    // battle; their actual persistent records replace them, never add copies.
    u16 count = category == 1 ? 2 : 0;
    for (u16 id = 1; id <= RunState_SpecialCount(RunSession_State()); id++) {
        RunSpecialRecord event;
        RunState_GetSpecial(RunSession_State(), id, &event);
        if (!RunMythicalRecord(&event) && RunSpecialCategory(&event) == category) count++;
    }
    return count;
}

static BOOL RunOptionEnabled(const RunMenuData *menu, const RunMenuOption *option)
{
    if ((option->label == Run_Text_Tolerance || option->label == Run_Text_Weight || option->flag == RUN_LEGENDARY_POOLS)
        && !(menu->draft.flags & (RUN_RANDOMIZE_WILD | RUN_RANDOMIZE_TRAINERS | RUN_RANDOMIZE_STATICS | RUN_RANDOMIZE_GIFTS | RUN_RANDOMIZE_EGGS))) return FALSE;
    if ((option->flag & (RUN_DUPES_CLAUSE | RUN_SHINY_CLAUSE | RUN_SEPARATE_GIFTS | RUN_SEPARATE_EGGS))
        && menu->draft.captureAttempts == 0) {
        return FALSE;
    }
    // Party wipe is a mandatory project rule whenever permadeath is enabled.
    // Show the derived value in the summary, never expose a contradictory toggle.
    if (option->flag == RUN_END_ON_PARTY_WIPE) {
        return FALSE;
    }
    if ((option->label == Run_Text_Allowance || option->label == Run_Text_CapMode) && !(menu->draft.flags & RUN_LEVEL_CAPS)) {
        return FALSE;
    }
    if ((option->label == Run_Text_Bonus || option->flag == RUN_SCALE_BOSSES)
        && !(menu->draft.flags & RUN_TRAINER_SCALING)) {
        return FALSE;
    }
    return TRUE;
}

static void RunPrint(Window *window, String *text, int x, int y, BOOL inactive)
{
    Text_AddPrinterWithParamsAndColor(window, FONT_SYSTEM, text, x, y,
        TEXT_SPEED_NO_TRANSFER, inactive ? TEXT_COLOR(3, 4, 15) : TEXT_COLOR(1, 2, 15), NULL);
}

static void RunMessage(RunMenuData *menu, u16 id, String *text)
{
    MessageLoader_GetString(menu->messages, id, text);
}

static void RunSeedString(RunMenuData *menu)
{
    charcode_t chars[11];
    int at = 0;
    for (int i = 0; i < 8; i++) {
        if (menu->seedEditing && i == menu->seedDigit) chars[at++] = menu->digits->data[16];
        chars[at++] = menu->digits->data[(menu->draft.seed >> (28 - 4 * i)) & 15];
        if (menu->seedEditing && i == menu->seedDigit) chars[at++] = menu->digits->data[17];
    }
    chars[at] = CHAR_EOS;
    String_CopyChars(menu->value, chars);
}

static void RunNumber(RunMenuData *menu, u32 number)
{
    String_FormatU64(menu->value, number, 10, PADDING_MODE_NONE, CHARSET_MODE_EN);
}

static void RunRow(RunMenuData *menu, int row, BOOL inactive)
{
    int visible = row - menu->scroll;
    int width = Font_CalcStringWidth(FONT_SYSTEM, menu->value, 0);
    while (menu->line->size && Font_CalcStringWidth(FONT_SYSTEM, menu->line, 0) > 218 - width) {
        menu->line->data[--menu->line->size] = CHAR_EOS;
    }
    RunPrint(&menu->native.windows.entries, menu->line, 12, visible * RUN_ROW_HEIGHT, inactive);
    if (width < 230) {
        RunPrint(&menu->native.windows.entries, menu->value, 234 - width, visible * RUN_ROW_HEIGHT, inactive);
    }
    if (row == menu->cursor) {
        RunMessage(menu, Run_Text_Cursor, menu->line);
        RunPrint(&menu->native.windows.entries, menu->line, 0, visible * RUN_ROW_HEIGHT, FALSE);
    }
}

static void RunOptionValue(RunMenuData *menu, const RunMenuOption *option)
{
    if (option->flag) {
        RunMessage(menu, (menu->draft.flags & option->flag) ? Run_Text_On : Run_Text_Off, menu->value);
    } else {
        u8 value = *((u8 *)&menu->draft + option->offset);
        if (option->label == Run_Text_SeedMode) {
            RunMessage(menu, value ? Run_Text_Manual : Run_Text_Auto, menu->value);
        } else if (option->label == Run_Text_ShinyRate) {
            RunMessage(menu, sRunShinyRates[value], menu->value);
        } else if (option->label == Run_Text_CapMode) {
            RunMessage(menu, value == RUN_CAP_OBEDIENCE ? Run_Text_ObedienceCap : Run_Text_ExpLock, menu->value);
        } else if (option->label == Run_Text_Attempts && value == 0) {
            RunMessage(menu, Run_Text_Off, menu->value);
        } else {
            RunNumber(menu, value);
            if (option->label == Run_Text_Tolerance || option->label == Run_Text_Weight) {
                charcode_t chars[16];
                u32 i = 0;
                if (option->label == Run_Text_Tolerance) { chars[i++] = CHAR_PLUS; chars[i++] = CHAR_SLASH; chars[i++] = CHAR_MINUS; }
                for (u32 j = 0; j < menu->value->size; j++) chars[i++] = menu->value->data[j];
                chars[i++] = CHAR_PERCENT;
                chars[i] = CHAR_EOS;
                String_CopyChars(menu->value, chars);
            }
        }
    }
}

static int RunRecordID(RunMenuData *menu, int index)
{
    if (!RunSession_Configured()) return -1;
    RunState *run = RunSession_State();
    if (menu->page == 1) {
        return index < menu->zoneCount ? menu->zoneOrder[index] : -1;
    } else if (menu->page == 4) {
        if (menu->specialCategory == 1) {
            if (index < 2) return 0xFFFE - index;
            index -= 2;
        }
        for (u16 id = 1; id <= RunState_SpecialCount(run); id++) {
            RunSpecialRecord event;
            RunState_GetSpecial(run, id, &event);
            if (!RunMythicalRecord(&event) && RunSpecialCategory(&event) == menu->specialCategory && index-- == 0) return id;
        }
    } else {
        for (u16 id = 1; id <= RunState_MonCount(run); id++) {
            RunMonRecord mon;
            RunState_GetMon(run, id, &mon);
            if ((menu->page != 3 || (mon.flags & RUN_MON_EXHAUSTED)) && index-- == 0) {
                return id;
            }
        }
    }
    return -1;
}

static int RunRowCount(RunMenuData *menu)
{
    if (menu->newRun) {
        return SNELEMS(sRunOptions) + 3;
    }
    if (menu->settingsView) return SNELEMS(sRunOptions) + 2;
    if (menu->details) return menu->page == 1 ? 8 : 7;
    if (menu->page == 0) {
        return RunState_Status(RunSession_State()) == RUN_STATUS_GAME_OVER ? 33 : 32;
    }
    if (!RunSession_Configured() && menu->page < 5) return 1;
    if (menu->page == 4 && !menu->specialList) return 5;
    int count = 0;
    if (menu->page == 1) {
        count = menu->zoneCount;
    } else if (menu->page == 4) {
        count = RunSpecialCount(menu->specialCategory);
    } else if (menu->page == 3) {
        for (u16 id = 1; id <= RunState_MonCount(RunSession_State()); id++) {
            RunMonRecord mon;
            RunState_GetMon(RunSession_State(), id, &mon);
            count += (mon.flags & RUN_MON_EXHAUSTED) != 0;
        }
    } else count = RunState_MonCount(RunSession_State());
    return count ? count : 1;
}

static u16 RunOutcomeText(u8 outcome)
{
    static const u16 outcomes[] = { Run_Text_Available, Run_Text_EncounterPending,
        Run_Text_Caught, Run_Text_Defeated, Run_Text_Fled, Run_Text_Failed, Run_Text_Interrupted };
    return outcome < NELEMS(outcomes) ? outcomes[outcome] : Run_Text_Failed;
}

static u16 RunAreaStatus(const RunMenuData *menu, u16 zone)
{
    if (RunState_Status(RunSession_State()) == RUN_STATUS_GAME_OVER) return Run_Text_GameOver;
    if (!menu->draft.captureAttempts) return Run_Text_Off;
    if (!RunState_CaptureActivated(RunSession_State())) return Run_Text_InactiveShort;
    if (!RunState_Remaining(RunSession_State(), zone)) return Run_Text_ExhaustedArea;
    return RunState_Remaining(RunSession_State(), zone) == menu->draft.captureAttempts ? Run_Text_Available : Run_Text_Partial;
}

static void RunOverview(RunMenuData *menu, int row)
{
    static const u16 labels[33] = {
        Run_Text_Area, Run_Text_AreaStatus, Run_Text_Remaining, Run_Text_FirstEncounter,
        Run_Text_Result, Run_Text_AttemptDetails, Run_Text_ProgressSection,
        Run_Text_Badges, Run_Text_Cap, Run_Text_NextCap, Run_Text_CurrentBonus,
        Run_Text_Result, Run_Text_RandomSection, Run_Text_Preset, Run_Text_Seed,
        Run_Text_Wild, Run_Text_Trainers, Run_Text_Tolerance, Run_Text_Weight,
        Run_Text_ShinyRate, Run_Text_FieldItems, Run_Text_CaptureSection,
        Run_Text_TotalAreas, Run_Text_AvailableAreas, Run_Text_ExhaustedAreas,
        Run_Text_Caught, Run_Text_Failures, Run_Text_StaticCaught, Run_Text_ShinyCaught,
        Run_Text_EligibleParty, Run_Text_RefusingParty, Run_Text_Settings, Run_Text_Reset
    };
    RunMessage(menu, labels[row], menu->line);
    RunState *run = RunSession_State();
    u16 zone = RunSession_Zone();
    int badges = TrainerInfo_BadgeCount(SaveData_GetTrainerInfo(menu->save));
    if (row == 0) MessageLoader_GetString(menu->locations, zone, menu->value);
    if (row == 1) RunMessage(menu, RunAreaStatus(menu, zone), menu->value);
    if (row == 2) {
        if (!menu->draft.captureAttempts) RunMessage(menu, Run_Text_Off, menu->value);
        else {
            charcode_t ratio[] = { CHAR_0 + RunState_Remaining(run, zone), CHAR_SLASH,
                CHAR_0 + menu->draft.captureAttempts, CHAR_EOS };
            String_CopyChars(menu->value, ratio);
        }
    }
    if (row == 3 || row == 4) {
        RunOpportunity first;
        RunState_GetOpportunity(run, zone, 0, &first);
        if (row == 3 && first.species) MessageLoader_GetString(menu->species, first.species, menu->value);
        else RunMessage(menu, first.species ? RunOutcomeText(first.outcome) : Run_Text_NotEncountered, menu->value);
    }
    if (row == 7) RunNumber(menu, badges);
    if (row == 6) RunMessage(menu, !(menu->draft.flags & RUN_LEVEL_CAPS) ? Run_Text_Off
        : menu->draft.levelCapMode == RUN_CAP_OBEDIENCE ? Run_Text_ObedienceCap : Run_Text_ExpLock, menu->value);
    if (row == 8) RunNumber(menu, RunWorld_LevelCap(&menu->draft, badges));
    if (row == 9) {
        static const u16 targets[] = { TRAINER_LEADER_ROARK, TRAINER_LEADER_GARDENIA,
            TRAINER_LEADER_FANTINA, TRAINER_LEADER_MAYLENE, TRAINER_LEADER_WAKE,
            TRAINER_LEADER_BYRON, TRAINER_LEADER_CANDICE, TRAINER_LEADER_VOLKNER, TRAINER_CHAMPION_CYNTHIA };
        MessageBank_GetStringFromNARC(NARC_INDEX_MSGDATA__PL_MSG, TEXT_BANK_NPC_TRAINER_NAMES,
            targets[badges < 8 ? badges : 8], HEAP_ID_OPTIONS_MENU, menu->value);
    }
    if (row == 10) RunNumber(menu, (menu->draft.flags & RUN_TRAINER_SCALING) ? badges * menu->draft.trainerLevelsPerBadge : 0);
    if (row == 11) RunMessage(menu, RunState_Status(run) == RUN_STATUS_GAME_OVER ? Run_Text_GameOver : Run_Text_Active, menu->value);
    if (row == 13) RunMessage(menu, Run_Text_Classic + RunConfig_EffectivePreset(&menu->draft), menu->value);
    if (row == 14) RunSeedString(menu);
    if (row == 15 || row == 16 || row == 20) RunMessage(menu,
        (menu->draft.flags & (row == 15 ? RUN_RANDOMIZE_WILD : row == 16 ? RUN_RANDOMIZE_TRAINERS : RUN_RANDOMIZE_FIELD_ITEMS)) ? Run_Text_On : Run_Text_Off, menu->value);
    if (row == 17) RunNumber(menu, menu->draft.bstTolerance);
    if (row == 18) RunNumber(menu, menu->draft.bstWeight);
    if (row == 19) RunMessage(menu, sRunShinyRates[menu->draft.shinyRate], menu->value);
    if (row == 12 && menu->draft.rulesVersion == 2) RunMessage(menu, Run_Text_Legacy, menu->value);
    if (row >= 22 && row <= 28) RunNumber(menu, menu->summary[row - 22]);
    if (row == 29) RunNumber(menu, RunGame_EligibleCount(SaveData_GetParty(menu->save)));
    if (row == 30) RunNumber(menu, RunGame_RefusingCount(SaveData_GetParty(menu->save)));
}

static u16 RunOptionHelp(u16 label)
{
    switch (label) {
    case Run_Text_Wild: return Run_Text_HelpWild;
    case Run_Text_Trainers: return Run_Text_HelpTrainers;
    case Run_Text_Tolerance: return Run_Text_HelpRange;
    case Run_Text_Weight: return Run_Text_HelpSimilarity;
    case Run_Text_Legendaries: return Run_Text_HelpLegendary;
    case Run_Text_Static: return Run_Text_HelpStatic;
    case Run_Text_Gifts: return Run_Text_HelpGifts;
    case Run_Text_Eggs: return Run_Text_HelpEggs;
    case Run_Text_FieldItems: return Run_Text_HelpFieldItems;
    case Run_Text_SeedMode: return Run_Text_HelpSeedMode;
    case Run_Text_Attempts: return Run_Text_HelpAttempts;
    case Run_Text_Dupes: return Run_Text_HelpDupes;
    case Run_Text_Shiny: return Run_Text_HelpShinyClause;
    case Run_Text_ShinyRate: return Run_Text_HelpShinyRate;
    case Run_Text_SeparateGifts: return Run_Text_HelpSeparateGifts;
    case Run_Text_SeparateEggs: return Run_Text_HelpSeparateEggs;
    case Run_Text_Permadeath: return Run_Text_HelpPermadeath;
    case Run_Text_Wipe: return Run_Text_HelpWipe;
    case Run_Text_DayCare: return Run_Text_HelpDayCare;
    case Run_Text_Cap: return Run_Text_HelpCap;
    case Run_Text_CapMode: return Run_Text_HelpCapMode;
    case Run_Text_Allowance: return Run_Text_HelpAllowance;
    case Run_Text_Scaling: return Run_Text_HelpScaling;
    case Run_Text_Bonus: return Run_Text_HelpBonus;
    case Run_Text_Bosses: return Run_Text_HelpBosses;
    case Run_Text_Notifications: return Run_Text_HelpNotifications;
    }
    return Run_Text_HelpSettings;
}

static u16 RunContextHelp(RunMenuData *menu)
{
    if (menu->page == 4 && menu->specialList && menu->specialCategory == 1
        && (menu->details ? menu->detailID >= 0xFFFD : menu->cursor < 2)) return Run_Text_HelpMythicalEvent;
    if (menu->newRun || menu->settingsView) {
        if (!menu->cursor) return Run_Text_HelpPreset;
        if (menu->cursor <= SNELEMS(sRunOptions)) {
            u16 label = sRunOptions[menu->cursor - 1].label;
            if (label == Run_Text_CapMode) return menu->draft.levelCapMode == RUN_CAP_OBEDIENCE ? Run_Text_HelpObedience : Run_Text_HelpExpLock;
            if (label == Run_Text_Weight && menu->draft.rulesVersion == 2) return Run_Text_HelpLegacySimilarity;
            return RunOptionHelp(label);
        }
        return menu->cursor == SNELEMS(sRunOptions) + 1 ? Run_Text_HelpSeed : Run_Text_HelpReview;
    }
    if (menu->page == 0) {
        int row = menu->cursor;
        if (row <= 5) return !menu->draft.captureAttempts ? Run_Text_HelpTrackingOff
            : !RunState_CaptureActivated(RunSession_State()) ? Run_Text_HelpActivation : Run_Text_HelpCurrentArea;
        if (row == 8 || row == 9) return Run_Text_HelpCap;
        if (row == 10) return Run_Text_HelpBonus;
        if (row == 14) return Run_Text_HelpSeed;
        if (row == 15) return Run_Text_HelpWild;
        if (row == 16) return Run_Text_HelpTrainers;
        if (row == 17) return Run_Text_HelpRange;
        if (row == 18) return menu->draft.rulesVersion == 2 ? Run_Text_HelpLegacySimilarity : Run_Text_HelpSimilarity;
        if (row == 19) return Run_Text_HelpShinyRate;
        if (row == 20) return Run_Text_HelpFieldItems;
        if (row >= 21 && row <= 28) return Run_Text_HelpCaptureSummary;
        if (row == 29 || row == 30) return RunConfig_ObedienceCapEnabled(&menu->draft)
            && RunGame_EligibleCount(SaveData_GetParty(menu->save)) == 1 ? Run_Text_HelpOneEligible : Run_Text_HelpEligibility;
        if (row == 31) return Run_Text_HelpSettings;
        if (row == 32) return Run_Text_ResetWarning;
        if (row == 6) return menu->draft.levelCapMode == RUN_CAP_OBEDIENCE ? Run_Text_HelpObedience : Run_Text_HelpExpLock;
        if (row == 11 && RunState_LossReason(RunSession_State()) == 2) return Run_Text_HelpNoEligibleLoss;
        return Run_Text_HelpProgress;
    }
    static const u16 help[] = { Run_Text_HelpProgress, Run_Text_HelpAreas,
        Run_Text_HelpHistory, Run_Text_HelpMemorial, Run_Text_HelpSpecial };
    return help[menu->page];
}

static void RunPrepareRegistry(RunMenuData *menu)
{
    RunState *run = RunSession_State();
    for (int group = 0; group < 4; group++) {
        for (u16 zone = 0; zone < RUN_ZONE_COUNT; zone++) {
            if (!RunWorld_ZoneKnown(zone)) continue;
            int priority = zone == RunSession_Zone() ? 0 : !RunState_Visited(run, zone) ? 3
                : menu->draft.captureAttempts && !RunState_Remaining(run, zone) ? 2 : 1;
            if (priority == group) menu->zoneOrder[menu->zoneCount++] = zone;
        }
    }
    if (!RunSession_Configured()) return;
    for (u16 zone = 0; zone < RUN_ZONE_COUNT; zone++) {
        BOOL eligible = RunWorld_ZoneHasWild(zone);
        for (u8 slot = 0; slot < 3; slot++) {
            RunOpportunity attempt;
            RunState_GetOpportunity(run, zone, slot, &attempt);
            if (attempt.species) eligible = TRUE; // Also include a shared gift-only zone.
            if (attempt.outcome == RUN_OUTCOME_CAUGHT) menu->summary[3]++;
            else if (attempt.outcome >= RUN_OUTCOME_DEFEATED) menu->summary[4]++;
        }
        if (eligible) {
            menu->summary[0]++;
            if (menu->draft.captureAttempts) menu->summary[RunState_Remaining(run, zone) ? 1 : 2]++;
        }
    }
    for (u16 id = 1; id <= RunState_SpecialCount(run); id++) {
        RunSpecialRecord event;
        RunState_GetSpecial(run, id, &event);
        if (event.source == RUN_SOURCE_STATIC) menu->summary[5] += event.caught;
        if (event.source == RUN_SOURCE_SHINY) menu->summary[6] += event.caught;
    }
}

static void RunDrawModalHelp(RunMenuData *menu)
{
    static const u16 descriptions[] = { Run_Text_DetailWild, Run_Text_DetailTrainers, Run_Text_DetailTolerance, Run_Text_DetailWeight, Run_Text_DetailLegendaries, Run_Text_DetailStatic, Run_Text_DetailGifts, Run_Text_DetailEggs, Run_Text_DetailFieldItems, Run_Text_DetailSeedMode, Run_Text_DetailAttempts, Run_Text_DetailDupes, Run_Text_DetailShiny, Run_Text_DetailShinyRate, Run_Text_DetailSeparateGifts, Run_Text_DetailSeparateEggs, Run_Text_DetailPermadeath, Run_Text_DetailDayCare, Run_Text_DetailCap, Run_Text_DetailCapMode, Run_Text_DetailAllowance, Run_Text_DetailScaling, Run_Text_DetailBonus, Run_Text_DetailBosses, Run_Text_DetailNotifications };
    u16 title = menu->cursor == 0 ? Run_Text_Preset
        : menu->cursor <= SNELEMS(sRunOptions) ? sRunOptions[menu->cursor - 1].label
        : menu->cursor == SNELEMS(sRunOptions) + 1 ? Run_Text_Seed : Run_Text_Review;
    u16 detail = menu->cursor == 0 ? Run_Text_DetailPreset
        : menu->cursor <= SNELEMS(sRunOptions) ? descriptions[menu->cursor - 1]
        : menu->cursor == SNELEMS(sRunOptions) + 1 ? Run_Text_DetailSeed : Run_Text_DetailReview;
    Window *window = &menu->helpWindow;
    Window_FillTilemap(window, PIXEL_FILL(15));
    RunMessage(menu, title, menu->line);
    RunPrint(window, menu->line, 0, 0, FALSE);
    RunMessage(menu, Run_Text_HelpModalControls, menu->line);
    RunPrint(window, menu->line, 0, 140, FALSE);
    if (menu->cursor > 0 && menu->cursor <= SNELEMS(sRunOptions)
        && !RunOptionEnabled(menu, &sRunOptions[menu->cursor - 1])) {
        RunMessage(menu, Run_Text_HelpDisabled, menu->line);
        RunPrint(window, menu->line, 0, 124, FALSE);
    }
    RunMessage(menu, detail, menu->line);
    u32 at = 0, line = 0;
    while (at < menu->line->size) {
        u32 end = at, space = at;
        while (end < menu->line->size && menu->line->data[end] != CHAR_CR && end - at < 120) {
            if (menu->line->data[end] == CHAR_SPACE) space = end;
            String_CopyNumChars(menu->value, menu->line->data + at, end - at + 1);
            if (Font_CalcStringWidth(FONT_SYSTEM, menu->value, 0) > 216) break;
            end++;
        }
        if (end < menu->line->size && menu->line->data[end] != CHAR_CR && space > at) end = space;
        if (end == at) end++;
        if (line / 6 == menu->helpPage) {
            String_CopyNumChars(menu->value, menu->line->data + at, end - at);
            RunPrint(window, menu->value, 0, 24 + (line % 6) * 16, FALSE);
        }
        line++;
        at = end;
        while (at < menu->line->size && (menu->line->data[at] == CHAR_SPACE || menu->line->data[at] == CHAR_CR)) at++;
    }
    menu->helpMore = line > (menu->helpPage + 1) * 6;
    Window_CopyToVRAM(window);
}

static void RunDraw(RunMenuData *menu)
{
    if (menu->restarting) {
        for (u32 i = 0; i < NUM_WINDOWS; i++) {
            Window_FillTilemap(&menu->native.windows.asArray[i], PIXEL_FILL(15));
            Window_CopyToVRAM(&menu->native.windows.asArray[i]);
        }
        Window_FillTilemap(&menu->helpWindow, PIXEL_FILL(15));
        RunMessage(menu, menu->restartFailed ? Run_Text_RestartFailed : Run_Text_RestartWorking, menu->line);
        RunPrint(&menu->helpWindow, menu->line, 0, 16, FALSE);
        Window_CopyToVRAM(&menu->helpWindow);
        return;
    }
    Window *entries = &menu->native.windows.entries;
    Window *description = &menu->native.windows.description;
    Window_FillTilemap(entries, PIXEL_FILL(15));
    Window_FillTilemap(description, PIXEL_FILL(15));
    Window_FillTilemap(&menu->native.windows.title, 0);
    static const u16 registryTitles[] = { Run_Text_Overview, Run_Text_Areas, Run_Text_History,
        Run_Text_Memorial, Run_Text_Specials };
    u16 title = menu->newRun ? (menu->reviewing ? Run_Text_Review : Run_Text_Title)
        : menu->settingsView ? Run_Text_Settings : registryTitles[menu->page];
    RunMessage(menu, title, menu->line);
    while (menu->line->size && Font_CalcStringWidth(FONT_SYSTEM, menu->line, 0) > MENU_TITLE_WIDTH * 8 - 4) {
        u16 end = menu->line->size;
        while (end && menu->line->data[end - 1] != CHAR_SPACE) end--;
        menu->line->size = end ? end - 1 : menu->line->size - 1;
        menu->line->data[menu->line->size] = CHAR_EOS;
    }
    Text_AddPrinterWithParamsAndColor(&menu->native.windows.title, FONT_SYSTEM, menu->line,
        2, 2, TEXT_SPEED_NO_TRANSFER, TEXT_COLOR(1, 2, 0), NULL);
    int count = RunRowCount(menu);
    if (menu->cursor >= count) {
        menu->cursor = 0;
    }
    if (menu->cursor < menu->scroll) menu->scroll = menu->cursor;
    if (menu->cursor >= menu->scroll + RUN_VISIBLE_ROWS) menu->scroll = menu->cursor - RUN_VISIBLE_ROWS + 1;
    if (menu->damaged || menu->resetPrompt) {
        RunMessage(menu, menu->damaged ? Run_Text_Damaged : Run_Text_ResetWarning, menu->line);
        RunPrint(entries, menu->line, 0, 0, FALSE);
    } else if (!menu->newRun && !RunSession_Configured() && menu->page > 0 && menu->page < 5) {
        RunMessage(menu, Run_Text_Empty, menu->line);
        RunPrint(entries, menu->line, 0, 0, FALSE);
    } else for (int row = menu->scroll; row < count && row < menu->scroll + RUN_VISIBLE_ROWS; row++) {
        String_Clear(menu->line);
        String_Clear(menu->value);
        BOOL inactive = FALSE;
        if (menu->newRun || menu->settingsView) {
            if (row == 0) {
                RunMessage(menu, Run_Text_Preset, menu->line);
                RunMessage(menu, Run_Text_Classic + RunConfig_EffectivePreset(&menu->draft), menu->value);
            } else if (row <= SNELEMS(sRunOptions)) {
                const RunMenuOption *option = &sRunOptions[row - 1];
                RunMessage(menu, option->label, menu->line);
                RunOptionValue(menu, option);
                inactive = !RunOptionEnabled(menu, option);
            } else if (row == SNELEMS(sRunOptions) + 1) {
                RunMessage(menu, Run_Text_Seed, menu->line);
                RunSeedString(menu);
            } else {
                RunMessage(menu, menu->reviewing ? Run_Text_Confirm : Run_Text_Review, menu->line);
            }
        } else if (menu->page == 0) {
            RunOverview(menu, row);
            inactive = row == 6 || row == 12 || row == 21;
        } else if (menu->page == 4 && !menu->specialList) {
            RunMessage(menu, Run_Text_StaticCategory + row, menu->line);
            RunNumber(menu, RunSpecialCount(row));
        } else {
            int id = menu->details ? menu->detailID : RunRecordID(menu, row);
            if (id < 0) {
                RunMessage(menu, Run_Text_Empty, menu->line);
            } else if (menu->page == 1) {
                MessageLoader_GetString(menu->locations, id, menu->line);
                if (!menu->details) {
                    RunMessage(menu, !RunState_Visited(RunSession_State(), id) ? Run_Text_NotVisited
                        : RunAreaStatus(menu, id), menu->value);
                } else if (row == 0) {
                    RunNumber(menu, RunState_Remaining(RunSession_State(), id));
                } else if (row <= 6) {
                    RunOpportunity opportunity;
                    int attempt = (row - 1) / 2;
                    RunState_GetOpportunity(RunSession_State(), id, attempt, &opportunity);
                    if (opportunity.species) MessageLoader_GetString(menu->species, opportunity.species, menu->line);
                    else RunNumber(menu, attempt + 1);
                    if (!opportunity.species) String_Copy(menu->line, menu->value);
                    RunMessage(menu, RunOutcomeText(opportunity.outcome), menu->value);
                    if (!(row & 1)) {
                        RunMonRecord mon;
                        String_Clear(menu->line);
                        String_Clear(menu->value);
                        if (RunState_GetMon(RunSession_State(), opportunity.monID, &mon)) {
                            String_CopyNumChars(menu->line, mon.nickname, MON_NAME_LEN);
                            RunMessage(menu, mon.flags & RUN_MON_EXHAUSTED ? Run_Text_ExhaustedMon : Run_Text_Alive, menu->value);
                        }
                    }
                    if (attempt >= menu->draft.captureAttempts) {
                        RunMessage(menu, Run_Text_Off, menu->value);
                        inactive = TRUE;
                    }
                } else {
                    RunMessage(menu, Run_Text_Dupes, menu->line);
                    RunNumber(menu, RunState_DuplicatesSeen(RunSession_State(), id));
                }
            } else if (menu->page == 4) {
                RunSpecialRecord special;
                u16 status = 0;
                if (id >= 0xFFFD) {
                    static const u16 statuses[] = { Run_Text_EventNotAvailable, Run_Text_EventAvailable,
                        Run_Text_EventEncountered, Run_Text_EventCaught, Run_Text_EventFailed };
                    status = statuses[RunGame_EventStatus(menu->save, 0xFFFE - id, &special)];
                } else RunState_GetSpecial(RunSession_State(), id, &special);
                if (!menu->details) {
                    MessageLoader_GetString(menu->species, special.species, menu->line);
                    RunMessage(menu, status ? status : RunOutcomeText(special.outcome), menu->value);
                } else {
                    static const u16 labels[] = { Run_Text_Original, Run_Text_Origin, Run_Text_Attempts,
                        Run_Text_Caught, Run_Text_Failures, Run_Text_Result, Run_Text_History };
                    RunMessage(menu, labels[row], menu->line);
                    if (row == 0) MessageLoader_GetString(menu->species, special.species, menu->value);
                    if (row == 1) MessageLoader_GetString(menu->locations, special.zone, menu->value);
                    if (row == 2) RunNumber(menu, special.attempts);
                    if (row == 3) RunNumber(menu, special.caught);
                    if (row == 4) RunNumber(menu, special.failed);
                    if (row == 5) RunMessage(menu, status ? status : RunOutcomeText(special.outcome), menu->value);
                    if (row == 6) {
                        RunMonRecord mon;
                        if (RunState_GetMon(RunSession_State(), special.lastMonID, &mon))
                            String_CopyNumChars(menu->value, mon.nickname, MON_NAME_LEN);
                    }
                }
            } else {
                RunMonRecord mon;
                RunState_GetMon(RunSession_State(), id, &mon);
                String_CopyNumChars(menu->line, mon.nickname, MON_NAME_LEN);
                if (!menu->details) {
                    if (mon.flags & RUN_MON_EXHAUSTED) RunMessage(menu, Run_Text_ExhaustedMon, menu->value);
                    else MessageLoader_GetString(menu->species, mon.species, menu->value);
                } else {
                    static const u16 labels[] = { Run_Text_History, Run_Text_Original, Run_Text_Origin,
                        Run_Text_Death, Run_Text_Evolution, Run_Text_Evolution, Run_Text_Evolution };
                    RunMessage(menu, labels[row], menu->line);
                    if (row == 0) String_CopyNumChars(menu->value, mon.nickname, MON_NAME_LEN);
                    if (row == 1) MessageLoader_GetString(menu->species, mon.originalSpecies, menu->value);
                    if (row == 2) MessageLoader_GetString(menu->locations, mon.originZone, menu->value);
                    if (row == 3) {
                        if (mon.flags & RUN_MON_EXHAUSTED) MessageLoader_GetString(menu->locations, mon.deathZone, menu->value);
                        else RunMessage(menu, Run_Text_Alive, menu->value);
                    }
                    if (row >= 4 && row - 4 < mon.evolutionCount) MessageLoader_GetString(menu->species, mon.evolutions[row - 4], menu->value);
                }
            }
        }
        RunRow(menu, row, inactive);
    }
    u16 help = menu->newRun ? (menu->reviewing ? Run_Text_ReviewControls : Run_Text_Controls) : Run_Text_ReadOnly;
    if (menu->newRun && menu->cursor > 0 && menu->cursor <= SNELEMS(sRunOptions)
        && sRunOptions[menu->cursor - 1].flag != RUN_END_ON_PARTY_WIPE
        && !RunOptionEnabled(menu, &sRunOptions[menu->cursor - 1])) help = Run_Text_OffHelp;
    if (menu->seedEditing) help = Run_Text_SeedHelp;
    if (menu->damaged) help = Run_Text_DamagedHelp;
    if (menu->confirmFailed) help = Run_Text_ConfirmFailed;
    RunMessage(menu, help, menu->line);
    RunPrint(description, menu->line, 0, 0, FALSE);
    if (menu->helpReady) {
        Window_FillTilemap(&menu->helpWindow, PIXEL_FILL(15));
        RunMessage(menu, menu->newRun ? Run_Text_ConfigNav : Run_Text_NavRun + menu->page, menu->line);
        RunPrint(&menu->helpWindow, menu->line, 0, 0, FALSE);
        u16 context = menu->damaged ? Run_Text_DamagedHelp : menu->resetPrompt ? Run_Text_ResetWarning : RunContextHelp(menu);
        RunMessage(menu, context, menu->line);
        RunPrint(&menu->helpWindow, menu->line, 0, 32, FALSE);
        Window_CopyToVRAM(&menu->helpWindow);
    }
    if (menu->modalHelp) RunDrawModalHelp(menu);
    Bg_SetOffset(menu->native.bgConfig, BG_LAYER_MAIN_0, BG_OFFSET_UPDATE_SET_Y,
        -(FIRST_ENTRY_OFFSET + (menu->cursor - menu->scroll) * RUN_ROW_HEIGHT));
    for (u32 i = 0; i < NUM_WINDOWS; i++) Window_CopyToVRAM(&menu->native.windows.asArray[i]);
}

static BOOL RunMenu_InitCommon(ApplicationManager *appMan, BOOL newRun, BOOL standalone)
{
    Heap_Create(HEAP_ID_APPLICATION, HEAP_ID_OPTIONS_MENU, HEAP_ALLOCATION_SIZE);
    RunMenuData *menu = ApplicationManager_NewData(appMan, sizeof(RunMenuData), HEAP_ID_OPTIONS_MENU);
    if (menu == NULL) { OS_ResetSystem(RESET_CLEAN); return FALSE; }
    memset(menu, 0, sizeof(*menu));
    menu->save = standalone ? ((ApplicationArgs *)ApplicationManager_Args(appMan))->saveData : ApplicationManager_Args(appMan);
    menu->native.heapID = HEAP_ID_OPTIONS_MENU;
    menu->native.saveOptions = SaveData_GetOptions(menu->save);
    menu->native.options.messageBoxStyle = Options_Frame(menu->native.saveOptions);
    menu->damaged = RunSession_Damaged();
    menu->newRun = newRun && !menu->damaged;
    if (menu->newRun) RunConfig_Init(&menu->draft, RUN_PRESET_CLASSIC, MTRNG_Next());
    else menu->draft = *RunSession_Config();
    menu->messages = MessageLoader_Init(MSG_LOADER_LOAD_ON_DEMAND, NARC_INDEX_MSGDATA__PL_MSG, TEXT_BANK_RUN_ENGINE, HEAP_ID_OPTIONS_MENU);
    menu->locations = MessageLoader_Init(MSG_LOADER_LOAD_ON_DEMAND, NARC_INDEX_MSGDATA__PL_MSG, TEXT_BANK_LOCATION_NAMES, HEAP_ID_OPTIONS_MENU);
    menu->species = MessageLoader_Init(MSG_LOADER_LOAD_ON_DEMAND, NARC_INDEX_MSGDATA__PL_MSG, TEXT_BANK_SPECIES_NAME, HEAP_ID_OPTIONS_MENU);
    menu->line = String_Init(512, HEAP_ID_OPTIONS_MENU);
    menu->value = String_Init(128, HEAP_ID_OPTIONS_MENU);
    if (!menu->messages || !menu->locations || !menu->species || !menu->line || !menu->value) {
        OS_ResetSystem(RESET_CLEAN);
        return FALSE;
    }
    menu->digits = MessageLoader_GetNewString(menu->messages, Run_Text_Digits);
    if (!menu->digits) { OS_ResetSystem(RESET_CLEAN); return FALSE; }
    if (!menu->newRun && !standalone) RunPrepareRegistry(menu);
    return TRUE;
}

BOOL RunConfigMenu_Init(ApplicationManager *appMan, int *state)
{
    return RunMenu_InitCommon(appMan, TRUE, TRUE);
}

BOOL RunRegistryMenu_Init(ApplicationManager *appMan, int *state)
{
    return RunMenu_InitCommon(appMan, FALSE, FALSE);
}

BOOL RunRestartMenu_Init(ApplicationManager *appMan, int *state)
{
    BOOL result = RunMenu_InitCommon(appMan, FALSE, TRUE);
    ((RunMenuData *)ApplicationManager_Data(appMan))->restarting = TRUE;
    return result;
}

BOOL RunMenu_Main(ApplicationManager *appMan, int *state)
{
    RunMenuData *menu = ApplicationManager_Data(appMan);
    if (*state == 0) {
        if (!SetupMenuVisuals(&menu->native)) return FALSE;
        // The shared setup printed retail Options choices. Run screens do not
        // use them; release their strings before allocating the help window.
        for (u32 entry = 0; entry < MAX_ENTRIES; entry++) {
            for (u32 choice = 0; choice < menu->native.entries.asArray[entry].numChoices; choice++)
                String_Free(menu->native.entries.asArray[entry].choices[choice]);
            menu->native.entries.asArray[entry].numChoices = 0;
        }
        Window_Add(menu->native.bgConfig, &menu->helpWindow, BG_LAYER_SUB_0, 2, 2, 28, 20, 13, 64);
        LoadStandardWindowGraphics(menu->native.bgConfig, BG_LAYER_SUB_0, 650, 14,
            STANDARD_WINDOW_SYSTEM, HEAP_ID_OPTIONS_MENU);
        Window_DrawStandardFrame(&menu->helpWindow, TRUE, 650, 14);
        menu->helpReady = TRUE;
        GXLayers_TurnBothDispOn();
        RunDraw(menu);
        StartScreenFade(FADE_BOTH_SCREENS, FADE_TYPE_BRIGHTNESS_IN, FADE_TYPE_BRIGHTNESS_IN, COLOR_BLACK, 6, 1, HEAP_ID_OPTIONS_MENU);
        (*state)++;
    } else if (*state == 1) {
        if (IsScreenFadeDone()) (*state)++;
    } else if (*state == 2) {
        BOOL close = FALSE;
        if (menu->restarting) {
            if (menu->restartFailed) {
                if (JOY_NEW(PAD_BUTTON_B)) close = TRUE;
                else if (JOY_NEW(PAD_BUTTON_A)) {
                    menu->restartFailed = FALSE;
                    menu->restartWait = 0;
                }
            } else if (++menu->restartWait >= 2) {
                // Give VBlank time to display the notice before native flash
                // erasure. No input is needed for a successful restart.
                menu->restartErased = RunGame_Restart(menu->save);
                menu->restartFailed = !menu->restartErased;
                close = menu->restartErased;
            }
        } else if (menu->modalHelp) {
            if (JOY_NEW(PAD_BUTTON_B | PAD_BUTTON_X)) menu->modalHelp = FALSE;
            else if (JOY_NEW(PAD_BUTTON_A)) menu->helpPage = menu->helpMore ? menu->helpPage + 1 : 0;
        } else if ((menu->newRun || menu->settingsView) && !menu->seedEditing && JOY_NEW(PAD_BUTTON_X)) {
            menu->modalHelp = TRUE;
            menu->helpPage = 0;
        } else if (menu->resetPrompt) {
            if (JOY_NEW(PAD_BUTTON_B)) menu->resetPrompt = FALSE;
            if (JOY_NEW(PAD_BUTTON_A) && SaveData_Erase(menu->save)) OS_ResetSystem(RESET_CLEAN);
        } else if (menu->damaged) {
            if (JOY_NEW(PAD_BUTTON_B)) close = TRUE;
        } else if (menu->seedEditing) {
            if (JOY_REPEAT(PAD_KEY_LEFT)) menu->seedDigit = (menu->seedDigit + 7) % 8;
            if (JOY_REPEAT(PAD_KEY_RIGHT)) menu->seedDigit = (menu->seedDigit + 1) % 8;
            if (JOY_REPEAT(PAD_KEY_UP | PAD_KEY_DOWN)) {
                u32 shift = 28 - menu->seedDigit * 4;
                u32 value = ((menu->draft.seed >> shift) + (JOY_REPEAT(PAD_KEY_UP) ? 1 : 15)) & 15;
                menu->draft.seed = (menu->draft.seed & ~(15u << shift)) | (value << shift);
            }
            if (JOY_NEW(PAD_BUTTON_B)) { menu->draft.seed = menu->oldSeed; menu->seedEditing = FALSE; }
            if (JOY_NEW(PAD_BUTTON_A)) menu->seedEditing = FALSE;
        } else {
            int count = RunRowCount(menu);
            if (JOY_REPEAT(PAD_KEY_UP)) menu->cursor = (menu->cursor + count - 1) % count;
            if (JOY_REPEAT(PAD_KEY_DOWN)) menu->cursor = (menu->cursor + 1) % count;
            if (!menu->newRun && JOY_NEW(PAD_BUTTON_L | PAD_BUTTON_R)) {
                if (!menu->settingsView) {
                    menu->pageCursor[menu->page] = menu->details ? menu->listCursor
                        : menu->specialList ? menu->specialCategory : menu->cursor;
                    menu->pageScroll[menu->page] = menu->scroll;
                }
                menu->page = (menu->page + (JOY_NEW(PAD_BUTTON_R) ? 1 : 4)) % 5;
                menu->cursor = menu->pageCursor[menu->page];
                menu->scroll = menu->pageScroll[menu->page];
                menu->details = menu->specialList = menu->settingsView = menu->currentAreaDetails = FALSE;
            }
            if (JOY_NEW(PAD_BUTTON_B)) {
                if (menu->newRun) {
                    if (menu->reviewing) { menu->reviewing = FALSE; menu->cursor = SNELEMS(sRunOptions) + 2; }
                    else menu->cursor = menu->scroll = 0;
                } else if (menu->settingsView) { menu->settingsView = FALSE; menu->cursor = 31; menu->scroll = menu->pageScroll[0]; }
                else if (menu->details) {
                    menu->details = FALSE;
                    menu->cursor = menu->listCursor;
                    menu->scroll = menu->pageScroll[menu->page];
                    if (menu->currentAreaDetails) { menu->page = 0; menu->cursor = 5; menu->currentAreaDetails = FALSE; }
                } else if (menu->page == 4 && menu->specialList) { menu->specialList = FALSE; menu->cursor = menu->specialCategory; }
                else close = TRUE;
            }
            BOOL action = JOY_NEW(PAD_BUTTON_A);
            BOOL change = action || JOY_REPEAT(PAD_KEY_LEFT | PAD_KEY_RIGHT);
            if (menu->newRun && !menu->reviewing && change) {
                if (menu->cursor == 0) {
                    u8 mode = menu->draft.seedMode;
                    int preset = (RunConfig_EffectivePreset(&menu->draft) + (JOY_REPEAT(PAD_KEY_LEFT) ? RUN_PRESET_COUNT - 1 : 1)) % RUN_PRESET_COUNT;
                    RunConfig_Init(&menu->draft, preset, menu->draft.seed);
                    menu->draft.seedMode = mode;
                } else if (menu->cursor <= SNELEMS(sRunOptions)) {
                    const RunMenuOption *option = &sRunOptions[menu->cursor - 1];
                    if (RunOptionEnabled(menu, option)) {
                        if (option->flag) {
                            menu->draft.flags ^= option->flag;
                            if (option->flag == RUN_PERMADEATH) {
                                if (menu->draft.flags & RUN_PERMADEATH) menu->draft.flags |= RUN_END_ON_PARTY_WIPE;
                                else menu->draft.flags &= ~RUN_END_ON_PARTY_WIPE;
                            }
                        } else {
                            u8 *value = (u8 *)&menu->draft + option->offset;
                            *value = RunConfig_AdjustValue(*value, option->low, option->high, option->step, JOY_REPEAT(PAD_KEY_LEFT));
                            if (option->label == Run_Text_SeedMode && *value == RUN_SEED_AUTO) menu->draft.seed = MTRNG_Next();
                        }
                    }
                } else if (menu->cursor == SNELEMS(sRunOptions) + 1 && action) {
                    menu->draft.seedMode = RUN_SEED_MANUAL;
                    menu->oldSeed = menu->draft.seed;
                    menu->seedEditing = TRUE;
                } else if (action) {
                    menu->reviewing = TRUE;
                    menu->cursor = menu->scroll = 0;
                }
            } else if (menu->newRun && menu->reviewing && action && menu->cursor == SNELEMS(sRunOptions) + 2) {
                u32 nonce = MTRNG_Next();
                if (RunSession_Confirm(menu->save, &menu->draft, nonce ? nonce : 1)) { menu->confirmed = TRUE; close = TRUE; }
                else menu->confirmFailed = TRUE;
            } else if (!menu->newRun && !menu->settingsView && action) {
                if (menu->page == 0 && menu->cursor == 32) menu->resetPrompt = TRUE;
                else if (menu->page == 0 && menu->cursor == 31) {
                    menu->pageCursor[0] = menu->cursor; menu->pageScroll[0] = menu->scroll;
                    menu->settingsView = TRUE; menu->cursor = menu->scroll = 0;
                }
                else if (menu->page == 0 && menu->cursor == 5 && RunSession_Configured()) {
                    menu->pageCursor[0] = menu->cursor;
                    menu->pageScroll[0] = menu->scroll;
                    menu->page = 1; menu->detailID = RunSession_Zone();
                    menu->details = menu->currentAreaDetails = TRUE; menu->cursor = menu->scroll = 0;
                } else if (RunSession_Configured() && menu->page > 0) {
                    if (menu->page == 4 && !menu->specialList) {
                        menu->specialCategory = menu->cursor; menu->specialList = TRUE;
                        menu->cursor = menu->scroll = 0;
                    } else if (!menu->details) {
                        int id = RunRecordID(menu, menu->cursor);
                        if (id >= 0) {
                            menu->detailID = id; menu->listCursor = menu->cursor;
                            menu->pageScroll[menu->page] = menu->scroll;
                            menu->details = TRUE; menu->cursor = menu->scroll = 0;
                        }
                    } else {
                        menu->details = FALSE; menu->cursor = menu->listCursor;
                        menu->scroll = menu->pageScroll[menu->page];
                    }
                }
            }
        }
        if (gSystem.pressedKeys || gSystem.pressedKeysRepeatable) {
            Sound_PlayEffect(SE_CONFIRM_sseq_3);
            RunDraw(menu);
        }
        if (close) {
            StartScreenFade(FADE_BOTH_SCREENS, FADE_TYPE_BRIGHTNESS_OUT, FADE_TYPE_BRIGHTNESS_OUT, COLOR_BLACK, 6, 1, HEAP_ID_OPTIONS_MENU);
            (*state)++;
        }
    } else if (*state == 3) {
        if (IsScreenFadeDone()) (*state)++;
    } else if (*state == 4) {
        SetVBlankCallback(NULL, NULL);
        DisableHBlank();
        if (menu->helpReady) {
            Window_EraseStandardFrame(&menu->helpWindow, FALSE);
            Window_Remove(&menu->helpWindow);
            menu->helpReady = FALSE;
        }
        return TeardownMenuData(&menu->native);
    }
    return FALSE;
}

BOOL RunMenu_Exit(ApplicationManager *appMan, int *state)
{
    RunMenuData *menu = ApplicationManager_Data(appMan);
    BOOL newRun = menu->newRun;
    BOOL damaged = menu->damaged;
    BOOL restarting = menu->restarting;
    BOOL restartErased = menu->restartErased;
    MessageLoader_Free(menu->messages);
    MessageLoader_Free(menu->locations);
    MessageLoader_Free(menu->species);
    String_Free(menu->line);
    String_Free(menu->value);
    String_Free(menu->digits);
    ApplicationManager_FreeData(appMan);
    Heap_Destroy(HEAP_ID_OPTIONS_MENU);
    if (newRun) {
        extern const ApplicationManagerTemplate gRowanIntroAppTemplate;
        EnqueueApplication(FS_OVERLAY_ID_NONE, &gRowanIntroAppTemplate);
    } else if (restartErased) {
        EnqueueApplication(FS_OVERLAY_ID_NONE, &gGameStartRowanIntroAppTemplate);
    } else if (damaged || restarting) {
        EnqueueApplication(FS_OVERLAY_ID(game_opening), &gTitleScreenAppTemplate);
    }
    return TRUE;
}
