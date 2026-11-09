#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "run_extras.h"
#include "run_randomizer.h"
#include "generated/items.h"

static u16 ShinyXor(u32 p, u32 ot) { return (p >> 16) ^ p ^ (ot >> 16) ^ ot; }

static void TestShiny(void)
{
    u32 ot = 0xAABBCCDDu;
    for (u8 rate = 0; rate < 5; rate++) {
        for (u8 constraint = 0; constraint <= 2; constraint += 2) {
            u32 shinies = 0;
            for (u32 x = 0; x < 65536; x++) {
                u32 p = 0x34ABu | ((u32)(0x34ABu ^ (ot >> 16) ^ ot ^ x) << 16);
                u32 result = RunExtras_ShinyPID(p, ot, rate, constraint);
                assert(result % 25 == p % 25);
                assert((result & 255) == (p & 255));
                if (constraint == 2) assert(((result >> 16) % 10 < 5) == ((p >> 16) % 10 < 5));
                if (x < 8 || x >= (8u << rate)) assert(result == p);
                assert((ShinyXor(result, ot) < 8) == (x < (8u << rate)));
                shinies += ShinyXor(result, ot) < 8;
            }
            assert(shinies == (8u << rate));
        }
    }
    for (u32 tid = 0; tid < 16; tid++) {
        u32 id = tid * 0x11111111u;
        for (u32 lowByte = 0; lowByte < 256; lowByte++) for (u16 x = 8; x < 128; x++) {
            u16 low = lowByte | ((lowByte ^ 0xA5) << 8);
            u32 p = low | ((u32)(low ^ id ^ (id >> 16) ^ x) << 16);
            u32 result = RunExtras_ShinyPID(p, id, 4, 2);
            assert(ShinyXor(result, id) < 8 && (result & 255) == lowByte && result % 25 == p % 25);
            assert(((result >> 16) % 10 < 5) == ((p >> 16) % 10 < 5));
        }
    }
    puts("PASS: all five shiny rates over every XOR value; forced shinies, gender byte, nature, ability bit and Wurmple branch");
}

static void TestItems(void)
{
    RunConfig c;
    RunConfig_Init(&c, RUN_PRESET_HARDCORE_RANDOMIZER, 0x12345678);
    assert(c.shinyRate == 0 && !(c.flags & RUN_RANDOMIZE_FIELD_ITEMS));
    for (u16 item = 0; item <= 536; item++) assert(RunExtras_FieldItem(&c, 7000, item) == item);
    c.flags |= RUN_RANDOMIZE_FIELD_ITEMS;
    const u16 protected[] = { ITEM_NONE, ITEM_SAFARI_BALL, ITEM_ODD_KEYSTONE, ITEM_GRISEOUS_ORB,
        ITEM_ADAMANT_ORB, ITEM_LUSTROUS_ORB, ITEM_ARMOR_FOSSIL, ITEM_TM01, ITEM_HM01, ITEM_HM08,
        ITEM_BICYCLE, ITEM_PARCEL, ITEM_OAKS_LETTER, ITEM_MEMBER_CARD };
    for (u32 i = 0; i < sizeof(protected) / sizeof(protected[0]); i++) {
        assert(!RunExtras_SafeFieldItem(protected[i]));
        assert(RunExtras_FieldItem(&c, 7000, protected[i]) == protected[i]);
    }
    u32 changed = 0, differentLocation = 0;
    for (u32 key = 0; key < 1000; key++) {
        u16 a = RunExtras_FieldItem(&c, key, ITEM_POKE_BALL);
        assert(RunExtras_SafeFieldItem(a));
        assert(a == RunExtras_FieldItem(&c, key, ITEM_POKE_BALL));
        changed += a != ITEM_POKE_BALL;
        differentLocation += a != RunExtras_FieldItem(&c, key + 1, ITEM_POKE_BALL);
    }
    assert(changed > 900 && differentLocation > 900);
    assert(RunExtras_UsableBall(ITEM_POKE_BALL) && RunExtras_UsableBall(ITEM_MASTER_BALL));
    assert(!RunExtras_UsableBall(ITEM_SAFARI_BALL) && !RunExtras_UsableBall(65535));
    assert(!RunExtras_UsableBall(ITEM_POTION));
    puts("PASS: field items OFF/ON, safe pool, protected progression/TM/HM items, location determinism and usable Balls");
}

