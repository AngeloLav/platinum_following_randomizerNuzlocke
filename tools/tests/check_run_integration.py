#!/usr/bin/env python3
"""Lightweight source/resource checks; does not build a ROM or run an emulator."""

import ast
import json
import re
import struct
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    symbols = set()
    glyphs = {line[5:] for line in (ROOT / "tools/msgenc/charmap.txt").read_text(encoding="utf-8").splitlines()
              if re.match(r"^[0-9A-Fa-f]{4}=.$", line)} | {"\n", "\r"}
    for name in ("run_engine", "battle_strings", "box_messages", "common_strings",
                 "day_care_common", "start_menu", "battle_party", "eterna_city_south_house"):
        path = ROOT / "res/text" / (name + ".json")
        data = json.loads(path.read_text(encoding="utf-8-sig"))
        ids = [message["id"] for message in data["messages"]]
        assert len(ids) == len(set(ids)), f"Duplicate message ID in {path}"
        symbols.update(ids)
        for message in data["messages"]:
            if re.match(r"Run_Text_|CommonStrings_Text_Run|BoxText_Run|DayCareCommon_Text_Run|BattleStrings_Text_Run|BattleParty_Text_Run|EternaCitySouthHouse_Text_Run|StartMenu_Text_Run", message["id"]):
                value = message["en_US"]
                value = "".join(value) if isinstance(value, list) else value
                assert set(value) <= glyphs, f"Unsupported native glyphs in {message['id']}: {set(value) - glyphs}"
    for path in list((ROOT / "src").rglob("*.c")) + list((ROOT / "res/field/scripts").glob("*.s")):
        references = set(re.findall(r"\b(?:Run_Text_\w+|CommonStrings_Text_Run\w+|BoxText_Run\w+|DayCareCommon_Text_Run\w+|BattleStrings_Text_Run\w+|BattleParty_Text_Run\w+|EternaCitySouthHouse_Text_Run\w+|StartMenu_Text_Run\w+)\b", path.read_text(encoding="utf-8")))
        assert references <= symbols, f"Missing localized text: {path}: {references - symbols}"
    common = (ROOT / "res/field/scripts/scripts_common.s").read_text()
    entries = re.findall(r"^\s+ScriptEntry\s+(\w+)", common, re.M)
    assert entries[58:61] == ["CommonScript_RunEnded", "CommonScript_RunEndedSaveFailed", "CommonScript_RunGiftDenied"]
    assert entries[61] == "CommonScript_RunActivated"
    script_table = (ROOT / "include/data/scripts/scrcmd.h").read_text()
    commands = re.findall(r"ScriptCommand\((\w+),", script_table)
    assert commands[-1] == "SCRCMD_GETRUNRULE" and len(commands) == len(set(commands))
    assert "POKETCH_APPID_RUN_TRACKER\nPOKETCH_APPID_MAX" in (ROOT / "generated/poketch_apps.txt").read_text()
    assert "BATTLE_SUBSCRIPT_RUN_CAPTURE_DENIED" in (ROOT / "generated/battle_subscripts.txt").read_text()
    assert (ROOT / "res/battle/scripts/subscripts/sub_seq.order").read_text().strip().endswith("subscript_run_capture_denied")
    for name in ("run_species.py", "run_world.py"):
        ast.parse((ROOT / "tools/datagen" / name).read_text())
    charmap = {line[5:]: int(line[:4], 16) for line in
               (ROOT / "tools/msgenc/charmap.txt").read_text(encoding="utf-8").splitlines()
               if re.match(r"^[0-9A-Fa-f]{4}=.$", line)}
    widths = json.loads((ROOT / "res/fonts/font_system.json").read_text())["glyphWidths"]
    run_text = json.loads((ROOT / "res/text/run_engine.json").read_text(encoding="utf-8"))["messages"]
    run_ids = [m["id"] for m in run_text]
    battle_text = json.loads((ROOT / "res/text/battle_strings.json").read_text(encoding="utf-8-sig"))["messages"]
    for message in battle_text:
        if message["id"] in ("BattleStrings_Text_RunCatchAllowed", "BattleStrings_Text_RunCatchBlocked", "BattleStrings_Text_RunOneFoe"):
            assert sum(widths[charmap[c] - 1] for c in message["en_US"]) <= 72, "Catch flag exceeds Bag panel"
    assert sum(widths[charmap[c] - 1] for c in "NO BTL") <= 39, "Refusal mark exceeds smallest party level window"
    for group in (("RateOriginal", "RateIncreased", "RateHigh", "RateVeryHigh"),
                  ("NavRun", "NavAreas", "NavHistory", "NavMemorial", "NavSpecial"),
                  ("Classic", "Randomizer", "Nuzlocke", "Hardcore", "Custom")):
        at = run_ids.index("Run_Text_" + group[0])
        assert run_ids[at:at + len(group)] == ["Run_Text_" + x for x in group], "Arithmetic message IDs must remain contiguous"
    for message in run_text:
        name, value = message["id"], message["en_US"]
        assert len(value) < 512, f"Message exceeds run UI scratch capacity: {name}"
        if name.startswith("Run_Text_Help") or name.startswith("Run_Text_Nav") or name == "Run_Text_ConfigNav":
            assert len(value.splitlines()) <= 8, f"Help height exceeds lower-screen window: {name}"
            for line in value.splitlines():
                pixels = sum(widths[charmap[c] - 1] for c in line)
                assert pixels <= 224, f"Help text clips at native font width: {name}: {pixels}px: {line}"
        if name in ("Run_Text_Controls", "Run_Text_ReviewControls", "Run_Text_ReadOnly", "Run_Text_SeedHelp"):
            assert len(value.splitlines()) <= 2
            assert max(sum(widths[charmap[c] - 1] for c in line) for line in value.splitlines()) <= 216
    assert "appRegistry[POKETCH_REGISTRY_SIZE]" in (ROOT / "include/poketch.h").read_text()
    message_widths = json.loads((ROOT / "res/fonts/font_message.json").read_text())["glyphWidths"]
    common_messages = json.loads((ROOT / "res/text/common_strings.json").read_text(encoding="utf-8-sig"))["messages"]
    for message in common_messages:
        if message["id"] in ("CommonStrings_Text_RunActivatedOne", "CommonStrings_Text_RunActivatedMultiple"):
            for page in message["en_US"].split("\r"):
                assert len(page.splitlines()) <= 2
                assert all(sum(message_widths[charmap[c] - 1] for c in line) <= 224 for line in page.splitlines()), message["id"]
    compiled_starter = ROOT / "build/res/field/scripts/scr_seq.narc.p/scripts_route_201"
    if compiled_starter.exists():
        raw = compiled_starter.read_bytes()
        opcode = commands.index("SCRCMD_GIVEPOKEMON")
        pattern = struct.pack("<4H", opcode, 0x8000, 5, 0)
        assert raw.count(pattern) == 1 and raw.index(pattern) + 2 == 0x045C, "Review pinned legacy starter key"
    print("PASS: native glyphs, font-measured help/footer bounds, localized references, script IDs, appended IDs, generator syntax")
    print("NOT TESTED: ARM execution, graphics, real flash writes, emulator gameplay")


if __name__ == "__main__":
    main()
