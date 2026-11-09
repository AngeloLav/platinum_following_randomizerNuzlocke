#include "run_menu.h"

#include "game_overlay.h"

FS_EXTERN_OVERLAY(options_menu);

const ApplicationManagerTemplate gRunRestartMenuTemplate = {
    RunRestartMenu_Init, RunMenu_Main, RunMenu_Exit, FS_OVERLAY_ID(options_menu)
};

const ApplicationManagerTemplate gRunConfigMenuTemplate = {
    RunConfigMenu_Init, RunMenu_Main, RunMenu_Exit, FS_OVERLAY_ID(options_menu)
};

const ApplicationManagerTemplate gRunRegistryMenuTemplate = {
    RunRegistryMenu_Init, RunMenu_Main, RunMenu_Exit, FS_OVERLAY_ID(options_menu)
};
