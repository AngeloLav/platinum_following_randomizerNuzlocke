#include "pokemon_color_mod.h"

// Compatibility entry points for existing sprite loaders. Preserve the raw
// normal/shiny resource palette on every screen, independently of personality.
// Keeping these symbols avoids changing the overlay and sprite-loading paths.
void PersonalityRotatePalette(u16 *palette, int count, u32 personality)
{
    (void)palette;
    (void)count;
    (void)personality;
}

void PersonalityRotatePaletteWithHueOffsets(u16 *palette, int count, u32 personality, const s8 *hueOffsetsHalfDeg)
{
    (void)palette;
    (void)count;
    (void)personality;
    (void)hueOffsetsHalfDeg;
}
