# Integrated run engine

## Current source restoration and automatic restart — 2026-11-08

The user explicitly requested the original gameplay dataset, accepted that
development runs will be incompatible, and changed the Game Over policy:
erase the ended adventure and open fresh run configuration automatically.
They also requested removal of the HELP reminder from configuration footers.
This section supersedes the earlier title/retained-history policy below.

Native-data audit and restoration:
- All4411 JSON definitions under res/pokemon, res/moves, res/items and
  res/trainers now match Following-only commit
  8f7ccec26e697e16df4fe4155915fbf28a038310. This includes508 species/forms.
- The initial audit found360 changed level-up lists plus a TM-only learnset
  change,108 altered move definitions, base-stat/EXP/held-item changes,
 14 changed Ball prices and two changed Cynthia item sets. These are restored.
  Read-only audit references and pre-restoration data copies are in artifacts/.
- Native type/ability getters, trainer personalities/ivScale, default/level-up
  learning,20-entry learnset capacity, trainer AI, immunity/critical/speed-item
  logic, weather modifiers, catch bonuses, Pickup chance, breeding compatibility,
  species/IV inheritance, egg production and hatch speed are restored.
- Cynthia gives Togepi again; the tutorial gives5 Poke Balls for either gender.
  Native HM forgetting restrictions, Pokedex first-registration behavior,
  Premier Ball shop bonus and Save confirmation prompts are restored.
- Following movement/palettes, field-heap and battle-introduction fixes remain.
  Explicit Randomizer/Nuzlocke controls, friendly-partner/tutorial safeguards,
  registry/Poketch, field-HM assistance and requested postgame event access remain.
  Compatibility symbols for removed Arceus/type-inference callbacks do nothing;
  they retain ABI without enabling the removed mechanics.

The original-stat dataset changes RUN_SPECIES_DATA_VERSION automatically
from0x29370BBA to0x59FB5FE7. Old development runs fail validation as
unsupported rather than being migrated or silently rerolled. Schema/rules
versions and binary save layouts are unchanged. No actual .sav file was opened
or modified by the agent. New development runs are required for this dataset.

Automatic Game Over implementation:
1. Existing loss rules finalize the battle DTO and records. First-battle,
   tutorial, link/facility exemptions and Classic ordinary blackout remain.
2. RunGame_EndRunTask closes the map and marks field shutdown. It no longer
   writes a final checkpoint or opens a child menu while field objects exist.
3. ReturnToTitleScreen frees the whole FieldSystem before enqueuing the
   standalone restart menu. This menu reads ApplicationArgs->saveData;
   the previous standalone path incorrectly interpreted ApplicationArgs as
   a direct SaveData pointer.
4. Both displays fade in; a native notice is visible for at least two frames
   before erasure. Success needs no button press. Script fallbacks release
   their tasks instead of waiting on a Game Over message.
5. RunGame_Restart checks the actual global SaveData pointer, configured/ended
   status, overwrite protection and matching run nonce before calling the
   native eraser. Erasure clears all128 flash sectors (including both run
   snapshots), RAM save data and session state. This explicitly clears the
   ended adventure's party, PC, Bag and history under the user's new policy.
6. After normal options-overlay teardown, native GameStartRowanIntro clears
   new-game RAM and enqueues run configuration before Rowan. Erase/identity
   failure cannot open configuration as success; it offers retry/title.
   Hardware flash faults retain the native save driver's fatal-error handling.

Controls/ReviewControls/ConfigNav no longer advertise X:HELP. The lower-screen
descriptions and modal X help still work; A/B edit/back controls remain.

Host tests passed for the portable engine, actual native getter/palette code,
all4411 data definitions and native core-function contracts. Restart tests
exercise the actual restart helper and native eraser in host RAM across64
identity/status/failure combinations,128 sectors and both snapshot ranges.
They also check teardown ordering and standalone arguments. These tests do
not execute ARM overlays, real flash, or visible emulator transitions.

