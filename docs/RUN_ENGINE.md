# Configurable Randomizer and Nuzlocke engine

## Current status and acceptance gate

Branch: `nuzlocke-base`. No branch switch, commit, merge, cherry-pick or push was
performed. The isolated Following/field-heap/battle-introduction fixes remain.
The requirements in `Prompt.TXT` and `Prompt2 (2).TXT` were read in full.

The engine and its interfaces are implemented in source. **Incremental `make rom`
completed successfully (exit 0) in WSL Ubuntu on 2026-11-05 at 01:34 local time.**
The new `build/pokeplatinum.us.nds` is 134217728 bytes. Native portable C tests,
resource checks, final ARM layout and NDS binary checks passed. No emulator
gameplay, visual, hardware or real-flash test has been performed. This ROM is
ready for user acceptance testing; it is not yet a validated stable gameplay base.

If work resumes after an interruption:

1. The build process has finished; there is no build session left to poll.
2. Preserve the current ROM and compare its SHA-256 with the build record below.
3. User acceptance testing is the next step. Follow the DeSmuME checklist below;
   the user explicitly reserved emulator tests for the completed ROM.
4. If fixing a discovered issue, rerun relevant lightweight checks and use
   incremental `make rom`. Do not clean or remove `build/`.
5. Record the exact preset, seed, map/event and save lineage for any regression.

## Implemented systems

- Five presets, individual overrides, effective Custom profile, all rule options,
  automatic/manual 32-bit hexadecimal seed and a scrolling full confirmation.
  Configuration appears after New Game initialization and before Rowan.
  Rules are immutable after confirmation; Continue loads the saved run.
- Native Options-screen graphics, windows, fonts, palettes, sounds, input and
  fades. Six configuration categories plus summary. A confirms; B returns;
  L/R changes pages; directions select/change; the seed edits eight hex digits.
  New English strings are centralized in `res/text/run_engine.json`; native
  script/battle/PC strings extend their respective existing banks.
- Permanent Start-menu NUZLOCKE entry before receiving the Poketch. Eight-row
  menus use 20-pixel spacing; smaller menus preserve 24 pixels. Existing Options
  icon is reused. Union Room retains its communication menu.
- Registry: overview, seed, settings, badges, cap, next cap, trainer bonus,
  current area, remaining opportunities, ACTIVE/GAME OVER; all zones and their
  three individual opportunity slots; nicknames/alive/dead links; ownership and
  evolution history; memorial; five separate special-encounter categories and
  per-event details/counters. B returns through details/categories.
- A compact Poketch Run Tracker reuses the already linked unused-app-4 overlay.
  It registers only when the normal Poketch acquisition enables the device.
  App ID 25 occupies an existing unused registry byte; native save size is
  unchanged. The display refreshes every 30 frames and has no sprites.
- Route capture reservation at battle establishment, before any Ball use. An
  eligible KO/flee/failure uses an opportunity. Finalization is idempotent.
  Two wild enemy parties are addressed independently by battler and party slot.
  Each distinct eligible wild opponent reserves an opportunity; a repeated
  callback for the same battle does not reserve again.
- Evolution-family Dupes Clause uses persistent ownership bits, including
  branches, dead/released Pokemon and the Nidoran/Volbeat/Manaphy families.
  Shiny bonuses override dupes/exhaustion without spending ordinary slots.
- Genuine scripted statics and roamers are identified by encounter source,
  never by legendary species. Statics always bypass route limits/dupes; native
  story restrictions on otherwise uncatchable battles remain. Event attempts,
  successes and failures are tracked separately. Shiny statics retain their
  static category, avoiding double counting.
- Script gifts/Eggs have map/script-offset event identities, optional separate
  allowances and repeat-event checks. Hatching updates identity/history without
  reserving the hatch location. Day Care Eggs, when enabled, are tracked as
  breeding bonuses; gift-Egg randomization does not alter breeding species.
- Pokeball permission is checked in player command selection before Bag removal
  and turn submission; the command execution path has a second guard. Native
  battle alerts explain dupes, exhausted areas and history capacity. Safari and
  Pal Park selections have the same pre-consumption guard. Native capture
  mechanics still apply, including the restriction while two wild foes remain.
- Story activation latches `VAR_ROUTE_202_STATE >= 1`, after the catching lesson
  and first Ball gift. Possessing a Ball or entering a route is not activation.
  Starter selection, the first Starly battle and the tutorial spend no slots.
- Area notifications append a short count popup after the native map-name popup,
  using its existing window/task/frame. They suppress repeated logical zones,
  pre-activation, scripts and fades, and remain independent of tracking. They
  require the native location-name request on a map with wild encounters.
