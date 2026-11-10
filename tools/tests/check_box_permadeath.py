#!/usr/bin/env python3
"""Exercise native box conversion and real permadeath hooks with host fixtures.

Crypto, stat data and flash are fixtures; no ROM or save file is opened.
"""
import argparse
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def function(source, name):
    match = re.search(r"^(?:static )?(?:BOOL|void|int|u16) " + name + r"\([^;{}]*\)\n\{", source, re.M)
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
    pokemon = (ROOT / "src/pokemon.c").read_text(encoding="utf-8")
    game = (ROOT / "src/run_game.c").read_text(encoding="utf-8")
    pc = (ROOT / "src/pc_boxes.c").read_text(encoding="utf-8")
    actual = "\n".join(function(game, name) for name in
        ["RunGame_MonID", "RunGame_Exhausted", "RunGame_Exhaust", "RunGame_FilterHP"])
    actual += "\n" + "\n".join(function(pokemon, name) for name in
        ["Pokemon_EnterDecryptionContext", "Pokemon_ExitDecryptionContext", "Pokemon_SetValue", "Pokemon_FromBoxPokemon"])
    actual += "\n" + "\n".join(function(pc, name) for name in
        ["PCBoxes_TryStoreBoxMonInBox", "PCBoxes_TryStoreBoxMonAt", "PCBoxes_TryStoreBoxMon"])
    source = r'''
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
typedef uint8_t u8; typedef uint16_t u16; typedef uint32_t u32; typedef int BOOL;
#define TRUE 1
#define FALSE 0
#define GF_ASSERT assert
#define RUN_MON_TRACKED 0x40
#define RUN_MON_DEAD 0x80
#define RUN_PERMADEATH 1u
#define RUN_LEVEL_CAPS 2u
#define HEAP_ID_SYSTEM 0
#define MAX_PC_BOXES 18
#define MAX_MONS_PER_BOX 30
#define USE_CURRENT_BOX 0xFFFFFFFFu
#define SHAYMIN_FORM_LAND 0
#define SPECIES_NONE 0
enum PokemonDataParam { MON_DATA_HP, MON_DATA_MAX_HP, MON_DATA_STATUS, MON_DATA_MAIL,
 MON_DATA_BALL_CAPSULE_ID, MON_DATA_BALL_CAPSULE, MON_DATA_EXPERIENCE, MON_DATA_SPECIES,
 MON_DATA_NICKNAME, MON_DATA_NICKNAME_AND_FLAG, MON_DATA_NICKNAME_STRING,
 MON_DATA_NICKNAME_STRING_AND_FLAG, MON_DATA_SPECIES_NAME, MON_DATA_IS_EGG,
 MON_DATA_UNUSED_113, MON_DATA_UNUSED_114, MON_DATA_OT_ID };
typedef struct { u32 data[8]; } PokemonDataBlock;
typedef struct {
 u32 personality, otID; BOOL boxDecrypted, partyDecrypted, checksumFailed;
 u16 checksum, species, id; u8 marker, isEgg;
 PokemonDataBlock dataBlocks[4];
} BoxPokemon;
typedef struct { u32 status; u16 hp, maxHP; u8 ballCapsuleID; } PartyPokemon;
typedef struct { BoxPokemon box; PartyPokemon party; } Pokemon;
typedef struct { u8 bytes[16]; } BallCapsule;
typedef struct { int unused; } Mail;
typedef struct { u32 flags; } RunConfig;
typedef struct { u8 dead; } RunState;
typedef struct { u32 personality, trainerID; } RunMonRecord;
typedef struct { BoxPokemon boxMons[MAX_PC_BOXES][MAX_MONS_PER_BOX]; u32 currentBoxID; } PCBoxes;
static RunConfig config;
static RunState run;
static BOOL configured = TRUE, sSuspended = FALSE;
static int deaths, fullSaves;
static int checksumChecks;
static Mail mail;
static Pokemon scratch;
static PCBoxes boxes;
static const u16 recordID = 1;
static u32 recordPID, recordOT;
static BOOL RunSession_Configured(void) { return configured; }
static const RunConfig *RunSession_Config(void) { return &config; }
static RunState *RunSession_State(void) { return &run; }
static u16 RunSession_Zone(void) { return 7; }
static BOOL RunState_GetMon(RunState *state, u16 id, RunMonRecord *record)
{ (void)state; if (id != recordID) return FALSE; record->personality=recordPID; record->trainerID=recordOT; return TRUE; }
static BOOL RunState_ExhaustMon(RunState *state, u16 id, u16 zone)
{ assert(id == recordID && zone == 7); state->dead = 1; deaths++; return TRUE; }
static u32 BoxPokemon_GetValue(BoxPokemon *mon, enum PokemonDataParam param, void *dest)
{
 (void)dest;
 switch(param) {
 case MON_DATA_UNUSED_113: return mon->marker;
 case MON_DATA_UNUSED_114: return mon->id;
 case MON_DATA_OT_ID: return mon->otID;
 case MON_DATA_IS_EGG: return mon->isEgg;
 case MON_DATA_SPECIES: return mon->species;
 default: assert(0); return 0;
 }
}
static void BoxPokemon_SetValue(BoxPokemon *mon, enum PokemonDataParam param, const void *value)
{ assert(param == MON_DATA_UNUSED_113); mon->marker = *(const u8 *)value; }
static BoxPokemon *Pokemon_GetBoxPokemon(Pokemon *mon) { return &mon->box; }
static u32 Pokemon_GetValue(Pokemon *mon, enum PokemonDataParam param, void *dest)
{
 (void)dest;
 if (param == MON_DATA_HP) return mon->party.hp;
 if (param == MON_DATA_MAX_HP) return mon->party.maxHP;
 if (param == MON_DATA_EXPERIENCE) return 0;
 return BoxPokemon_GetValue(&mon->box, param, dest);
}
static void Pokemon_SetDataInternal(Pokemon *mon, enum PokemonDataParam param, const void *value)
{
 switch(param) {
 case MON_DATA_HP: mon->party.hp = *(const u16 *)value; break;
 case MON_DATA_MAX_HP: mon->party.maxHP = *(const u16 *)value; break;
 case MON_DATA_STATUS: mon->party.status = *(const u32 *)value; break;
 case MON_DATA_BALL_CAPSULE_ID: mon->party.ballCapsuleID = *(const u8 *)value; break;
 case MON_DATA_MAIL: case MON_DATA_BALL_CAPSULE: break;
 default: assert(0);
 }
}
static void Pokemon_DecryptData(void *data, u32 size, u32 key) { (void)data; (void)size; (void)key; }
static void Pokemon_EncryptData(void *data, u32 size, u32 key) { (void)data; (void)size; (void)key; }
static u16 Pokemon_GetDataChecksum(const void *data, u32 size) { (void)data; (void)size; checksumChecks++; return 0xCAFE; }
static u32 RunGame_Experience(BoxPokemon *mon, u32 current, u32 award) { (void)mon; return current + award; }
static void RunGame_Sync(BoxPokemon *mon) { (void)mon; }
static Mail *Mail_New(int heap) { (void)heap; return &mail; }
static void Heap_Free(void *ptr) { assert(ptr == &mail); }
static void MI_CpuClearFast(void *ptr, u32 size) { memset(ptr, 0, size); }
static void BoxPokemon_RestorePP(BoxPokemon *mon) { (void)mon; }
static void BoxPokemon_SetShayminForm(BoxPokemon *mon, int form) { (void)mon; (void)form; }
static void SaveData_SetFullSaveRequired(void) { fullSaves++; }
static BOOL RunGame_Exhausted(BoxPokemon *mon);
static void RunGame_Exhaust(Pokemon *mon);
static u16 RunGame_FilterHP(Pokemon *mon, u16 hp);
void Pokemon_SetValue(Pokemon *mon, enum PokemonDataParam param, const void *value);
static void Pokemon_CalcLevelAndStats(Pokemon *mon)
{
 u16 maxHP = 42, hp = mon->party.hp;
 if (hp || mon->party.maxHP == 0) hp = hp ? hp + maxHP - mon->party.maxHP : maxHP;
 Pokemon_SetValue(mon, MON_DATA_MAX_HP, &maxHP);
 if (hp) Pokemon_SetValue(mon, MON_DATA_HP, &hp);
}
''' + actual + r'''
static void fresh(BoxPokemon *mon, int decrypted, int dead)
{
 memset(mon, 0, sizeof(*mon));
 mon->personality = recordPID = 0x12345678u;
 mon->otID = recordOT = 0x98765432u;
 mon->species = 25; mon->id = recordID;
 mon->marker = RUN_MON_TRACKED | (dead ? RUN_MON_DEAD : 0);
 mon->boxDecrypted = decrypted;
 mon->checksum = 0xCAFE;
 run.dead = dead; deaths = 0; checksumChecks = 0;
}
int main(void)
{
 const u16 staleHP[] = { 0, 1, 42, 12345, 65535 };
 for (int mode=0; mode<4; mode++) {
  configured = mode != 0; config.flags = mode == 1 ? 0 : RUN_PERMADEATH;
  sSuspended = mode == 3;
  for (int decrypted=0; decrypted<2; decrypted++) for(int dead=0; dead<2; dead++) {
   for(u32 old=0; old<sizeof(staleHP)/sizeof(staleHP[0]); old++) {
    BoxPokemon source, before;
    fresh(&source, decrypted, dead); before=source;
    memset(&scratch, 0xA5, sizeof(scratch)); scratch.party.hp=staleHP[old];
    Pokemon_FromBoxPokemon(&source, &scratch);
    BOOL enforce = configured && !sSuspended && (config.flags & RUN_PERMADEATH);
    if (scratch.party.hp != (enforce && dead ? 0 : 42) || deaths || run.dead != dead) {
     fprintf(stderr,"FAIL box conversion: mode=%d decrypted=%d dead=%d staleHP=%u hp=%u deaths=%d record=%u\n",mode,decrypted,dead,staleHP[old],scratch.party.hp,deaths,run.dead);
     return 1;
    }
    assert(memcmp(&source,&before,sizeof(source))==0);
    assert(decrypted || checksumChecks >= 3);
    assert(scratch.box.marker == source.marker && scratch.box.id == source.id);
    assert(scratch.box.personality == source.personality && scratch.box.otID == source.otID);
    assert(scratch.box.boxDecrypted == source.boxDecrypted && scratch.box.partyDecrypted == decrypted);
   }
  }
 }
 configured=TRUE; config.flags=RUN_PERMADEATH; sSuspended=FALSE;
 for(u32 box=0; box<MAX_PC_BOXES; box++) {
  BoxPokemon source, before;
  fresh(&source, FALSE, FALSE); before=source;
  memset(&boxes,0,sizeof(boxes)); boxes.currentBoxID=box;
  assert(PCBoxes_TryStoreBoxMon(&boxes,&source));
  assert(PCBoxes_TryStoreBoxMonAt(&boxes,box,1,&source));
  for(int cycle=0; cycle<3; cycle++) for(int slot=0; slot<2; slot++) {
   scratch.party.hp=12345;
   Pokemon_FromBoxPokemon(&boxes.boxMons[box][slot],&scratch);
   assert(scratch.party.hp == 42 && !deaths && !run.dead);
   assert(memcmp(&boxes.boxMons[box][slot],&before,sizeof(before))==0);
  }
  Pokemon_FromBoxPokemon(&source,&scratch);
  u16 zero=0, heal=42;
  Pokemon_SetValue(&scratch,MON_DATA_HP,&zero);
  assert(deaths==1 && run.dead && (scratch.box.marker & RUN_MON_DEAD) && !scratch.party.hp);
  Pokemon_SetValue(&scratch,MON_DATA_HP,&heal);
  assert(!scratch.party.hp);
  assert(PCBoxes_TryStoreBoxMonAt(&boxes,box,2,&scratch.box));
  Pokemon_FromBoxPokemon(&boxes.boxMons[box][2],&scratch);
  assert(!scratch.party.hp && run.dead && deaths==1);
 }
 puts("PASS: actual box conversion/HP hooks, stale preview buffers, both decryption modes, permadeath OFF/ON/suspended");
 puts("PASS: real automatic/manual PC storage functions across all 18 boxes and repeated previews; live boxes unchanged, real deaths remain locked");
 return 0;
}
'''
    # The extracted functions keep their real public linkage.
    source = source.replace("static BOOL RunGame_Exhausted(BoxPokemon *mon);", "BOOL RunGame_Exhausted(BoxPokemon *mon);")
    source = source.replace("static void RunGame_Exhaust(Pokemon *mon);", "void RunGame_Exhaust(Pokemon *mon);")
    source = source.replace("static u16 RunGame_FilterHP(Pokemon *mon, u16 hp);", "u16 RunGame_FilterHP(Pokemon *mon, u16 hp);")
    directory = ROOT / "artifacts/box-permadeath-tests"
    directory.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="case-", dir=directory) as temporary:
        work = Path(temporary)
        test, exe = work / "box.c", work / "box.exe"
        test.write_text(source, encoding="utf-8")
        subprocess.run([compiler, "-std=c99", "-Wall", "-Wextra", "-Werror", "-O2", str(test), "-o", str(exe)], check=True)
        subprocess.run([str(exe)], check=True)
    print("NOT TESTED: native crypto/stat data, real flash or ARM/emulator execution")


if __name__ == "__main__":
    main()
