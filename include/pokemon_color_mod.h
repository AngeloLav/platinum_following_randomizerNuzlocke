#ifndef POKEPLATINUM_POKEMON_COLOR_MOD_H
#define POKEPLATINUM_POKEMON_COLOR_MOD_H

#include <nitro.h>

// COLOR MOD (port of PokeHeartGold's src/pokepic.c personality-based palette rotation - see that project's
// Changelog, Entries A2/C/G/N/AH etc., for the full history and derivation): rotates a Pokemon's palette by
// a hue shift, saturation scale and value scale all derived from its personality value, so different
// individual Pokemon of the same species show a unique but fully repeatable color variation, matching every
// other screen that shows the same Pokemon. Index 0 (palette transparency) is always left untouched.

/**
 * @brief Rotate a palette of `count` 15-bit BGR555 colors in place, using the personality-derived hue
 * shift/saturation scale/value scale. Index 0 is skipped (palette transparency).
 *
 * @param palette     Palette buffer to rotate in place (BGR555 colors).
 * @param count       Number of colors in the buffer (palette[0] is always skipped regardless).
 * @param personality The Pokemon's personality value.
 */
void PersonalityRotatePalette(u16 *palette, int count, u32 personality);

/**
 * @brief Same as PersonalityRotatePalette, but each color index can additionally be nudged by its own extra
 * hue offset on top of the personality-derived hue. Used for small per-species color corrections where the
 * generic rotation alone looks slightly off on a particular Pokemon's palette.
 *
 * @param palette           Palette buffer to rotate in place (BGR555 colors).
 * @param count             Number of colors in the buffer (palette[0] is always skipped regardless).
 * @param personality       The Pokemon's personality value.
 * @param hueOffsetsHalfDeg Per-index signed hue nudge, in units of 2 degrees; must have at least `count`
 *                          entries (index 0 is never read, same as `palette`).
 */
void PersonalityRotatePaletteWithHueOffsets(u16 *palette, int count, u32 personality, const s8 *hueOffsetsHalfDeg);

#endif // POKEPLATINUM_POKEMON_COLOR_MOD_H
