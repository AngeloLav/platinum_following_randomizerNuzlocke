#include "pokemon_color_mod.h"

// COLOR MOD: ported verbatim from PokeHeartGold's src/pokepic.c (RotateColorHue_HGSS, PersonalityRotatePalette,
// PersonalityRotatePaletteWithHueOffsets) - see that project's Changelog for the full history/derivation of
// this exact hue/saturation/value rotation formula. This is pure integer math with no HeartGold-specific
// types involved, so it ports unchanged. Colors are BGR555 (5 bits each of blue/green/red, bit 15 unused).
static u16 RotateColorHue_HGSS(u16 gbaColor, u16 hueShift, s32 satScale, s32 valScale)
{
    s32 r = ((gbaColor >> 0) & 0x1F) * 255 / 31;
    s32 g = ((gbaColor >> 5) & 0x1F) * 255 / 31;
    s32 b = ((gbaColor >> 10) & 0x1F) * 255 / 31;
    s32 max = r > g ? (r > b ? r : b) : (g > b ? g : b);
    s32 min = r < g ? (r < b ? r : b) : (g < b ? g : b);
    s32 delta = max - min;
    s32 hue, sat, val = max;
    s32 C, X, m, rr, gg, bb;

    if (delta == 0) {
        val = val * valScale / 100;
        if (val > 255) {
            val = 255;
        }
        r = g = b = val;
    } else {
        sat = delta * 255 / max;
        if (max == r) {
            hue = 60 * (((g - b) * 100 / delta) % 600) / 100;
        } else if (max == g) {
            hue = 60 * (((b - r) * 100 / delta) + 200) / 100;
        } else {
            hue = 60 * (((r - g) * 100 / delta) + 400) / 100;
        }
        if (hue < 0) {
            hue += 360;
        }
        hue = (hue + hueShift) % 360;

        sat = sat * satScale / 100;
        if (sat > 255) {
            sat = 255;
        }
        val = val * valScale / 100;
        if (val > 255) {
            val = 255;
        }

        C = val * sat / 255;
        X = C * (60 - (hue % 120 > 60 ? hue % 120 - 60 : 60 - hue % 120)) / 60;
        m = val - C;

        if (hue < 60) {
            rr = C;
            gg = X;
            bb = 0;
        } else if (hue < 120) {
            rr = X;
            gg = C;
            bb = 0;
        } else if (hue < 180) {
            rr = 0;
            gg = C;
            bb = X;
        } else if (hue < 240) {
            rr = 0;
            gg = X;
            bb = C;
        } else if (hue < 300) {
            rr = X;
            gg = 0;
            bb = C;
        } else {
            rr = C;
            gg = 0;
            bb = X;
        }

        r = rr + m;
        g = gg + m;
        b = bb + m;
    }

    return (u16)((r * 31 / 255) | ((g * 31 / 255) << 5) | ((b * 31 / 255) << 10));
}

void PersonalityRotatePalette(u16 *palette, int count, u32 personality)
{
    u16 hueShift = (u16)(((personality & 0x3FF) * 360) / 1024);
    u16 satSeed = (personality >> 10) & 0x1F;
    u16 valSeed = (personality >> 15) & 0x1F;
    s32 satScale = 70 + ((u32)satSeed * 60 / 31);
    s32 valScale = 85 + ((u32)valSeed * 30 / 31);
    int j;

    for (j = 1; j < count; j++) {
        palette[j] = RotateColorHue_HGSS(palette[j], hueShift, satScale, valScale);
    }
}

void PersonalityRotatePaletteWithHueOffsets(u16 *palette, int count, u32 personality, const s8 *hueOffsetsHalfDeg)
{
    u16 baseHue = (u16)(((personality & 0x3FF) * 360) / 1024);
    u16 satSeed = (personality >> 10) & 0x1F;
    u16 valSeed = (personality >> 15) & 0x1F;
    s32 satScale = 70 + ((u32)satSeed * 60 / 31);
    s32 valScale = 85 + ((u32)valSeed * 30 / 31);
    int j;

    for (j = 1; j < count; j++) {
        int hue = ((int)baseHue + 2 * hueOffsetsHalfDeg[j]) % 360;

        if (hue < 0) {
            hue += 360;
        }
        palette[j] = RotateColorHue_HGSS(palette[j], (u16)hue, satScale, valScale);
    }
}
