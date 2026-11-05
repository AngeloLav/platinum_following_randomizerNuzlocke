#ifndef POKEPLATINUM_RUN_MENU_H
#define POKEPLATINUM_RUN_MENU_H

#include "overlay_manager.h"
#include "savedata.h"

extern const ApplicationManagerTemplate gRunConfigMenuTemplate;
extern const ApplicationManagerTemplate gRunRegistryMenuTemplate;
BOOL RunConfigMenu_Init(ApplicationManager *appMan, int *state);
BOOL RunRegistryMenu_Init(ApplicationManager *appMan, int *state);
BOOL RunMenu_Main(ApplicationManager *appMan, int *state);
BOOL RunMenu_Exit(ApplicationManager *appMan, int *state);

#endif // POKEPLATINUM_RUN_MENU_H
