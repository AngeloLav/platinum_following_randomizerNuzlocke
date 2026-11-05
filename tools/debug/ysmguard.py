#!/usr/bin/env python3

### ENTRY P110 (R4 / YSMenu compatibility, same fix as HeartGold ENTRY DV).
###
### YSMenu (the R4 / DSTT menu) recognises the game code CPUE and, before starting the game, writes its anti-piracy fix for the
### ORIGINAL Platinum to a fixed address (TTMenu/extinfo.dat: C0 46 02 E0 at 0x02017C28). In this modified game other code lives
### there, so a build could hang on a white screen on an R4. src/main.c holds a table of those places (gYSMenuGuard); this script
### copies the build's own bytes of every place into the table, so the game can put them back at startup.
###
### Usage (meson, pokeplatinum.us.nds target):  ysmguard.py ROM -- <the nitrorom command that writes ROM>
### Runs the command, then fills the table in the finished ROM in place (Platinum's ARM9 static image is stored uncompressed).
### Fails if the command fails or the table is missing / malformed, so a ROM without a filled table is never produced.

import struct
import subprocess
import sys

MAGIC = b'YSMENU_GUARD_V1\x00'
SITE_LEN = 24  # u32 addr, u32 size, u8 ysmenu[8], u8 orig[8]
NITROCODE = 0xDEC00621


def fail(msg):
    print('ysmguard.py: ' + msg, file=sys.stderr)
    sys.exit(1)


def main():
    args = sys.argv[1:]
    if not args:
        fail('usage: ysmguard.py ROM [-- command...]')
    rom = args[0]
    if '--' in args:
        cmd = args[args.index('--') + 1:]
        result = subprocess.run(cmd)
        if result.returncode != 0:
            sys.exit(result.returncode)

    with open(rom, 'r+b') as f:
        header = f.read(0x200)
        arm9_off, _, arm9_load, arm9_size = struct.unpack_from('<4I', header, 0x20)
        f.seek(arm9_off)
        arm9 = bytearray(f.read(arm9_size))
        footer = f.read(12)
        magic, params_off, _ = struct.unpack('<3I', footer)
        if magic != NITROCODE:
            fail('no nitrocode footer after the ARM9 image')
        if struct.unpack_from('<I', arm9, params_off + 20)[0] != 0:
            fail('the ARM9 image is compressed - this script only handles uncompressed images (see HG tools/ysmguard)')

        guard = arm9.find(MAGIC)
        if guard < 0 or guard % 4 or arm9.find(MAGIC, guard + 1) >= 0:
            fail('guard table (gYSMenuGuard, src/main.c) not found exactly once in the ARM9 image')
        num_sites = struct.unpack_from('<I', arm9, guard + 16)[0]
        table_end = guard + 24 + num_sites * SITE_LEN
        if num_sites == 0 or num_sites > 8 or table_end > len(arm9):
            fail('bad site count %d' % num_sites)
        if arm9_off + table_end > 0x4000 and arm9_off + guard < 0x8000:
            fail('guard table lies in the secure area (ROM 0x4000-0x7FFF)')

        for s in range(num_sites):
            site = guard + 24 + s * SITE_LEN
            addr, size = struct.unpack_from('<2I', arm9, site)
            off = addr - arm9_load
            if size == 0 or size > 8 or off < 0 or off + size > len(arm9):
                fail('site %d (0x%08X, %d bytes) is outside the ARM9 image' % (s, addr, size))
            if off < table_end and off + size > guard:
                fail('site %d (0x%08X) overlaps the guard table itself' % (s, addr))
            arm9[site + 16:site + 16 + size] = arm9[off:off + size]
            same = arm9[site + 8:site + 8 + size] == arm9[off:off + size]
            print('ysmguard.py: 0x%08X: %s%s' % (addr, arm9[off:off + size].hex(' ').upper(),
                                                  '  (same as YSMenu\'s patch - nothing to restore)' if same else ''))
        struct.pack_into('<I', arm9, guard + 20, 1)  # filled

        f.seek(arm9_off + guard)
        f.write(arm9[guard:table_end])


if __name__ == '__main__':
    main()
