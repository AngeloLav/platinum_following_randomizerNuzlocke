# Colour mod (personality-based HSV rotation)

The second Delta Platinum commit gives every individual Pokémon its own colouring, the same way Delta Heart Gold does.
The Pokémon's personality value (PID) drives a hue, saturation and value change that is applied to its palette.

| PID bits | Effect |
|---|---|
| 0-9 | hue rotation, 0..360° |
| 10-14 | saturation, 70%..130% |
| 15-19 | value (brightness), 85%..115% |

Palette index 0 (transparency) is never changed. Shiny Pokémon use their shiny palette and are then rotated as well.

## Where it is applied

- **All 2D Pokémon sprites** built through `PokemonSpriteManager` go through one hook in `BufferPokemonSpritePlttData`
  (`src/pokemon_sprite.c`): battle, summary screen, PC boxes, starter selection and every other screen that uses it. `BuildPokemonSpriteTemplate[DP]` (`src/pokemon.c`)
  now pass the real personality.
- **Battle animations** that load a Pokémon's palette directly (`src/battle_anim/battle_anim_system.c`, 3 places).
- **Trade room preview** of the hovered Pokémon on the bottom screen (`src/applications/trade_room/application.c`).
- **Overworld followers** (`src/follower_mon.c`), with HeartGold's per-species fixes for shiny Totodile/Feraligatr.
- **Trainer Pokémon** get a random personality per encounter, so trainers' Pokémon are coloured individually too
  (`src/trainer_data.c`).

The colour functions themselves are in `src/pokemon_color_mod.c` / `include/pokemon_color_mod.h` (ported from Delta
Heart Gold's `pokepic.c`).