The first correction build (session69945) was interrupted intentionally before
compilation to incorporate the user's additional moves/gameplay requests.
A Ninja dry run with -n -d explain is recorded in
artifacts/native-gameplay-build-plan.log; it selected747 tasks (685 compile,
60 generate,2 link), zero SDK targets and zero missing-command-log reasons.
The actual incremental make rom completed successfully in session76576:
exit0,746/746 tasks. No clean,
distclean, build-directory deletion, Git history change or remote push occurred.
The restoration ROM is build/pokeplatinum.us.nds,134217728 bytes, timestamp
2026-11-09 20:33:08. Final
`python tools/tests/check_run_rom.py artifacts/prompt5-before-map.json` passed:
header CRC16=AE71, ARM9/FAT bounds, all122 overlay tables and parent/BSS
ranges. Main static end02113F60, BSS end021F30C0, arena02293600; static
budget70128 bytes before metadata/OS costs. Arena change from the original
Prompt4 baseline is+4736 bytes. SHA256:
F120F864D1B1C9089100A1AD43C5EB1B2D10389FD0488B11366EDEBD046381A1.
The build's R4/YSMenu guard was filled from the new ARM9 bytes as intended;
its diagnostic is not a compiler error. No running build remains. These
binary/layout checks do not establish runtime heap/VRAM or emulator behavior.

Remaining manual acceptance tests: cold-boot a fresh run; confirm native types,
abilities/stats/moves,5 tutorial Balls, Togepi gift and native catch behavior;
test X help/clean footers; exercise individual death and full party wipe, then
verify automatic empty-save configuration without title input. Repeat with
Obedience-only loss, first rival defeat and Classic blackout, plus map/battle/
overlay/Following transitions and save/reload. The agent has not run an emulator.

## Earlier targeted native correction — superseded by the restoration above

The user's next playtest found randomized types/abilities and50 Poke Balls
after the catching tutorial. The working tree still contained IV-based
overrides from the earlier mod: HP/Speed IVs replaced type slots, and
Def/SpDef IVs selected another species' ability. These reads ignored all run
settings. They were missed by the preceding palette correction.

The correction restores the exact Following-only native cases for
MON_DATA_ABILITY and MON_DATA_TYPE_1/2 in src/pokemon.c. Ability reads return
the stored ability; types come from species/form data, with the original
Arceus+Multitype plate exception affecting both slots. The donor/custom-type
helpers are removed; the existing AI's native ability-slot lookup remains.
Native ability calculation is unchanged. No Pokemon PID/IV, run schema,
configuration, ownership, history or existing save file is rewritten.

Directly coupled changes are removed too: CanBoxPokemonLearnTM again consults
the species/form TM table, and TrainerData_BuildParty matches the native
Following-only function except for the existing RunGame_AdjustTrainerParty
hook. This removes Cynthia's donor/type IV builds, random trainer IVs and
the injected Mersenne seed; native ivScale and deterministic personalities
return. The run hook still handles chosen species, levels and friendly NPCs.
No Following, field-heap, battle-introduction or shiny/help fix is reverted.

Route202_GivePokeballs now supplies5 via the original shared Dawn/Lucas path.
Only the quantity constant and its misleading mod comment changed; native
event flags, tutorial and run-activation handling remain. A save that already
received50 keeps its Bag contents; this patch does not silently remove items.

tools/tests/check_native_traits.py compares the actual getter cases, ability
calculation, TM wrapper and trainer builder against commit
8f7ccec26e697e16df4fe4155915fbf28a038310. All508 species/form type and ability
definitions match that base. A host C harness executes the actual getter
cases over every HP/Speed and Def/SpDef IV pair, both native ability slots,
egg flags and both Arceus type slots. The tutorial gift is checked against
the same base. This check is included in run_engine_tests.py.

This was the earlier limited checkpoint. The subsequent user-requested audit
removed the per-type bonus learnsets and custom AI and restored original
gameplay data; see the current section above.

The 19:55 ROM is preserved at
artifacts/pokeplatinum.shiny-help-before-traits.nds (SHA256
626000742B5EBE5A8E08238328CB7ABAC4286DAB5A7A86ABD98E33DD7C2E92BB).
The next correction build/result will be recorded here after verification.
The shiny/help build described below predates these additional corrections.

## Shiny colors and X-help regression fixes — 2026-11-08

User playtesting of the 17:50 Prompt5 ROM reported PID-dependent colors,
inconsistent shiny behavior at 1/512, and X help capturing input without a
visible window. These are runtime reports from the user; the agent has not
run an emulator.

