"""Ports HeartGold's data-merge results into Platinum's per-species / per-move JSON files.

This is the Platinum-side equivalent of PokeHeartGold's tools/apply_firered_data.py,
tools/apply_firered_items.py, tools/apply_ultimatecrystal_items.py, and
tools/apply_base_stats_txt_edits.py combined - except instead of re-deriving each historical
merge step from the original FireRed/UltimateCrystal sources, it reads HeartGold's CURRENT
(already fully merged, already shipped) data files directly, since those already hold the
project owner's approved final numbers, and copies them into Platinum by NUMERIC ID.

Species IDs (1..493, National Dex order) and move IDs (1..467, with the Gen3/Gen4 boundary at
354/355) were verified identical between the two decomp projects (generated/species.txt and
generated/moves.txt here match personal.json's baseStats array order and include/constants/moves.h
in ../ (pokeheartgold-master/) exactly) - so this script joins by ID, never by name, sidestepping
every species/move NAME mismatch HeartGold's own merge scripts had to work around (e.g. MR__MIME
vs MR_MIME, or SELF_DESTRUCT/SMOKESCREEN/SOFT_BOILED differing between constant sets).

SCOPE (matches exactly what HeartGold's own changelog says it touched):
  * Base stats + EXP yield: HG species 1..493 (HeartGold's manual rebalance pass, ENTRY AL,
    reached Gen-4-only species like Rotom/Darkrai/Porygon-Z/Deoxys, not just the FireRed-covered
    1..386 range - so the full 1..493 range is read here).
  * Held items: HG species 1..493 (superset; the real UltimateCrystal/FireRed changes only cover
    1..386, everything past that will simply show 0 diffs, which is safe and expected).
  * Level-up learnsets: HG species 1..386 ONLY, matching HeartGold's own stated scope exactly
    ("IMPLEMENTED for species 1..386" / "Species/forms above 386 ... are untouched" - ENTRY AE).
    HeartGold's OWN vanilla Gen 4 (387-493) learnsets are not guaranteed identical to Platinum's
    own vanilla Gen 4 learnsets (unlike stats/items/move data, which Nintendo keeps identical
    across paired Gen 4 versions), so copying those would risk an unintended, unrequested change.
  * Move power/accuracy/type/PP: HG moves 1..354 ONLY, matching HeartGold's own stated scope
    exactly ("Moves that did not exist in Gen 3 are never touched" - ENTRY AE/AF). Same reasoning
    as learnsets applies to Gen-4-only moves (355-467).

Run from pokeplatinum-main/ (needs ../ to be the pokeheartgold-master sibling checkout):
    python3 tools/port_heartgold_data.py             # dry run: prints what WOULD change, writes nothing
    python3 tools/port_heartgold_data.py --apply      # writes res/pokemon/*/data.json, res/moves/*/data.json

Edits are done as surgical regex substitutions on the existing JSON text (not a full
json.load/json.dump round-trip), so every file's existing 4-space-indent, compact-array
formatting is preserved byte-for-byte outside of the specific fields this script changes -
matching this repo's existing generator-produced file style and keeping diffs minimal/reviewable.
"""
import json, os, re, struct, sys

APPLY = '--apply' in sys.argv
HG = '../'

PERSONAL = HG + 'files/poketool/personal/personal.json'
WAZA = HG + 'files/poketool/waza/waza_tbl.narc'
WOTBL = HG + 'files/poketool/personal/wotbl.narc'
HG_MOVES_H = HG + 'include/constants/moves.h'
HG_TYPES_H = HG + 'include/constants/pokemon.h'

LEARNSET_SPECIES_MAX = 386
MOVE_DATA_MAX = 354


class Narc:
    """Minimal read-only NARC reader (same layout as HeartGold's tools/apply_firered_data.py)."""
    def __init__(self, path):
        b = open(path, 'rb').read()
        assert b[:4] == b'NARC', 'not a NARC: %s' % path
        btaf = 0x10
        count = struct.unpack_from('<H', b, btaf + 8)[0]
        btaf_size = struct.unpack_from('<I', b, btaf + 4)[0]
        btnf = btaf + btaf_size
        btnf_size = struct.unpack_from('<I', b, btnf + 4)[0]
        gmif = btnf + btnf_size
        data = gmif + 8
        self.members = []
        for i in range(count):
            s, e = struct.unpack_from('<II', b, btaf + 0xC + i * 8)
            self.members.append(bytes(b[data + s:data + e]))


