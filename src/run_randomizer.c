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

// round(65535 * exp(-2 * (i/64)^2)), sigma = half the permitted deviation.
// Linear interpolation uses a Q8 fraction; no floating point or candidate heap.
static const u16 sGaussian[65] = {
    65535,65503,65407,65248,65025,64740,64393,63986,63519,62994,62412,61775,61085,
    60344,59554,58717,57834,56910,55946,54944,53908,52839,51741,50617,49468,48299,
    47111,45907,44691,43464,42230,40991,39749,38507,37268,36033,34805,33587,32379,
    31184,30004,28841,27695,26569,25464,24381,23322,22286,21276,20292,19334,18404,
    17501,16627,15780,14962,14173,13412,12680,11976,11300,10651,10031,9437,8869
};

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

    if (config->rulesVersion == 2) {
        // Published runs keep their exact old mappings and probability law.
        return 100 + 9u * config->bstWeight * (radius - distance) / radius;
    }
    if (config->bstWeight == 0) return 100;
    u32 position = distance * 16384u / radius;
    u32 index = position >> 8;
    if (index == 64) return sGaussian[64];
    return sGaussian[index] - ((sGaussian[index] - sGaussian[index + 1]) * (position & 255) + 128) / 256;
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
    u32 eligibleCount = 0;
    u32 nearestDistance = 0xFFFFFFFFu;
    u32 nearestCount = 0;

    for (u16 species = 1; species <= RUN_SPECIES_COUNT; species++) {
        u32 weight = RunRandomizer_CandidateWeight(config, originalSpecies, species);
        total += weight;
        if (weight) eligibleCount++;

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
        // Two-stage exact mixture: (1-s)/N + s*G_i/sum(G).
        // Integer G is the positive fixed-point approximation above.
        BOOL uniform = config->rulesVersion >= 3
            && (config->bstWeight == 0 || Below(&state, 100) >= config->bstWeight);
        u32 pick = Below(&state, uniform ? eligibleCount : total);
        for (u16 species = 1; species <= RUN_SPECIES_COUNT; species++) {
            u32 weight = RunRandomizer_CandidateWeight(config, originalSpecies, species);
            if (uniform && weight) weight = 1;
            if (pick < weight) {
                return species;
            }
            pick -= weight;
        }
    }

    return originalSpecies;
}
