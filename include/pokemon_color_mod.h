#ifndef POKEPLATINUM_POKEMON_COLOR_MOD_H
#define POKEPLATINUM_POKEMON_COLOR_MOD_H

#include <nitro/types.h>

// Legacy sprite-loader compatibility hooks. Both functions deliberately leave
// the entire palette unchanged, including transparency. Only the native shiny
// predicate selects the normal or shiny resource; PID-based coloring is disabled.
void PersonalityRotatePalette(u16 *palette, int count, u32 personality);
void PersonalityRotatePaletteWithHueOffsets(u16 *palette, int count, u32 personality, const s8 *hueOffsetsHalfDeg);

#endif // POKEPLATINUM_POKEMON_COLOR_MOD_H