static void TestCompatibility(void)
{
    RunConfig c, d;
    u8 bytes[36], again[36];
    RunConfig_Init(&c, RUN_PRESET_HARDCORE_RANDOMIZER, 123);
    c.rulesVersion = 2;
    assert(RunConfig_Encode(&c, bytes, 36) && bytes[4] == 2 && bytes[8] == 2);
    assert(RunConfig_Decode(&d, bytes, 36) == RUN_CONFIG_READ_OK);
    assert(d.rulesVersion == 2 && !d.shinyRate && !(d.flags & RUN_RANDOMIZE_FIELD_ITEMS));
    assert(RunConfig_Encode(&d, again, 36) && memcmp(bytes, again, 36) == 0);
    for (u16 a = 1; a <= 493; a++) for (u16 b = 1; b <= 493; b++) {
        u32 base = RunSpecies_BaseStatTotal(a), other = RunSpecies_BaseStatTotal(b);
        u32 distance = base > other ? base - other : other - base;
        BOOL pool = (RunSpecies_Class(a) == RUN_SPECIES_ORDINARY) == (RunSpecies_Class(b) == RUN_SPECIES_ORDINARY);
        u32 expected = !pool || distance * 100 > base * c.bstTolerance ? 0
            : 100 + 9u * c.bstWeight * (base * c.bstTolerance - distance * 100) / (base * c.bstTolerance);
        assert(RunRandomizer_CandidateWeight(&d, a, b) == expected);
    }
    RunConfig_Init(&c, RUN_PRESET_HARDCORE_RANDOMIZER, 123);
    for (u8 rate = 0; rate < 5; rate++) {
        c.shinyRate = rate;
        c.flags |= RUN_RANDOMIZE_FIELD_ITEMS;
        assert(RunConfig_Encode(&c, bytes, 36));
        assert(RunConfig_Decode(&d, bytes, 36) == RUN_CONFIG_READ_OK);
        assert(d.shinyRate == rate && d.rulesVersion == 3 && (d.flags & RUN_RANDOMIZE_FIELD_ITEMS));
    }
    puts("PASS: schema-2 byte-exact roundtrip/legacy weighting, schema-3 options, no uninitialized new settings");
}

static void TestIntro(void)
{
    RunConfig c;
    RunConfig_Init(&c, RUN_PRESET_RANDOMIZER, 0);
    for (u32 seed = 0; seed < 1000; seed++) {
        c.seed = seed;
        assert(RunSpecies_IsValid(RunExtras_IntroSpecies(&c)));
        assert(RunExtras_IntroSpecies(&c) == RunExtras_IntroSpecies(&c));
        assert(RunExtras_Starter(&c, 0) == 387 && RunExtras_Starter(&c, 1) == 390 && RunExtras_Starter(&c, 2) == 393);
        c.flags |= RUN_RANDOMIZE_GIFTS;
        for (u8 slot = 0; slot < 3; slot++) {
            u16 species = RunExtras_Starter(&c, slot);
            assert(RunSpecies_IsValid(species) && species == RunExtras_Starter(&c, slot));
        }
        c.flags &= ~RUN_RANDOMIZE_GIFTS;
    }
    puts("PASS: valid seeded Rowan species and stable starter-slot mappings, vanilla when disabled");
}

static void TestGaussian(void)
{
    RunConfig c;
    RunConfig_Init(&c, RUN_PRESET_RANDOMIZER, 0x10203040u);
    c.bstWeight = 100;
    u32 n = 0, nearCount = 0, total = 0, nearWeight = 0;
    u32 base = RunSpecies_BaseStatTotal(133), radius = base * c.bstTolerance;
    for (u16 species = 1; species <= 493; species++) {
        u32 w = RunRandomizer_CandidateWeight(&c, 133, species);
        if (!w) continue;
        u32 bst = RunSpecies_BaseStatTotal(species);
        u32 distance = (bst > base ? bst - base : base - bst) * 100;
        double ratio = (double)distance / radius;
        assert(fabs(w - 65535.0 * exp(-2.0 * ratio * ratio)) <= 20.0);
        n++; total += w;
        if (distance * 4 <= radius) { nearCount++; nearWeight += w; }
    }
    double previous = 0;
    for (u8 strength = 0; strength <= 100; strength += 25) {
        c.bstWeight = strength;
        u32 observed = 0;
        for (u32 key = 0; key < 20000; key++) {
            u16 species = RunRandomizer_SelectSpecies(&c, RUN_DOMAIN_WILD, key, 0, 133);
            u32 bst = RunSpecies_BaseStatTotal(species);
            u32 distance = (bst > base ? bst - base : base - bst) * 100;
            observed += distance * 4 <= radius;
        }
        double expected = (100.0 - strength) / 100.0 * nearCount / n
            + strength / 100.0 * nearWeight / total;
        double actual = observed / 20000.0;
        assert(fabs(actual - expected) < 0.02);
        if (strength) assert(actual > previous);
        previous = actual;
    }
    puts("PASS: fixed-point Gaussian versus exp reference; 100000 seeded draws match five uniform/Gaussian mixtures");
}

int main(void) { TestShiny(); TestItems(); TestCompatibility(); TestIntro(); TestGaussian(); return 0; }