- Permadeath tags tracked owned Pokemon as soon as party HP reaches zero or the
  battle faint handler runs. HP recovery, item/revive use, Rare Candy, EXP gains
  and PC PP restoration cannot revive exhausted Pokemon. Native conversion from
  a boxed Pokemon also passes through the HP guard. Exhausted Pokemon do not
  appear as field-move helpers or followers (the original follower rejects 0 HP).
  Field return, storage and battle entry also reassert zero HP on tagged members,
  including after native temporary facility/link restoration paths.
- Exhausted party Pokemon move to box 18, named Memorial if it was empty, or any
  free box. A full PC never deletes them: they stay locked in the party. One
  exhausted member is retained on a wipe to preserve native nonempty-party
  assumptions. Withdrawal and party placement/swap are blocked; viewing and
  moving between boxes remain possible. Release of exhausted Pokemon is blocked.
- A total active-party wipe ends the run even with living boxed Pokemon.
  The wipe rule is derived from permadeath and cannot be toggled off in the UI.
  Parent story script contexts stop through their normal cleanup; field tasks unwind
  before a deferred blackout. All subsequent map loads are confined to Sandgem
  Pokemon Center 1F, at a valid tile. Nurse/common dialogue explains the ending.
  A checkpoint is attempted after loading the Center, before the explanation.
  A different prior adventure is never automatically overwritten. Save failure
  is shown explicitly. A failed checkpoint means persistence is not guaranteed
  until a successful native save. The registry offers an explicit destructive
  restart confirmation, warning that the adventure and memorial will be erased.
- Hardcore disables native Day Care Egg generation/claiming and the man explains
  the quarantine. Deposits and experience remain; exhausted Pokemon cannot be
  deposited. Day Care preview and withdrawal experience obey caps.
- Caps are generated from the actual unscaled vanilla leader aces in story order,
  then Cynthia with eight badges, plus configurable allowance 0..5. EXP is
  clamped to the exact threshold without banking surplus. Battle messages and
  gauges receive the bounded amount. Rare Candies cannot bypass the cap.
- Trainer scaling runs centrally after all four native party formats are built:
  min(100, base level + badge count * configured bonus). Boss classification and
  the boss switch are centralized. Player partners, link and Frontier parties
  are excluded. No source trainer JSON is modified.
- Deterministic species-only randomization hooks cover ordinary wild generation,
  radar/honey-tree paths, trainers, scripted statics and script gifts/Eggs. Wild
  levels remain native. Replaced trainers get native level-up moves, correct
  recalculated stats/ability and form zero, retaining held items, PID, IVs and
  Ball Capsules. Unreplaced trainers keep their authored movesets.
- For random/capture-restricted runs, field HM interactions can use a living
  helper when no party member knows the move. The actual HM disc and correct
  badge remain mandatory. Learnsets, TM/HM compatibility and battle moves are
  unchanged. This prevents restricted encounter pools from blocking mandatory
  field interactions; it does not add extra moves to the party menu.

## Persistent format and compatibility

Native SaveData table sizes, offsets, Pokemon/BoxPokemon sizes and Poketch size
are unchanged. New native IDs are appended, never inserted into existing lists.
`MiscSaveBlock.unused` stores a nonzero run nonce. A retail/Following save whose
anchor is zero remains Classic; no run extension is written for it. A nonzero
anchor requires matching valid run data. Corruption, missing extension, future
schema or changed data fingerprint fails closed with a title-return message;
there is no silent Classic reset or save replacement.

Compatibility is forward: old unconfigured saves can be read by this ROM.
New configured-run saves require this engine and its matching schema/data.
Older ROMs do not understand the run rules or the new Poketch app ID; reverse
compatibility is not promised despite unchanged native structure sizes.

`RunState` is a 65536-byte explicit little-endian wire buffer in root BSS, not a
field-heap allocation. The flash format is independent of C struct padding:

| Offset | Bytes | Contents |
| --- | --- | --- |
| 0 | 32 | RNS1 magic, schema 1, length/4, status, activation, nonce, native normal-block counter, record counts, world fingerprint, CRC32 |
| 32 | 36 | RUNC configuration schema 2 |
| 68 | 64 | Persistent evolutionary-family ownership bitmap |
| 132 | 7168 | 256 zones * 28 bytes: visited, used count, skipped duplicates, three 8-byte opportunities |
| 7300 | 5120 | 256 special events * 20 bytes: stable key, zone/species, last Pokemon ID, attempts/caught/failed, source/result |
| 12420 | 49152 | 1024 Pokemon * 48 bytes: PID, OTID, original/current species, origin/death zone, special-event ID, three evolutions, 11 native 16-bit name codes, flags |
| 61572 | 3964 | Reserved for future schema revisions |