The native predicate in this working tree used SHINY_ODDS=32768, while the
run PID promotion algorithm assumes the original XOR<8 identity. A new host
integration test compiled the actual predicate and promotion functions and
reproduced 32768/65536 shinies at every rate before the fix. Both actual
palette hooks also failed byte-preservation tests before the fix.

Corrections:
- Restore native XOR<8 identity; configurable odds remain generation-time PID
  promotion, including value4 (1/512 Ultra High). Restore OTID_NOT_SHINY,
  real-shiny-only summary stars and the three native battle sparkle paths.
- Retain the public palette-hook symbols as no-op compatibility entry points.
  Native normal/shiny resource colors pass through unchanged in battle,
  summary, trade, hatching and Following. Disable the extra Totodile follower
  brightness adjustment; follower movement, raw HG palettes, white flash,
  loading paths and existing heap fixes remain.
- RunMenu_Main now fades both displays in and out. Help is drawn on SUB0;
  the previous MAIN_ONLY fade left the lower display black after New Game.
  Existing modal controls remain X open, A next/first, B/X close. Retail
  Options fades are unchanged.

No persistent field, schema, stored Pokemon PID, history, seed or quota is
rewritten. Pokemon previously classified shiny only by the erroneous 50%
criterion now display normally unless their stored PID is truly shiny.
Earlier special/shiny registry classifications are retained as historical
records; there is no silent quota/history migration.

The old build session55692 completed successfully (exit0,16831/16831) at
2026-11-08 17:50:25. Its binary/layout checks passed: 122 overlay records,
header CRC16=2534, arena02294DA0, static budget64080 bytes before metadata/OS
costs. SHA256 E409504B8A69B005C885EC460BFC45F42AD842B800999B924A31A6D627F70726.
An independent diagnostic copy is preserved as
`artifacts/pokeplatinum.prompt5-before-shiny-fix.nds`.

The correction build completed via incremental `make rom`, session4439:
exit0,1830/1830. The new ROM timestamp is 2026-11-08 19:55:23; size134217728
bytes. Final `check_run_rom.py artifacts/prompt5-before-map.json` passed:
header CRC16=87E4, ARM9/FAT bounds, all122 overlay tables and parent/BSS
ranges; arena02294AC0, static budget64816 bytes before metadata/OS costs.
SHA256: 626000742B5EBE5A8E08238328CB7ABAC4286DAB5A7A86ABD98E33DD7C2E92BB.
These binary/layout checks do not exercise runtime allocations or graphics.
No clean/distclean or build-directory deletion was performed. The earlier
broad rebuild was caused by missing command records in .ninja_log; the log
is now preserved. The build session is finished; no second build was launched.

Host verification passed after the fixes: portable engine C tests, all five
native shiny rates over every XOR value/four trainer IDs, both palette hooks
over all 65536 color words/four PIDs, native icon/sparkle wiring, resources,
font bounds and two-display run-menu fades. These checks do not prove visible
help or battle graphics on Nintendo DS; the user must retest those behaviors.

## Current revision and verification

Prompt5, Ultra High Shiny Rate and the above corrections are implemented on
`nuzlocke-base`. The user explicitly authorized switching to this feature
branch; main, Git history and the remote have not been changed. Existing
Following/field-heap/battle-introduction fixes are preserved.

The previous user-tested Prompt4 ROM remains independently protected at
`artifacts/pokeplatinum.prompt4-working.nds`, SHA256
E3B63FBC93A0F1C5BFB426C429E39FA960AB5E5CBA6EEC375C7F6B0E1FC85522.
Its map baseline is `artifacts/prompt5-before-map.json`.
The correction ROM build and binary/layout checks passed as recorded above.
Emulator checks remain pending: cold-boot the new ROM without an older build's
savestate; verify X help on both displays (A paging, B/X close), original normal
and shiny palettes, matching summary stars/battle sparkles/Following, first
battle and return to the overworld. Existing native save files were not edited.

## Prompt5 implementation

- X opens a modal explanation for every configurable option, preset and seed.
  Native framed lower-screen window, selected-option title, automatic wrapping
  at216 pixels, six lines per page, A next/first and B/X close. Input is captured
  while open; the same cursor/scroll is restored and closing cannot confirm the
  run. All25 option explanations are separate localized strings; disabled
  settings show a parent-rule notice and descriptions explain the dependency.
