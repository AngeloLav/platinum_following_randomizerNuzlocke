# Integrated run engine

## Current revision and verification

Work is confined to `nuzlocke-base`; no commit, history rewrite, branch switch,
merge, cherry-pick or push was performed. Following and its isolated field-heap
and battle-introduction fixes are preserved. No Delta gameplay/balance import.

Prompt3, Prompt4, the first-Ball advice, first-encounter rule and battle capture
indicator are implemented in source. Portable C tests and localized resource /
native-font bounds checks passed on 2026-11-05. The current ARM build has not yet
completed; the build record below will be updated after incremental `make rom`.
Do not delete `build/` or clean. No emulator, graphics, real-flash or hardware
test has been executed. User acceptance testing is still required.

## Configuration and native interfaces

New Game opens the native Options-based configuration before Rowan. It is one
continuous vertical list, including preset, every option, seed and final review.
Six visible rows scroll; UP/DOWN selects, LEFT/RIGHT changes values, A edits or
confirms, B returns to the top / previous review. On the final Review row, press
A, inspect the read-only summary, scroll to Confirm, then A starts Rowan.
The lower screen shows persistent controls and contextual descriptions.
Disabled options are gray; their dependencies remain visible. Settings become
immutable once confirmed. Continue reads the saved configuration.

Classic Platinum, Randomizer, Classic Nuzlocke, Hardcore Randomizer and Custom
presets initialize consistent defaults. An override changes the effective
profile to Custom. Level Cap Mode defaults to EXP Lock in all presets. Shiny
Rate defaults to Original (1/8192); Field Item Randomizer defaults OFF.
Shiny Rate and Shiny Clause are independent. Shiny Rate offers 1/8192, 1/4096,
1/2048 and 1/1024 and affects newly generated wild/gift/Egg Pokemon only.
Forced shinies remain shiny; gender byte, nature, ability parity and Wurmple's
evolution branch are preserved. Already generated Pokemon and native stored
roamers are unchanged. Manaphy's special Egg anti-shiny behavior is preserved.

The Start menu Registry is available before the Poketch. L/R moves between RUN,
AREAS, POKEMON, MEMORIAL and SPECIAL; each remembers its scroll position.
RUN begins with the current zone, capture status, remaining attempts, first
encounter/result and A for details, followed by progression, randomizer and
capture summaries. Full settings are read-only. Areas show current first, then
visited available, visited exhausted and unvisited; all mapped zones remain
inspectable. Aggregate area totals exclude unused gift-only/town zones.
History links survive PC moves, nicknames, evolution and death. The optional
Poketch tracker uses the preexisting unused-4 app after obtaining the device.
Map-name notifications reuse the native popup, without an extra window/task.

All strings are centralized in native message banks; language is the ROM's
English. Native graphics, fonts, palettes, sounds, fades and cursors are reused.
The lower help window shares the existing 64KB Options heap; unused retail
choice strings are freed before allocating it. Font-measured help fits 224px
and eight lines; the six-row main list fits its existing 112px window.

## Capture activation, enforcement and battle flag

The first successful addition of a usable ordinary Ball to the player's own
Bag activates capture rules permanently. Shops, gifts and field pickups all
reach this hook. Loading a Bag that already has Balls also activates them;
the defensive field scan reads 15 Bag slots directly without item-NARC loads.
Tutorial/script progression alone does not activate restrictions. Before that
point, encounters do not spend attempts. Running out of Balls later does not
disable the rules or restore opportunities.

A persistent pending-advice bit queues native overworld dialogue at the next
safe field frame, after shops/scripts/fades finish. Dismissing it acknowledges
it once; interruption before dismissal keeps it pending. The message explains
the configured quota. With quota 1, the FIRST eligible encounter is the only
ordinary capture opportunity, regardless of catch, KO, flee, failure or an
empty Bag. Later encounters cannot replace that spent opportunity. With quota
2/3, the first two/three distinct eligible species each reserve an attempt;
a repeated attempted species remains blocked even with Dupes Clause OFF.

