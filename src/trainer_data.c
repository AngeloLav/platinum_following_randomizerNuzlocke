#include "trainer_data.h"

#include "constants/battle.h"
#include "constants/pokemon.h"
#include "generated/trainer_message_types.h"
#include "generated/trainers.h"

#include "struct_defs/trainer.h"

#include "data/trainer_class_genders.h"

#include "charcode_util.h"
#include "field_battle_data_transfer.h"
#include "heap.h"
#include "math_util.h"
#include "message.h"
#include "narc.h"
#include "party.h"
#include "pokemon.h"
#include "run_game.h"
#include "savedata.h"
#include "savedata_misc.h"
#include "string_gf.h"

static void TrainerData_BuildParty(FieldBattleDTO *dto, int battler, enum HeapID heapID);

void Trainer_Encounter(FieldBattleDTO *dto, const SaveData *saveData, enum HeapID heapID)
{
    Trainer trdata;
    MessageLoader *msgLoader = MessageLoader_Init(MSG_LOADER_LOAD_ON_DEMAND, NARC_INDEX_MSGDATA__PL_MSG, TEXT_BANK_NPC_TRAINER_NAMES, heapID);
    const charcode_t *rivalName = MiscSaveBlock_RivalName(SaveData_MiscSaveBlockConst(saveData));

    for (int i = 0; i < MAX_BATTLERS; i++) {
        if (!dto->trainerIDs[i]) {
            continue;
        }

        Trainer_Load(dto->trainerIDs[i], &trdata);
        dto->trainer[i] = trdata;

        if (trdata.header.trainerType == TRAINER_CLASS_RIVAL) {
            CharCode_Copy(dto->trainer[i].name, rivalName);
        } else {
            String *trainerName = MessageLoader_GetNewString(msgLoader, dto->trainerIDs[i]);
            String_ToChars(trainerName, dto->trainer[i].name, TRAINER_NAME_LEN + 1);
            String_Free(trainerName);
        }

        TrainerData_BuildParty(dto, i, heapID);
    }

    dto->battleType |= trdata.header.battleType;
    MessageLoader_Free(msgLoader);
}

u32 Trainer_LoadParam(int trainerID, enum TrainerDataParam paramID)
{
    // TODO: can this be trainerheader?
    u32 result;
    Trainer trdata;

    Trainer_Load(trainerID, &trdata);

    switch (paramID) {
    case TRDATA_TYPE:
        result = trdata.header.monDataType;
        break;

    case TRDATA_CLASS:
        result = trdata.header.trainerType;
        break;

    case TRDATA_SPRITE:
        result = trdata.header.sprite;
        break;

    case TRDATA_PARTY_SIZE:
        result = trdata.header.partySize;
        break;

    case TRDATA_ITEM_1:
    case TRDATA_ITEM_2:
    case TRDATA_ITEM_3:
    case TRDATA_ITEM_4:
        result = trdata.header.items[paramID - TRDATA_ITEM_1];
        break;

    case TRDATA_AI_MASK:
        result = trdata.header.aiMask;
        break;

    case TRDATA_BATTLE_TYPE:
        result = trdata.header.battleType;
        break;
    }

    return result;
}

BOOL Trainer_HasMessageType(int trainerID, enum TrainerMessageType msgType, enum HeapID heapID)
{
    NARC *narc; // must declare up here to match
    u16 offset, data[2];

    BOOL result = FALSE;
    int size = NARC_GetMemberSizeByIndexPair(NARC_INDEX_POKETOOL__TRMSG__TRTBL, 0);
    NARC_ReadFromMemberByIndexPair(&offset, NARC_INDEX_POKETOOL__TRMSG__TRTBLOFS, 0, trainerID * 2, 2);
    narc = NARC_ctor(NARC_INDEX_POKETOOL__TRMSG__TRTBL, heapID);

    while (offset != size) {
        NARC_ReadFromMember(narc, 0, offset, 4, data);

        if (data[0] == trainerID && data[1] == msgType) {
            result = TRUE;
            break;
        }

        if (data[0] != trainerID) {
            break;
        }

        offset += 4;
    }

    NARC_dtor(narc);
    return result;
}