RUNC layout: magic at 0, schema at 4, length at 6, rules version at 8, seed mode
at 10, reserved at 11, species fingerprint at 12, seed at 16, flags at 20,
tolerance/weight/attempts/preset at 24..27, cap allowance/bonus at 28..29,
reserved 30..31, CRC32 of bytes 0..31 at 32. All integers are little endian.
CRC32 uses reflected IEEE polynomial 0xEDB88320, initial/final XOR 0xFFFFFFFF.
Old prototype configuration schema 1 is rejected, not migrated.

Two snapshots occupy flash sectors 44..59 and 108..123 of the native 512KB
backup. Native main data (0..31 and 64..95) and extras (32..43 and 96..107)
are preserved. Runtime size checks reject any future overlapping native table.
Before committing a new native normal-block footer, the opposite run slot is
invalidated, its payload written, and its CRC-bearing header written LAST.
Loading requires both matching nonce and exact native normal counter. A torn
extension write leaves the previous native-paired copy usable. A pending
opportunity on a recovered snapshot becomes INTERRUPTED and remains spent.
The first-save path validates the run before native old-footer invalidation.
The loader uses a temporary 64KB application-heap buffer only during loading.
Real cartridge power-loss behavior has not been tested.

History links use an ID in native encrypted unused field 114, verified against
PID and OTID. Unused byte 113 has TRACKED 0x40 and EXHAUSTED 0x80. No party-slot
identity is persisted. Rename setters, hatching and evolution synchronize the
record. Shedinja receives an independent ID before any copied-ID setters run,
because vanilla deliberately shares PID/OTID with Ninjask; its derived record
retains the original capture origin. Removing/releasing a living Pokemon never
clears historical ownership.

## Area policy, randomizer and memory

`tools/datagen/run_zones.json` explicitly assigns every native map header to a
logical zone. Grass/surf/fishing and internal route sections share an allowance;
all floors of each named cave, lake, forest or facility share a zone. Town
interiors use that town's zone; unknown maps use Mystery Zone. New maps require
manifest review. Runtime uses map IDs and the committed table, not translated
names. Zone IDs are native location-bank indices; changing their order requires
review/versioning. The registry includes all mapped logical zones, including
towns and special locations without ordinary wild tables.

Species tables derive from local data: 493 BSTs, unioned evolution families,
26 legendary and 9 mythical species. Legendary and mythical species share the
restricted legendary pool; ordinary species remain outside it. Identity
replacements are allowed. The SHA-256-derived species fingerprint covers BSTs,
families and classes; the world fingerprint covers map grouping, caps and boss
classification. No Renegade or unrelated Delta gameplay data is imported.

Mappings are generated lazily without caches or persistent randomizer RNG.
A local 32-bit stream is keyed by resolved seed, species fingerprint, domain,
event key, slot, reference species and randomizer settings. Wild keys use map
ID and destination battler plus reference species; trainer keys use trainer ID
and team slot; gifts use map/script offset; static keys use map/original species.
Normal battle RNG is never used to select a replacement. Native generation
still uses normal RNG for IVs, nature, gender and other ordinary properties.
Future changes to mapping keys or gift-script instruction offsets must bump
the rules version, even if BST/world-table fingerprints have not changed.
The same explicit version review is required for encounter/trainer/gift tables,
growth rules and any future balance import: those inputs are not all covered
by the compact BST/world fingerprints. Do not reuse a version after such edits.

For reference BST B, tolerance T, strength S and distance d:

```
eligible = (100*d <= B*T) && poolAllows
weight = 100 + floor(9*S*(B*T - 100*d)/(B*T))
P(candidate) = weight / sum(eligible weights)
```

At strength 0, candidates are uniform. At 100, preference ranges from 1000 to
100, keeping every eligible candidate possible. If future exclusions empty a
pool, the documented fallback uniformly selects nearest BST within the required
legendary class; with current rules the original species always qualifies.
No candidate heap, species NARC scan or normal-RNG state is used.

Run state uses 64KB root BSS; species tables use about 2.5KB of read-only data.
World tables are compact byte arrays. Registry graphics share the 64KB Options
heap. Tracker uses the existing 48KB Poketch app heap, one 160x128 text window,
existing background graphics and palette. Popup adds no second window/task.
The completed ARM map gives the following measured static layout, compared with
the preexisting Following/prototype build. Sizes below are text plus data:

