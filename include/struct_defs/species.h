#ifndef POKEPLATINUM_SPECIES_H
#define POKEPLATINUM_SPECIES_H

#define MAX_ABILITIES  2
#define MAX_EGG_GROUPS 2
#define MAX_TYPES      2

#define MAX_EVOLUTIONS 7

// COLOR MOD (port of PokeHeartGold ENTRY AE's learnset buffer fix): raised from 20 - the FireRed-merged
// level-up learnsets (tools/port_heartgold_data.py) can be longer than vanilla Platinum's own longest list;
// Scizor's merged learnset is 23 moves, matching HG's own finding exactly (HG also raised its equivalent
// buffer size, to 40, for the same reason - mirrored here for consistency and headroom against future edits).
// This single constant sizes BOTH the data-processing tool's validation bound (tools/dataproc/src/
// speciesproc.c) AND every runtime learnset buffer (src/pokemon.c, all 3 call it via sizeof(SpeciesLearnset)),
// so raising it here is the only change needed on either side - unlike HG, which needed two separate local
// buffer-size constants in src/pokemon.c and src/move_relearner.c because it lacked this shared header.
// Raised 40 -> 48 for the 3-way HGSS+FireRed+Crystal learnset merge (HG ENTRY AS / tools/
// merge_learnsets_3way.py), whose longest list (Mr. Mime, 42 moves) exceeds the old 40-entry bound -
// matching HG's own LEARNSET_BUFFER_ENTRIES/RELEARNER_LEARNSET_ENTRIES, both also raised to 48.
#define MAX_LEARNSET_ENTRIES        48
#define LEARNSET_NO_MOVE_TO_LEARN   0
#define LEARNSET_MOVE_ALREADY_KNOWN 0xFFFE
#define LEARNSET_ALL_SLOTS_FILLED   0xFFFF
#define LEARNSET_SENTINEL_ENTRY     0xFFFF

typedef struct SpeciesBaseStats {
    u8 hp;
    u8 attack;
    u8 defense;
    u8 speed;
    u8 spAttack;
    u8 spDefense;
} SpeciesBaseStats;

typedef struct SpeciesEVYields {
    u16 hp : 2;
    u16 attack : 2;
    u16 defense : 2;
    u16 speed : 2;
    u16 spAttack : 2;
    u16 spDefense : 2;
} SpeciesEVYields;

typedef struct SpeciesWildHeldItems {
    u16 common;
    u16 rare;
} SpeciesWildHeldItems;

typedef struct SpeciesData {
    SpeciesBaseStats baseStats;
    u8 types[MAX_TYPES];
    u8 catchRate;
    u8 baseExpReward;
    SpeciesEVYields evYields;
    SpeciesWildHeldItems wildHeldItems;
    u8 genderRatio;
    u8 hatchCycles;
    u8 baseFriendship;
    u8 expRate;
    u8 eggGroups[MAX_EGG_GROUPS];
    u8 abilities[MAX_ABILITIES];
    u8 safariFleeRate;
    u8 bodyColor : 7;
    u8 flipSprite : 1;
    // u16 padding;

    u32 tmLearnsetMasks[4]; // Bitflags for whether this pokemon can learn a TM
} SpeciesData;

typedef struct SpeciesEvolution {
    u16 method;
    u16 param;
    u16 targetSpecies;
} SpeciesEvolution;

typedef struct SpeciesLearnsetEntry {
    u16 move : 9;
    u16 level : 7;
} SpeciesLearnsetEntry;

// This struct is not explicitly used; it is provided to document and enforce the size of
// the learnset entries.
typedef struct SpeciesLearnset {
    ALIGN_4 SpeciesLearnsetEntry entries[MAX_LEARNSET_ENTRIES + 1];
} SpeciesLearnset;

typedef struct SpeciesPalPark {
    u8 landArea;
    u8 waterArea;
    u8 catchingPoints;
    u8 rarity;

    union {
        u8 asU8[2];
        u16 asU16;
    } unused;
} SpeciesPalPark;

#endif // POKEPLATINUM_SPECIES_H