void Trainer_LoadMessage(int trainerID, enum TrainerMessageType msgType, String *string, enum HeapID heapID)
{
    NARC *narc; // must declare up here to match
    u16 offset, data[2];

    int size = NARC_GetMemberSizeByIndexPair(NARC_INDEX_POKETOOL__TRMSG__TRTBL, 0);
    NARC_ReadFromMemberByIndexPair(&offset, NARC_INDEX_POKETOOL__TRMSG__TRTBLOFS, 0, trainerID * 2, 2);
    narc = NARC_ctor(NARC_INDEX_POKETOOL__TRMSG__TRTBL, heapID);

    while (offset != size) {
        NARC_ReadFromMember(narc, 0, offset, 4, data);

        if (data[0] == trainerID && data[1] == msgType) {
            MessageBank_GetStringFromNARC(NARC_INDEX_MSGDATA__PL_MSG, TEXT_BANK_NPC_TRAINER_MESSAGES, offset / 4, heapID, string);
            break;
        }

        offset += 4;
    }

    NARC_dtor(narc);

    if (offset == size) {
        String_Clear(string);
    }
}

void Trainer_Load(int trainerID, Trainer *trdata)
{
    NARC_ReadWholeMemberByIndexPair(trdata, NARC_INDEX_POKETOOL__TRAINER__TRDATA, trainerID);
}

void Trainer_LoadParty(int trainerID, void *trparty)
{
    NARC_ReadWholeMemberByIndexPair(trparty, NARC_INDEX_POKETOOL__TRAINER__TRPOKE, trainerID);
}

u8 TrainerClass_Gender(int trclass)
{
    return sTrainerClassGender[trclass];
}

// ENTRY P113: Champion Cynthia's set builds (both champion battles). Her Pokemon are otherwise rolled like every trainer's
// (random personality and IVs); these pin the IVs that decide the custom types (HP IV -> type 1, Speed IV -> type 2) and
// the ability donor (species = (Def IV << 5) | Sp.Def IV, plus NATIONAL_DEX_COUNT for the second pass, which gives higher
// IVs - see ApplyAbilityDonor in pokemon.c), and the personality's low bit, which picks the donor's ability slot (a donor
// without a second ability always gives its first). Held items: res/trainers/data/champion_cynthia*.json.
#define CHAMPION_IV_ROLLED 0xFF

typedef struct ChampionMonBuild {
    u16 species;
    u8 hpIV; // custom type 1, or CHAMPION_IV_ROLLED
    u8 speedIV; // custom type 2, or CHAMPION_IV_ROLLED
    u16 donorSeed; // (Def IV << 5) | Sp.Def IV
    u8 abilitySlot; // personality low bit: 0 = the donor's ability 1, 1 = ability 2
} ChampionMonBuild;

static const ChampionMonBuild sChampionCynthiaBuilds[] = {
    { SPECIES_SPIRITOMB, TYPE_DARK, TYPE_GHOST, SPECIES_SHEDINJA + NATIONAL_DEX_COUNT, 0 }, // Wonder Guard (Def 24, SpDef 17)
    { SPECIES_ROSERADE, CHAMPION_IV_ROLLED, CHAMPION_IV_ROLLED, SPECIES_DRAPION + NATIONAL_DEX_COUNT, 1 }, // Sniper (29, 17)
    { SPECIES_TOGEKISS, TYPE_FLYING, TYPE_STEEL, SPECIES_ELEKID + NATIONAL_DEX_COUNT, 0 }, // Static (22, 28)
    { SPECIES_LUCARIO, CHAMPION_IV_ROLLED, CHAMPION_IV_ROLLED, SPECIES_PORYGON_Z, 0 }, // Adaptability (14, 26)
    { SPECIES_MILOTIC, TYPE_WATER, TYPE_ICE, SPECIES_ABOMASNOW + NATIONAL_DEX_COUNT, 0 }, // Snow Warning (29, 25)
    { SPECIES_GARCHOMP, TYPE_DRAGON, TYPE_STEEL, SPECIES_MEDICHAM, 0 }, // Pure Power (9, 20)
};