def defs(path, prefix, exclude=()):
    """Parses '#define <prefix>NAME value' lines into {value: name}. HG's own constants headers reuse a
    prefix for TWO unrelated families sharing the same small integer range - e.g. constants/moves.h has both
    real move ids (MOVE_POUND=1..) AND MOVE_ATTRIBUTE_* struct-field-index constants (0..11), and
    constants/pokemon.h has both real type ids (TYPE_NORMAL=0..) AND TYPE_MUL_* effectiveness-multiplier
    constants (0, 5, 10, 20) - each pair collides on value for several entries. HG's OWN gen_base_stats_txt.py
    hit and fixed this exact bug for the move case (Changelog ENTRY AL item 1: "skip any matched constant
    whose name contains ATTRIBUTE"); `exclude` generalizes that fix to every prefix this script reads.
    """
    d = {}
    for line in open(path, encoding='utf-8', errors='replace'):
        m = re.match(r'#define\s+(%s[A-Z0-9_]+)\s+(\d+)\b' % prefix, line)
        if m and not any(bad in m.group(1) for bad in exclude):
            d[int(m.group(2))] = m.group(1)
    return d


def parse_ls(m):
    """Same 16-bit (level<<9 | moveId) learnset record format HeartGold's own wotbl.narc uses."""
    out = []
    for k in range(0, len(m) - 1, 2):
        v = struct.unpack_from('<H', m, k)[0]
        if v == 0xFFFF:
            break
        out.append((v >> 9, v & 0x1FF))
    return out


def report(title, lines, limit=10):
    print('\n== %s: %d' % (title, len(lines)))
    for l in lines[:limit]:
        print('   ', l)
    if len(lines) > limit:
        print('    ...')


# ----------------------------------------------------------------------------------------- id tables
species_list = open('generated/species.txt', encoding='utf-8').read().splitlines()  # index == species id
id_to_move = defs(HG_MOVES_H, 'MOVE_', exclude=('ATTRIBUTE',))  # move id -> "MOVE_XXX"
id_to_type = defs(HG_TYPES_H, 'TYPE_', exclude=('MUL',))  # type id -> "TYPE_XXX" (0..17, same numbering both games)

# Sanity check (would have caught this exact bug earlier): the real move/type tables are dense, contiguous
# ranges starting at a known id with a known name - assert that holds instead of silently trusting the regex.
assert id_to_move.get(0) == 'MOVE_NONE' and id_to_move.get(1) == 'MOVE_POUND' and id_to_move.get(354) == 'MOVE_PSYCHO_BOOST', \
    'id_to_move sanity check failed - HG constants/moves.h format may have changed: %r' % {k: id_to_move.get(k) for k in (0, 1, 354)}
assert id_to_type.get(0) == 'TYPE_NORMAL' and id_to_type.get(17) == 'TYPE_DARK', \
    'id_to_type sanity check failed - HG constants/pokemon.h format may have changed: %r' % {k: id_to_type.get(k) for k in (0, 17)}

pj = json.load(open(PERSONAL, encoding='utf-8'))
waza = Narc(WAZA)
wot = Narc(WOTBL)

stat_changes, item_changes, ls_changes, move_changes = [], [], [], []
missing_species_dirs, missing_move_dirs = [], []

STAT_FIELDS = [('hp', 'hp'), ('atk', 'attack'), ('def', 'defense'), ('speed', 'speed'),
               ('spatk', 'special_attack'), ('spdef', 'special_defense')]


def sub_int_field(text, key, indent, new_value):
    pattern = re.compile(r'("%s"\s*:\s*)(-?\d+)' % re.escape(key))
    m = pattern.search(text, indent)
    assert m, 'field %r not found' % key
    return text[:m.start(2)] + str(new_value) + text[m.end(2):]


