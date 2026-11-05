#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "run_config.h"
#include "run_randomizer.h"

static void TestPresets(void)
{
    RunConfig config;
    for (int preset = 0; preset < RUN_PRESET_COUNT; preset++) {
        assert(RunConfig_Init(&config, preset, 0));
        assert(RunConfig_IsValid(&config));
        assert(RunConfig_EffectivePreset(&config) == (enum RunPreset)preset);
        config.seedMode = RUN_SEED_MANUAL;
        config.seed = 0xFFFFFFFFu;
        assert(RunConfig_EffectivePreset(&config) == (enum RunPreset)preset);
        config.bstTolerance = 30;
        assert(RunConfig_EffectivePreset(&config) == RUN_PRESET_CUSTOM);
    }

    assert(!RunConfig_Init(&config, RUN_PRESET_COUNT, 1));
    assert(!RunConfig_Init(&config, -1, 1));
    assert(!RunConfig_Init(NULL, RUN_PRESET_CLASSIC, 1));
    assert(RunConfig_Init(&config, RUN_PRESET_CLASSIC, 1));
    assert(!RunConfig_CaptureLimitsEnabled(&config));
    assert(RunConfig_DayCareEggsEnabled(&config));
    assert(!RunConfig_EndOnPartyWipe(&config));
    assert(!(config.flags & (RUN_RANDOMIZE_WILD | RUN_RANDOMIZE_TRAINERS | RUN_PERMADEATH | RUN_LEVEL_CAPS | RUN_TRAINER_SCALING)));
    assert(RunConfig_Init(&config, RUN_PRESET_RANDOMIZER, 1));
    assert(!RunConfig_CaptureLimitsEnabled(&config));
    assert(config.flags & RUN_RANDOMIZE_WILD);
    assert(!(config.flags & RUN_PERMADEATH));
    assert(RunConfig_Init(&config, RUN_PRESET_NUZLOCKE, 1));
    assert(RunConfig_CaptureLimitsEnabled(&config));
    assert(RunConfig_DupesEnabled(&config));
    assert(RunConfig_ShinyClauseEnabled(&config));
    assert(!(config.flags & RUN_RANDOMIZE_WILD));
    assert(RunConfig_DayCareEggsEnabled(&config));
    assert(RunConfig_EndOnPartyWipe(&config));
    assert(RunConfig_Init(&config, RUN_PRESET_HARDCORE_RANDOMIZER, 1));
    assert(config.bstTolerance == 25 && config.bstWeight == 75 && config.captureAttempts == 1);
    assert(config.levelCapAllowance == 0 && config.trainerLevelsPerBadge == 1);
    assert(!RunConfig_DayCareEggsEnabled(&config));
    assert(config.flags & RUN_SCALE_BOSSES);
    assert(RunConfig_EndOnPartyWipe(&config));
    config.flags &= ~RUN_PERMADEATH;
    assert(!RunConfig_EndOnPartyWipe(&config));
    config.flags |= RUN_PERMADEATH;
    assert(!(config.flags & (RUN_RANDOMIZE_STATICS | RUN_RANDOMIZE_GIFTS | RUN_RANDOMIZE_EGGS)));
    config.captureAttempts = 0;
    assert(!RunConfig_CaptureLimitsEnabled(&config));
    assert(!RunConfig_DupesEnabled(&config));
    assert(!RunConfig_ShinyClauseEnabled(&config));
    assert(config.flags & RUN_RANDOMIZE_WILD);

    for (u8 allowance = 0; allowance <= 5; allowance++) {
        for (u8 bonus = 1; bonus <= 3; bonus++) {
            config.levelCapAllowance = allowance;
            config.trainerLevelsPerBadge = bonus;
            assert(RunConfig_IsValid(&config));
        }
    }
    assert(RunConfig_Init(&config, RUN_PRESET_HARDCORE_RANDOMIZER, 1));
    config.levelCapAllowance = 1;
    assert(RunConfig_EffectivePreset(&config) == RUN_PRESET_CUSTOM);
    config.levelCapAllowance = 0;
    config.trainerLevelsPerBadge = 2;
    assert(RunConfig_EffectivePreset(&config) == RUN_PRESET_CUSTOM);
    config.levelCapAllowance = 6;
    assert(!RunConfig_IsValid(&config));
    config.levelCapAllowance = 0;
    config.trainerLevelsPerBadge = 0;
    assert(!RunConfig_IsValid(&config));
    config.trainerLevelsPerBadge = 4;
    assert(!RunConfig_IsValid(&config));
}