Reservation occurs once at battle establishment, result finalization later.
The cached permission prevents duplicate callbacks and permits a reserved
first encounter even when the remaining counter has already reached zero.
Historical evolution-family dupes (including dead/released Pokemon) skip the
ordinary quota and are blocked before Ball consumption. Shiny Clause grants
bonus opportunities without spending the quota. Genuine scripted statics and
native roamers always bypass quota and dupes. A randomized wild legendary is
still an ordinary wild encounter. Native story catching restrictions remain.
Gifts/Eggs use stable script identities and configurable shared/separate quotas;
hatching does not spend a new area's opportunity. Failed/interrupted ordinary
encounters remain spent after saving and reloading.

The battle lower screen now shows a small native-font flag above BAG:
`CATCH OK`, `NO CATCH`, or `ONE FOE` while two opponents remain. It queries the
same cached permission as Ball enforcement, rather than guessing from the
remaining counter. It indicates rule eligibility, not guaranteed capture or
Ball ownership. Trainer/tutorial/facility/link/Pal Park menus omit it. Safari
positions it above BALL instead. The flag shares the relevant panel palette
and press animation, uses the action menu's otherwise-unused FontOAM slot 4,
and is freed by the existing menu teardown. The FontOAM manager already has
13 slots for move selection, so no manager/struct/overlay heap increase is
needed. Actual rendering and repeated menu transitions require emulator QA.

## Level Cap modes

Both modes use the next vanilla boss ace plus allowance 0..5, clamped at 100.
Badge trainer scaling is separate and does not increase this cap. The current
ladder stays at Cynthia after eight badges; no postgame ladder is invented.

EXP Lock clips each recipient's FINAL normal EXP award after native participant,
EXP Share, trainer, traded, Lucky Egg and other modifiers. The participant/share
denominators are unchanged. At/above cap, award is zero; excess is discarded,
never reassigned. Existing above-cap Pokemon keep their level. Rare Candy and
Day Care obey the same cap. There is no hidden accumulation.

Obedience Cap allows normal EXP, Rare Candy and Day Care. Exactly at cap remains
eligible; strictly above cap refuses initial selection, voluntary switches,
replacement, AI/automatic replacement and forced entry. All candidate paths
use virtual eligibility HP without changing real HP. The party interface marks
refusers `NO BTL` and uses the existing unavailable palette; refusal messages
are native battle/party messages. Actual statuses retain native icon priority.
An already-active Pokemon that overlevels may finish that battle, but cannot
reenter after switching out. Whirlwind chooses only eligible bench Pokemon and
fails normally when none exist; it cannot reroll forever. Existing optional
double-trainer field gates now require two eligible combatants. Native partner
battles retain their separate parties. Any future forced own-party double must
use the same two-combatant gate; arbitrary new battle formats are not supported.

After battle, zero eligible ACTIVE-PARTY members ends the run in Obedience mode,
even without permadeath and even with living boxed reserves. Overlevel refusal
never sets a death marker. The loss reason is separate from fainting wipe and
appears in the safe Game Over dialogue/Registry. Zero eligible members before
a battle prevents invalid battler initialization and reuses encounter cleanup.
A newly won Gym badge is considered for post-battle eligibility before its
native script awards the badge, avoiding a false loss on the cap-raising win;
no story flag is set early. Ordinary badge updates restore eligibility without
editing Pokemon. Registry shows the mode, cap, eligible/refusing counts and a
warning when just one eligible party member remains.

Permadeath, exhausted-PC handling, living boxed reserves not preventing a wipe,
safe end-run Center confinement, automatic checkpoint and explicit restart are
preserved. A checkpoint does not overwrite another adventure; failed I/O or
overwrite protection produces a warning and keeps Game Over enforced in memory.

## Original mythical events

Professor Oak in Eterna City's south house offers Oak's Letter after game
completion and the National Dex. The Member Card additionally requires the
original Sailor Eldritch boy-awakened flag, reached through Cresselia/Lunar Wing.
Rewards use normal Bag-fit/give-item scripts. Distribution magic is set only
after the item is present; existing legitimate items are recognized. Native
distribution magic prevents duplicate rewards. Full Bag leaves the event
available. No dates, external gifts, artificial warps or Pokemon awards.

Canalave/Harbor Inn/Newmoon Island and Route 224/Seabreak Path/Flower Paradise
scripts remain original, including National Dex, story requirements, nightmare,
gratitude, travel, return, caught flags and retry behavior. Oak's existing
Upgrade/roaming-bird dialogue continues after the optional key-item subroutines.
Both key items are excluded from field-item randomization.

