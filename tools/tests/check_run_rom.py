#!/usr/bin/env python3
"""Check linked ARM layout and NDS structure, without executing the ROM."""

import hashlib
import json
import re
import struct
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]


def main():
    raw = (ROOT / 'build/pokeplatinum.us.nds').read_bytes()
    assert len(raw) == 128 * 1024 * 1024
    crc = 0xFFFF
    for byte in raw[:0x15E]:
        crc ^= byte
        for _ in range(8):
            crc = (crc >> 1) ^ (0xA001 if crc & 1 else 0)
    assert crc == struct.unpack_from('<H', raw, 0x15E)[0], 'NDS header CRC16'
    arm_offset, entry, arm_start, arm_size = struct.unpack_from('<4I', raw, 0x20)
    assert arm_offset + arm_size <= len(raw) and arm_start <= entry < arm_start + arm_size
    fat_offset, fat_size, ov_offset, ov_size = struct.unpack_from('<4I', raw, 0x48)
    assert fat_offset + fat_size <= len(raw) and ov_offset + ov_size <= len(raw)
    assert fat_size % 8 == 0 and ov_size % 32 == 0
    fat = list(struct.iter_unpack('<2I', raw[fat_offset:fat_offset + fat_size]))
    assert all(0 <= start <= end <= len(raw) for start, end in fat)
    symbols = {name: int(value, 16) for value, name in re.findall(
        r'^#>([0-9A-Fa-f]{8})\s+(SDK_\S+)',
        (ROOT / 'build/main.nef.xMAP').read_text(encoding='utf-8', errors='replace'), re.M)}
    definitions = re.findall(r'^Overlay (\w+)\s*\{(.*?)^\}',
                             (ROOT / 'platinum.us/main.lsf').read_text(), re.M | re.S)
    assert len(definitions) == ov_size // 32 == 122
    for index, (name, body) in enumerate(definitions):
        prefix = 'SDK_OVERLAY.' + name
        start, end, bss = (symbols[prefix + x] for x in ('.START', '.END', '.BSS_END'))
        limit = 0x02000000 if name == 'dummy4' else 0x023E0000
        assert start <= end <= bss < limit, (name, hex(bss))
        parent = re.search(r'\bAfter\s+(\w+)', body)[1]
        parent_end = ('SDK_STATIC_BSS_END' if parent == 'main' else
                      'SDK_AUTOLOAD.ITCM.BSS_END' if parent == 'ITCM' else
                      'SDK_OVERLAY.' + parent + '.BSS_END')
        assert start >= symbols[parent_end], (name, parent)
        ov_id, address, size, zeroes, init_start, init_end, file_id, flags = struct.unpack_from('<8I', raw, ov_offset + index * 32)
        assert ov_id == index and address == start and size == end - start and zeroes == bss - end, name
        assert file_id < len(fat) and fat[file_id][0] < fat[file_id][1]
        assert init_start <= init_end and (init_start == init_end == 0 or start <= init_start <= init_end <= end)
    arena = symbols['SDK_MAIN_ARENA_LO']
    assert arena >= max(symbols['SDK_OVERLAY.' + name + '.BSS_END'] for name, _ in definitions if name != 'dummy4')
    heap_header = (ROOT / 'include/constants/heap.h').read_text()
    base_heaps = sum(int(re.search(r'\bHEAP_SIZE_' + name + r'\s*=\s*(0x[0-9A-Fa-f]+)', heap_header)[1], 16)
                     for name in ('SYSTEM', 'SAVE', 'DEBUG', 'APPLICATION'))
    # Excludes arena metadata, initialization offset and other OS allocations.
    remainder = 0x023E0000 - arena - base_heaps
    assert remainder > 0, 'Static base heaps exceed main-memory boundary'
    print(f'PASS: NDS CRC16={crc:04X}, ARM9/FAT bounds, 122 overlay tables and parent/BSS ranges')
    print(f'Main static end={symbols["SDK_STATIC_END"]:08X}, BSS end={symbols["SDK_STATIC_BSS_END"]:08X}, arena={arena:08X}')
    print(f'Unallocated static budget before metadata/OS costs: {remainder} bytes')
    if len(sys.argv) > 1:
        before = {k: int(v, 16) if isinstance(v, str) else v
                  for k, v in json.loads(Path(sys.argv[1]).read_text()).items()}
        for name in ('overlay5', 'overlay6', 'battle', 'battle_sub_menus', 'options_menu', 'rowan_intro', 'choose_starter'):
            prefix = 'SDK_OVERLAY.' + name
            size = symbols[prefix + '.BSS_END'] - symbols[prefix + '.START']
            old = before[prefix + '.BSS_END'] - before[prefix + '.START']
            print(f'{name}: {size} bytes, change {size - old:+d}')
        print(f'Arena change: {arena - before["SDK_MAIN_ARENA_LO"]:+d} bytes')
    print('ROM SHA256:', hashlib.sha256(raw).hexdigest().upper())
    print('NOT TESTED: runtime heaps, VRAM, ARM execution or emulator gameplay')


if __name__ == '__main__':
    main()
