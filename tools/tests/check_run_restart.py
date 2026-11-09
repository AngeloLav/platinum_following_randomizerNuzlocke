#!/usr/bin/env python3
"""Exercise actual restart/erase functions in host RAM; never opens a save file."""
import argparse
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def function(source, name):
    match = re.search(r"^(?:static )?(?:BOOL|void|int) " + name + r"\([^;{}]*\)\n\{", source, re.M)
    assert match, name
    end, depth = match.end(), 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[match.start():end]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cc", default="gcc")
    compiler = shutil.which(parser.parse_args().cc)
    assert compiler, "C99 compiler required"
    game = (ROOT / "src/run_game.c").read_text(encoding="utf-8")
    field = (ROOT / "src/field_system.c").read_text(encoding="utf-8")
    menu = (ROOT / "src/applications/options_menu.c").read_text(encoding="utf-8")
    savedata = (ROOT / "src/savedata.c").read_text(encoding="utf-8")
    task = function(game, "RunGame_EndRunTask")
    assert "SaveData_Save(" not in task and "FieldSystem_StartChildProcess(" not in task
    assert "FieldTransition_FinishMap(task)" in task and "processManager->kill = TRUE" in task
    exit_field = function(field, "ReturnToTitleScreen")
    assert exit_field.index("TeardownFieldSystem(appMan)") < exit_field.index("&gRunRestartMenuTemplate")
    assert "RunMenu_InitCommon(appMan, FALSE, TRUE)" in function(menu, "RunRestartMenu_Init")
    assert "standalone ? ((ApplicationArgs *)ApplicationManager_Args(appMan))->saveData" in menu
    assert "close = menu->restartErased" in function(menu, "RunMenu_Main")
    assert "&gGameStartRowanIntroAppTemplate" in function(menu, "RunMenu_Exit")
    assert "OS_ResetSystem" not in function(game, "RunGame_Restart")
    common = (ROOT / "res/field/scripts/scripts_common.s").read_text(encoding="utf-8")
    ended_scripts = common.split("CommonScript_RunEndedSaveFailed:", 1)[1].split("CommonScript_Dummy2010:", 1)[0]
    assert "WaitButton" not in ended_scripts and "Message " not in ended_scripts
    source = r'''
#include <stdint.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
typedef uint8_t u8;
typedef uint32_t u32;
typedef int BOOL;
#define TRUE 1
#define FALSE 0
#define SAVE_SECTOR_SIZE 4096
#define SAVE_PAGE_MAX 32
#define SECTOR_ID_MAX 2
#define PRIMARY_SECTOR_START 0
#define BACKUP_SECTOR_START 64
enum { SAVE_BLOCK_ID_NORMAL, SAVE_BLOCK_ID_BOXES, HEAP_ID_APPLICATION, SLEEP_TYPE_SAVE_DATA };
typedef struct { u32 unused; } Misc;
typedef struct { int blockOffsets[2]; BOOL dataExists; Misc misc; } SaveData;
typedef struct { u32 nonce; } RunState;
static u8 flash[512 * 1024];
static u32 buffer[SAVE_SECTOR_SIZE / 4];
static u32 sDefeatedNonce;
static RunState state;
static SaveData *currentSave;
static int configured, over, protectedSave, failErase;
static int resets, writes, footers, eraseCalls, locked;
static unsigned char sectors[128];
static void *Heap_AllocAtEnd(int heap, int size) { (void)heap; assert(size == SAVE_SECTOR_SIZE); return buffer; }
static void Heap_Free(void *ptr) { assert(ptr == buffer); }
static void SleepLock(int kind) { (void)kind; assert(!locked); locked = 1; }
static void SleepUnlock(int kind) { (void)kind; assert(locked); locked = 0; }
static BOOL SaveBlockFooter_Erase(const SaveData *save, int block, int sector)
{ (void)save; assert(block < 2 && sector < 2); footers++; return TRUE; }
static void MI_CpuFillFast(void *ptr, u32 value, int size) { assert(value == 0xFFFFFFFFu); memset(ptr, 255, size); }
static BOOL SaveData_CardSave(u32 address, void *data, u32 size)
{
    assert(locked && size == SAVE_SECTOR_SIZE && address % SAVE_SECTOR_SIZE == 0);
    assert(address + size <= sizeof(flash));
    assert(!sectors[address / SAVE_SECTOR_SIZE]++);
    memcpy(flash + address, data, size); writes++; return TRUE;
}
static void SaveData_Clear(SaveData *save) { save->misc.unused = 0; }
''' + function(savedata, "SaveData_Erase").replace("BOOL SaveData_Erase(", "static BOOL NativeErase(", 1) + r'''
static BOOL SaveData_Erase(SaveData *save) { eraseCalls++; return failErase ? FALSE : NativeErase(save); }
static BOOL RunSession_Configured(void) { return configured; }
static SaveData *SaveData_Ptr(void) { return currentSave; }
static BOOL RunGame_Over(void) { return over; }
static BOOL SaveData_OverwriteCheck(SaveData *save) { (void)save; return protectedSave; }
static Misc *SaveData_MiscSaveBlock(SaveData *save) { return &save->misc; }
static RunState *RunSession_State(void) { return &state; }
static u32 RunState_Nonce(RunState *run) { return run->nonce; }
static void RunSession_Reset(void) { resets++; configured = over = 0; state.nonce = 0; }
''' + function(game, "RunGame_Restart") + r'''
int main(void)
{
    for (int mask = 0; mask < 64; mask++) {
        configured = !!(mask & 1); over = !!(mask & 2); protectedSave = !!(mask & 4);
        failErase = !!(mask & 8);
        SaveData save = { { 0, 1 }, TRUE, { (mask & 16) ? 123u : 456u } };
        SaveData other = save;
        currentSave = (mask & 32) ? &other : &save;
        state.nonce = sDefeatedNonce = 123;
        resets = writes = footers = eraseCalls = locked = 0;
        memset(flash, 0x3C, sizeof(flash)); memset(sectors, 0, sizeof(sectors));
        BOOL allowed = currentSave == &save && configured && over && !protectedSave && save.misc.unused == state.nonce;
        BOOL expected = allowed && !failErase;
        assert(RunGame_Restart(&save) == expected);
        assert(eraseCalls == allowed);
        assert(resets == expected && writes == (expected ? 128 : 0));
        assert(footers == (expected ? 4 : 0) && !locked);
        for (u32 i = 0; i < sizeof(flash); i++) assert(flash[i] == (expected ? 255 : 0x3C));
        if (expected) assert(!configured && !over && !state.nonce && !sDefeatedNonce && !save.dataExists && !save.misc.unused);
        else assert(save.dataExists && state.nonce == 123 && sDefeatedNonce == 123);
    }
    puts("PASS: actual restart guards across 64 state/failure combinations; no erase of active/unconfigured/foreign saves");
    puts("PASS: actual native eraser covers all 128 sectors, both run snapshots, four footers and clears RAM only on successful restart");
    return 0;
}
'''
    directory = ROOT / "artifacts/run-restart-tests"
    directory.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="case-", dir=directory) as temporary:
        work = Path(temporary)
        source_path, executable = work / "restart.c", work / "restart.exe"
        source_path.write_text(source, encoding="utf-8")
        subprocess.run([compiler, "-std=c99", "-pedantic", "-Wall", "-Wextra", "-Werror", "-O2", str(source_path), "-o", str(executable)], check=True)
        subprocess.run([str(executable)], check=True)
    print("PASS: field teardown precedes restart app; correct standalone arguments; automatic configuration route after erase")
    print("NOT TESTED: actual flash failures, ARM overlay execution, visible transition or emulator gameplay")


if __name__ == "__main__":
    main()