def sub_str_field(text, key, new_value):
    pattern = re.compile(r'("%s"\s*:\s*)"([A-Za-z0-9_]*)"' % re.escape(key))
    m = pattern.search(text)
    assert m, 'field %r not found' % key
    return text[:m.start(2)] + new_value + text[m.end(2):]


# ----------------------------------------------------------------------------------------- 1. species: stats/items/learnset
for sid in range(1, 494):
    hg_entry = pj['baseStats'][sid]
    dirname = species_list[sid].replace('SPECIES_', '').lower()
    path = 'res/pokemon/%s/data.json' % dirname
    if not os.path.exists(path):
        missing_species_dirs.append((sid, dirname))
        continue

    text = open(path, encoding='utf-8').read()
    orig_text = text

    m = re.search(r'"base_stats"\s*:\s*\{', text)
    assert m, '%s: no base_stats block' % path
    block_start = m.end()
    for hgf, pf in STAT_FIELDS:
        cur = int(re.search(r'"%s"\s*:\s*(-?\d+)' % pf, text[block_start:block_start + 200]).group(1))
        new = hg_entry[hgf]
        if cur != new:
            stat_changes.append('%s %s %d -> %d' % (dirname, pf, cur, new))
        text = sub_int_field(text, pf, 0, new) if False else text  # placeholder, replaced below

    # Re-do base_stats precisely (rebuild the whole 6-field block to avoid re-searching stale offsets)
    def build_stats_block(mo):
        body = mo.group(1)
        for hgf, pf in STAT_FIELDS:
            body = re.sub(r'("%s"\s*:\s*)-?\d+' % re.escape(pf), lambda mm: mm.group(1) + str(hg_entry[hgf]), body)
        return '"base_stats": {' + body + '}'
    text = re.sub(r'"base_stats"\s*:\s*\{([^}]*)\}', build_stats_block, orig_text, count=1)

    # base_exp_reward
    cur = int(re.search(r'"base_exp_reward"\s*:\s*(-?\d+)', text).group(1))
    if cur != hg_entry['expYield']:
        stat_changes.append('%s base_exp_reward %d -> %d' % (dirname, cur, hg_entry['expYield']))
    text = re.sub(r'("base_exp_reward"\s*:\s*)-?\d+', lambda mm: mm.group(1) + str(hg_entry['expYield']), text, count=1)

    # held_items
    hg_common, hg_rare = hg_entry['items'][0], hg_entry['items'][1]
    m = re.search(r'"held_items"\s*:\s*\{\s*"common"\s*:\s*"([A-Z0-9_]+)"\s*,\s*"rare"\s*:\s*"([A-Z0-9_]+)"', text)
    assert m, '%s: no held_items block' % path
    cur_common, cur_rare = m.group(1), m.group(2)
    if cur_common != hg_common:
        item_changes.append('%s common %s -> %s' % (dirname, cur_common, hg_common))
    if cur_rare != hg_rare:
        item_changes.append('%s rare %s -> %s' % (dirname, cur_rare, hg_rare))

    def build_items_block(mo):
        return '"held_items": {\n        "common": "%s",\n        "rare": "%s"\n    }' % (hg_common, hg_rare)
    text = re.sub(r'"held_items"\s*:\s*\{\s*"common"\s*:\s*"[A-Z0-9_]+"\s*,\s*"rare"\s*:\s*"[A-Z0-9_]+"\s*\}', build_items_block, text, count=1)

    # learnset.by_level (species 1..386 only)
    if sid <= LEARNSET_SPECIES_MAX:
        hg_ls_raw = parse_ls(wot.members[sid])
        unknown = [mv for _, mv in hg_ls_raw if mv not in id_to_move]
        if unknown:
            print('WARNING: %s (species %d) learnset has unrecognized HG move id(s) %s - learnset skipped' % (dirname, sid, unknown))
        else:
            hg_ls = [(lvl, id_to_move[mv]) for lvl, mv in hg_ls_raw]
            # NOTE: anchored on the following "by_tm" key, not just the next "]" - by_level's entries are
            # themselves bracketed ([ lvl, "MOVE_X" ]), so a naive non-greedy "\[(.*?)\]" stops at the FIRST
            # entry's closing bracket instead of the whole array's, making every species look like it always
            # had 0 existing moves (caught by inspecting a dry run before ever applying this script).
            m = re.search(r'"by_level"\s*:\s*\[(.*?)\]\s*,\s*"by_tm"', text, re.S)
            assert m, '%s: no learnset.by_level block' % path
            cur_entries = re.findall(r'\[\s*(\d+)\s*,\s*"(MOVE_[A-Z0-9_]+)"\s*\]', m.group(1))
            cur_ls = [(int(l), mv) for l, mv in cur_entries]
            if cur_ls != hg_ls:
                ls_changes.append('%s: %d -> %d moves' % (dirname, len(cur_ls), len(hg_ls)))
                new_body = '\n' + ''.join('            [ %d, "%s" ],\n' % (l, mv) for l, mv in hg_ls)
                new_body = new_body.rstrip(',\n') + '\n        '
                text = text[:m.start(1)] + new_body + text[m.end(1):]

    if APPLY and text != orig_text:
        open(path, 'w', encoding='utf-8').write(text)