static const ChampionMonBuild *TrainerData_GetChampionBuild(int trainerID, u16 species)
{
    int i;

    if (trainerID != TRAINER_CHAMPION_CYNTHIA && trainerID != TRAINER_CHAMPION_CYNTHIA_REMATCH) {
        return NULL;
    }

    for (i = 0; i < NELEMS(sChampionCynthiaBuilds); i++) {
        if (sChampionCynthiaBuilds[i].species == species) {
            return &sChampionCynthiaBuilds[i];
        }
    }

    return NULL;
}

// Before Pokemon_InitWith: the ability slot bit. The low byte is the gender byte, so this can only move it by one (Cynthia's
// 120 -> 121), which changes no gender ratio's outcome.
static u32 TrainerData_ChampionPersonality(const ChampionMonBuild *build, u32 personality)
{
    if (build == NULL) {
        return personality;
    }

    return (personality & ~1) | build->abilitySlot;
}

// After Pokemon_InitWith: the IVs, then the stats again (full HP, as Pokemon_InitWith leaves it).
static void TrainerData_ApplyChampionIVs(const ChampionMonBuild *build, Pokemon *mon)
{
    u8 iv;

    if (build == NULL) {
        return;
    }

    if (build->hpIV != CHAMPION_IV_ROLLED) {
        iv = build->hpIV;
        Pokemon_SetValue(mon, MON_DATA_HP_IV, &iv);
    }

    if (build->speedIV != CHAMPION_IV_ROLLED) {
        iv = build->speedIV;
        Pokemon_SetValue(mon, MON_DATA_SPEED_IV, &iv);
    }

    iv = build->donorSeed >> 5;
    Pokemon_SetValue(mon, MON_DATA_DEF_IV, &iv);
    iv = build->donorSeed & 0x1F;
    Pokemon_SetValue(mon, MON_DATA_SPDEF_IV, &iv);

    Pokemon_CalcStats(mon);
}

/**
 * @brief Build the party for a trainer as loaded in the FieldBattleDTO struct.
 *
 * @param dto  The parent FieldBattleDTO struct containing trainer data.
 * @param battler       Which battler's party is to be loaded.
 * @param heapID        Heap on which to perform any allocations.
 */
