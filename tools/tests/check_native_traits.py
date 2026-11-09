#!/usr/bin/env python3
"""Check the real native type/ability getters against the Following-only base.

Host tests only: no ROM or save file is modified.
"""
import argparse
import json
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
BASE = "8f7ccec26e697e16df4fe4155915fbf28a038310"


def git(*args, **kwargs):
    return subprocess.run(["git", *args], cwd=ROOT, check=True, capture_output=True, **kwargs).stdout


def function(source, signature):
    start = source.index(signature + "\n{")
    depth, end = 1, source.index("{", start) + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


def compact(source):
    return re.sub(r"\s+", "", re.sub(r"//[^\n]*|/\*.*?\*/", "", source, flags=re.S))


def named_function(source, name):
    match = re.search(r"^(?:static )?(?:BOOL|void|int|s32|u32|u16|u8) " + name + r"\([^;{}]*\)\n\{", source, re.M)
    assert match, name
    return function(source, match[0][:-2])


def cases(source):
    getter = function(source, "static u32 BoxPokemon_GetDataInternal(BoxPokemon *boxMon, enum PokemonDataParam param, void *dest)")
    ability = "case MON_DATA_ABILITY:" + getter.split("case MON_DATA_ABILITY:", 1)[1].split("case MON_DATA_MARKINGS:", 1)[0]
    types = "case MON_DATA_TYPE_1:" + getter.split("case MON_DATA_TYPE_1:", 1)[1].split("case MON_DATA_SPECIES_NAME:", 1)[0]
    return ability + types


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cc", default="gcc")
    compiler = shutil.which(parser.parse_args().cc)
    assert compiler, "C99 compiler required"
    pokemon = (ROOT / "src/pokemon.c").read_text(encoding="utf-8")
    baseline = git("show", BASE + ":src/pokemon.c").decode("utf-8")
    assert compact(cases(pokemon)) == compact(cases(baseline)), "Native type/ability getter differs from Following-only base"
    for signature in ["static BOOL CanBoxPokemonLearnTM(BoxPokemon *boxMon, u8 tmID)",
                      "static void BoxPokemon_CalcAbility(BoxPokemon *boxMon)",
                      "static void BoxPokemon_SetDefaultMoves(BoxPokemon *boxMon)",
                      "u16 Pokemon_LevelUpMove(Pokemon *mon, int *index, u16 *moveID)"]:
        assert compact(function(pokemon, signature)) == compact(function(baseline, signature)), signature
    trainer = (ROOT / "src/trainer_data.c").read_text(encoding="utf-8")
    original_trainer = git("show", BASE + ":src/trainer_data.c").decode("utf-8")
    signature = "static void TrainerData_BuildParty(FieldBattleDTO *dto, int battler, enum HeapID heapID)"
    trainer_function = function(trainer, signature).replace(
        "    RunGame_AdjustTrainerParty(dto->parties[battler], dto->trainerIDs[battler], battler, dto->battleType);\n", "")
    assert compact(trainer_function) == compact(function(original_trainer, signature)), "Trainer construction differs beyond run hook"
    for token in ("ApplyCustomType", "ApplyAbilityDonor", "GetAbilityDonorSpecies", "ChampionMonBuild",
                  "TypeLevelUpMove *", "TYPE_LEVEL_UP_MOVE_INDEX_BASE", "Pokemon_TypeLevelUpMove"):
        assert token not in pokemon + trainer, token
    print("PASS: getters, ability calculation, TM permissions and trainer construction match Following-only base (run hook retained)")
    route = (ROOT / "res/field/scripts/scripts_route_202.s").read_text(encoding="utf-8")
    native_route = git("show", BASE + ":res/field/scripts/scripts_route_202.s").decode("utf-8")
    def tutorial_gift(script):
        return script.split("Route202_GivePokeballs:", 1)[1].split("GetPlayerGender", 1)[0]
    assert compact(tutorial_gift(route)) == compact(tutorial_gift(native_route))
    assert "SetVar VAR_0x8005, 5\n" in tutorial_gift(route)
    print("PASS: shared Dawn/Lucas tutorial gift restores the native five Poke Balls")

    paths = [p for p in git("ls-tree", "-r", "--name-only", BASE, "res/pokemon", "res/moves", "res/items", "res/trainers").decode().splitlines() if p.endswith(".json")]
    batch = git("cat-file", "--batch", input="".join(BASE + ":" + p + "\n" for p in paths).encode())
    offset, records = 0, []
    for path in paths:
        end = batch.index(b"\n", offset)
        header = batch[offset:end].split()
        assert header[1] == b"blob", path
        size = int(header[2])
        native = json.loads(batch[end + 1:end + 1 + size])
        current = json.loads((ROOT / path).read_text(encoding="utf-8"))
        assert current == native, "Non-native gameplay data: " + path
        if path.startswith("res/pokemon/") and path.endswith("/data.json"):
            records.append((path, current))
        offset = end + size + 2
    print(f"PASS: all {len(paths)} Pokemon/move/item/trainer JSON definitions match Following-only base, including {len(records)} species/forms")
    for path in ["src/battle/trainer_ai/trainer_ai.c", "src/move_reminder_data.c",
                 "src/applications/pokemon_summary_screen/main.c", "src/overlay007/shop_menu.c",
                 "include/struct_defs/species.h"]:
        assert compact((ROOT / path).read_text(encoding="utf-8")) == compact(git("show", BASE + ":" + path).decode("utf-8")), path
    for number in ("0041", "0087", "0088", "0130"):
        path = "res/battle/scripts/effects/effect_script_" + number + ".s"
        assert compact((ROOT / path).read_text(encoding="utf-8")) == compact(git("show", BASE + ":" + path).decode("utf-8")), path
    print("PASS: original trainer AI, move reminder, summary HM protection, shop, learnset capacity and fixed-damage scripts")
    for path, names_to_check in [
        ("src/battle/battle_lib.c", ["BattleSystem_CompareBattlerSpeed", "BasicTypeMulApplies",
         "BattleSystem_TriggerFormChange", "BattleSystem_CalcMoveDamage", "BattleSystem_CalcCriticalMulti", "Battler_MonType"]),
        ("src/battle/battle_script.c", ["BtlCmd_EndOfTurnWeatherEffect", "BtlCmd_GenerateEndOfBattleItem", "BattleScript_CalcCatchShakes"]),
        ("src/battle_sub_menus/battle_party.c", ["CheckSelectedMoveIsHM"]),
    ]:
        actual = (ROOT / path).read_text(encoding="utf-8")
        original = git("show", BASE + ":" + path).decode("utf-8")
        for name in names_to_check:
            assert compact(named_function(actual, name)) == compact(named_function(original, name)), (path, name)
    daycare_path = "src/overlay005/daycare.c"
    daycare = (ROOT / daycare_path).read_text(encoding="utf-8")
    original_daycare = git("show", BASE + ":" + daycare_path).decode("utf-8")
    stripped = daycare.replace('#include "run_game.h"\n', '').replace('#include "run_session.h"\n', '')
    stripped = stripped.replace('    if (RunGame_Exhausted(Pokemon_GetBoxPokemon(Party_GetPokemonBySlotIndex(party, partySlot)))) return;\n', '')
    stripped = stripped.replace('experience = RunGame_Experience(Pokemon_GetBoxPokemon(mon), experience, DaycareMon_GetSteps(daycareMon));', 'experience += DaycareMon_GetSteps(daycareMon);')
    stripped = stripped.replace('exp = RunGame_Experience(boxMonRef, exp, givenExp);', 'exp += givenExp;')
    stripped = re.sub(r'    RunGame_ApplyShiny\((egg|mon)\);\n', '', stripped)
    stripped = stripped.replace('    if (!RunGame_DayCareEggs()) return;\n', '')
    stripped = re.sub(r'    if \(RunSession_Configured\(\)\) \{.*?\n    }\n', '', stripped, flags=re.S)
    stripped = stripped.replace('if (RunGame_DayCareEggs() && (Daycare_HasEgg(daycare) == FALSE)', 'if ((Daycare_HasEgg(daycare) == FALSE)')
    assert compact(stripped) == compact(original_daycare), 'Non-native breeding changes beyond the explicit run hooks'
    assert 'GiveEgg SPECIES_TOGEPI, SPECIAL_METLOC_NAME_CYNTHIA' in (ROOT / 'res/field/scripts/scripts_eterna_city.s').read_text(encoding='utf-8')
    print("PASS: native damage, immunities, critical hits, speed items, capture odds and breeding; only explicit run hooks retained")

    def names(roster):
        return (ROOT / "generated" / (roster + ".txt")).read_text(encoding="utf-8").splitlines()
    species, types, abilities = names("species"), names("pokemon_types"), names("abilities")
    rows = []
    for index, (path, record) in enumerate(records):
        values = [species.index("SPECIES_" + path.split("/")[2].upper()), index % 32]
        values += [types.index(t) for t in record["types"]]
        values += [abilities.index(a) for a in record["abilities"]]
        rows.append("{ " + ", ".join(map(str, values)) + " },")
    fixture = "static const unsigned short fixtures[][6] = {\n" + "\n".join(rows) + "\n};\n"
    declarations = "\n".join("enum { " + ", ".join(roster) + " };" for roster in (species, types, abilities))
    source_text = '#include <stdint.h>\n#include <assert.h>\n#include <stdio.h>\n' + declarations + r'''
typedef uint32_t u32;
enum { MON_DATA_ABILITY, MON_DATA_TYPE_1, MON_DATA_TYPE_2 };
enum { SPECIES_DATA_TYPE_1, SPECIES_DATA_TYPE_2 };
enum { ITEM_PARAM_HOLD_EFFECT, HEAP_ID_SYSTEM };
typedef struct { u32 species, ability, heldItem; } PokemonDataBlockA;
typedef struct { u32 form, hpIV, speedIV, defIV, spDefIV, isEgg; } PokemonDataBlockB;
static u32 expectedSpecies, expectedForm, plateType;
static const unsigned short *entry;
static u32 SpeciesData_GetFormValue(u32 species, u32 form, u32 attribute)
{
    assert(species == expectedSpecies && form == expectedForm);
    assert(attribute == SPECIES_DATA_TYPE_1 || attribute == SPECIES_DATA_TYPE_2);
    return entry[2 + attribute];
}
static u32 Item_LoadParam(u32 item, u32 param, u32 heap)
{
    assert(param == ITEM_PARAM_HOLD_EFFECT && heap == HEAP_ID_SYSTEM);
    return item;
}
static u32 Pokemon_GetArceusTypeOf(u32 effect) { (void)effect; return plateType; }
static u32 ReadTraits(PokemonDataBlockA *monDataBlockA, PokemonDataBlockB *monDataBlockB, int param)
{
    u32 result = 0;
    switch (param) {
''' + cases(pokemon) + r'''
    default: assert(0); break;
    }
    return result;
}
''' + fixture + r'''
int main(void)
{
    PokemonDataBlockA a = { 0, 0, 0 };
    PokemonDataBlockB b = { 0, 0, 0, 0, 0, 0 };
    plateType = TYPE_NORMAL;
    for (u32 row = 0; row < sizeof(fixtures) / sizeof(fixtures[0]); row++) {
        entry = fixtures[row];
        a.species = expectedSpecies = entry[0];
        b.form = expectedForm = entry[1];
        for (u32 slot = 0; slot < 2; slot++) {
            a.ability = entry[4 + (slot && entry[5] != ABILITY_NONE)];
            for (u32 hp = 0; hp < 32; hp++) {
                for (u32 speed = 0; speed < 32; speed++) {
                    b.hpIV = hp; b.speedIV = speed; b.defIV = speed; b.spDefIV = hp;
                    b.isEgg = hp & 1;
                    assert(ReadTraits(&a, &b, MON_DATA_TYPE_1) == entry[2]);
                    assert(ReadTraits(&a, &b, MON_DATA_TYPE_2) == entry[3]);
                    assert(ReadTraits(&a, &b, MON_DATA_ABILITY) == a.ability);
                }
            }
        }
    }
    a.species = SPECIES_ARCEUS;
    a.ability = ABILITY_MULTITYPE;
    for (u32 type = 0; type < NUM_POKEMON_TYPES; type++) {
        plateType = type;
        assert(ReadTraits(&a, &b, MON_DATA_TYPE_1) == type);
        assert(ReadTraits(&a, &b, MON_DATA_TYPE_2) == type);
    }
    puts("PASS: actual getter cases preserve species/form types and stored abilities for every IV pair, both ability slots and eggs; Arceus Multitype affects both types");
    return 0;
}
'''
    directory = ROOT / "artifacts/native-traits-tests"
    directory.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="case-", dir=directory) as temporary:
        work = Path(temporary)
        source, executable = work / "native.c", work / "native.exe"
        source.write_text(source_text, encoding="utf-8")
        subprocess.run([compiler, "-std=c99", "-pedantic", "-Wall", "-Wextra", "-Werror", "-O2", str(source), "-o", str(executable)], check=True)
        subprocess.run([str(executable)], check=True)
    print("NOT TESTED: ARM battles, save loading or emulator gameplay")


if __name__ == "__main__":
    main()