report('SPECIES base stats / EXP yield changed (field-level)', stat_changes, 10)
report('SPECIES held items changed', item_changes, 10)
report('SPECIES learnsets changed (species 1-%d only)' % LEARNSET_SPECIES_MAX, ls_changes, 10)
if missing_species_dirs:
    report('SPECIES with no Platinum data.json (skipped)', ['%d %s' % t for t in missing_species_dirs])


# ----------------------------------------------------------------------------------------- 2. moves (1..354 only)
for mid in range(1, MOVE_DATA_MAX + 1):
    name = id_to_move.get(mid)
    if not name:
        continue
    dirname = name.replace('MOVE_', '').lower()
    path = 'res/moves/%s/data.json' % dirname
    if not os.path.exists(path):
        missing_move_dirs.append((mid, dirname))
        continue

    text = open(path, encoding='utf-8').read()
    orig_text = text
    m = waza.members[mid]
    power, mtype_id, acc, pp = m[3], m[4], m[5], m[6]
    hg_type = id_to_type.get(mtype_id)
    assert hg_type, 'move %d: unrecognized type id %d' % (mid, mtype_id)

    cur_power = int(re.search(r'"power"\s*:\s*(\d+)', text).group(1))
    cur_acc = int(re.search(r'"accuracy"\s*:\s*(\d+)', text).group(1))
    cur_pp = int(re.search(r'"pp"\s*:\s*(\d+)', text).group(1))
    cur_type = re.search(r'"type"\s*:\s*"([A-Z0-9_]+)"', text).group(1)

    if cur_power != power:
        move_changes.append('%s power %d -> %d' % (dirname, cur_power, power))
    if cur_acc != acc:
        move_changes.append('%s accuracy %d -> %d' % (dirname, cur_acc, acc))
    if cur_pp != pp:
        move_changes.append('%s pp %d -> %d' % (dirname, cur_pp, pp))
    if cur_type != hg_type:
        move_changes.append('%s type %s -> %s' % (dirname, cur_type, hg_type))

    text = re.sub(r'("power"\s*:\s*)\d+', lambda mm: mm.group(1) + str(power), text, count=1)
    text = re.sub(r'("accuracy"\s*:\s*)\d+', lambda mm: mm.group(1) + str(acc), text, count=1)
    text = re.sub(r'("pp"\s*:\s*)\d+', lambda mm: mm.group(1) + str(pp), text, count=1)
    text = re.sub(r'("type"\s*:\s*)"[A-Z0-9_]+"', lambda mm: mm.group(1) + '"%s"' % hg_type, text, count=1)

    if APPLY and text != orig_text:
        open(path, 'w', encoding='utf-8').write(text)

report('MOVES power/accuracy/pp/type changed (field-level, moves 1-%d only)' % MOVE_DATA_MAX, move_changes, 10)
if missing_move_dirs:
    report('MOVES with no Platinum data.json (skipped)', ['%d %s' % t for t in missing_move_dirs])

if not APPLY:
    print('\nDRY RUN - nothing written. Re-run with --apply to write the files.')
else:
    print('\nWRITTEN.')
