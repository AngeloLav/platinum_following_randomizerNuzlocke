#!/usr/bin/env python3
"""Compile the actual native shiny predicate, PID promotion and palette hooks.

This is a host integration test, not an emulator test. No ROM or save is changed.
"""
import argparse
import re
import shutil
import subprocess
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def function(source, signature):
    start = source.index(signature + "\n{")
    opening = source.index("{", start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cc", default="gcc")
    args = parser.parse_args()
    compiler = shutil.which(args.cc)
    assert compiler, "C99 compiler is required: " + args.cc
    pokemon = (ROOT / "src/pokemon.c").read_text(encoding="utf-8")
    extras = (ROOT / "src/run_extras.c").read_text(encoding="utf-8")
    follower = function((ROOT / "src/follower_mon.c").read_text(encoding="utf-8"),
                        "static u32 FollowerMon_GetHGBrightnessPercent(u16 graphicsID, BOOL shiny)")
    constants = (ROOT / "include/constants/pokemon.h").read_text(encoding="utf-8")
    odds = re.search(r"^#define SHINY_ODDS\s+(\d+)\b", constants, re.M)[1]
    native = function(pokemon, "static inline BOOL Pokemon_InlineIsPersonalityShiny(u32 monOTID, u32 monPersonality)")
    promotion = function(extras, "static u8 Unown(u32 p)") + "\n" + function(extras, "u32 RunExtras_ShinyPID(u32 personality, u32 trainerID, u8 rate, u8 constraint)")
    directory = ROOT / "artifacts/shiny-native-tests"
    directory.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="case-", dir=directory) as temporary:
        work = Path(temporary)
        (work / "nitro").mkdir()
        (work / "nitro/types.h").write_text(
            "#ifndef TEST_NITRO_TYPES_H\n#define TEST_NITRO_TYPES_H\n#include <stdint.h>\ntypedef uint8_t u8; typedef int8_t s8;\n"
            "typedef uint16_t u16; typedef uint32_t u32; typedef int32_t s32;\n"
            "typedef int BOOL;\n#define TRUE 1\n#define FALSE 0\n#endif\n", encoding="ascii")
        (work / "nitro.h").write_text('#include "nitro/types.h"\n', encoding="ascii")
        test = work / "native.c"
        test.write_text('#include <nitro/types.h>\n#include <stdio.h>\n#include <string.h>\n'
                        '#include "pokemon_color_mod.h"\n#define SHINY_ODDS ' + odds + "\n"
                        + native + "\n" + promotion + "\n" + follower + r'''
int main(void)
{
    const u32 trainers[] = { 0, 0x12345678u, 0xFFFFFFFFu, 0x87654321u };
    const u32 personalities[] = { 0, 0x12345678u, 0xFFFFFFFFu, 0xDEADBEEFu };
    s8 offsets[16];
    int errors = 0;
    for (u32 graphics = 0; graphics < 65536; graphics++) {
        if (FollowerMon_GetHGBrightnessPercent((u16)graphics, FALSE) != 100
            || FollowerMon_GetHGBrightnessPercent((u16)graphics, TRUE) != 100) errors++;
    }
    for (u32 t = 0; t < 4; t++) {
        for (u8 rate = 0; rate < 5; rate++) {
            u32 count = 0;
            for (u32 x = 0; x < 65536; x++) {
                u32 low = 0xA5C3u;
                u32 high = low ^ (trainers[t] >> 16) ^ (trainers[t] & 65535u) ^ x;
                u32 pid = low | (high << 16);
                u32 adjusted = RunExtras_ShinyPID(pid, trainers[t], rate, 0);
                BOOL shiny = Pokemon_InlineIsPersonalityShiny(trainers[t], adjusted);
                count += shiny;
                if (shiny != (x < (8u << rate))) errors++;
            }
            if (count != (8u << rate)) {
                fprintf(stderr, "FAIL native rate %u OT %08X: %u/65536, expected %u/65536\n",
                        rate, trainers[t], count, 8u << rate);
                errors++;
            }
        }
    }
    for (int i = 0; i < 16; i++) offsets[i] = (s8)(i * 13 - 100);
    for (u32 p = 0; p < 4; p++) {
        for (u32 base = 0; base < 65536; base += 16) {
            u16 original[16], plain[16], shifted[16];
            for (int i = 0; i < 16; i++) original[i] = (u16)(base + i);
            memcpy(plain, original, sizeof(original));
            memcpy(shifted, original, sizeof(original));
            PersonalityRotatePalette(plain, 16, personalities[p]);
            PersonalityRotatePaletteWithHueOffsets(shifted, 16, personalities[p], offsets);
            if (memcmp(original, plain, sizeof(original)) || memcmp(original, shifted, sizeof(original))) {
                errors++;
                if (base == 0) fprintf(stderr, "FAIL palette modified for PID %08X\n", personalities[p]);
            }
        }
    }
    if (errors) return 1;
    puts("PASS: actual native shiny predicate + PID promotion: all 65536 XORs, five rates, four OT IDs");
    puts("PASS: both palette hooks preserve all 65536 color words, transparency and four PIDs byte for byte");
    puts("PASS: raw follower palette brightness preserved for all graphic IDs and both shiny states");
    return 0;
}
''', encoding="utf-8")
        executable = work / "native.exe"
        subprocess.run([compiler, "-std=c99", "-pedantic", "-Wall", "-Wextra", "-Werror", "-O2",
                        "-I", str(work), "-I", str(ROOT / "include"), str(test),
                        str(ROOT / "src/pokemon_color_mod.c"), "-o", str(executable)], check=True, cwd=ROOT)
        subprocess.run([str(executable)], check=True, cwd=ROOT)
    summary = (ROOT / "src/applications/pokemon_summary_screen/sprites.c").read_text(encoding="utf-8")
    assert "summaryScreen->monData.isEgg == FALSE && summaryScreen->monData.isShiny" in summary
    display = (ROOT / "src/battle/battle_display.c").read_text(encoding="utf-8")
    assert len(re.findall(r"(?:monEncounterData|monShowData)->isShiny = message->isShiny;", display)) == 3
    assert "} while (Pokemon_IsPersonalityShiny(monOTID, monPersonality));" in pokemon
    print("PASS: native shiny summary icon, three battle sparkle paths and non-shiny OT constructor wired")


if __name__ == "__main__":
    main()