SPECIAL's legendary list always includes Darkrai and Shaymin, with virtual
availability derived from native flags/items and real persistent static records:
NOT AVAILABLE, AVAILABLE, ENCOUNTERED, CAUGHT or FAILED. No fake persistent
encounters are allocated. A real attempt replaces the virtual view, preventing
double-counting. Event identity is original map/species, even if static
randomization replaces the battle species. Both remain exempt from route limits
and dupes. Native intentionally non-catchable battles keep their restrictions.

## Deterministic randomization and safeguards

493 species, 246 evolution edges, 26 legendaries and 9 mythicals derive from
local vanilla data. Legendary/mythical share a centralized restricted class.
Stats, types, abilities, moves, TMs and ordinary battle RNG are not randomized.
Native trainer party sizes and regenerated usable moves are retained. Wild
levels are unchanged. Native roamers keep stored species to protect story/HP
bookkeeping; facilities retain native parties. Field HM assistance requires the
actual HM and badge and supports contextual native interactions with a living
party member, preserving battle learnsets; Fly/Flash remain optional menu moves.

Mappings are lazy, cache-free and keyed to resolved seed, species fingerprint,
domain, stable encounter/trainer/event ID, slot, reference species and settings.
Schema/rules 2 retains the published linear law and starter gift key unchanged.
Rules 3 uses the distinct BST Range (T) and Similarity Strength (S):

```
eligible: 100 * abs(candidateBST - B) <= B * T, with required legendary class
sigma = B * T / 200
G(i) = exp(-distance(i)^2 / (2 * sigma^2))
P(i) = (1 - S/100) / N + (S/100) * G(i) / sum(G)
```

Runtime approximates G with a 65-entry u16 exp(-2*x*x) LUT and Q8 interpolation;
all weights remain positive. A local integer two-stage sampler chooses the
uniform/Gaussian branch. At S=0 the eligible pool is uniform; S=100 strongly
favors the reference BST while leaving every eligible candidate possible.
Empty pools fall back to nearest BST within the required class; vanilla's
original species always qualifies. No heap pool, floating point or normal RNG.
Old rules 2 retains `100 + floor(9*S*(B*T - 100*d)/(B*T))` and its old stream.

Rowan's presentation Pokemon is seeded and cosmetic. Gift randomization uses
stable starter slots for names, cries, sprites, award and first rival battle;
native story slot variables retain their canonical starter identity. Field-item
randomization is separately seeded by visible/hidden location before Bag-fit,
messages and awarding. Its explicit ordinary-item whitelist excludes Key Items,
TMs/HMs, fossils, mail, progression orbs and special event tools; shops/NPC gifts
are excluded. Existing quantity and one-time item flags remain native.

## Persistent format and compatibility

Native SaveData/BoxPokemon/Poketch sizes and offsets are unchanged. New message,
script and app IDs are appended. `MiscSaveBlock.unused` holds a nonzero run nonce.
Anchor-zero retail/Following saves remain Classic without extension writes.
Configured saves require matching valid extension; missing/corrupt/future data
fails closed, without replacing a save. Backups and savestates are not anti-cheat
protected. Older ROMs do not understand configured runs or the new Poketch ID.

RunState is a 65536-byte explicit little-endian root-BSS wire buffer:

| Offset | Bytes | Meaning |
| --- | --- | --- |
| 0 | 32 | RNS1 schema 1 header, status, activation, nonce/counter, counts, world fingerprint, CRC32 |
| 32 | 36 | RUNC configuration |
| 68 | 64 | Persistent evolutionary-family ownership bitmap |
| 132 | 7168 | 256 zones x 28 bytes, three opportunities per zone |
| 7300 | 5120 | 256 aggregated special events x 20 bytes |
| 12420 | 49152 | 1024 Pokemon history records x 48 bytes |
| 61572 | 3964 | Reserved |

RNS1 header byte 9 is capture activation, byte 10 pending activation advice,
byte 11 loss reason (0 legacy/unknown, 1 fainting wipe, 2 no eligible combatant).
Legacy zero reserved bytes remain valid. Loss reason is written only on the
first transition to Game Over. Refusal does not mark a Pokemon exhausted.