static void TestCodec(void)
{
    RunConfig original, restored, before;
    u8 encoded[RUN_CONFIG_WIRE_SIZE], damaged[RUN_CONFIG_WIRE_SIZE];
    assert(RunConfig_Init(&original, RUN_PRESET_HARDCORE_RANDOMIZER, 0x12345678u));
    original.seedMode = RUN_SEED_MANUAL;
    original.levelCapAllowance = 5;
    original.trainerLevelsPerBadge = 3;
    original.levelCapMode = RUN_CAP_OBEDIENCE;
    assert(RunConfig_Encode(&original, encoded, sizeof(encoded)));
    assert(memcmp(encoded, "RUNC", 4) == 0);
    assert(encoded[16] == 0x78 && encoded[17] == 0x56 && encoded[18] == 0x34 && encoded[19] == 0x12);
    assert(RunConfig_Decode(&restored, encoded, sizeof(encoded)) == RUN_CONFIG_READ_OK);
    assert(restored.seed == original.seed && restored.flags == original.flags);
    assert(restored.levelCapMode == RUN_CAP_OBEDIENCE);
    assert(restored.speciesDataVersion == original.speciesDataVersion);
    assert(restored.seedMode == original.seedMode && restored.preset == original.preset);
    assert(restored.bstWeight == original.bstWeight && restored.bstTolerance == original.bstTolerance);
    assert(restored.captureAttempts == original.captureAttempts);
    assert(restored.levelCapAllowance == original.levelCapAllowance);
    assert(restored.trainerLevelsPerBadge == original.trainerLevelsPerBadge);

    before = restored;
    for (int byte = 0; byte < RUN_CONFIG_WIRE_SIZE; byte++) {
        for (int bit = 0; bit < 8; bit++) {
            memcpy(damaged, encoded, sizeof(damaged));
            damaged[byte] ^= 1u << bit;
            assert(RunConfig_Decode(&restored, damaged, sizeof(damaged)) == RUN_CONFIG_READ_INVALID);
            assert(memcmp(&restored, &before, sizeof(before)) == 0);
        }
    }
    for (u32 size = 0; size < RUN_CONFIG_WIRE_SIZE; size++) {
        assert(RunConfig_Decode(&restored, encoded, size) == RUN_CONFIG_READ_INVALID);
        assert(memcmp(&restored, &before, sizeof(before)) == 0);
    }
    assert(RunConfig_Decode(&restored, encoded, RUN_CONFIG_WIRE_SIZE + 1) == RUN_CONFIG_READ_INVALID);
    assert(RunConfig_Decode(NULL, encoded, sizeof(encoded)) == RUN_CONFIG_READ_INVALID);
    assert(!RunConfig_Encode(&original, NULL, sizeof(encoded)));
    assert(!RunConfig_Encode(&original, encoded, sizeof(encoded) - 1));
    original.captureAttempts = 4;
    assert(!RunConfig_Encode(&original, encoded, sizeof(encoded)));
}