- Permadeath is the single visible switch in configuration, review and Registry
  settings. Presets/new edits set/clear both old internal death/wipe bits.
  Legacy wire data is not rewritten: PERMADEATH is authoritative, the old wipe
  bit remains encoded byte-exactly but is ignored for effective-preset comparison
  and loss-policy decisions. Thus a legacy death-ON/wipe-OFF combination now
  has the mandatory wipe behavior; death-OFF/wipe-ON remains ordinary fainting.
  No history or ownership is erased. Obedience-only loss remains independent.
- Numeric controls use one shared tested clamp helper. BST Range and Similarity
  move by5 percentage points, display percentages, and stop at10/75 and0/100.
  Other numeric domains retain steps of1; hexadecimal seed editing is separate.
- Shiny Rate adds `1/512 Ultra High` as value4. Values0..3 are unchanged and the
  new message ID is appended; an explicit display table avoids shifting old IDs.
  RUNC remains36 bytes/schema3. Older ROMs that only accept0..3 cannot load an
  Ultra High run; this ROM continues to read old runs unchanged. The generator
  promotes XOR values8..127 and preserves original forced shinies, nature,
  gender/ability byte and Wurmple branch. Shiny Clause remains independent.
- Regi ruins now accept postgame + National Dex instead of requiring an imported
  fateful Regigigas. Native door conditions, maps, dot puzzles, statue states,
  levels and retry/caught rules remain. Regigigas still requires the three
  titans in the party; with Static Randomizer ON, native caught states for all
  three ruins also prove completion when their species were replaced.
- Friendly field/story allies use the same single trainer loader: Cheryl, Mira,
  Riley, Marley, Buck, Dawn/Lucas and Barry. All three wild-partner loaders and
  the scripted cooperative-trainer path reach it. The old enemy-side-only
  exclusion is removed, PLAYER1 is protected, and allies keep their original
  levels. Each trainer ID/slot uses the existing seeded BST mapping exactly
  once. Link/Frontier facilities remain outside the existing run randomizer.
- Dawn/Lucas's demo uses distinct stable tutorial keys for wild/ally species.
  Wild OFF / Trainer OFF retains each respective original Pokemon and moves.
  Randomized demo parties have temporary Tackle-only moves to prevent Teleport
  or self-destruction; demo damage is nonlethal. The native ALWAYS_CATCH flag
  already guarantees capture. Tutorial bags/parties stay temporary, history,
  death/EXP/capture rules are suspended, and the player's first-Ball latch is
  unaffected. The native FIRST_BATTLE loss story branch is also exempt from
  permadeath; it keeps its original return-home/healing behavior.
- A fatal battle finalizes opportunity/death records before a native field task
  saves the final GAME OVER state. It waits for map and battle overlays to close,
  shows a brief native information screen (120 frames, A/B may skip), then ends
  the FieldSystem through its existing title-screen exit. No Center respawn or
  uncontrolled hardware reset. Continue on a saved defeated run shows the notice
  and returns to title; New Game stays usable. Failed/protected saves require
  acknowledgement and warn explicitly; an in-process nonce latch also blocks
  reloading that run after a failed write. A power loss cannot preserve history
  that the storage device refused to write. Unrelated adventures are never
  silently overwritten.
- Poketch drawing is redesigned for the192x160 LCD: native active-theme palette,
  title/location header, rounded opportunity marks, large seven-segment fraction,
  separate capture state, compact cap/badge/bonus panel and run-status footer.
  Towns such as Jubilife show NO WILD ENCOUNTERS, not a fake allowance. OFF and
  tracking-inactive states omit the fraction. One eligible Obedience combatant
  shows ACTIVE:1 READY and a warning mark. A small state cache refreshes only
  changed data, checked every15 frames. Long names are bounded to184 pixels.
  Window24x20 tiles uses15360 bytes of heap pixels and tiles32..511 (ending at
  char offset0x4000, below the BG map at0x7000). No new bitmap/sprite resources,
  persistent save fields or app lifecycle changes are introduced.

Native-asset layout preview: `artifacts/prompt5-tracker-preview.png`. This is a
host rendering of the layout/font/palette, NOT an emulator screenshot.