RUNC is still 36 bytes: magic0, schema4, size6, rules8, seedMode10, reserved11,
species fingerprint12, seed16, flags20, tolerance/weight/attempts/preset24..27,
allowance/bonus28..29, Shiny Rate30, Cap Mode31, CRC32 of0..31 at32.
Flags bit17 is field-item randomization. Schema/rules 3 uses the new bytes;
exact legacy 2/2 decodes with original shiny rate, items OFF and EXP Lock,
preserving old mappings. New options must be zero in legacy data. Unsupported
schema/rules pairs, fingerprints, flags and ranges are rejected, not guessed.
CRC32 is reflected IEEE 0xEDB88320, initial/final XOR 0xFFFFFFFF.

Two extension snapshots occupy sectors44..59 and108..123 of the 512KB backup;
native main0..31/64..95 and extras32..43/96..107 are preserved. Opposite slot is
invalidated then payload written, CRC header LAST, before committing the native
normal-block footer. Loading requires nonce AND exact normal counter. Torn
writes retain the previous native-paired copy. Interrupted pending opportunities
remain spent. Loading briefly uses a 64KB application-heap scratch buffer.
Real flash/power-loss has not been tested; host simulated torn-write tests have.

History ID is encrypted unused field114, verified against PID/OTID. Byte113 uses
TRACKED0x40/EXHAUSTED0x80. Three evolution slots, origin/death zone, native nickname
and source/event link are stored. Rename/hatch/evolution synchronize records;
Shedinja gets an independent identity before copied-ID setters can mutate its
parent's record. Releasing never clears ownership. At history capacity ordinary
captures fail closed, but statics/shinies/exempt gifts remain obtainable and can
use aggregate counters with no detailed record. Special retries aggregate.

`tools/datagen/run_zones.json` centrally groups logical routes, named caves,
forests/lakes/facilities and town interiors. Grass/surf/fishing share a zone;
all floors of each named location share one allowance. Native map IDs select
committed location-index zones, not translated strings. New maps require review.
Species fingerprint covers BST/families/classification; world fingerprint covers
zones/caps/bosses. New tables, stable key/offset changes or balance imports need
explicit version/migration review. No Renegade data has been imported.

## Validation and emulator acceptance

Executed host checks: `python tools/tests/run_engine_tests.py` (actual portable
C99 kernels built with GCC, pedantic/Wall/Wextra/Werror),
`python tools/tests/check_run_integration.py`, and `git diff --check`.
Tests cover presets/codecs, legacy byte-exact roundtrips, invalid payloads,
all species/families, deterministic mappings, Gaussian law vs exp reference and
100000 draws per mixture, all shiny XORs/rates and protected PID properties,
safe field items, first Ball/advice persistence, quotas1..3, repeated species,
all clause/rate pairs, statics/shiny/gift exemptions, history/exhaustion,
both cap modes/exact boundaries/badge recovery/no false death, per-recipient
EXP clipping, persistent loss reason and 18 simulated torn-write boundaries.
Source/resource checks verify glyphs, message references, help/flag widths,
native script/app IDs and the pinned legacy starter instruction offset.
These checks do not execute native ARM battle callbacks or render graphics.

Use disposable ROM/save copies and preserve backups. All DeSmuME items below
are NOT YET EXECUTED:

1. New Game, every preset/override, continuous scrolling, lower-screen help,
   disabled parents, manual seed including zero, final Review -> Confirm -> Rowan.
   Continue must never reopen configuration. Compare Classic with Following base.
2. Seeded Rowan and randomized starters: correct names/cry/sprite/palette, reopen
   selection, correct awarded species and first rival choice; first battle,
   return to overworld, follower, doors/maps and strong-trainer introductions.
3. Before first Balls: no quota used. First gift/shop/pickup triggers advice once
   only after other dialogue ends. Reload with Balls, empty Bag later and repeat
   acquisitions. With quota1, KO/flee the first eligible encounter and verify NO
   CATCH and no Ball/turn consumption on every later ordinary encounter.