static void TestSpecies(void)
{
    assert(RunSpecies_Count() == 493);
    assert(!RunSpecies_IsValid(0) && !RunSpecies_IsValid(494) && !RunSpecies_IsValid(65535));
    assert(RunSpecies_BaseStatTotal(0) == 0);
    assert(RunSpecies_EvolutionFamily(494) == 0);
    assert(RunSpecies_BaseStatTotal(1) == 318); // Bulbasaur, vanilla
    assert(RunSpecies_BaseStatTotal(493) == 720); // Arceus, vanilla
    assert(RunSpecies_Class(150) == RUN_SPECIES_LEGENDARY);
    assert(RunSpecies_Class(151) == RUN_SPECIES_MYTHICAL);
    assert(RunSpecies_Class(489) == RUN_SPECIES_MYTHICAL);
    const u16 eevee[] = { 133, 134, 135, 136, 196, 197, 470, 471 };
    for (u32 i = 0; i < sizeof(eevee) / sizeof(eevee[0]); i++) {
        assert(RunSpecies_EvolutionFamily(eevee[i]) == 133);
    }
    assert(RunSpecies_EvolutionFamily(292) == RunSpecies_EvolutionFamily(291)); // Shedinja/Ninjask
    assert(RunSpecies_EvolutionFamily(29) == RunSpecies_EvolutionFamily(32));
    assert(RunSpecies_EvolutionFamily(313) == RunSpecies_EvolutionFamily(314));
    assert(RunSpecies_EvolutionFamily(489) == RunSpecies_EvolutionFamily(490));
    for (u16 species = 1; species <= RunSpecies_Count(); species++) {
        u16 family = RunSpecies_EvolutionFamily(species);
        assert(family > 0 && family <= species);
        assert(RunSpecies_EvolutionFamily(family) == family);
        assert(RunSpecies_BaseStatTotal(species) > 0);
    }
}

static void TestWeights(void)
{
    RunConfig config;
    assert(RunConfig_Init(&config, RUN_PRESET_RANDOMIZER, 42));
    for (u16 original = 1; original <= RunSpecies_Count(); original++) {
        for (u16 candidate = 1; candidate <= RunSpecies_Count(); candidate++) {
            u32 a = RunSpecies_BaseStatTotal(original), b = RunSpecies_BaseStatTotal(candidate);
            u32 difference = a > b ? a - b : b - a;
            BOOL samePool = (RunSpecies_Class(original) == RUN_SPECIES_ORDINARY)
                == (RunSpecies_Class(candidate) == RUN_SPECIES_ORDINARY);
            BOOL eligible = samePool && difference * 100 <= a * config.bstTolerance;
            config.bstWeight = 0;
            u32 uniform = RunRandomizer_CandidateWeight(&config, original, candidate);
            assert(uniform == (eligible ? 100u : 0u));
            config.bstWeight = 100;
            u32 weighted = RunRandomizer_CandidateWeight(&config, original, candidate);
            assert((weighted > 0) == eligible);
            if (eligible) {
                assert(weighted >= 8869 && weighted <= 65535);
                if (difference == 0) {
                    assert(weighted == 65535);
                }
            }
        }
    }
    // Check both sides of a fractional tolerance boundary without rounded BST bounds.
    config.bstWeight = 100;
    assert(RunRandomizer_CandidateWeight(&config, 290, 129) > 0); // 266 -> 200, inside
    assert(RunRandomizer_CandidateWeight(&config, 290, 13) == 0); // 266 -> 195, outside
    assert(RunRandomizer_CandidateWeight(&config, 0, 1) == 0);
    assert(RunRandomizer_CandidateWeight(&config, 1, 494) == 0);
    config.bstTolerance = 75;
    assert(RunRandomizer_CandidateWeight(&config, 150, 445) == 0); // Mewtwo -> Garchomp forbidden by pool
    config.flags &= ~RUN_LEGENDARY_POOLS;
    assert(RunRandomizer_CandidateWeight(&config, 150, 445) > 0);
}