static void TrainerData_BuildParty(FieldBattleDTO *dto, int battler, enum HeapID heapID)
{
    // must make declarations C89-style to match
    void *buf;
    int i, j;
    u32 genderMod, rnd, oldSeed;
    u8 ivs;
    Pokemon *mon;
    const ChampionMonBuild *build;

    oldSeed = LCRNG_GetSeed();
    // ENTRY P113: the random personalities and IVs below used to come straight from the overworld RNG state, which is put
    // back at the end, so building a party never advanced it: two trainers in one double battle got identical Pokemon in the
    // same slot (same colour, ability, types, nature), and back-to-back battles repeated whenever nothing else had advanced
    // the RNG in between. Each party now starts from a fresh value of the Mersenne Twister RNG (seeded from the clock at
    // boot, advanced by every call). The overworld state is still restored at the end, so nothing else changes.
    LCRNG_SetSeed(MTRNG_Next());

    // alloc enough space to support the maximum possible data size
    Party_InitWithCapacity(dto->parties[battler], MAX_PARTY_SIZE);
    buf = Heap_Alloc(heapID, sizeof(TrainerMonWithMovesAndItem) * MAX_PARTY_SIZE);
    mon = Pokemon_New(heapID);

    Trainer_LoadParty(dto->trainerIDs[battler], buf);

    // determine which magic gender-specific modifier to use for the RNG function
    genderMod = TrainerClass_Gender(dto->trainer[battler].header.trainerType) == GENDER_FEMALE
        ? 120
        : 136;

    switch (dto->trainer[battler].header.monDataType) {
    case TRDATATYPE_BASE: {
        TrainerMonBase *trmon = (TrainerMonBase *)buf;
        for (i = 0; i < dto->trainer[battler].header.partySize; i++) {
            u16 species = trmon[i].species & 0x3FF;
            u8 form = (trmon[i].species & 0xFC00) >> TRAINER_MON_FORM_SHIFT;

            // HG SYNC (port of PokeHeartGold ENTRY AY item 1): personality used to be seeded deterministically from
            // ivScale/level/species/trainerID and rolled a fixed number of times, so a given trainer's given party
            // slot had the exact same personality (nature, ability slot, colour) on every encounter. Now a random
            // roll from the global RNG (restored to oldSeed at the end of this function, as before, so the overworld
            // RNG is unaffected). The gender byte is still packed into the low 8 bits exactly as before.
            rnd = (LCRNG_Next() | ((u32)LCRNG_Next() << 16));
            rnd = (rnd << 8) + genderMod;
            // COLOR MOD (port of PokeHeartGold ENTRY AM): trainer Pokemon used to get one uniform IV
            // (all 6 stats identical) derived from ivScale here, so every Pokemon of a given species on
            // a given trainer had identical stats. Passing INIT_IVS_RANDOM (32) makes Pokemon_InitWith's
            // underlying IV setup take its normal random-IV path instead (the same one wild/player
            // Pokemon use), rolling each of the 6 IVs independently for real stat variety.
            ivs = INIT_IVS_RANDOM;

            build = TrainerData_GetChampionBuild(dto->trainerIDs[battler], species); // ENTRY P113
            rnd = TrainerData_ChampionPersonality(build, rnd);

            Pokemon_InitWith(mon, species, trmon[i].level, ivs, TRUE, rnd, OTID_NOT_SHINY, 0);
            Pokemon_SetBallSeal(trmon[i].cbSeal, mon, heapID);
            Pokemon_SetValue(mon, MON_DATA_FORM, &form);
            TrainerData_ApplyChampionIVs(build, mon); // ENTRY P113
            Party_AddPokemon(dto->parties[battler], mon);
        }

        break;
    }

    case TRDATATYPE_WITH_MOVES: {
        TrainerMonWithMoves *trmon = (TrainerMonWithMoves *)buf;
        for (i = 0; i < dto->trainer[battler].header.partySize; i++) {
            u16 species = trmon[i].species & 0x3FF;
            u8 form = (trmon[i].species & 0xFC00) >> TRAINER_MON_FORM_SHIFT;

            // HG SYNC (port of PokeHeartGold ENTRY AY item 1): personality used to be seeded deterministically from
            // ivScale/level/species/trainerID and rolled a fixed number of times, so a given trainer's given party
            // slot had the exact same personality (nature, ability slot, colour) on every encounter. Now a random
            // roll from the global RNG (restored to oldSeed at the end of this function, as before, so the overworld
            // RNG is unaffected). The gender byte is still packed into the low 8 bits exactly as before.
            rnd = (LCRNG_Next() | ((u32)LCRNG_Next() << 16));
            rnd = (rnd << 8) + genderMod;
            // COLOR MOD: random per-stat IVs, see the identical change/comment above for TRDATATYPE_BASE.
            ivs = INIT_IVS_RANDOM;

            build = TrainerData_GetChampionBuild(dto->trainerIDs[battler], species); // ENTRY P113
            rnd = TrainerData_ChampionPersonality(build, rnd);

            Pokemon_InitWith(mon, species, trmon[i].level, ivs, TRUE, rnd, OTID_NOT_SHINY, 0);

            for (j = 0; j < 4; j++) {
                Pokemon_SetMoveSlot(mon, trmon[i].moves[j], j);
            }

            Pokemon_SetBallSeal(trmon[i].cbSeal, mon, heapID);
            Pokemon_SetValue(mon, MON_DATA_FORM, &form);
            TrainerData_ApplyChampionIVs(build, mon); // ENTRY P113
            Party_AddPokemon(dto->parties[battler], mon);
        }

        break;
    }

    case TRDATATYPE_WITH_ITEM: {
        TrainerMonWithItem *trmon = (TrainerMonWithItem *)buf;
        for (i = 0; i < dto->trainer[battler].header.partySize; i++) {
            u16 species = trmon[i].species & 0x3FF;
            u8 form = (trmon[i].species & 0xFC00) >> TRAINER_MON_FORM_SHIFT;

            // HG SYNC (port of PokeHeartGold ENTRY AY item 1): personality used to be seeded deterministically from
            // ivScale/level/species/trainerID and rolled a fixed number of times, so a given trainer's given party
            // slot had the exact same personality (nature, ability slot, colour) on every encounter. Now a random
            // roll from the global RNG (restored to oldSeed at the end of this function, as before, so the overworld
            // RNG is unaffected). The gender byte is still packed into the low 8 bits exactly as before.
            rnd = (LCRNG_Next() | ((u32)LCRNG_Next() << 16));
            rnd = (rnd << 8) + genderMod;
            // COLOR MOD: random per-stat IVs, see the identical change/comment above for TRDATATYPE_BASE.
            ivs = INIT_IVS_RANDOM;

            build = TrainerData_GetChampionBuild(dto->trainerIDs[battler], species); // ENTRY P113
            rnd = TrainerData_ChampionPersonality(build, rnd);

            Pokemon_InitWith(mon, species, trmon[i].level, ivs, TRUE, rnd, OTID_NOT_SHINY, 0);
            Pokemon_SetValue(mon, MON_DATA_HELD_ITEM, &trmon[i].item);
            Pokemon_SetBallSeal(trmon[i].cbSeal, mon, heapID);
            Pokemon_SetValue(mon, MON_DATA_FORM, &form);
            TrainerData_ApplyChampionIVs(build, mon); // ENTRY P113
            Party_AddPokemon(dto->parties[battler], mon);
        }

        break;
    }

    case TRDATATYPE_WITH_MOVES_AND_ITEM: {
        TrainerMonWithMovesAndItem *trmon = (TrainerMonWithMovesAndItem *)buf;
        for (i = 0; i < dto->trainer[battler].header.partySize; i++) {
            u16 species = trmon[i].species & 0x3FF;
            u8 form = (trmon[i].species & 0xFC00) >> TRAINER_MON_FORM_SHIFT;

            // HG SYNC (port of PokeHeartGold ENTRY AY item 1): personality used to be seeded deterministically from
            // ivScale/level/species/trainerID and rolled a fixed number of times, so a given trainer's given party
            // slot had the exact same personality (nature, ability slot, colour) on every encounter. Now a random
            // roll from the global RNG (restored to oldSeed at the end of this function, as before, so the overworld
            // RNG is unaffected). The gender byte is still packed into the low 8 bits exactly as before.
            rnd = (LCRNG_Next() | ((u32)LCRNG_Next() << 16));
            rnd = (rnd << 8) + genderMod;
            // COLOR MOD: random per-stat IVs, see the identical change/comment above for TRDATATYPE_BASE.
            ivs = INIT_IVS_RANDOM;

            build = TrainerData_GetChampionBuild(dto->trainerIDs[battler], species); // ENTRY P113
            rnd = TrainerData_ChampionPersonality(build, rnd);

            Pokemon_InitWith(mon, species, trmon[i].level, ivs, TRUE, rnd, OTID_NOT_SHINY, 0);
            Pokemon_SetValue(mon, MON_DATA_HELD_ITEM, &trmon[i].item);

            for (j = 0; j < 4; j++) {
                Pokemon_SetMoveSlot(mon, trmon[i].moves[j], j);
            }

            Pokemon_SetBallSeal(trmon[i].cbSeal, mon, heapID);
            Pokemon_SetValue(mon, MON_DATA_FORM, &form);
            TrainerData_ApplyChampionIVs(build, mon); // ENTRY P113
            Party_AddPokemon(dto->parties[battler], mon);
        }

        break;
    }
    }

    RunGame_AdjustTrainerParty(dto->parties[battler], dto->trainerIDs[battler], battler, dto->battleType);
    Heap_Free(buf);
    Heap_Free(mon);
    LCRNG_SetSeed(oldSeed);
}