4. Quotas2/3, same species with dupes OFF, branching-family dupes after release/
   death, shiny exceptions, statics in exhausted areas, partner wilds/two foes,
   Safari, tutorial and trainer menus. Flag remains CATCH OK for the reserved
   first encounter and ONE FOE with two live opponents. Repeated Bag/Fight/Party/
   Cancel transitions, touch/D-pad and menu animation must leave no stale flag.
5. Registry before Poketch, all Start entries, all five L/R pages, current area
   status, counts, detail/back and settings help. Quotas/results/identity persist
   through PC/nickname/evolution/hatch/Nincada/Shedinja/save/reload. Test popup
   across same-zone floors, rapid doors, scripts/fades and notification OFF.
6. EXP Lock: exact cap, surplus, existing above-cap Pokemon, candy/Day Care,
   participants plus EXP Share. Verify another recipient gets only its normal
   share when a capped recipient gets zero. Check all badges/allowances/scaling.
7. Obedience: at cap eligible; one above marked NO BTL, refused at initial entry,
   voluntary/forced switch, replacement and doubles. Active mid-battle overlevel
   may finish but cannot reenter. Whirlwind with no eligible bench fails safely.
   Badge restores eligibility; Gym win raising the cap must not cause false loss.
8. Obedience zero-eligible loss with living overleveled party and living PC reserve,
   including permadeath OFF, and before a mandatory encounter. Refusers retain
   HP/alive history. Check safe Center/Game Over reason/save/reload/restart. One
   eligible member shows warning but does not end the run. Test ordinary death
   wipe separately, healing/revival/storage protection and Following/HMs.
9. Oak before/after National Dex, game completion and boy/Cresselia quest; no
   duplicate items after reload/repeated dialogue, full Bag preserves reward.
   Original Darkrai Harbor Inn/nightmare/return and Shaymin gratitude/Seabreak/
   return. KO/flee/retry/catch, registry statuses/counters, static randomizer
   OFF/ON and exhausted routes/dupes. Key items stay unrandomized.
10. Field items OFF/ON: visible/hidden, message/pocket/quantity/Bag full, reload,
    same/different seeds; protected items/TMs/HMs/fossils, shops/NPC gifts.
    Shiny rates/Clause combinations, forced shinies and unchanged existing mons.
11. Original unconfigured and schema2 saves load safely; new modes persist;
    corrupt/missing extension fails closed on copies. Record preset, seed, save
    lineage, exact event and frame for regressions; test tracker and overlay/map
    transitions. No automatic stability claim until these gameplay tests pass.

## Memory and build record

Run state stays 64KB root BSS. Gaussian LUT is130 bytes; no candidate heap/cache.
Registry uses the existing64KB Options heap, Poketch the existing48KB app heap.
The battle flag adds a short transient font object/tiles below the existing
13-label move-menu manager capacity; allocation/VRAM high-water needs emulator
verification. Native base heaps and Following heap fixes are unchanged.
Current final ARM layout/ROM hash will be recorded when this build completes.

Current build attempts: session92207 regenerated Meson/Ninja and reached249/282,
then stopped on an undefined `fieldSystem` in the new hidden-item hook. The
static helper now receives that context explicitly from its caller. Native
resource checks passed again; incremental retry session12335 compiled all ARM
sources through1088/1090 and is linking the final image. No emulator was run.
The failed attempt is not a successful ROM build and did not replace the ROM.

The records below refer to the PREVIOUS ROM, not validation of this revision.

## ARM build log

Final incremental `make rom` started 2026-11-04 in WSL Ubuntu. Meson regeneration
completed without I/O errors. The first build stopped at text encoding: square
brackets, ASCII apostrophes and the ASCII cursor were replaced with glyphs from
the native charmap. The lightweight resource check now validates those glyphs.
The second build encoded the resources and compiled the run kernel, save path
and many other ARM modules without compiler errors. At 3608/4038 it was stopped
with SIGINT before linking: final review had updated an already compiled bridge
module (zero-HP enforcement on field return and shared-party doubles). Completed
objects were preserved. Incremental `make rom` resumed on Nov 5 around 00:38 in
unified-exec session 97574. That run stopped at 85/431: the battle controller
needed the generated battle-message header. The include was added. Review also
decoupled gift acceptance from an optional detailed-history ID at capacity;
portable tests now cover full-history exempt gifts/statics/shinies and passed.
Session 13592 compiled the corrected battle controller and continued to the
Poketch tracker, where its character-code constants header was missing. That
include was added, along with explicit standard/string-constant includes for
the configuration UI. Session 54176 reached 138/190 without a compiler error;
it was interrupted before linking to add the native dropdown-close task to the
PC's exhausted-withdrawal rejection path. DisplayBoxMessage only redraws the
message window, so this cleanup is needed just as in the native party-full path.
The final native tests, resource checks and diff whitespace check passed again.
The final incremental retry, unified-exec session **67137**, completed with
**exit 0, 53/53**. ARM linking ran approximately 01:27..01:34 on Nov 5; ROM
generation finished at **2026-11-05 01:34:34 local time**. The session is closed.
There were no WSL filesystem I/O errors. No build directory was deleted and
completed objects were reused throughout. Earlier intermediate failures and
interruptions above are not successful builds; this final run is successful.