static void TestMappings(void)
{
    RunConfig config;
    assert(RunConfig_Init(&config, RUN_PRESET_RANDOMIZER, 0x10203040u));
    u32 differentSeed = 0, differentDomain = 0, differentSlot = 0;
    config.flags |= RUN_RANDOMIZE_STATICS | RUN_RANDOMIZE_GIFTS | RUN_RANDOMIZE_EGGS;
    for (int tolerance = 10; tolerance <= 75; tolerance += 5) {
        config.bstTolerance = tolerance;
        for (u16 species = 1; species <= RunSpecies_Count(); species++) {
            for (int domain = RUN_DOMAIN_WILD; domain <= RUN_DOMAIN_TRAINER; domain++) {
                u16 result = RunRandomizer_SelectSpecies(&config, domain, 203, 2, species);
                assert(RunSpecies_IsValid(result));
                assert(RunRandomizer_CandidateWeight(&config, species, result) > 0);
                assert(result == RunRandomizer_SelectSpecies(&config, domain, 203, 2, species));
            }
        }
    }
    config.bstTolerance = 25;
    for (u32 key = 0; key < 128; key++) {
        u16 original = 133;
        u16 result = RunRandomizer_SelectSpecies(&config, RUN_DOMAIN_WILD, key, 0, original);
        // Deliberately interleave unrelated mappings and the host RNG.
        (void)rand();
        (void)RunRandomizer_SelectSpecies(&config, RUN_DOMAIN_TRAINER, key + 7, 5, 150);
        assert(result == RunRandomizer_SelectSpecies(&config, RUN_DOMAIN_WILD, key, 0, original));
        differentDomain += result != RunRandomizer_SelectSpecies(&config, RUN_DOMAIN_TRAINER, key, 0, original);
        differentSlot += result != RunRandomizer_SelectSpecies(&config, RUN_DOMAIN_WILD, key, 1, original);
        config.seed++;
        differentSeed += result != RunRandomizer_SelectSpecies(&config, RUN_DOMAIN_WILD, key, 0, original);
        config.seed--;
    }
    assert(differentSeed > 100 && differentDomain > 100 && differentSlot > 100);
    for (int domain = 0; domain < RUN_DOMAIN_COUNT; domain++) {
        u32 bit = 1u << domain;
        config.flags &= ~bit;
        assert(RunRandomizer_SelectSpecies(&config, domain, 203, 0, 133) == 133);
        config.flags |= bit;
    }
    assert(RunRandomizer_SelectSpecies(&config, -1, 1, 1, 133) == 133);
    assert(RunRandomizer_SelectSpecies(&config, RUN_DOMAIN_COUNT, 1, 1, 133) == 133);
    assert(RunRandomizer_SelectSpecies(&config, RUN_DOMAIN_WILD, 1, 1, 494) == 494);
    assert(RunConfig_Init(&config, RUN_PRESET_CLASSIC, 0));
    for (u16 species = 1; species <= RunSpecies_Count(); species++) {
        assert(RunRandomizer_SelectSpecies(&config, RUN_DOMAIN_WILD, 1, 1, species) == species);
    }
}

int main(int argc, char **argv)
{
    TestPresets();
    TestCodec();
    TestSpecies();
    TestWeights();
    TestMappings();

    if (argc == 2) {
        FILE *file = fopen(argv[1], "wb");
        RunConfig config;
        u8 encoded[RUN_CONFIG_WIRE_SIZE];
        assert(file != NULL);
        assert(RunConfig_Init(&config, RUN_PRESET_HARDCORE_RANDOMIZER, 0x12345678u));
        assert(RunConfig_Encode(&config, encoded, sizeof(encoded)));
        assert(fwrite(encoded, 1, sizeof(encoded), file) == sizeof(encoded));
        assert(fclose(file) == 0);
    } else if (argc == 4 && strcmp(argv[1], "decode") == 0) {
        FILE *file = fopen(argv[2], "rb");
        u8 encoded[RUN_CONFIG_WIRE_SIZE];
        RunConfig config, before;
        assert(file != NULL);
        assert(fread(encoded, 1, sizeof(encoded), file) == sizeof(encoded));
        assert(fclose(file) == 0);
        assert(RunConfig_Init(&config, RUN_PRESET_CLASSIC, 999));
        before = config;
        assert(RunConfig_Decode(&config, encoded, sizeof(encoded)) == (enum RunConfigReadResult)atoi(argv[3]));
        if (atoi(argv[3]) != RUN_CONFIG_READ_OK) {
            assert(memcmp(&config, &before, sizeof(config)) == 0);
        }
    }

    puts("PASS: presets, codec, 493 species/families, exhaustive candidate weights, deterministic mappings");
    return 0;
}