### Static encounter audit

| Encounters | Native path / result of source audit |
| --- | --- |
| Regirock / Regice / Registeel | Rock Peak / Iceberg / Iron Ruins; original level30 puzzles; postgame National Dex unlock |
| Regigigas | Snowpoint Temple B5F, level1; native party prerequisite or randomized-ruins capture proof |
| Uxie / Azelf | Acuity / Valor Cavern, StartLegendaryBattle |
| Mesprit / Cresselia / roaming birds | Native roaming state; battle ROAMER bit maps to STATIC permission; stored species retained |
| Giratina | Distortion origin-form and Turnback Cave paths both call scripted static generation |
| Dialga / Palkia | Original Spear Pillar level70 scripts |
| Heatran | Original Stark Mountain room3 level50 script |
| Darkrai / Shaymin | Original Newmoon / Flower Paradise scripts and existing obtainable key-item access |
| Rotom / Drifloon / Spiritomb / other fixed scripts | Shared scripted-wild constructor assigns STATIC event identity |

All these permissions use encounter source/event identity, never a legendary
species test. Static priority bypasses quota/dupes/history capacity, records
Special counters and never spends a route attempt. Retried events reuse their
map/original-species key and native one-time rules. Ordinary randomized wild
legendaries remain WILD. Native trainer/story catching restrictions remain.
Actual Ball use in these ARM formats still needs the playtests below.

### Prompt5 acceptance checklist — not yet executed by the agent

1. All25 X explanations plus preset/seed/review: wrapping/page navigation,
   disabled reasons, exact cursor/scroll return, simultaneous A/X and B/X,
   no edits or accidental confirmation. Check every option in read-only settings.
2. One Permadeath row only; all presets, ON/OFF, legacy mixed flag combinations,
   +5/-5 numeric taps/repeats and clamped boundaries; other increments1 and
   seed editing; saved values survive reload. All five Shiny Rates, including
   Ultra High1/512, with Shiny Clause ON/OFF and unchanged existing Pokemon.
3. Each Regi entrance before/after postgame/National Dex, dots/statues, catch,
   KO/flee/native retries, Regigigas party requirement and randomized proof.
   Every listed static with exhausted route and dupes, Static Randomizer OFF/ON,
   Ball count/turn behavior, special record identity and reload. Mesprit/Cresselia
   roaming HP/state persistence and ordinary randomized wild legendary denial.
4. Each named ally, wild partner and scripted cooperative fight: Trainer ON/OFF,
   same seed teams, original ally levels, legal moves, native AI/healing/party
   structure, no changes to player Pokemon, no double randomization.
5. Both-gender catching tutorials with Wild/Trainer ON/OFF combinations,
   difficult replacement species, guaranteed catch, correct name/cry/sprite,
   no player Ball/ownership/opportunity changes or premature activation.
   First rival loss remains nonfatal and the story heals/returns home normally.
6. Permadeath wipe with living PC reserve, Obedience-only zero eligible,
   nonfatal fainting with a survivor, cap-raising Gym win, Classic normal loss.
   Fatal cases go directly to title, save GAME OVER/history, and cannot Continue;
   New Game works. Simulate protected/failing save only on disposable copies:
   explicit warning, no unrelated overwrite, in-process Continue blocked.
7. Poketch on the actual DS resolution, all color themes, route/town transitions,
   quota1/2/3 and exhaustion, activation/OFF, catches/KO/flee, badges/cap/bonus,
   Obedience warning, long names and sleeping/battle/app switching. Repeated
   unload/reload must not leak or leave stale pixels; Following and map loads
   remain stable. Compare to the supplied photo and the layout preview.

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
1/2048, 1/1024 and 1/512 and affects newly generated wild/gift/Egg Pokemon only.
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
automatic final checkpoint and direct native title transition are implemented. A checkpoint does not overwrite another adventure; failed I/O or
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
   HP/alive history. Check direct title/Game Over reason/save/reload/restart. One
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

The preceding Prompt3/4 build completed Nov5 at15:02:39. Its current binary was
independently verified on Nov8: CRC16=9E48, main static end02112E40, BSS end021F1F40,
arena02292380, static budget74864 bytes before metadata/OS costs. The new Prompt5
build result will be appended here after it actually completes.

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