Final artifacts:

- `build/pokeplatinum.us.nds`: **134217728 bytes (128 MiB)**.
- `build/main.nef`: 36604708 bytes; `build/main.nef.xMAP`: 12105573 bytes.
- ROM SHA-256: `D9C554E749F5CE2A73FE03CEE71EE607F0F5147DC446CC7390C12154D9E888DB`.

The old Nov 4 ROM has been replaced by this newly generated artifact. No emulator
was launched, no existing save was opened or rewritten for tests, and no branch
switch, commit, merge, cherry-pick or push was performed.

## Changed paths

Source and documentation paths for this implementation; build artifacts are ignored.

```text
asm/macros/scrcmd.inc
docs/RUN_ENGINE.md
generated/battle_subscripts.txt
generated/poketch_apps.txt
generated/text_banks.txt
include/applications/poketch/unused/4/graphics.h
include/data/scripts/scrcmd.h
include/field_battle_data_transfer.h
include/overlay005/map_name_popup.h
include/run_config.h
include/run_game.h
include/run_menu.h
include/run_randomizer.h
include/run_session.h
include/run_state.h
include/run_store.h
include/run_world.h
include/script_manager.h
include/start_menu.h
meson.build
platinum.us/main.lsf
res/battle/scripts/subscripts/meson.build
res/battle/scripts/subscripts/sub_seq.order
res/battle/scripts/subscripts/subscript_run_capture_denied.s
res/field/scripts/scripts_common.s
res/field/scripts/scripts_day_care_common.s
res/field/scripts/scripts_eterna_city.s
res/field/scripts/scripts_hearthome_city_northwest_house.s
res/field/scripts/scripts_iron_island_b2f_left_room.s
res/field/scripts/scripts_mining_museum.s
res/field/scripts/scripts_veilstone_city_northeast_house.s
res/text/battle_strings.json
res/text/box_messages.json
res/text/common_strings.json
res/text/day_care_common.json
res/text/run_engine.json
res/text/start_menu.json
src/applications/options_menu.c
src/applications/party_menu/main.c
src/applications/pc_boxes/box_app_manager.c
src/applications/poketch/poketch_system.c
src/applications/poketch/unused/4/graphics.c
src/applications/poketch/unused/4/main.c
src/battle/battle_controller_player.c
src/battle/battle_script.c
src/encounter.c
src/evolution.c
src/field_battle_data_transfer.c
src/field_map_change.c
src/field_system.c
src/game_start.c
src/item_use_pokemon.c
src/meson.build
src/overlay005/daycare.c
src/overlay005/fieldmap.c
src/overlay005/map_name_popup.c
src/overlay006/wild_encounters.c
src/pokemon.c
src/poketch.c
src/run_config.c
src/run_game.c
src/run_menu.c
src/run_randomizer.c
src/run_session.c
src/run_state.c
src/run_store.c
src/run_world.c
src/savedata.c
src/scrcmd.c
src/scrcmd_party.c
src/script_manager.c
src/start_menu.c
src/trainer_data.c
src/unk_020528D0.c
src/unk_02054884.c
tools/datagen/run_species.py
tools/datagen/run_species_classes.json
tools/datagen/run_world.py
tools/datagen/run_zones.json
tools/tests/check_run_integration.py
tools/tests/run_engine_tests.c
tools/tests/run_engine_tests.py
tools/tests/run_state_tests.c
```
