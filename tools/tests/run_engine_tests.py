#!/usr/bin/env python3
"""Compile and exercise the actual portable C modules; no ROM/save file is opened."""

import argparse
import importlib.util
import json
import shutil
import struct
import subprocess
import sys
import tempfile
import zlib
from pathlib import Path


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cc", default="gcc")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    compiler = shutil.which(args.cc)
    if compiler is None:
        raise SystemExit(f"C99 compiler not found: {args.cc}")

    build = root / "build/run-engine-tests"
    build.mkdir(parents=True, exist_ok=True)
    spec = importlib.util.spec_from_file_location("run_species", root / "tools/datagen/run_species.py")
    generator = importlib.util.module_from_spec(spec)
    sys.dont_write_bytecode = True
    spec.loader.exec_module(generator)
    generated = generator.generate(root)
    assert generated == generator.generate(root), "Nondeterministic data generator"
    world_spec = importlib.util.spec_from_file_location("run_world", root / "tools/datagen/run_world.py")
    world_generator = importlib.util.module_from_spec(world_spec)
    world_spec.loader.exec_module(world_generator)
    world_generated = world_generator.generate(root)

    # Confine all temporary files and automatic cleanup to the ignored test directory.
    with tempfile.TemporaryDirectory(prefix="case-", dir=build) as temporary:
        work = Path(temporary).resolve()
        assert work.is_relative_to(build.resolve())
        (work / "nitro").mkdir()
        (work / "nitro/types.h").write_text(
            "#include <stdint.h>\ntypedef uint8_t u8;\ntypedef uint16_t u16;\n"
            "typedef uint32_t u32;\ntypedef int BOOL;\n#define TRUE 1\n#define FALSE 0\n",
            encoding="ascii",
        )
        (work / "run_species_data.h").write_text(generated, encoding="ascii")
        (work / "run_world_data.h").write_text(world_generated, encoding="ascii")
        executable = work / "run_engine_tests.exe"
        command = [compiler, "-std=c99", "-pedantic", "-Wall", "-Wextra", "-Werror", "-O2",
                   "-I", str(work), "-I", str(root / "include"),
                   str(root / "src/run_config.c"), str(root / "src/run_randomizer.c"),
                   str(root / "tools/tests/run_engine_tests.c"), "-o", str(executable)]
        subprocess.run(command, check=True, cwd=root)
        encoded = work / "config.bin"
        subprocess.run([str(executable), str(encoded)], check=True, cwd=root)
        original = encoded.read_bytes()
        assert len(original) == 36
        assert struct.unpack_from("<I", original, 32)[0] == zlib.crc32(original[:32])

        mutations = [
            ("old prototype schema", 4, b"\x01\x00", 2),
            ("future schema", 4, b"\x03\x00", 2),
            ("old prototype rules", 8, b"\x01\x00", 2),
            ("future rules", 8, b"\x03\x00", 2),
            ("foreign data", 12, b"\x00\x00\x00\x00", 2),
            ("unknown flags", 20, b"\xff\xff\xff\xff", 1),
            ("invalid tolerance", 24, b"\x0b", 1),
            ("invalid weight", 25, b"\x65", 1),
            ("invalid attempts", 26, b"\x04", 1),
            ("invalid preset", 27, b"\xff", 1),
            ("invalid seed mode", 10, b"\x02", 1),
            ("reserved byte", 11, b"\x01", 1),
            ("invalid cap allowance", 28, b"\x06", 1),
            ("zero trainer bonus", 29, b"\x00", 1),
            ("invalid trainer bonus", 29, b"\x04", 1),
            ("reserved word", 30, b"\x01", 1),
        ]
        for name, offset, replacement, expected in mutations:
            data = bytearray(original)
            data[offset:offset + len(replacement)] = replacement
            struct.pack_into("<I", data, 32, zlib.crc32(data[:32]))
            encoded.write_bytes(data)
            result = subprocess.run([str(executable), "decode", str(encoded), str(expected)],
                                    check=True, capture_output=True, text=True, cwd=root)
            assert "PASS:" in result.stdout, name
        print(f"PASS: independent CRC32 check and {len(mutations)} valid-checksum rejection cases")

        # Verify every evolution edge against the generated families, not only named examples.
        values = generated.split("sRunSpeciesFamily[494] = {", 1)[1].split("};", 1)[0]
        families = [int(value.strip()) for value in values.split(",") if value.strip()]
        names = (root / "generated/species.txt").read_text().splitlines()[:494]
        ids = {name: i for i, name in enumerate(names)}
        edges = 0
        for species, name in enumerate(names[1:], 1):
            path = root / "res/pokemon" / name.removeprefix("SPECIES_").lower() / "data.json"
            for evolution in json.loads(path.read_text(encoding="utf-8")).get("evolutions", []):
                assert families[species] == families[ids[evolution[-1]]]
                edges += 1
        print(f"PASS: all {edges} vanilla evolution edges preserve ownership family")

        state_executable = work / "run_state_tests.exe"
        subprocess.run([compiler, "-std=c99", "-pedantic", "-Wall", "-Wextra", "-Werror", "-O2",
                        "-I", str(work), "-I", str(root / "include"),
                        str(root / "src/run_config.c"), str(root / "src/run_randomizer.c"),
                        str(root / "src/run_state.c"), str(root / "src/run_store.c"),
                        str(root / "src/run_world.c"),
                        str(root / "tools/tests/run_state_tests.c"), "-o", str(state_executable)],
                       check=True, cwd=root)
        subprocess.run([str(state_executable)], check=True, cwd=root)


if __name__ == "__main__":
    main()
