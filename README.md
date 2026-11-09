# Pokémon Platinum — Following Pokémon, Randomizer & Nuzlocke

**A customizable Pokémon Platinum (Nintendo DS) adventure with HGSS-style following Pokémon, a built-in seeded Randomizer, and an in-game Nuzlocke rules engine.**

> [!IMPORTANT]
> **Active development / playtesting build.** The gameplay branch is [`nuzlocke-base`](https://github.com/AngeloLav/platinum_following_randomizerNuzlocke/tree/nuzlocke-base), **not** the repository's current `main` branch. The ROM builds locally, and early-game features have been playtested, but the latest changes still require comprehensive emulator regression testing. This is not a polished, fully validated release.

[Features](#features) · [Game modes](#game-modes) · [All settings](#all-configurable-settings) · [Nuzlocke rules](#capture-rules) · [Build guide](#building-from-source) · [Development status](#development-status)

## Overview

This project extends the [pret Pokémon Platinum decompilation](https://github.com/pret/pokeplatinum) and the **following-Pokémon-only** portion of [Delta Platinum](https://github.com/Dusknight4/deltaplatinum). Its aim is to make self-enforced, replayable Nuzlocke and randomized runs possible **inside the Nintendo DS game itself**, without an external randomizer or a manual encounter spreadsheet.

The player selects a preset or customizes individual rules **before Professor Rowan's introduction**. The selected settings and seed are saved with the adventure and become read-only once confirmed. The game then enforces capture opportunities, level restrictions, permanent fainting, and other enabled rules.

The interfaces deliberately reuse Pokémon Platinum's own text, windows, palettes, cursors, transitions, and menu conventions.

### Features

- **HGSS-style following Pokémon**, with follower interaction and support for normal story transitions.
- **Seeded species Randomizer** for wild Pokémon and trainer parties, plus independently configurable static encounters, gifts, and gift Eggs.
- **BST-aware randomization**, with an adjustable eligible range and probability bias toward similarly strong species.
- **Custom Nuzlocke capture limits** of 1, 2, or 3 eligible encounters per area, including enforcement before Poké Ball consumption.
- **Persistent encounter registry**, evolutionary-family Dupes Clause, optional Shiny Clause, and separate static/gift tracking.
- **Permadeath**, an EXHAUSTED memorial, and mandatory party-wipe Game Over under the single Permadeath switch.
- **Two level-cap modes:** EXP Lock and Obedience Cap.
- **Dynamic trainer levels**, configurable at +1 to +3 levels per badge.
- **Shiny Rate** options from 1/8192 to 1/512, original normal/shiny colors, and an optional safe Field Item Randomizer.
- **Native Run Registry** available from the Start menu before obtaining the Pokétch, plus a compact Pokétch tracker later.
- **Brief route notifications** showing remaining encounter opportunities.
- **Accessible original Darkrai and Shaymin events** through normal postgame progression, without expired event distributions.

## Game modes

At the start of a new game, choose a preset, adjust any setting, review the seed and rules, and confirm the run.

| Preset | What it does |
| --- | --- |
| **Classic Platinum** | Standard-style gameplay without Randomizer or Nuzlocke enforcement. Following Pokémon remains available. |
| **Randomizer** | Randomized wild Pokémon and trainer teams, without automatically imposing Nuzlocke capture or death restrictions. |
| **Classic Nuzlocke** | One ordinary encounter opportunity per area, evolutionary-family Dupes Clause, Shiny Clause, permadeath, and party-wipe Game Over. |
| **Hardcore Randomizer** | Wild/trainer randomization, capture restrictions, permadeath, EXP Lock level caps, trainer scaling, and Day Care breeding restrictions. |
| **Custom** | Any supported combination of settings, subject to rules/dependency validation. |

Overriding a preset's values makes the effective profile **Custom**. The chosen run settings cannot be changed after confirmation.

## All configurable settings

The initial configuration screen uses **one vertically scrolling list**, with context-sensitive explanations on the lower screen. Press **X: HELP** on any setting for a titled, automatically wrapped explanation; A changes help pages and B/X closes without changing your selection. Numeric boundaries clamp; BST controls move by exactly 5 percentage points. Values below are the default configuration for **Hardcore Randomizer** where a preset-specific value is applicable; other presets may differ.

### Randomizer

| Setting | Available values | Hardcore default | Explanation |
| --- | --- | --- | --- |
| **Wild Randomizer** | ON / OFF | ON | Replaces ordinary wild encounter species. |
| **Trainer Randomizer** | ON / OFF | ON | Replaces trainer party species while preserving valid battle parties. |
| **BST Range** | ±10% to ±75%, steps of 5% | **±25%** | Maximum allowed difference from the original species' Base Stat Total. |
| **BST Similarity** | 0% to 100%, steps of 5% | **75%** | How strongly species with a closer BST are favored within that range. |
| **Legendary Pool** | Separate / Unrestricted | Separate | Keeps legendary/mythical replacements in their own restricted class when enabled. |
| **Static Encounter Randomizer** | ON / OFF | OFF | Randomizes supported scripted, fixed Pokémon encounters without changing their event identity. |
| **Gift Pokémon Randomizer** | ON / OFF | OFF | Randomizes supported gifted Pokémon, including the starter choices when enabled. |
| **Gift Egg Randomizer** | ON / OFF | OFF | Randomizes supported gift Eggs. |
| **Randomize Field Items** | ON / OFF | **OFF** | Randomizes supported ordinary visible/hidden field pickups; important items are protected. |
| **Seed** | Automatic / Manual | Automatic | Controls reproducible random mappings. |

### Captures and shinies

| Setting | Available values | Hardcore default | Explanation |
| --- | --- | --- | --- |
| **Capture Opportunities** | OFF / 1 / 2 / 3 per area | **1** | Number of distinct eligible ordinary encounters, **not** guaranteed successful catches. |
| **Dupes Clause** | ON / OFF | ON | Previously owned evolution families cannot be caught again as ordinary encounters. |
| **Shiny Clause** | ON / OFF | ON | Eligible shiny wild encounters can be captured as bonus encounters without spending an ordinary attempt. |
| **Shiny Rate** | 1/8192, 1/4096, 1/2048, 1/1024 / **1/512 Ultra High** | **1/8192** | Changes shiny generation odds in supported new-Pokémon generation paths. |
| **Gift Pokémon Separate** | ON / OFF | ON | Gifts can be tracked independently of ordinary area limits. |
| **Gift Eggs Separate** | ON / OFF | ON | Gift Eggs can be tracked independently of ordinary area limits. |

### Hardcore rules and difficulty

| Setting | Available values | Hardcore default | Explanation |
| --- | --- | --- | --- |
| **Permadeath** | ON / OFF | ON | A Pokémon reaching 0 HP becomes permanently **EXHAUSTED**. |
| **Day Care Eggs** | Allowed / Disabled | Disabled | Prevents generation of ordinary Day Care Eggs; unrelated gift Eggs remain separate. |
| **Level Cap** | ON / OFF | ON | Uses the next original Gym Leader ace level, plus any allowance. |
| **Level Cap Mode** | **EXP Lock / Obedience Cap** | **EXP Lock** | Determines how Pokémon behave at or above the cap. |
| **Level Cap Allowance** | +0 to +5 levels | +0 | Raises the effective cap without changing the original reference boss. |
| **Trainer Scaling** | ON / OFF | ON | Dynamically increases opposing trainer levels as badges are earned. |
| **Levels per Badge** | +1 / +2 / +3 | **+1** | Extra levels applied per earned badge when scaling is on. |
| **Scale Bosses** | ON / OFF | ON | Includes supported major bosses in trainer-level scaling. |

### Interface

| Setting | Available values | Hardcore default | Explanation |
| --- | --- | --- | --- |
| **Area Capture Notifications** | ON / OFF | ON | Briefly displays the area's remaining ordinary encounter opportunities on entry. |

> [!NOTE]
> Some settings have dependencies. For example, Level Cap Mode has no gameplay effect when Level Cap is OFF, and the ordinary capture clauses depend on capture restrictions being enabled. A run's saved configuration is validated before use.

## Understanding BST Range and BST Similarity

**BST (Base Stat Total)** is the sum of a species' six **base** stats: HP, Attack, Defense, Special Attack, Special Defense, and Speed. It is **not** the Pokémon's displayed stats at its current level. A species has the same BST at level 5 and level 50.

Suppose the original encounter species has **BST 400**:

- **BST Range ±25%** permits replacement species whose BST is between **300 and 500**.
- **BST Similarity 0%** gives each eligible species equal probability.
- **BST Similarity 75%** favors species closer to BST 400 but still permits eligible species near the edges.
- **BST Similarity 100%** uses the strongest available Gaussian bias; it is **not** a guarantee that the replacement has exactly the same BST.

The current rules use a **truncated Gaussian mixed with a uniform distribution** over the eligible species:

```text
Gaussian_i = exp(-0.5 * ((candidateBST_i - originalBST) / sigma)^2)
Probability_i = (1 - similarity) / N
              + similarity * Gaussian_i / sum(Gaussian)
```

Here, `similarity` is a fraction from 0 to 1, `N` is the number of eligible species, and the Gaussian width is derived from the selected BST range. The Nintendo DS implementation uses an integer lookup-table approximation rather than floating-point sampling. Legendary-pool restrictions are applied **before** choosing a candidate. The same principle applies to ordinary wild replacements and trainers.

The result aims to keep early encounters broadly comparable to the Pokémon they replace while allowing variety across the whole game. **BST is not recalculated based on level or badges.**

## Capture rules

### When tracking begins

Capture restrictions activate **the first time the player obtains a usable Poké Ball**, whether from a Poké Mart, gift, field pickup, or the normal tutorial. The activation is permanent for that run, even if the player later uses or loses every Ball.

Before the first Ball is obtained, wild encounters—including the mandatory early Starly battle—**do not consume route opportunities**. Previously visited routes remain available.

### First encounter, failure, and multiple attempts

With **1 opportunity per area**, the first eligible ordinary wild encounter spends the area's only chance, even if it is defeated, flees, or is not caught. A later Pokémon of the **same or different species** cannot be caught as an ordinary encounter there.

With **2 or 3 opportunities**, each opportunity is reserved by a **different eligible species**. An already-attempted species cannot be retried in that area; encountering it again does not create a new capture opportunity. The registry records every attempt and its result.

For example, with three opportunities on Route 203:

| Opportunity | Encounter | Outcome |
| --- | --- | --- |
| 1 | Shinx | Caught |
| 2 | Starly | Defeated — opportunity spent |
| 3 | Not encountered | Still available |

When a capture is forbidden, the game prevents invalid Poké Ball use **before consuming the Ball**. The battle interface also shows a compact capture-eligibility indicator where supported.

### Evolution-family Dupes Clause

If enabled, a Pokémon from an evolutionary family already owned during the run is not a new ordinary opportunity and cannot be captured normally. Ownership history persists after a Pokémon evolves, is released, or becomes exhausted. Branched families, such as Eevee's evolutions, count as one family.

### Shiny Clause versus Shiny Rate

These settings do different jobs:

- **Shiny Rate** changes the chance of new eligible Pokémon being generated shiny. Supported rates are **1/8192**, **1/4096**, **1/2048**, and **1/1024**. The implementation covers newly generated wild Pokémon, gifts, and Eggs; special scripted restrictions and already existing Pokémon are preserved.
- **Shiny Clause** determines whether a shiny **wild encounter** may be caught outside the normal route quota. Turning up the Shiny Rate does **not** automatically grant extra captures if Shiny Clause is OFF.

### Static Pokémon, gifts, and Eggs

- **Genuine, normally catchable static encounters** are always eligible independently of the area's ordinary quota, including when the area is exhausted. They appear in the **SPECIAL** registry.
- Random wild Pokémon that happen to be legendary are **not** treated as scripted static encounters.
- Gift Pokémon and gift Eggs can have separate encounter allowances when the relevant settings are enabled.
- Hatching a gifted Egg does not spend the ordinary capture opportunity of the hatching location.
- Ordinary Day Care Eggs can be disabled separately.

The area's logical identity—not a translated map name—is used for quota tracking. Grass, surfing, and fishing within the same route share its allowance; related floors of a named cave are grouped together.

## Level caps and trainer difficulty

The effective level cap is the **original next Gym Leader's ace level + the configured allowance** (maximum level 100). After eight badges, the current progression uses **Cynthia's original ace** as the reference. Enemy trainer scaling does **not** raise the player's cap.

### EXP Lock — default

Pokémon may fight at the cap, but cannot gain EXP above it. They receive **0 EXP** once capped, and any excess EXP from an award that would cross the cap is discarded. Rare Candies and Day Care leveling respect this restriction.

EXP is evaluated **per recipient** after the game's normal distribution rules. If two Pokémon would each receive 50 EXP, but one is capped, that Pokémon receives 0 and the other still receives **50**, not 100. No discarded EXP is reassigned.

### Obedience Cap

Pokémon continue gaining EXP and may exceed the cap. A Pokémon **strictly above** the cap, however, refuses to enter battle. This is a firm eligibility restriction, not ordinary random disobedience.

- At exactly the cap: **eligible**.
- Above the cap: **cannot be sent out or switched in**.
- After a badge increases the cap sufficiently: **eligible again**.
- Overleveled Pokémon remain alive and are **not** marked EXHAUSTED solely for refusing.
- An active Pokémon that overlevels during a battle may finish that battle but cannot re-enter after switching out.

**If no eligible active-party Pokémon remain, the run ends in Game Over**, even if the party contains living overleveled Pokémon or the PC contains healthy reserves. This additional Obedience loss condition applies even when permadeath is disabled.

### Trainer scaling

When enabled, trainer Pokémon use:

```text
Effective trainer level = min(100, original level + badges × levels per badge)
```

For a level-30 trainer after earning three badges:

| Setting | Effective level |
| --- | --- |
| +1 per badge | 33 |
| +2 per badge | 36 |
| +3 per badge | 39 |

Wild encounter levels remain unchanged. Boss scaling is a separate toggle.

## Permadeath and Game Over

When **Permadeath** is active, a Pokémon that reaches 0 HP becomes permanently **EXHAUSTED**. It cannot be returned to combat through ordinary healing, revives, Pokémon Centers, or PC withdrawal. Exhausted Pokémon are tracked in the memorial, and an exhausted follower is no longer permitted to follow the player.

With **Permadeath ON**, losing every eligible active-party Pokémon ends the run even with living PC reserves. After battle and field cleanup, a native notice appears, the ended adventure is erased, and the run configuration menu opens automatically. This clears that run's party, PC, Bag and history. Obedience Cap can separately end a run even with Permadeath OFF. An identity/protection failure blocks erasure and offers retry or return to title; native flash errors retain the game's error handling.

> [!WARNING]
> Save restoration and emulator savestates are outside the in-game enforcement system. Back up your saves before updating development builds; configured saves are not guaranteed to work in older ROM revisions.

## In-game Nuzlocke Registry

The **NUZLOCKE / Run Registry** is available from the ordinary **Start menu before the Pokétch is obtained**.

The first screen prioritizes the **current area**: remaining capture opportunities, the first eligible encounter, its result, and a quick route status. Below that, the list summarizes badges, next cap, trainer bonus, seed, Randomizer settings, and run progress.

Press **L/R** to switch between five dedicated pages:

| Page | Contents |
| --- | --- |
| **RUN** | Current area first, remaining encounters, badges, level cap, scaling, seed, and overall progress. |
| **AREAS** | All tracked zones, available/used opportunities, encountered species, and capture outcomes. |
| **POKÉMON** | Owned Pokémon, nicknames, origin, evolutions, and current status. |
| **MEMORIAL** | Permanently exhausted Pokémon and their recorded history. |
| **SPECIAL** | Static encounters, gifts, gift Eggs, and shiny bonus records; includes Darkrai/Shaymin event status. |

The pages display explicit **L/R navigation hints** and use scrollable native-style menus. A native LCD **Pokétch tracker** becomes available after receiving the Pokétch in Jubilife City. It emphasizes remaining/total opportunities with large digits and small opportunity marks, distinguishes unavailable/exhausted/inactive/OFF areas, and shows no invented wild allowance in towns such as Jubilife. Secondary information shows cap, badges, trainer bonus and run status. It refreshes only changed data and follows the native color theme. Brief area-entry notifications reuse the familiar map-name popup style and can be disabled.

## Randomized starter selection and Rowan introduction

When gift/starter randomization is enabled, the starter briefcase shows the **actual randomized names and sprites before selection**. The displayed Pokémon, the awarded Pokémon, and the initial rival battle share a consistent seeded mapping.

Professor Rowan's introductory Poké Ball demonstration can show a **seeded random species** as a cosmetic presentation change, without altering the normal encounter Randomizer's results.

## Original mythical events: Darkrai and Shaymin

The original Pokémon Platinum event areas are accessible through normal postgame progression rather than expired Mystery Gift distributions:

- **Darkrai — Harbor Inn / Newmoon Island:** obtain the **Member Card** through the implemented in-game event path. The original Cresselia/Lunar Wing and Sailor Eldritch family prerequisites still matter.
- **Shaymin — Route 224 / Seabreak Path / Flower Paradise:** obtain **Oak's Letter** and follow Professor Oak's original event sequence.

The event-item acquisition path uses Professor Oak in **Eterna City's south house** after game completion and the National Pokédex. The Member Card has the additional Cresselia-related prerequisite. Both are genuine static encounters, separate from normal route capture limits, and their event items are excluded from Field Item Randomization.

These are **original event sequences being made accessible**, not direct Pokémon gifts or arbitrary wild-table additions. Full event progression still needs dedicated emulator testing.

## Field Item Randomizer

If enabled, supported **visible Poké Ball pickups and hidden overworld items** are replaced deterministically using the run seed and item location. Pickup quantities, normal one-time flags, and item acquisition messages remain tied to the actual awarded item.

The safe replacement pool explicitly excludes **Key Items, TMs/HMs, fossils, mail, progression-related orbs, and important event tools**. Shops, NPC gifts, and held items are not randomized by this option. The default is **OFF**.

## Building from source

### Requirements

The project currently targets the **English (US) Pokémon Platinum** decompilation/build pipeline. The recommended development environment on Windows is **WSL2 with Ubuntu**. A native Debian/Ubuntu Linux environment also works with the same general dependencies.

**Windows:** install WSL2 from PowerShell:

```powershell
wsl --install -d Ubuntu
wsl --list --verbose
wsl -d Ubuntu
```

**Inside Ubuntu**, install the dependencies used by the upstream [INSTALL.md](INSTALL.md):

```bash
sudo dpkg --add-architecture i386
sudo apt update
sudo apt install -y \
  bison flex g++ gcc-arm-none-eabi git make ninja-build \
  pkg-config wget python3 xz-utils nasm libc6:i386 libpng-dev
```

The `libc6:i386` package matters because the project's MetroWerks-compatible build toolchain includes 32-bit executables. The build system downloads/configures its required toolchain and SDK subprojects; a separate retail ROM extraction step is not part of the documented standard build process.

For other systems, or additional debugger setup, consult [INSTALL.md](INSTALL.md) and [pret/pokeplatinum](https://github.com/pret/pokeplatinum).

### Clone the gameplay branch

```bash
git clone --branch nuzlocke-base \
  https://github.com/AngeloLav/platinum_following_randomizerNuzlocke.git
cd platinum_following_randomizerNuzlocke
```

> [!TIP]
> For better WSL2 build performance, clone inside the **Linux filesystem** (for example, under `~/projects/`) and open it through VS Code's WSL extension. Building a repository stored on `/mnt/c` or `/mnt/d` also works but can have significantly slower filesystem access and linking.

If you already cloned this repository on a different branch:

```bash
git fetch origin
git switch nuzlocke-base
```

Do not confuse this branch with `main`, which currently follows a different Delta Platinum history.

### Compile the ROM

From the repository root:

```bash
make rom
```

The output is:

```text
build/pokeplatinum.us.nds
```

**Use `make rom` for this modified game.** The upstream `make` target also runs comparison/check tests against retail Platinum, whose original checksums naturally do **not** match a modified ROM.

For an incremental dry run, after the first successful configuration:

```bash
ninja -C build -n pokeplatinum.us.nds
```

To build with debugging support:

```bash
make debug
```

Initial builds and ARM linking may take a long time. Ninja reuses completed work; **do not run `make clean` or `make distclean` unless you actually need a clean rebuild**.

### Run the native tests

These tests exercise portable C logic and source/resource integration without launching a Nintendo DS emulator:

```bash
python3 tools/tests/run_engine_tests.py
python3 tools/tests/check_run_integration.py
git diff --check
```

Run `make rom` separately to check ARM compilation, linking, and `.nds` creation. A successful build alone does not prove that battle transitions, graphics, saves, or special event scripts are correct in an emulator.

### Play in an emulator

Open `build/pokeplatinum.us.nds` in a compatible Nintendo DS emulator such as **DeSmuME** or **melonDS**. Create a new save for first-time testing and keep backups of important `.sav`/`.dsv` files. The project is being tested as a development build, and save compatibility may change.

## Project layout

| Path | Purpose |
| --- | --- |
| `src/` | Game C source, including battle, field, menus, and `run_*` engine modules. |
| `include/` | Headers, configuration types, constants, and gameplay interfaces. |
| `res/` | Pokémon/trainer data, text, scripts, graphics, and other game assets. |
| `asm/` | Assembly and scripting macros. |
| `tools/datagen/` | Generates species/family/BST, map-zone, and boss-reference data. |
| `tools/tests/` | Portable C tests and integration checks for the run engine. |
| `docs/RUN_ENGINE.md` | Technical implementation, persistence details, compatibility, and test matrix. |
| `docs/FOLLOWER_POKEMON.md` | Following Pokémon implementation notes (where present in the branch). |
| `INSTALL.md` | Upstream-supported cross-platform build instructions. |
| `build/` | Generated objects, maps, and ROM (ignored by Git). |

### Technical notes for contributors

- The Randomizer uses **seeded, domain-separated mappings** rather than consuming the regular battle RNG for species selection.
- Run data is versioned and validated. Changing species tables, zone mappings, or save layouts may require an explicit compatibility/version migration.
- The existing Following Pokémon field-heap and battle-introduction fixes must be preserved.
- Renegade Platinum's gameplay/balance data **has not yet been imported**. Do not describe this branch as a complete Renegade Platinum port.
- Prefer small, independently testable changes and update `docs/RUN_ENGINE.md` when altering rules or persistent formats.

## Development status

**Status: playable development build; full verification in progress.**

- Local compilation of the modified `.nds` has succeeded, including incremental ARM linking.
- Earlier builds were manually played through the early game and Jubilife City, including ordinary wild catches.
- Current documented native C tests cover configuration validation, random mappings, captures, level-cap mechanics, and save-state simulations.
- The latest large feature revision (including both cap modes, new UI, shiny options, field items, and mythical-event access) still needs a **full emulator regression pass**. Build success and unit tests are **not** equivalent to feature-complete gameplay verification.
- Bugs and compatibility issues may remain, especially around scripted battles, overlay memory, special encounters, and save migration.

See [`docs/RUN_ENGINE.md`](docs/RUN_ENGINE.md) on `nuzlocke-base` for the detailed engineering log and acceptance checklist.

## Troubleshooting

**`make` fails a checksum/test after producing a ROM.** Use `make rom` for modified builds. Retail-matching checksum tests do not apply to ROM hacks.

**The build seems stuck at `main.nef` or 99%.** Linking is expensive and may take much longer than individual compilations. Check processes before interrupting:

```bash
ps -eo pid,stat,etime,%cpu,cmd | grep -E '[n]inja|[m]wldarm|[m]wccarm'
```

**A build fails because `mwccarm` cannot run.** On Ubuntu/WSL, confirm the documented dependencies, including `libc6:i386`, are installed.

**WSL reports I/O errors or `emergency_ro`.** Stop builds; check the free space on the **Windows volume hosting WSL's `ext4.vhdx`**, not just `df -h /` inside Linux. Back up the VHDX before attempting filesystem repair, and follow [Microsoft's WSL disk repair guidance](https://learn.microsoft.com/en-us/windows/wsl/disk-space).

**The ROM is old even after code changes.** Confirm the source branch and the timestamp of `build/pokeplatinum.us.nds`; check `ninja -C build -n pokeplatinum.us.nds` for outstanding work.

**Older saves fail to load with new features.** Do not overwrite the only copy of a save. Back it up first and check the format/version notes in `docs/RUN_ENGINE.md`. New configured runs are not expected to load in older ROM revisions.

## Credits and acknowledgments

- **[pret/pokeplatinum](https://github.com/pret/pokeplatinum)** — Pokémon Platinum decompilation, build infrastructure, and research.
- **[Dusknight4/deltaplatinum](https://github.com/Dusknight4/deltaplatinum)** — HGSS-style following Pokémon implementation. The project is based on the [following-only commit `8f7ccec`](https://github.com/Dusknight4/deltaplatinum/tree/8f7ccec26e697e16df4fe4155915fbf28a038310), with selected subsequent stability fixes; not the full Delta gameplay mod.
- **The Pokémon modding/decompilation community** — tools, testing, documentation, and research that make projects like this possible.

## Legal notice

This is an **unofficial, fan-made, noncommercial source-code project**, not affiliated with or endorsed by Nintendo, Game Freak, or The Pokémon Company. Pokémon and related trademarks belong to their respective owners.

**No copyrighted commercial Pokémon Platinum ROM is supplied by this repository.** Do not upload or redistribute proprietary ROM images, SDK files, save files containing private information, or copyrighted game assets outside any permissions that apply. Follow the applicable laws and the terms of upstream components.

---

**Choose your rules. Know your encounters. Make every decision count.**

## Latest playtest polish

The current feature branch adds X-button explanations, one Permadeath setting,
clamped numerical controls, Ultra High1/512 shinies, friendly story-partner
randomization, safe randomized catching demonstrations, automatic fresh-run configuration after Game Over,
and a redesigned native Pokétch LCD. Postgame National Dex unlocks the original
Regi ruins without an imported event Pokémon; original puzzles remain.
See [implementation, save compatibility and regression checklist](docs/RUN_ENGINE.md).
The player supplied playtest feedback and a physical-console photo; the new
revision still requires its own emulator/hardware acceptance testing.

The latest source restoration compares all 4411 Pokémon, move, item and trainer
JSON definitions with the Following-only base. Native types, abilities, base
stats, learnsets, move parameters, prices, catch bonuses, trainer AI and breeding
rules are restored. The new original-stat dataset changes the Randomizer data
fingerprint: older development runs are incompatible; start a new run. Existing
save files are not migrated or erased merely by updating the ROM. Automatic
erasure applies only when the currently configured run ends.