| Region | New bytes | Growth |
| --- | ---: | ---: |
| Main static text/data | 1121792 | 16224 |
| Main through BSS, measured from 0x02000000 | 2035456 | 81824 |
| Field overlay 5 | 228384 | 448 |
| Encounter overlay 6 | 47168 | 64 |
| Battle overlay | 220320 | 480 |
| PC boxes overlay | 63808 | 224 |
| Options/configuration/registry overlay | 10176 | 6176 |
| Reused Poketch unused-4/tracker overlay | 1472 | 672 |

Main BSS ends at `0x021F0F00`; the arena starts at `0x02290F60`, 82304 bytes
above the baseline `0x0227CDE0`. The four unchanged native base heaps total
1292304 bytes. Against the SDK's `0x023E0000` main-memory boundary, that leaves
80016 bytes before arena metadata, randomized initialization offset and other
OS allocations. This is a static budget, not measured runtime free memory.
All 122 linked overlay ranges fit their assigned RAM/TCM region. The 121
main-memory `After` relationships follow their parent's BSS end; the remaining
dummy overlay uses ITCM. NDS overlay table/FAT bounds were checked independently.
Actual heap high-water marks, VRAM behavior, timing and simultaneous runtime
overlay transitions require emulator testing.

## Known limits and unavailable work

- No emulator, visual, real-flash or hardware test has been executed. Allocation
  pressure, transitions and all native callbacks still require acceptance tests.
- Roamers keep their native stored species even with static randomization on,
  preserving native roamer species/HP/story bookkeeping. They are tracked and
  exempt as statics. Special facilities retain native parties rather than
  participating in run randomization/permadeath.
- Capture history is bounded to 1024 Pokemon and 256 aggregated special events.
  Ordinary captures fail closed at history capacity. Mandatory statics remain
  eligible; exhausted tagging still works if a new detailed record cannot fit.
  Exempt shinies/gifts also remain obtainable with aggregate event counters and
  a zero detailed-record link; acceptance never depends on a nonzero record ID.
  Repeated special events aggregate counters rather than keeping an unlimited
  per-attempt log. Additional death details such as fatal move/attacker are not
  stored; origin, final species, nickname and defeat location are stored.
- The registry has text/species pages; it does not add new Pokemon icon/sprite
  assets. Disabled rules display inactive values; notification toggles are set
  before the run and remain immutable with the other configuration.
- Common script gifts and Eggs are guarded and show native rejection dialogue.
  Rejected Eevee/Porygon/Riolu gifts and fossils preserve their pending native
  gift state and skip nickname prompts. Fossil revival is deliberately repeatable
  and aggregates event counters. Cynthia's restricted Togepi gift is recorded as
  failed and her story event advances so she cannot block the Bicycle shop.
  Some native pre-transfer dialogue/fanfares still precede the rejection message;
  they do not add a Pokemon or launch a nickname prompt for a denied transfer.
  Acquisitions outside these paths are observed into history;
  they do not acquire invented script identities or randomized trade tables.
- The HM safety helper supports contextual native field interactions, not a
  new all-HM submenu or changed learnsets. Fly/Flash remain optional menu moves.
- Caps use vanilla aces and remain at Cynthia's cap after eight badges; no
  separately designed postgame cap ladder or Renegade dataset exists here.
- A checkpoint cannot overwrite a different adventure. On I/O failure or when
  that overwrite protection applies, Game Over remains enforced in memory and
  displays a save warning; a successful native save is required for persistence.
- No defense against external save editors, restored backups or savestates is
  claimed. Schema/data-version changes require an explicit migration design.
- Verified Renegade Platinum balance data is absent. Species stats/types,
  abilities, items, TMs, evolution requirements, trainer and encounter balance
  files remain vanilla. Do not invent such data or import full Delta commits.

## Files and validation

Portable modules: `run_config`, `run_randomizer`, `run_state`, `run_store`,
`run_world`. Native bridge: `run_session`, `run_game`, `run_menu`. UI lives in
Options, Start menu, map-name popup and the reused unused Poketch app. Gameplay
hooks touch encounter/DTO, trainer generation, Pokemon setters, battle scripts
and item selection, party/PC menus, Day Care, gifts, evolution, script manager,
map transitions/blackout, save and game start. Meson/main.lsf register new root
objects and generated tables. No original species/trainer gameplay JSON changed.

Executed successfully; native/resource/whitespace checks were repeated after
the final PC-menu correction:

