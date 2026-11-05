#ifndef POKEPLATINUM_RUN_RANDOMIZER_H
#define POKEPLATINUM_RUN_RANDOMIZER_H

#include "run_config.h"

enum RunRandomizerDomain {
    RUN_DOMAIN_WILD = 0,
    RUN_DOMAIN_TRAINER,
    RUN_DOMAIN_STATIC,
    RUN_DOMAIN_GIFT,
    RUN_DOMAIN_GIFT_EGG,
    RUN_DOMAIN_COUNT
};

enum RunSpeciesClass {
    RUN_SPECIES_ORDINARY = 0,
    RUN_SPECIES_LEGENDARY,
    RUN_SPECIES_MYTHICAL
};

// Keys must describe an encounter slot/event, never a live RNG draw or party slot.
u16 RunRandomizer_SelectSpecies(const RunConfig *config, enum RunRandomizerDomain domain, u32 eventKey, u32 slot, u16 originalSpecies);
u32 RunRandomizer_CandidateWeight(const RunConfig *config, u16 originalSpecies, u16 candidateSpecies);
BOOL RunSpecies_IsValid(u16 species);
u16 RunSpecies_BaseStatTotal(u16 species);
u16 RunSpecies_EvolutionFamily(u16 species);
enum RunSpeciesClass RunSpecies_Class(u16 species);
u16 RunSpecies_Count(void);

#endif // POKEPLATINUM_RUN_RANDOMIZER_H
