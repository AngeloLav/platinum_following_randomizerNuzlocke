#include "run_randomizer.h"

#define RUN_INCLUDE_SPECIES_TABLES
#include "run_species_data.h"

static u32 Mix(u32 value)
{
    value ^= value >> 16;
    value *= 0x7FEB352Du;
    value ^= value >> 15;
    value *= 0x846CA68Bu;
    return value ^ (value >> 16);
}

static u32 Next(u32 *state)
{
    *state += 0x9E3779B9u;
    return Mix(*state);
}

static u32 Below(u32 *state, u32 bound)
{
    // Rejection removes modulo bias; no dependence on the game's RNG.
    u32 threshold = (0u - bound) % bound;
    u32 value;

    do {
        value = Next(state);
    } while (value < threshold);

    return value % bound;
}

u16 RunSpecies_Count(void)
{
    return RUN_SPECIES_COUNT;
}

BOOL RunSpecies_IsValid(u16 species)
{
    return species > 0 && species <= RUN_SPECIES_COUNT;
}

u16 RunSpecies_BaseStatTotal(u16 species)
{
    return RunSpecies_IsValid(species) ? sRunSpeciesBST[species] : 0;
}

u16 RunSpecies_EvolutionFamily(u16 species)
{
    return RunSpecies_IsValid(species) ? sRunSpeciesFamily[species] : 0;
}

enum RunSpeciesClass RunSpecies_Class(u16 species)
{
    return RunSpecies_IsValid(species) ? sRunSpeciesClass[species] : RUN_SPECIES_ORDINARY;
}

static BOOL PoolAllows(const RunConfig *config, u16 originalSpecies, u16 candidateSpecies)
{
    if (!RunSpecies_IsValid(originalSpecies) || !RunSpecies_IsValid(candidateSpecies)) {
        return FALSE;
    }

    if (config->flags & RUN_LEGENDARY_POOLS) {
        // Mythicals share the legendary replacement pool, retaining a distinct registry class.
        return (RunSpecies_Class(originalSpecies) != RUN_SPECIES_ORDINARY)
            == (RunSpecies_Class(candidateSpecies) != RUN_SPECIES_ORDINARY);
    }

    return TRUE;
}

static u32 Distance(u16 first, u16 second)
{
    u32 a = sRunSpeciesBST[first];
    u32 b = sRunSpeciesBST[second];
    return a > b ? a - b : b - a;
}

u32 RunRandomizer_CandidateWeight(const RunConfig *config, u16 originalSpecies, u16 candidateSpecies)
{
    if (!RunConfig_IsValid(config) || !PoolAllows(config, originalSpecies, candidateSpecies)) {
        return 0;
    }

    u32 radius = sRunSpeciesBST[originalSpecies] * config->bstTolerance;
    u32 distance = Distance(originalSpecies, candidateSpecies) * 100;

    if (distance > radius) {
        return 0;
    }

    // Integer weights 100..1000. Even a candidate on the tolerance boundary stays possible.
    return 100 + 9u * config->bstWeight * (radius - distance) / radius;
}

u16 RunRandomizer_SelectSpecies(const RunConfig *config, enum RunRandomizerDomain domain, u32 eventKey, u32 slot, u16 originalSpecies)
{
    static const u32 domainFlags[RUN_DOMAIN_COUNT] = {
        RUN_RANDOMIZE_WILD, RUN_RANDOMIZE_TRAINERS, RUN_RANDOMIZE_STATICS,
        RUN_RANDOMIZE_GIFTS, RUN_RANDOMIZE_EGGS
    };

    if (!RunConfig_IsValid(config) || !RunSpecies_IsValid(originalSpecies)
        || (u32)domain >= RUN_DOMAIN_COUNT || !(config->flags & domainFlags[domain])) {
        return originalSpecies;
    }

    u32 state = Mix(config->seed ^ 0x52554E31u);
    state = Mix(state ^ config->speciesDataVersion);
    state = Mix(state ^ (u32)domain);
    state = Mix(state ^ eventKey);
    state = Mix(state ^ slot);
    state = Mix(state ^ originalSpecies);
    state = Mix(state ^ (config->bstTolerance | ((u32)config->bstWeight << 8)
        | ((config->flags & RUN_LEGENDARY_POOLS) << 16)));

    u32 total = 0;
    u32 nearestDistance = 0xFFFFFFFFu;
    u32 nearestCount = 0;

    for (u16 species = 1; species <= RUN_SPECIES_COUNT; species++) {
        total += RunRandomizer_CandidateWeight(config, originalSpecies, species);

        if (PoolAllows(config, originalSpecies, species)) {
            u32 distance = Distance(originalSpecies, species);
            if (distance < nearestDistance) {
                nearestDistance = distance;
                nearestCount = 1;
            } else if (distance == nearestDistance) {
                nearestCount++;
            }
        }
    }

    // Keeping the original species eligible guarantees a nonempty pool with current data.
    // Future exclusions must still fall back ONLY within the original legendary class.
    if (total == 0) {
        if (nearestCount == 0) {
            return originalSpecies;
        }

        u32 pick = Below(&state, nearestCount);
        for (u16 species = 1; species <= RUN_SPECIES_COUNT; species++) {
            if (PoolAllows(config, originalSpecies, species) && Distance(originalSpecies, species) == nearestDistance) {
                if (pick == 0) {
                    return species;
                }
                pick--;
            }
        }
    } else {
        u32 pick = Below(&state, total);
        for (u16 species = 1; species <= RUN_SPECIES_COUNT; species++) {
            u32 weight = RunRandomizer_CandidateWeight(config, originalSpecies, species);
            if (pick < weight) {
                return species;
            }
            pick -= weight;
        }
    }

    return originalSpecies;
}
