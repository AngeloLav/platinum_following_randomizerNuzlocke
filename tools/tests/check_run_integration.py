#!/usr/bin/env python3
"""Lightweight source/resource checks; does not build a ROM or run an emulator."""

import ast
import json
import re
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    symbols = set()
    glyphs = {line[5:] for line in (ROOT / "tools/msgenc/charmap.txt").read_text(encoding="utf-8").splitlines()
              if re.match(r"^[0-9A-Fa-f]{4}=.$", line)} | {"\n", "\r"}
    for name in ("run_engine", "battle_strings", "box_messages", "common_strings",
                 "day_care_common", "start_menu"):
        path = ROOT / "res/text" / (name + ".json")
        data = json.loads(path.read_text(encoding="utf-8-sig"))
        ids = [message["id"] for message in data["messages"]]
        assert len(ids) == len(set(ids)), f"Duplicate message ID in {path}"
        symbols.update(ids)
        for message in data["messages"]:
            if re.match(r"Run_Text_|CommonStrings_Text_Run|BoxText_Run|DayCareCommon_Text_Run|BattleStrings_Text_Run|StartMenu_Text_Run", message["id"]):
                value = message["en_US"]
                value = "".join(value) if isinstance(value, list) else value
                assert set(value) <= glyphs, f"Unsupported native glyphs in {message['id']}: {set(value) - glyphs}"
    for path in list((ROOT / "src").rglob("*.c")) + list((ROOT / "res/field/scripts").glob("*.s")):
        references = set(re.findall(r"\b(?:Run_Text_\w+|CommonStrings_Text_Run\w+|BoxText_Run\w+|DayCareCommon_Text_Run\w+|BattleStrings_Text_Run\w+|StartMenu_Text_Run\w+)\b", path.read_text(encoding="utf-8")))
        assert references <= symbols, f"Missing localized text: {path}: {references - symbols}"
    common = (ROOT / "res/field/scripts/scripts_common.s").read_text()
    entries = re.findall(r"^\s+ScriptEntry\s+(\w+)", common, re.M)
    assert entries[58:61] == ["CommonScript_RunEnded", "CommonScript_RunEndedSaveFailed", "CommonScript_RunGiftDenied"]
    script_table = (ROOT / "include/data/scripts/scrcmd.h").read_text()
    commands = re.findall(r"ScriptCommand\((\w+),", script_table)
    assert commands[-1] == "SCRCMD_GETRUNRULE" and len(commands) == len(set(commands))
    assert "POKETCH_APPID_RUN_TRACKER\nPOKETCH_APPID_MAX" in (ROOT / "generated/poketch_apps.txt").read_text()
    assert "BATTLE_SUBSCRIPT_RUN_CAPTURE_DENIED" in (ROOT / "generated/battle_subscripts.txt").read_text()
    assert (ROOT / "res/battle/scripts/subscripts/sub_seq.order").read_text().strip().endswith("subscript_run_capture_denied")
    for name in ("run_species.py", "run_world.py"):
        ast.parse((ROOT / "tools/datagen" / name).read_text())
    assert "appRegistry[POKETCH_REGISTRY_SIZE]" in (ROOT / "include/poketch.h").read_text()
    print("PASS: native text glyphs, localized references, script IDs, appended command/app/subscript IDs, generator syntax")
    print("NOT TESTED: ARM execution, graphics, real flash writes, emulator gameplay")


if __name__ == "__main__":
    main()