- `python tools/tests/run_engine_tests.py`: actual C99 sources under GCC
  -pedantic -Wall -Wextra -Werror, preset/codec checks, 493 species,
  exhaustive candidate weights, deterministic mappings, all 246 evolution
  edges, independent CRC32 plus 16 valid-checksum rejection cases, capture
  opportunities/exemptions/history/exhaustion, caps/scaling and mock flash with
  18 torn-write boundaries and native-sector preservation.
- `python tools/tests/check_run_integration.py`: JSON/message references,
  appended script/command/Poketch/subscript IDs and generator syntax.
- `git diff --check`.
- Final incremental WSL `make rom`: exit 0, ARM link and ROM generation completed.
- Final ARM map: 122 overlay bounds and 121 main-memory `After` relationships.
- Read-only NDS binary validation: header CRC16 `54C0`, game code `CPUE`, ARM9
  load/file bounds (1123544 bytes), all 122 overlay IDs, FAT references and
  RAM/TCM bounds. SHA-256 was computed with Windows `Get-FileHash`.

These tests exercise portable policy/storage and source/resource consistency.
They do not execute native UI, battle hooks, DS overlay transitions or flash.

## DeSmuME acceptance checklist (USER: all not yet executed)

Use a separate ROM/save copy; keep original saves and external backups intact.

1. New Game: each preset, every override/disabled option, eight-digit seed
   including zero, all summary rows, B navigation, confirmation; Rowan begins
   only after confirmation. Continue never reopens configuration.
2. Classic: normal intro, starter selection, first Starly, Following, maps,
   battle intros, bag, PC and saves; compare gameplay with Following-only base.
3. Before first Balls: visit 201/202, tutorial and starter battles; no consumed
   opportunities or capture notifications. Obtain first Balls normally; route
   201 must still offer its first opportunity.
4. Limits 1/2/3: catch, KO and flee consume exactly one opportunity per distinct
   eligible opponent. No-Ball encounters count. Try both wild foes in partner
   encounters and verify registry species/results and native two-foe behavior.
5. Duplicate branching family, including released/dead Eevee family: skipped,
   recorded and Ball blocked without item/turn loss. Disabling dupes restores
   ordinary capture eligibility. Test 0/OFF limits independently of randomizer.
6. Shiny in exhausted/duplicate areas: allowed bonus, no ordinary count change;
   KO/flee does not change count. Static legendary in exhausted/dupe area:
   allowed, tracked separately, native story restrictions still respected.
7. Gifts/Eggs: separate and shared allowances, repeat dialogue, party full,
   rejected gift, hatch/evolution/nickname, PC moves; verify persistent identity.
   Cancel an evolution; history must remain unchanged. Evolve Nincada with spare
   slot and Ball; Ninjask and Shedinja need separate IDs.
8. Registry before Poketch, all eight Start entries, scrolling/details/back,
   alive/dead nickname links, memorial and five special categories; save/reload.
9. Notifications with 1/2/3, exhaustion, same-zone floors/segments, rapid doors,
   scripts/fades, follower map transitions and OFF. Obtain Poketch through the
   three coupons; cycle to tracker and back, including sleep/save/area changes.
10. Faint by normal damage/recoil/poison; Center/NPC/story healing, Revive,
    candy, PC deposit/withdraw/swap, full memorial/PC, field moves and Following.
    Exhausted Pokemon must remain unusable and visible.
11. Wipe with a living boxed reserve: GAME OVER, safe Center, nurse explanation,
    automatic save result, doors/stairs/Fly/teleport/save/reload confinement,
    registry access and cancel/confirm restart. Verify no silent prior-save
    replacement when New Game was selected over another adventure.
12. Hardcore Day Care: no new Egg after many steps, quarantine dialogue,
    deposits/withdrawal/cap retained, unrelated gift Eggs allowed.
13. Exact cap EXP, surplus discarded, Rare Candy and Day Care; each badge unlocks
    next unscaled ace cap, allowance and scaling bonus; boss switch, rematches,
    doubles, scripted trainer fights and native partners.
14. Same seed/settings/data reproduce species across save/reload and different
    movement/battle RNG actions; different seeds vary. Check held items, legal
    moves/forms, legendary pools, wild levels and disabled domains. Obtain an HM
    and its badge with no compatible party species; contextual use must work.
15. Save/reload/new-slot/backup on emulator copies. Test read-only/truncated or
    missing extension on disposable saves: fail closed, no silent Classic reset.
    Record frame timing, memory failures, overlay/map/Following regressions.

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
