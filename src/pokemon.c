#include "pokemon.h"

#include <nitro.h>
#include <string.h>

#include "constants/charcode.h"
#include "constants/flavor.h"
#include "constants/forms.h"
#include "constants/heap.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/narc.h"
#include "constants/pokemon.h"
#include "constants/sound.h"
#include "generated/abilities.h"
#include "generated/evolution_methods.h"
#include "generated/exp_rates.h"
#include "generated/gender_ratios.h"
#include "generated/genders.h"
#include "generated/natures.h"
#include "generated/species_data_params.h"

#include "struct_defs/chatot_cry.h"
#include "struct_defs/seal_case.h"
#include "struct_defs/species_sprite_data.h"
#include "struct_defs/sprite_animation_frame.h"
#include "struct_defs/struct_02078B40.h"

#include "ball_seal_info.h"
#include "charcode_util.h"
#include "flags.h"
#include "heap.h"
#include "run_game.h"
#include "run_session.h"
#include "inlines.h"
#include "item.h"
#include "mail.h"
#include "math_util.h"
#include "message.h"
#include "message_util.h"
#include "move_table.h"
#include "narc.h"
#include "palette.h"
#include "party.h"
#include "pokemon_anim.h"
#include "pokemon_sprite.h"
#include "rtc.h"
#include "sound_chatot.h"
#include "sound_playback.h"
#include "sprite.h"
#include "sprite_system.h"
#include "string_gf.h"
#include "trainer_data.h"
#include "trainer_info.h"
#include "unk_02017038.h"
#include "unk_02092494.h"

#include "res/pokemon/regional_pokedex_size.h"
#include "res/trainers/classes/trbgra.naix"

#define FATEFUL_ENCOUNTER_LOCATION 3002

static const s8 sNatureFlavorAffinities[][5] = {
    [NATURE_HARDY] = {
        [FLAVOR_SPICY] = 0,
        [FLAVOR_DRY] = 0,
        [FLAVOR_SWEET] = 0,
        [FLAVOR_BITTER] = 0,
        [FLAVOR_SOUR] = 0,
    },
    [NATURE_LONELY] = {
        [FLAVOR_SPICY] = 1,
        [FLAVOR_DRY] = 0,
        [FLAVOR_SWEET] = 0,
        [FLAVOR_BITTER] = 0,
        [FLAVOR_SOUR] = -1,
    },
    [NATURE_BRAVE] = {
        [FLAVOR_SPICY] = 1,
        [FLAVOR_DRY] = 0,
        [FLAVOR_SWEET] = -1,
        [FLAVOR_BITTER] = 0,
        [FLAVOR_SOUR] = 0,
    },
    [NATURE_ADAMANT] = {
        [FLAVOR_SPICY] = 1,
        [FLAVOR_DRY] = -1,
        [FLAVOR_SWEET] = 0,
        [FLAVOR_BITTER] = 0,
        [FLAVOR_SOUR] = 0,
    },
    [NATURE_NAUGHTY] = {
        [FLAVOR_SPICY] = 1,
        [FLAVOR_DRY] = 0,
        [FLAVOR_SWEET] = 0,
        [FLAVOR_BITTER] = -1,
        [FLAVOR_SOUR] = 0,
    },
    [NATURE_BOLD] = {
        [FLAVOR_SPICY] = -1,
        [FLAVOR_DRY] = 0,
        [FLAVOR_SWEET] = 0,
        [FLAVOR_BITTER] = 0,
        [FLAVOR_SOUR] = 1,
    },
    [NATURE_DOCILE] = {
        [FLAVOR_SPICY] = 0,
        [FLAVOR_DRY] = 0,
        [FLAVOR_SWEET] = 0,
        [FLAVOR_BITTER] = 0,
        [FLAVOR_SOUR] = 0,
    },
    [NATURE_RELAXED] = {
        [FLAVOR_SPICY] = 0,
        [FLAVOR_DRY] = 0,
        [FLAVOR_SWEET] = -1,
        [FLAVOR_BITTER] = 0,
        [FLAVOR_SOUR] = 1,
    },
    [NATURE_IMPISH] = {
        [FLAVOR_SPICY] = 0,
        [FLAVOR_DRY] = -1,
        [FLAVOR_SWEET] = 0,
        [FLAVOR_BITTER] = 0,
        [FLAVOR_SOUR] = 1,
    },
    [NATURE_LAX] = {
        [FLAVOR_SPICY] = 0,
        [FLAVOR_DRY] = 0,
        [FLAVOR_SWEET] = 0,
        [FLAVOR_BITTER] = -1,
        [FLAVOR_SOUR] = 1,
    },
    [NATURE_TIMID] = {
        [FLAVOR_SPICY] = -1,
        [FLAVOR_DRY] = 0,
        [FLAVOR_SWEET] = 1,
        [FLAVOR_BITTER] = 0,
        [FLAVOR_SOUR] = 0,
    },
    [NATURE_HASTY] = {
        [FLAVOR_SPICY] = 0,
        [FLAVOR_DRY] = 0,
        [FLAVOR_SWEET] = 1,
        [FLAVOR_BITTER] = 0,
        [FLAVOR_SOUR] = -1,
    },
    [NATURE_SERIOUS] = {
        [FLAVOR_SPICY] = 0,
        [FLAVOR_DRY] = 0,
        [FLAVOR_SWEET] = 0,
        [FLAVOR_BITTER] = 0,
        [FLAVOR_SOUR] = 0,
    },
    [NATURE_JOLLY] = {
        [FLAVOR_SPICY] = 0,
        [FLAVOR_DRY] = -1,
        [FLAVOR_SWEET] = 1,
        [FLAVOR_BITTER] = 0,
        [FLAVOR_SOUR] = 0,
    },
    [NATURE_NAIVE] = {
        [FLAVOR_SPICY] = 0,
        [FLAVOR_DRY] = 0,
        [FLAVOR_SWEET] = 1,
        [FLAVOR_BITTER] = -1,
        [FLAVOR_SOUR] = 0,
    },
    [NATURE_MODEST] = {
        [FLAVOR_SPICY] = -1,
        [FLAVOR_DRY] = 1,
        [FLAVOR_SWEET] = 0,
        [FLAVOR_BITTER] = 0,
        [FLAVOR_SOUR] = 0,
    },
    [NATURE_MILD] = {
        [FLAVOR_SPICY] = 0,
        [FLAVOR_DRY] = 1,
        [FLAVOR_SWEET] = 0,
        [FLAVOR_BITTER] = 0,
        [FLAVOR_SOUR] = -1,
    },
    [NATURE_QUIET] = {
        [FLAVOR_SPICY] = 0,
        [FLAVOR_DRY] = 1,
        [FLAVOR_SWEET] = -1,
        [FLAVOR_BITTER] = 0,
        [FLAVOR_SOUR] = 0,
    },
    [NATURE_BASHFUL] = {
        [FLAVOR_SPICY] = 0,
        [FLAVOR_DRY] = 0,
        [FLAVOR_SWEET] = 0,
        [FLAVOR_BITTER] = 0,
        [FLAVOR_SOUR] = 0,
    },
    [NATURE_RASH] = {
        [FLAVOR_SPICY] = 0,
        [FLAVOR_DRY] = 1,
        [FLAVOR_SWEET] = 0,
        [FLAVOR_BITTER] = -1,
        [FLAVOR_SOUR] = 0,
    },
    [NATURE_CALM] = {
        [FLAVOR_SPICY] = -1,
        [FLAVOR_DRY] = 0,
        [FLAVOR_SWEET] = 0,
        [FLAVOR_BITTER] = 1,
        [FLAVOR_SOUR] = 0,
    },
    [NATURE_GENTLE] = {
        [FLAVOR_SPICY] = 0,
        [FLAVOR_DRY] = 0,
        [FLAVOR_SWEET] = 0,
        [FLAVOR_BITTER] = 1,
        [FLAVOR_SOUR] = -1,
    },
    [NATURE_SASSY] = {
        [FLAVOR_SPICY] = 0,
        [FLAVOR_DRY] = 0,
        [FLAVOR_SWEET] = -1,
        [FLAVOR_BITTER] = 1,
        [FLAVOR_SOUR] = 0,
    },
    [NATURE_CAREFUL] = {
        [FLAVOR_SPICY] = 0,
        [FLAVOR_DRY] = -1,
        [FLAVOR_SWEET] = 0,
        [FLAVOR_BITTER] = 1,
        [FLAVOR_SOUR] = 0,
    },
    [NATURE_QUIRKY] = {
        [FLAVOR_SPICY] = 0,
        [FLAVOR_DRY] = 0,
        [FLAVOR_SWEET] = 0,
        [FLAVOR_BITTER] = 0,
        [FLAVOR_SOUR] = 0,
    },
};

enum PokemonDataBlockID {
    DATA_BLOCK_A = 0,
    DATA_BLOCK_B,
    DATA_BLOCK_C,
    DATA_BLOCK_D
};

enum TrainerClassFilesTypes {
    TCFT_TILES = 0,
    TCFT_PALETTE,
    TCFT_CELLS,
    TCFT_ANIMS,
    TCFT_SCAN,
    TRAINER_CLASS_NUM_FILETYPES,
};

static void sub_02073E18(BoxPokemon *boxMon, int monSpecies, int monLevel, int monIVs, BOOL useMonPersonalityParam, u32 monPersonality, int monOTIDSource, u32 monOTID);
static u32 Pokemon_GetDataInternal(Pokemon *mon, enum PokemonDataParam param, void *dest);
static u32 BoxPokemon_GetDataInternal(BoxPokemon *boxMon, enum PokemonDataParam param, void *dest);
static void Pokemon_SetDataInternal(Pokemon *mon, enum PokemonDataParam param, const void *value);
static void BoxPokemon_SetDataInternal(BoxPokemon *boxMon, enum PokemonDataParam param, const void *value);
static void Pokemon_IncreaseDataInternal(Pokemon *mon, enum PokemonDataParam param, int value);
static void BoxPokemon_IncreaseDataInternal(BoxPokemon *boxMon, enum PokemonDataParam param, int value);
static u32 BoxPokemon_GetExpToNextLevel(BoxPokemon *boxMon);
static void Pokemon_LoadExperienceTableOf(enum ExpRate monExpRate, u32 *monExpTable);
static u32 Pokemon_GetExpRateBaseExpAt(enum ExpRate monExpRate, int monLevel);
static u16 Pokemon_GetNatureStatValue(u8 monNature, u16 monStatValue, u8 statType);
static u8 BoxPokemon_IsShiny(BoxPokemon *boxMon);
static inline BOOL Pokemon_InlineIsPersonalityShiny(u32 monOTID, u32 monPersonality);
static void BuildPokemonSpriteTemplateDP(PokemonSpriteTemplate *spriteTemplate, u16 species, u8 gender, u8 face, u8 shiny, u8 form, u32 personality);
static u8 LoadPokemonDPSpriteHeight(u16 species, u8 gender, u8 face, u8 form, u32 personality);
static void BoxPokemon_SetDefaultMoves(BoxPokemon *boxMon);
static u16 BoxPokemon_AddMove(BoxPokemon *boxMon, u16 moveID);
static void BoxPokemon_ReplaceMove(BoxPokemon *boxMon, u16 moveID);
static void BoxPokemon_SetMoveSlot(BoxPokemon *boxMon, u16 moveID, u8 moveSlot);
static BOOL Pokemon_HasMove(Pokemon *mon, u16 moveID);
static s8 BoxPokemon_GetFlavorAffinity(BoxPokemon *boxMon, enum Flavor flavor);
static BOOL IsBoxPokemonInfectedWithPokerus(BoxPokemon *boxMon);
static BOOL BoxPokemonHasCuredPokerus(BoxPokemon *boxMon);
static void InitializeBoxPokemonAfterCapture(BoxPokemon *boxMon, TrainerInfo *trainer, int monPokeball, int metLocation, int metTerrain, enum HeapID heapID);
static void PostCaptureBoxPokemonProcessing(BoxPokemon *boxMon, TrainerInfo *trainer, int monPokeball, int metLocation, int metTerrain, int heapID);
static BOOL CanBoxPokemonLearnTM(BoxPokemon *boxMon, u8 tmID);
static void BoxPokemon_CalcAbility(BoxPokemon *boxMon);
static void SpeciesData_LoadSpecies(int monSpecies, SpeciesData *speciesData);
static void SpeciesData_LoadForm(int monSpecies, int monForm, SpeciesData *speciesData);
static void LoadSpeciesEvolutions(int monSpecies, SpeciesEvolution speciesEvolutions[MAX_EVOLUTIONS]);
static void Pokemon_EncryptData(void *data, u32 bytes, u32 seed);
static void Pokemon_DecryptData(void *data, u32 bytes, u32 seed);
static u16 Pokemon_GetDataChecksum(void *data, u32 bytes);
static void *BoxPokemon_GetDataBlock(BoxPokemon *boxMon, u32 personality, enum PokemonDataBlockID dataBlockID);
static int Pokemon_GetFormNarcIndex(int monSpecies, int monForm);
static inline int Pokemon_Face(int clientType);

void Pokemon_Init(Pokemon *mon)
{
    MI_CpuClearFast(mon, sizeof(Pokemon));
    Pokemon_EncryptData(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4, mon->box.checksum);
    Pokemon_EncryptData(&mon->party, sizeof(PartyPokemon), mon->box.personality);
}

void BoxPokemon_Init(BoxPokemon *boxMon)
{
    MI_CpuClearFast(boxMon, sizeof(BoxPokemon));
    Pokemon_EncryptData(boxMon->dataBlocks, sizeof(PokemonDataBlock) * 4, boxMon->checksum);
}

int Pokemon_StructSize(void)
{
    return sizeof(Pokemon);
}

Pokemon *Pokemon_New(enum HeapID heapID)
{
    Pokemon *mon = Heap_Alloc(heapID, sizeof(Pokemon));
    Pokemon_Init(mon);
    return mon;
}

BOOL Pokemon_EnterDecryptionContext(Pokemon *mon)
{
    BOOL wasDecrypted = FALSE;

    if (mon->box.partyDecrypted == FALSE) {
        wasDecrypted = TRUE;
        GF_ASSERT(mon->box.boxDecrypted == FALSE);

        mon->box.partyDecrypted = TRUE;
        mon->box.boxDecrypted = TRUE;

        Pokemon_DecryptData(&mon->party, sizeof(PartyPokemon), mon->box.personality);
        Pokemon_DecryptData(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4, mon->box.checksum);
    }

    return wasDecrypted;
}

BOOL Pokemon_ExitDecryptionContext(Pokemon *mon, BOOL encrypt)
{
    BOOL wasEncrypted = FALSE;

    if (mon->box.partyDecrypted == TRUE && encrypt == TRUE) {
        wasEncrypted = TRUE;
        mon->box.partyDecrypted = FALSE;
        mon->box.boxDecrypted = FALSE;

        Pokemon_EncryptData(&mon->party, sizeof(PartyPokemon), mon->box.personality);
        mon->box.checksum = Pokemon_GetDataChecksum(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4);
        Pokemon_EncryptData(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4, mon->box.checksum);
    }

    return wasEncrypted;
}

BOOL BoxPokemon_EnterDecryptionContext(BoxPokemon *boxMon)
{
    BOOL wasDecrypted = FALSE;

    if (boxMon->boxDecrypted == FALSE) {
        wasDecrypted = TRUE;
        boxMon->boxDecrypted = TRUE;
        Pokemon_DecryptData(boxMon->dataBlocks, sizeof(PokemonDataBlock) * 4, boxMon->checksum);
    }

    return wasDecrypted;
}

BOOL BoxPokemon_ExitDecryptionContext(BoxPokemon *boxMon, BOOL encrypt)
{
    BOOL wasEncrypted = FALSE;

    if (boxMon->boxDecrypted == TRUE && encrypt == TRUE) {
        wasEncrypted = TRUE;

        boxMon->boxDecrypted = FALSE;
        boxMon->checksum = Pokemon_GetDataChecksum(boxMon->dataBlocks, sizeof(PokemonDataBlock) * 4);

        Pokemon_EncryptData(boxMon->dataBlocks, sizeof(PokemonDataBlock) * 4, boxMon->checksum);
    }

    return wasEncrypted;
}

void Pokemon_InitWith(Pokemon *mon, int monSpecies, int monLevel, int monIVs, BOOL useMonPersonalityParam, u32 monPersonality, int monOTIDSource, u32 monOTID)
{
    Pokemon_Init(mon);

    sub_02073E18(&mon->box, monSpecies, monLevel, monIVs, useMonPersonalityParam, monPersonality, monOTIDSource, monOTID);
    Pokemon_EncryptData(&mon->party, sizeof(PartyPokemon), 0);
    Pokemon_EncryptData(&mon->party, sizeof(PartyPokemon), mon->box.personality);
    Pokemon_SetValue(mon, MON_DATA_LEVEL, &monLevel);

    Mail *mail = Mail_New(HEAP_ID_SYSTEM);

    Pokemon_SetValue(mon, MON_DATA_MAIL, mail);
    Heap_Free(mail);

    u32 zero = 0;
    Pokemon_SetValue(mon, MON_DATA_BALL_CAPSULE_ID, &zero);

    BallCapsule v2;
    MI_CpuClearFast(&v2, sizeof(BallCapsule));

    Pokemon_SetValue(mon, MON_DATA_BALL_CAPSULE, &v2);
    Pokemon_CalcLevelAndStats(mon);
}

static void sub_02073E18(BoxPokemon *boxMon, int monSpecies, int monLevel, int monIVs, BOOL useMonPersonalityParam, u32 monPersonality, int monOTIDSource, u32 monOTID)
{
    BoxPokemon_Init(boxMon);

    BOOL reencrypt = BoxPokemon_EnterDecryptionContext(boxMon);

    if (!useMonPersonalityParam) {
        monPersonality = (LCRNG_Next() | (LCRNG_Next() << 16));
    }

    BoxPokemon_SetValue(boxMon, MON_DATA_PERSONALITY, &monPersonality);

    if (monOTIDSource == OTID_NOT_SHINY) {
        // COLOR MOD (port of PokeHeartGold ENTRY AD item 2): this used to reroll the OT ID until the mon
        // could not be shiny (a "no shiny trainer/starter/etc mons" guarantee). One unconstrained roll,
        // same as every other OT ID source, so these mons can be shiny too (at the new SHINY_ODDS rate).
        monOTID = (LCRNG_Next() | (LCRNG_Next() << 16));
    } else if (monOTIDSource != OTID_SET) {
        monOTID = 0;
    }

    BoxPokemon_SetValue(boxMon, MON_DATA_OT_ID, &monOTID);
    BoxPokemon_SetValue(boxMon, MON_DATA_LANGUAGE, &gGameLanguage);
    BoxPokemon_SetValue(boxMon, MON_DATA_SPECIES, &monSpecies);
    BoxPokemon_SetValue(boxMon, MON_DATA_SPECIES_NAME, NULL);

    u32 v1, v2; // TODO rename, these are used/reused as temp vars through the whole function.

    v1 = Pokemon_GetSpeciesBaseExpAt(monSpecies, monLevel);
    BoxPokemon_SetValue(boxMon, MON_DATA_EXPERIENCE, &v1);

    v1 = SpeciesData_GetSpeciesValue(monSpecies, SPECIES_DATA_BASE_FRIENDSHIP);
    BoxPokemon_SetValue(boxMon, MON_DATA_FRIENDSHIP, &v1);

    BoxPokemon_SetValue(boxMon, MON_DATA_MET_LEVEL, &monLevel);
    BoxPokemon_SetValue(boxMon, MON_DATA_MET_GAME, &gGameVersion);

    v1 = ITEM_POKE_BALL;
    BoxPokemon_SetValue(boxMon, MON_DATA_POKEBALL, &v1);

    if (monIVs < INIT_IVS_RANDOM) {
        BoxPokemon_SetValue(boxMon, MON_DATA_HP_IV, &monIVs);
        BoxPokemon_SetValue(boxMon, MON_DATA_ATK_IV, &monIVs);
        BoxPokemon_SetValue(boxMon, MON_DATA_DEF_IV, &monIVs);
        BoxPokemon_SetValue(boxMon, MON_DATA_SPEED_IV, &monIVs);
        BoxPokemon_SetValue(boxMon, MON_DATA_SPATK_IV, &monIVs);
        BoxPokemon_SetValue(boxMon, MON_DATA_SPDEF_IV, &monIVs);
    } else {
        v1 = LCRNG_Next();
        v2 = (v1 & (0x1f << 0)) >> 0;
        BoxPokemon_SetValue(boxMon, MON_DATA_HP_IV, &v2);

        v2 = (v1 & (0x1f << 5)) >> 5;
        BoxPokemon_SetValue(boxMon, MON_DATA_ATK_IV, &v2);

        v2 = (v1 & (0x1f << 10)) >> 10;
        BoxPokemon_SetValue(boxMon, MON_DATA_DEF_IV, &v2);

        v1 = LCRNG_Next();
        v2 = (v1 & (0x1f << 0)) >> 0;
        BoxPokemon_SetValue(boxMon, MON_DATA_SPEED_IV, &v2);

        v2 = (v1 & (0x1f << 5)) >> 5;
        BoxPokemon_SetValue(boxMon, MON_DATA_SPATK_IV, &v2);

        v2 = (v1 & (0x1f << 10)) >> 10;
        BoxPokemon_SetValue(boxMon, MON_DATA_SPDEF_IV, &v2);
    }

    v1 = SpeciesData_GetSpeciesValue(monSpecies, SPECIES_DATA_ABILITY_1);
    v2 = SpeciesData_GetSpeciesValue(monSpecies, SPECIES_DATA_ABILITY_2);

    if (v2 != ABILITY_NONE) {
        if (monPersonality & 1) {
            BoxPokemon_SetValue(boxMon, MON_DATA_ABILITY, &v2);
        } else {
            BoxPokemon_SetValue(boxMon, MON_DATA_ABILITY, &v1);
        }
    } else {
        BoxPokemon_SetValue(boxMon, MON_DATA_ABILITY, &v1);
    }

    v1 = BoxPokemon_GetGender(boxMon);

    BoxPokemon_SetValue(boxMon, MON_DATA_GENDER, &v1);
    BoxPokemon_SetDefaultMoves(boxMon);
    BoxPokemon_ExitDecryptionContext(boxMon, reencrypt);
}

void sub_02074044(Pokemon *mon, u16 monSpecies, u8 monLevel, u8 monIVs, u8 monNature)
{
    u32 monPersonality;

    do {
        monPersonality = (LCRNG_Next() | (LCRNG_Next() << 16));
    } while (monNature != Pokemon_GetNatureOf(monPersonality));

    Pokemon_InitWith(mon, monSpecies, monLevel, monIVs, TRUE, monPersonality, OTID_NOT_SET, 0);
}

void sub_02074088(Pokemon *mon, u16 monSpecies, u8 monLevel, u8 monIVs, u8 gender, u8 param5, u8 param6)
{
    u32 monPersonality;
    u16 unownLetter;

    // TODO enum value
    if (param6 && param6 < 29) {
        do {
            monPersonality = (LCRNG_Next() | (LCRNG_Next() << 16));
            unownLetter = (((monPersonality & 0x3000000) >> 18) | ((monPersonality & 0x30000) >> 12) | ((monPersonality & 0x300) >> 6) | (monPersonality & 0x3)) % 28;
        } while (param5 != Pokemon_GetNatureOf(monPersonality) || gender != Pokemon_GetGenderOf(monSpecies, monPersonality) || unownLetter != param6 - 1);
    } else {
        monPersonality = sub_02074128(monSpecies, gender, param5);
    }

    Pokemon_InitWith(mon, monSpecies, monLevel, monIVs, TRUE, monPersonality, OTID_NOT_SET, 0);
}

u32 sub_02074128(u16 monSpecies, u8 gender, u8 param2)
{
    u8 monGenderChance = SpeciesData_GetSpeciesValue(monSpecies, SPECIES_DATA_GENDER_RATIO);

    u32 result;
    switch (monGenderChance) {
    case GENDER_RATIO_MALE_ONLY:
    case GENDER_RATIO_FEMALE_ONLY:
    case GENDER_RATIO_NO_GENDER:
        result = param2;
        break;
    default:
        if (gender == GENDER_MALE) {
            result = 25 * ((monGenderChance / 25) + 1);
            result += param2;
        } else {
            result = param2;
        }
        break;
    }

    return result;
}

// only used when encountering a roamer
void Pokemon_InitAndCalcStats(Pokemon *mon, u16 monSpecies, u8 monLevel, u32 monCombinedIVs, u32 monPersonality)
{
    Pokemon_InitWith(mon, monSpecies, monLevel, 0, TRUE, monPersonality, OTID_NOT_SET, 0);
    Pokemon_SetValue(mon, MON_DATA_COMBINED_IVS, &monCombinedIVs);
    Pokemon_CalcLevelAndStats(mon);
}

void Pokemon_CalcLevelAndStats(Pokemon *mon)
{
    BOOL reencrypt = Pokemon_EnterDecryptionContext(mon);
    int monLevel = Pokemon_GetLevel(mon);

    Pokemon_SetValue(mon, MON_DATA_LEVEL, &monLevel);
    Pokemon_CalcStats(mon);
    Pokemon_ExitDecryptionContext(mon, reencrypt);
}

void Pokemon_CalcStats(Pokemon *mon)
{
    int monMaxHp;
    int monCurrentHp;
    int monHpIV, monAtkIV, monDefIV, monSpeedIV, monSpAtkIV, monSpDefIV;
    int monHpEV, monAtkEV, monDefEV, monSpeedEV, monSpAtkEV, monSpDefEV;

    BOOL reencrypt = Pokemon_EnterDecryptionContext(mon);

    int monLevel = Pokemon_GetValue(mon, MON_DATA_LEVEL, NULL);

    monMaxHp = Pokemon_GetValue(mon, MON_DATA_MAX_HP, NULL);
    monCurrentHp = Pokemon_GetValue(mon, MON_DATA_HP, NULL);

    monHpIV = Pokemon_GetValue(mon, MON_DATA_HP_IV, NULL);
    monHpEV = Pokemon_GetValue(mon, MON_DATA_HP_EV, NULL);
    monAtkIV = Pokemon_GetValue(mon, MON_DATA_ATK_IV, NULL);
    monAtkEV = Pokemon_GetValue(mon, MON_DATA_ATK_EV, NULL);
    monDefIV = Pokemon_GetValue(mon, MON_DATA_DEF_IV, NULL);
    monDefEV = Pokemon_GetValue(mon, MON_DATA_DEF_EV, NULL);
    monSpeedIV = Pokemon_GetValue(mon, MON_DATA_SPEED_IV, NULL);
    monSpeedEV = Pokemon_GetValue(mon, MON_DATA_SPEED_EV, NULL);
    monSpAtkIV = Pokemon_GetValue(mon, MON_DATA_SPATK_IV, NULL);
    monSpAtkEV = Pokemon_GetValue(mon, MON_DATA_SPATK_EV, NULL);
    monSpDefIV = Pokemon_GetValue(mon, MON_DATA_SPDEF_IV, NULL);
    monSpDefEV = Pokemon_GetValue(mon, MON_DATA_SPDEF_EV, NULL);

    int monForm = Pokemon_GetValue(mon, MON_DATA_FORM, NULL);
    int monSpecies = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
    SpeciesData *speciesData = Heap_Alloc(HEAP_ID_SYSTEM, sizeof(SpeciesData));

    SpeciesData_LoadForm(monSpecies, monForm, speciesData);

    int newMaxHp;
    if (monSpecies == SPECIES_SHEDINJA) {
        newMaxHp = 1;
    } else {
        newMaxHp = ((2 * speciesData->baseStats.hp + monHpIV + monHpEV / 4) * monLevel / 100 + monLevel + 10);
    }

    Pokemon_SetValue(mon, MON_DATA_MAX_HP, &newMaxHp);

    // TODO inline func maybe
    int newAtk = ((2 * speciesData->baseStats.attack + monAtkIV + monAtkEV / 4) * monLevel / 100 + 5);
    newAtk = Pokemon_GetNatureStatValue(Pokemon_GetNature(mon), newAtk, STAT_ATTACK);

    Pokemon_SetValue(mon, MON_DATA_ATK, &newAtk);

    int newDef = ((2 * speciesData->baseStats.defense + monDefIV + monDefEV / 4) * monLevel / 100 + 5);
    newDef = Pokemon_GetNatureStatValue(Pokemon_GetNature(mon), newDef, STAT_DEFENSE);

    Pokemon_SetValue(mon, MON_DATA_DEF, &newDef);

    int newSpeed = ((2 * speciesData->baseStats.speed + monSpeedIV + monSpeedEV / 4) * monLevel / 100 + 5);
    newSpeed = Pokemon_GetNatureStatValue(Pokemon_GetNature(mon), newSpeed, STAT_SPEED);

    Pokemon_SetValue(mon, MON_DATA_SPEED, &newSpeed);

    int newSpAtk = ((2 * speciesData->baseStats.spAttack + monSpAtkIV + monSpAtkEV / 4) * monLevel / 100 + 5);
    newSpAtk = Pokemon_GetNatureStatValue(Pokemon_GetNature(mon), newSpAtk, STAT_SPECIAL_ATTACK);

    Pokemon_SetValue(mon, MON_DATA_SP_ATK, &newSpAtk);

    int newSpDef = ((2 * speciesData->baseStats.spDefense + monSpDefIV + monSpDefEV / 4) * monLevel / 100 + 5);
    newSpDef = Pokemon_GetNatureStatValue(Pokemon_GetNature(mon), newSpDef, STAT_SPECIAL_DEFENSE);

    Pokemon_SetValue(mon, MON_DATA_SP_DEF, &newSpDef);
    Heap_Free(speciesData);

    if (monCurrentHp != 0 || monMaxHp == 0) {
        if (monSpecies == SPECIES_SHEDINJA) {
            monCurrentHp = 1;
        } else if (monCurrentHp == 0) {
            monCurrentHp = newMaxHp;
        } else {
            monCurrentHp += newMaxHp - monMaxHp;
        }
    }

    if (monCurrentHp) {
        Pokemon_SetValue(mon, MON_DATA_HP, &monCurrentHp);
    }

    Pokemon_ExitDecryptionContext(mon, reencrypt);
}

u32 Pokemon_GetValue(Pokemon *mon, enum PokemonDataParam param, void *dest)
{
    if (mon->box.partyDecrypted == FALSE) {
        Pokemon_DecryptData(&mon->party, sizeof(PartyPokemon), mon->box.personality);
        Pokemon_DecryptData(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4, mon->box.checksum);

        u16 checksum = Pokemon_GetDataChecksum(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4);

        if (checksum != mon->box.checksum) {
            GF_ASSERT(checksum == mon->box.checksum);
            mon->box.checksumFailed = TRUE;
        }
    }

    u32 result = Pokemon_GetDataInternal(mon, param, dest);

    if (mon->box.partyDecrypted == FALSE) {
        Pokemon_EncryptData(&mon->party, sizeof(PartyPokemon), mon->box.personality);
        Pokemon_EncryptData(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4, mon->box.checksum);
    }

    return result;
}

static u32 Pokemon_GetDataInternal(Pokemon *mon, enum PokemonDataParam param, void *dest)
{
    u32 result = 0;

    switch (param) {
    case MON_DATA_STATUS:
        result = mon->party.status;
        break;

    case MON_DATA_LEVEL:
        result = mon->party.level;
        break;

    case MON_DATA_BALL_CAPSULE_ID:
        result = mon->party.ballCapsuleID;
        break;

    case MON_DATA_HP:
        result = mon->party.hp;
        break;

    case MON_DATA_MAX_HP:
        result = mon->party.maxHP;
        break;

    case MON_DATA_ATK:
        result = mon->party.attack;
        break;

    case MON_DATA_DEF:
        result = mon->party.defense;
        break;

    case MON_DATA_SPEED:
        result = mon->party.speed;
        break;

    case MON_DATA_SP_ATK:
        result = mon->party.spAtk;
        break;

    case MON_DATA_SP_DEF:
        result = mon->party.spDef;
        break;

    case MON_DATA_MAIL:
        Mail_Copy(&mon->party.mail, dest);
        result = TRUE;
        break;

    case MON_DATA_BALL_CAPSULE:
        BallCapsule_Copy(&mon->party.ballCapsule, dest);
        result = TRUE;
        break;

    default:
        result = BoxPokemon_GetDataInternal(&mon->box, param, dest);
        break;
    }

    return result;
}

u32 BoxPokemon_GetValue(BoxPokemon *boxMon, enum PokemonDataParam param, void *dest)
{
    if (boxMon->boxDecrypted == FALSE) {
        Pokemon_DecryptData(boxMon->dataBlocks, sizeof(PokemonDataBlock) * 4, boxMon->checksum);
        u16 checksum = Pokemon_GetDataChecksum(boxMon->dataBlocks, sizeof(PokemonDataBlock) * 4);

        if (checksum != boxMon->checksum) {
            GF_ASSERT(checksum == boxMon->checksum);
            boxMon->checksumFailed = TRUE;
        }
    }

    u32 result = BoxPokemon_GetDataInternal(boxMon, param, dest);

    if (boxMon->boxDecrypted == FALSE) {
        Pokemon_EncryptData(boxMon->dataBlocks, sizeof(PokemonDataBlock) * 4, boxMon->checksum);
    }

    return result;
}

static inline u32 GetRibbon(u64 mask, enum PokemonDataParam param, enum PokemonDataParam ribbonStart)
{
    u64 bit = 1; // need to force a u64 to match
    return (mask & (bit << (param - ribbonStart))) != 0;
}

// COLOR MOD (port of PokeHeartGold ENTRY AD item 3): HP IV overrides type1 and Speed IV overrides type2
// when that IV is a valid type index (below CUSTOM_TYPE_COUNT and not TYPE_MYSTERY). There is one place
// every type is read from (the MON_DATA_TYPE_1 / MON_DATA_TYPE_2 case below), which every other reader
// (battle, summary screen, TM/HM compat) goes through, so the override lives here once and can't be
// missed or reset on a switch-in; nothing stored in a save changes.
#define CUSTOM_TYPE_COUNT 18
static u32 ApplyCustomType(u32 speciesType, u32 iv)
{
    if (iv < CUSTOM_TYPE_COUNT && iv != TYPE_MYSTERY) {
        return iv;
    }
    return speciesType;
}

// COLOR MOD (port of PokeHeartGold ENTRY AC): a mon's ability can be replaced by another species'
// ability ("donor"), decided only by its Def and Sp.Def IVs, so it is fixed and repeatable per
// individual (see PokeHeartGold's Changelog ENTRY AC for the full derivation of this seed -> donor
// mapping against the FireRed romhack it was ported from). Unlike HG, no separate generated lookup
// table is needed here: SpeciesData_GetSpeciesValue is already a per-species data accessor called
// unconditionally elsewhere in this same getter (see the type case below), so it's safe to call again
// here on every ability read.
// HG SYNC (port of PokeHeartGold ENTRY CE item 1): the seed -> donor mapping is now a direct, wrapping species index:
// seed 0 -> no donor (keep the real ability); 1..493 -> donor species = seed; 494..986 -> seed - 493 (a second pass
// through 1..493); 987..1023 -> no donor. The seed itself is unchanged: (defIV << 5) | spDefIV.
static u32 GetAbilityDonorSpecies(u32 seed)
{
    if (seed == 0 || seed > NATIONAL_DEX_COUNT * 2) {
        return SPECIES_NONE;
    }
    if (seed <= NATIONAL_DEX_COUNT) {
        return seed;
    }
    return seed - NATIONAL_DEX_COUNT;
}

// HG SYNC (ENTRY CE): which of the mon's OWN ability slots it uses (0 = ability 1, 1 = ability 2) - the same rule
// BoxPokemon_CalcAbility uses when it creates the mon. Used only by the Multitype type override now (ApplyAbilityDonor
// picks the donor slot from the personality's low bit alone - see there).
static u32 GetMonAbilitySlot(u32 species, u32 personality)
{
    if (species == SPECIES_NONE || species > NATIONAL_DEX_COUNT) {
        return 0;
    }

    return (SpeciesData_GetSpeciesValue(species, SPECIES_DATA_ABILITY_2) != ABILITY_NONE && (personality & 1)) ? 1 : 0;
}

// QOL (2026-10-11): the ability a Pokemon of this species and personality would have WITHOUT the ability-donor override
// (same slot rule the game uses when it creates the mon). Used by the trainer AI, which on the first turn of a battle
// is only allowed to know this "original" ability - see TrainerAI_HidePlayerInfo.
u32 Pokemon_GetOriginalAbilityOf(u32 species, u32 personality)
{
    if (species == SPECIES_NONE || species > NATIONAL_DEX_COUNT) {
        return ABILITY_NONE;
    }

    return SpeciesData_GetSpeciesValue(species, GetMonAbilitySlot(species, personality) == 1 ? SPECIES_DATA_ABILITY_2 : SPECIES_DATA_ABILITY_1);
}

static u32 ApplyAbilityDonor(u32 storedAbility, u32 species, u32 personality, const PokemonDataBlockB *blockB)
{
    if (species == SPECIES_NONE || species > NATIONAL_DEX_COUNT || blockB->isEgg) {
        return storedAbility;
    }

    u32 donor = GetAbilityDonorSpecies((blockB->defIV << 5) | blockB->spDefIV);
    if (donor == SPECIES_NONE) {
        return storedAbility;
    }

    // HG SYNC (QOL 2026-10-05): the donor slot follows the personality's low bit ALONE, so a species with only one ability of its
    // own (Rayquaza, Arceus, ...) can still receive the donor's SECOND ability. GetMonAbilitySlot (the game's own rule: odd AND
    // the mon's species has a second ability) is kept only for the Multitype type override so a real Arceus keeps overriding Type1.
    enum SpeciesDataParam slot = (personality & 1) ? SPECIES_DATA_ABILITY_2 : SPECIES_DATA_ABILITY_1;

    u32 ability = SpeciesData_GetSpeciesValue(donor, slot);
    if (ability == ABILITY_NONE) {
        ability = SpeciesData_GetSpeciesValue(donor, SPECIES_DATA_ABILITY_1);
    }
    if (ability == ABILITY_NONE) {
        return storedAbility;
    }
    return ability;
}

static u32 BoxPokemon_GetDataInternal(BoxPokemon *boxMon, enum PokemonDataParam param, void *dest)
{
    u32 result = 0;

    PokemonDataBlockA *monDataBlockA = BoxPokemon_GetDataBlock(boxMon, boxMon->personality, DATA_BLOCK_A);
    PokemonDataBlockB *monDataBlockB = BoxPokemon_GetDataBlock(boxMon, boxMon->personality, DATA_BLOCK_B);
    PokemonDataBlockC *monDataBlockC = BoxPokemon_GetDataBlock(boxMon, boxMon->personality, DATA_BLOCK_C);
    PokemonDataBlockD *monDataBlockD = BoxPokemon_GetDataBlock(boxMon, boxMon->personality, DATA_BLOCK_D);

    switch (param) {
    default:
        result = 0;
        break;

    case MON_DATA_PERSONALITY:
        result = boxMon->personality;
        break;

    case MON_DATA_IS_PARTY_DECRYPTED:
        result = boxMon->partyDecrypted;
        break;

    case MON_DATA_IS_BOX_DECRYPTED:
        result = boxMon->boxDecrypted;
        break;

    case MON_DATA_CHECKSUM_FAILED:
        result = boxMon->checksumFailed;
        break;

    case MON_DATA_CHECKSUM:
        result = boxMon->checksum;
        break;

    case MON_DATA_SPECIES_EXISTS:
        result = (monDataBlockA->species != SPECIES_NONE);
        break;

    case MON_DATA_SANITY_IS_EGG:
        if (boxMon->checksumFailed) {
            result = boxMon->checksumFailed;
        } else {
            result = monDataBlockB->isEgg;
        }

        break;

    case MON_DATA_SPECIES_OR_EGG:
        result = monDataBlockA->species;
        if (result != SPECIES_NONE && (monDataBlockB->isEgg || boxMon->checksumFailed)) {
            result = SPECIES_EGG;
        }

        break;

    case MON_DATA_LEVEL:
        result = Pokemon_GetSpeciesLevelAt(monDataBlockA->species, monDataBlockA->exp);
        break;

    case MON_DATA_SPECIES:
        if (boxMon->checksumFailed) {
            result = SPECIES_EGG;
        } else {
            result = monDataBlockA->species;
        }

        break;

    case MON_DATA_HELD_ITEM:
        result = monDataBlockA->heldItem;
        break;

    case MON_DATA_OT_ID:
        result = monDataBlockA->otID;
        break;

    case MON_DATA_EXPERIENCE:
        result = monDataBlockA->exp;
        break;

    case MON_DATA_FRIENDSHIP:
        result = monDataBlockA->friendship;
        break;

    case MON_DATA_ABILITY:
        // COLOR MOD (port of PokeHeartGold ENTRY AC): ability donor override, see ApplyAbilityDonor
        // above. Done on read, so nothing stored in a save changes and existing mons are covered too.
        result = ApplyAbilityDonor(monDataBlockA->ability, monDataBlockA->species, boxMon->personality, monDataBlockB);
        break;

    case MON_DATA_MARKINGS:
        result = monDataBlockA->markings;
        break;

    case MON_DATA_LANGUAGE:
        result = monDataBlockA->originLanguage;
        break;

    case MON_DATA_HP_EV:
        result = monDataBlockA->hpEV;
        break;

    case MON_DATA_ATK_EV:
        result = monDataBlockA->atkEV;
        break;

    case MON_DATA_DEF_EV:
        result = monDataBlockA->defEV;
        break;

    case MON_DATA_SPEED_EV:
        result = monDataBlockA->speedEV;
        break;

    case MON_DATA_SPATK_EV:
        result = monDataBlockA->spAtkEV;
        break;

    case MON_DATA_SPDEF_EV:
        result = monDataBlockA->spDefEV;
        break;

    case MON_DATA_COOL:
        result = monDataBlockA->cool;
        break;

    case MON_DATA_BEAUTY:
        result = monDataBlockA->beauty;
        break;

    case MON_DATA_CUTE:
        result = monDataBlockA->cute;
        break;

    case MON_DATA_SMART:
        result = monDataBlockA->smart;
        break;

    case MON_DATA_TOUGH:
        result = monDataBlockA->tough;
        break;

    case MON_DATA_SHEEN:
        result = monDataBlockA->sheen;
        break;

    case MON_DATA_SINNOH_CHAMP_RIBBON:
    case MON_DATA_ABILITY_RIBBON:
    case MON_DATA_GREAT_ABILITY_RIBBON:
    case MON_DATA_DOUBLE_ABILITY_RIBBON:
    case MON_DATA_MULTI_ABILITY_RIBBON:
    case MON_DATA_PAIR_ABILITY_RIBBON:
    case MON_DATA_WORLD_ABILITY_RIBBON:
    case MON_DATA_ALERT_RIBBON:
    case MON_DATA_SHOCK_RIBBON:
    case MON_DATA_DOWNCAST_RIBBON:
    case MON_DATA_CARELESS_RIBBON:
    case MON_DATA_RELAX_RIBBON:
    case MON_DATA_SNOOZE_RIBBON:
    case MON_DATA_SMILE_RIBBON:
    case MON_DATA_GORGEOUS_RIBBON:
    case MON_DATA_ROYAL_RIBBON:
    case MON_DATA_GORGEOUS_ROYAL_RIBBON:
    case MON_DATA_FOOTPRINT_RIBBON:
    case MON_DATA_RECORD_RIBBON:
    case MON_DATA_HISTORY_RIBBON:
    case MON_DATA_LEGEND_RIBBON:
    case MON_DATA_RED_RIBBON:
    case MON_DATA_GREEN_RIBBON:
    case MON_DATA_BLUE_RIBBON:
    case MON_DATA_FESTIVAL_RIBBON:
    case MON_DATA_CARNIVAL_RIBBON:
    case MON_DATA_CLASSIC_RIBBON:
    case MON_DATA_PREMIER_RIBBON:
    case MON_DATA_UNUSED_RIBBON_53:
        result = GetRibbon(monDataBlockA->ribbonsDS1, param, MON_DATA_SINNOH_CHAMP_RIBBON);
        break;

    case MON_DATA_MOVE1:
    case MON_DATA_MOVE2:
    case MON_DATA_MOVE3:
    case MON_DATA_MOVE4:
        result = monDataBlockB->moves[param - MON_DATA_MOVE1];
        break;

    case MON_DATA_MOVE1_PP:
    case MON_DATA_MOVE2_PP:
    case MON_DATA_MOVE3_PP:
    case MON_DATA_MOVE4_PP:
        result = monDataBlockB->moveCurrentPPs[param - MON_DATA_MOVE1_PP];
        break;

    case MON_DATA_MOVE1_PP_UPS:
    case MON_DATA_MOVE2_PP_UPS:
    case MON_DATA_MOVE3_PP_UPS:
    case MON_DATA_MOVE4_PP_UPS:
        result = monDataBlockB->movePPUps[param - MON_DATA_MOVE1_PP_UPS];
        break;

    case MON_DATA_MOVE1_MAX_PP:
    case MON_DATA_MOVE2_MAX_PP:
    case MON_DATA_MOVE3_MAX_PP:
    case MON_DATA_MOVE4_MAX_PP:
        result = MoveTable_CalcMaxPP(monDataBlockB->moves[param - MON_DATA_MOVE1_MAX_PP], monDataBlockB->movePPUps[param - MON_DATA_MOVE1_MAX_PP]);
        break;

    case MON_DATA_HP_IV:
        result = monDataBlockB->hpIV;
        break;

    case MON_DATA_ATK_IV:
        result = monDataBlockB->atkIV;
        break;

    case MON_DATA_DEF_IV:
        result = monDataBlockB->defIV;
        break;

    case MON_DATA_SPEED_IV:
        result = monDataBlockB->speedIV;
        break;

    case MON_DATA_SPATK_IV:
        result = monDataBlockB->spAtkIV;
        break;

    case MON_DATA_SPDEF_IV:
        result = monDataBlockB->spDefIV;
        break;

    case MON_DATA_IS_EGG:
        if (boxMon->checksumFailed) {
            result = boxMon->checksumFailed;
        } else {
            result = monDataBlockB->isEgg;
        }
        break;

    case MON_DATA_HAS_NICKNAME:
        result = monDataBlockB->hasNickname;
        break;

    case MON_DATA_COOL_RIBBON:
    case MON_DATA_COOL_RIBBON_SUPER:
    case MON_DATA_COOL_RIBBON_HYPER:
    case MON_DATA_COOL_RIBBON_MASTER:
    case MON_DATA_BEAUTY_RIBBON:
    case MON_DATA_BEAUTY_RIBBON_SUPER:
    case MON_DATA_BEAUTY_RIBBON_HYPER:
    case MON_DATA_BEAUTY_RIBBON_MASTER:
    case MON_DATA_CUTE_RIBBON:
    case MON_DATA_CUTE_RIBBON_SUPER:
    case MON_DATA_CUTE_RIBBON_HYPER:
    case MON_DATA_CUTE_RIBBON_MASTER:
    case MON_DATA_SMART_RIBBON:
    case MON_DATA_SMART_RIBBON_SUPER:
    case MON_DATA_SMART_RIBBON_HYPER:
    case MON_DATA_SMART_RIBBON_MASTER:
    case MON_DATA_TOUGH_RIBBON:
    case MON_DATA_TOUGH_RIBBON_SUPER:
    case MON_DATA_TOUGH_RIBBON_HYPER:
    case MON_DATA_TOUGH_RIBBON_MASTER:
    case MON_DATA_CHAMPION_RIBBON:
    case MON_DATA_WINNING_RIBBON:
    case MON_DATA_VICTORY_RIBBON:
    case MON_DATA_ARTIST_RIBBON:
    case MON_DATA_EFFORT_RIBBON:
    case MON_DATA_MARINE_RIBBON:
    case MON_DATA_LAND_RIBBON:
    case MON_DATA_SKY_RIBBON:
    case MON_DATA_COUNTRY_RIBBON:
    case MON_DATA_NATIONAL_RIBBON:
    case MON_DATA_EARTH_RIBBON:
    case MON_DATA_WORLD_RIBBON:
        result = GetRibbon(monDataBlockB->ribbonsGBA, param, MON_DATA_COOL_RIBBON);
        break;

    case MON_DATA_FATEFUL_ENCOUNTER:
        result = monDataBlockB->fatefulEncounter;
        break;

    case MON_DATA_GENDER:
        result = Pokemon_GetGenderOf(monDataBlockA->species, boxMon->personality);
        monDataBlockB->gender = result;
        boxMon->checksum = Pokemon_GetDataChecksum(&boxMon->dataBlocks, sizeof(PokemonDataBlock) * 4);
        break;

    case MON_DATA_FORM:
        result = monDataBlockB->form;
        break;

    case MON_DATA_UNUSED_113:
        result = monDataBlockB->unused1;
        break;

    case MON_DATA_UNUSED_114:
        result = monDataBlockB->unused2;
        break;

    case MON_DATA_NICKNAME:
        if (boxMon->checksumFailed) {
            // TODO confirm this should be SPECIES_BAD_EGG (lines up with checksum failure check but not throughly checked this call tree)
            MessageLoader_GetSpeciesName(SPECIES_BAD_EGG, 0, dest);
        } else {
            charcode_t *buf = dest;

            for (result = 0; result < MON_NAME_LEN; result++) {
                buf[result] = monDataBlockC->nickname[result];
            }

            buf[result] = CHAR_EOS;
        }
        break;
    case MON_DATA_NICKNAME_STRING_AND_FLAG:
        result = monDataBlockB->hasNickname;
        // fall-through
    case MON_DATA_NICKNAME_STRING:
        if (boxMon->checksumFailed) {
            String *string = MessageUtil_SpeciesName(SPECIES_BAD_EGG, HEAP_ID_SYSTEM);

            String_Copy(dest, string);
            String_Free(string);
        } else {
            String_CopyChars(dest, monDataBlockC->nickname);
        }
        break;

    case MON_DATA_UNUSED_121:
        result = monDataBlockC->unused;
        break;

    case MON_DATA_MET_GAME:
        result = monDataBlockC->originGame;
        break;

    case MON_DATA_SUPER_COOL_RIBBON:
    case MON_DATA_SUPER_COOL_RIBBON_GREAT:
    case MON_DATA_SUPER_COOL_RIBBON_ULTRA:
    case MON_DATA_SUPER_COOL_RIBBON_MASTER:
    case MON_DATA_SUPER_BEAUTY_RIBBON:
    case MON_DATA_SUPER_BEAUTY_RIBBON_GREAT:
    case MON_DATA_SUPER_BEAUTY_RIBBON_ULTRA:
    case MON_DATA_SUPER_BEAUTY_RIBBON_MASTER:
    case MON_DATA_SUPER_CUTE_RIBBON:
    case MON_DATA_SUPER_CUTE_RIBBON_GREAT:
    case MON_DATA_SUPER_CUTE_RIBBON_ULTRA:
    case MON_DATA_SUPER_CUTE_RIBBON_MASTER:
    case MON_DATA_SUPER_SMART_RIBBON:
    case MON_DATA_SUPER_SMART_RIBBON_GREAT:
    case MON_DATA_SUPER_SMART_RIBBON_ULTRA:
    case MON_DATA_SUPER_SMART_RIBBON_MASTER:
    case MON_DATA_SUPER_TOUGH_RIBBON:
    case MON_DATA_SUPER_TOUGH_RIBBON_GREAT:
    case MON_DATA_SUPER_TOUGH_RIBBON_ULTRA:
    case MON_DATA_SUPER_TOUGH_RIBBON_MASTER:
    case MON_DATA_UNUSED_RIBBON_143:
        result = GetRibbon(monDataBlockC->ribbonsDS2, param, MON_DATA_SUPER_COOL_RIBBON);
        break;

    case MON_DATA_OT_NAME:
        u16 *otName = dest;

        for (result = 0; result < TRAINER_NAME_LEN; result++) {
            otName[result] = monDataBlockD->otName[result];
        }

        otName[result] = CHAR_EOS;
        break;

    case MON_DATA_OT_NAME_STRING:
        String_CopyChars(dest, monDataBlockD->otName);
        break;

    case MON_DATA_EGG_YEAR:
        result = monDataBlockD->eggYear;
        break;

    case MON_DATA_EGG_MONTH:
        result = monDataBlockD->eggMonth;
        break;

    case MON_DATA_EGG_DAY:
        result = monDataBlockD->eggDay;
        break;

    case MON_DATA_MET_YEAR:
        result = monDataBlockD->metYear;
        break;

    case MON_DATA_MET_MONTH:
        result = monDataBlockD->metMonth;
        break;

    case MON_DATA_MET_DAY:
        result = monDataBlockD->metDay;
        break;

    case MON_DATA_EGG_LOCATION:
    case MON_DATA_EGG_LOCATION_PTHGSS:
        if (monDataBlockD->EggLocation_DP == FATEFUL_ENCOUNTER_LOCATION && monDataBlockB->EggLocation_PtHGSS) {
            result = monDataBlockB->EggLocation_PtHGSS;
        } else {
            result = monDataBlockD->EggLocation_DP;
        }
        break;

    case MON_DATA_MET_LOCATION:
    case MON_DATA_MET_LOCATION_PTHGSS:
        if (monDataBlockD->MetLocation_DP == FATEFUL_ENCOUNTER_LOCATION && monDataBlockB->MetLocation_PtHGSS) {
            result = monDataBlockB->MetLocation_PtHGSS;
        } else {
            result = monDataBlockD->MetLocation_DP;
        }
        break;

    case MON_DATA_POKERUS:
        result = monDataBlockD->pokerus;
        break;

    case MON_DATA_POKEBALL:
        result = monDataBlockD->pokeball;
        break;

    case MON_DATA_MET_LEVEL:
        result = monDataBlockD->metLevel;
        break;

    case MON_DATA_OT_GENDER:
        result = monDataBlockD->otGender;
        break;

    case MON_DATA_MET_TERRAIN:
        result = monDataBlockD->metTerrain;
        break;

    case MON_DATA_UNUSED_159:
        result = monDataBlockD->unused;
        break;

    case MON_DATA_COMBINED_IVS:
        result = (monDataBlockB->hpIV << 0)
            | (monDataBlockB->atkIV << 5)
            | (monDataBlockB->defIV << 10)
            | (monDataBlockB->speedIV << 15)
            | (monDataBlockB->spAtkIV << 20)
            | (monDataBlockB->spDefIV << 25);
        break;

    case MON_DATA_NO_PRINT_GENDER:
        if ((monDataBlockA->species == SPECIES_NIDORAN_F || monDataBlockA->species == SPECIES_NIDORAN_M)
            && monDataBlockB->hasNickname == FALSE) {
            result = FALSE;
        } else {
            result = TRUE;
        }
        break;

    case MON_DATA_TYPE_1:
    case MON_DATA_TYPE_2: {
        // HG SYNC (port of PokeHeartGold ENTRY CE item 2): Multitype's plate-based type now works on ANY species that has
        // Multitype through the ability donor, not just a real Arceus (the old test read the stored, un-donated ability).
        // Multitype in the mon's ability slot 1 overrides Type1, in slot 2 Type2 (a real Arceus is always slot 1).
        u32 effectiveAbility = ApplyAbilityDonor(monDataBlockA->ability, monDataBlockA->species, boxMon->personality, monDataBlockB);
        u32 abilitySlot = GetMonAbilitySlot(monDataBlockA->species, boxMon->personality);

        if (effectiveAbility == ABILITY_MULTITYPE && ((param == MON_DATA_TYPE_1 && abilitySlot == 0) || (param == MON_DATA_TYPE_2 && abilitySlot == 1))) {
            result = Pokemon_GetArceusTypeOf(Item_LoadParam(monDataBlockA->heldItem, ITEM_PARAM_HOLD_EFFECT, HEAP_ID_SYSTEM));
        } else {
            result = SpeciesData_GetFormValue(monDataBlockA->species, monDataBlockB->form, SPECIES_DATA_TYPE_1 + (param - MON_DATA_TYPE_1));
        }

        // COLOR MOD (port of PokeHeartGold ENTRY AD item 3): custom type override, see ApplyCustomType above.
        if (!monDataBlockB->isEgg) {
            result = ApplyCustomType(result, param == MON_DATA_TYPE_1 ? monDataBlockB->hpIV : monDataBlockB->speedIV);
        }
        break;
    }

    case MON_DATA_SPECIES_NAME:
        MessageLoader_GetSpeciesName(monDataBlockA->species, HEAP_ID_SYSTEM, dest);
        break;
    }

    return result;
}

void Pokemon_SetValue(Pokemon *mon, enum PokemonDataParam param, const void *value)
{
    u16 runHP;
    u32 runExperience;
    if (param == MON_DATA_HP) {
        runHP = RunGame_FilterHP(mon, *(const u16 *)value);
        value = &runHP;
    } else if (param == MON_DATA_EXPERIENCE && RunSession_Configured()
        && (RunSession_Config()->flags & RUN_LEVEL_CAPS)) {
        u32 current = Pokemon_GetValue(mon, MON_DATA_EXPERIENCE, NULL);
        u32 desired = *(const u32 *)value;
        runExperience = desired > current ? RunGame_Experience(&mon->box, current, desired - current) : desired;
        value = &runExperience;
    }
    if (mon->box.partyDecrypted == FALSE) {
        Pokemon_DecryptData(&mon->party, sizeof(PartyPokemon), mon->box.personality);
        Pokemon_DecryptData(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4, mon->box.checksum);
        u16 checksum = Pokemon_GetDataChecksum(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4);

        if (checksum != mon->box.checksum) {
            GF_ASSERT(checksum == mon->box.checksum);
            mon->box.checksumFailed = TRUE;
            Pokemon_EncryptData(mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4, mon->box.checksum);
            return;
        }
    }

    Pokemon_SetDataInternal(mon, param, value);

    if (mon->box.partyDecrypted == FALSE) {
        Pokemon_EncryptData(&mon->party, sizeof(PartyPokemon), mon->box.personality);
        mon->box.checksum = Pokemon_GetDataChecksum(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4);
        Pokemon_EncryptData(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4, mon->box.checksum);
    }
    if (param == MON_DATA_SPECIES || param == MON_DATA_NICKNAME || param == MON_DATA_NICKNAME_AND_FLAG
        || param == MON_DATA_NICKNAME_STRING || param == MON_DATA_NICKNAME_STRING_AND_FLAG || param == MON_DATA_SPECIES_NAME || param == MON_DATA_IS_EGG) RunGame_Sync(&mon->box);
}

static void Pokemon_SetDataInternal(Pokemon *mon, enum PokemonDataParam param, const void *value)
{
    u32 *u32Value = value;
    u16 *u16Value = value;
    u8 *u8Value = value;

    switch (param) {
    case MON_DATA_STATUS:
        mon->party.status = *u32Value;
        break;

    case MON_DATA_LEVEL:
        mon->party.level = *u8Value;
        break;

    case MON_DATA_BALL_CAPSULE_ID:
        mon->party.ballCapsuleID = *u8Value;
        break;

    case MON_DATA_HP:
        mon->party.hp = *u16Value;
        break;

    case MON_DATA_MAX_HP:
        mon->party.maxHP = *u16Value;
        break;

    case MON_DATA_ATK:
        mon->party.attack = *u16Value;
        break;

    case MON_DATA_DEF:
        mon->party.defense = *u16Value;
        break;

    case MON_DATA_SPEED:
        mon->party.speed = *u16Value;
        break;

    case MON_DATA_SP_ATK:
        mon->party.spAtk = *u16Value;
        break;

    case MON_DATA_SP_DEF:
        mon->party.spDef = *u16Value;
        break;

    case MON_DATA_MAIL:
        Mail_Copy(value, &mon->party.mail);
        break;

    case MON_DATA_BALL_CAPSULE:
        BallCapsule_Copy(value, &mon->party.ballCapsule);
        break;

    default:
        BoxPokemon_SetDataInternal(&mon->box, param, value);
        break;
    }
}

void BoxPokemon_SetValue(BoxPokemon *boxMon, enum PokemonDataParam param, const void *value)
{
    u32 runExperience;
    if (param == MON_DATA_EXPERIENCE && RunSession_Configured()
        && (RunSession_Config()->flags & RUN_LEVEL_CAPS)) {
        u32 current = BoxPokemon_GetValue(boxMon, MON_DATA_EXPERIENCE, NULL);
        u32 desired = *(const u32 *)value;
        runExperience = desired > current ? RunGame_Experience(boxMon, current, desired - current) : desired;
        value = &runExperience;
    }
    if (boxMon->boxDecrypted == FALSE) {
        Pokemon_DecryptData(boxMon->dataBlocks, sizeof(PokemonDataBlock) * 4, boxMon->checksum);
        u16 checksum = Pokemon_GetDataChecksum(boxMon->dataBlocks, sizeof(PokemonDataBlock) * 4);

        if (checksum != boxMon->checksum) {
            GF_ASSERT(checksum == boxMon->checksum);
            boxMon->checksumFailed = TRUE;
            Pokemon_EncryptData(boxMon->dataBlocks, sizeof(PokemonDataBlock) * 4, boxMon->checksum);
            return;
        }
    }

    BoxPokemon_SetDataInternal(boxMon, param, value);

    if (boxMon->boxDecrypted == FALSE) {
        boxMon->checksum = Pokemon_GetDataChecksum(boxMon->dataBlocks, sizeof(PokemonDataBlock) * 4);
        Pokemon_EncryptData(boxMon->dataBlocks, sizeof(PokemonDataBlock) * 4, boxMon->checksum);
    }
    if (param == MON_DATA_SPECIES || param == MON_DATA_NICKNAME || param == MON_DATA_NICKNAME_AND_FLAG
        || param == MON_DATA_NICKNAME_STRING || param == MON_DATA_NICKNAME_STRING_AND_FLAG || param == MON_DATA_SPECIES_NAME || param == MON_DATA_IS_EGG) RunGame_Sync(boxMon);
}

static void BoxPokemon_SetDataInternal(BoxPokemon *boxMon, enum PokemonDataParam param, const void *value)
{
    // TODO can this be restructured better?
    u32 *u32Value = value;
    u16 *u16Value = value;
    u8 *u8Value = value;

    PokemonDataBlockA *monDataBlockA = BoxPokemon_GetDataBlock(boxMon, boxMon->personality, DATA_BLOCK_A);
    PokemonDataBlockB *monDataBlockB = BoxPokemon_GetDataBlock(boxMon, boxMon->personality, DATA_BLOCK_B);
    PokemonDataBlockC *monDataBlockC = BoxPokemon_GetDataBlock(boxMon, boxMon->personality, DATA_BLOCK_C);
    PokemonDataBlockD *monDataBlockD = BoxPokemon_GetDataBlock(boxMon, boxMon->personality, DATA_BLOCK_D);

    switch (param) {
    case MON_DATA_PERSONALITY:
        boxMon->personality = *u32Value;
        break;

    case MON_DATA_IS_PARTY_DECRYPTED:
        GF_ASSERT(FALSE);
        boxMon->partyDecrypted = *u8Value;
        break;

    case MON_DATA_IS_BOX_DECRYPTED:
        GF_ASSERT(FALSE);
        boxMon->boxDecrypted = *u8Value;
        break;

    case MON_DATA_CHECKSUM_FAILED:
        boxMon->checksumFailed = *u8Value;
        break;

    case MON_DATA_CHECKSUM:
        boxMon->checksum = *u16Value;
        break;

    case MON_DATA_SPECIES:
        monDataBlockA->species = *u16Value;
        break;

    case MON_DATA_HELD_ITEM:
        monDataBlockA->heldItem = *u16Value;
        break;

    case MON_DATA_OT_ID:
        monDataBlockA->otID = *u32Value;
        break;

    case MON_DATA_EXPERIENCE:
        monDataBlockA->exp = *u32Value;
        break;

    case MON_DATA_FRIENDSHIP:
        monDataBlockA->friendship = *u8Value;
        break;

    case MON_DATA_ABILITY:
        monDataBlockA->ability = *u8Value;
        break;

    case MON_DATA_MARKINGS:
        monDataBlockA->markings = *u8Value;
        break;

    case MON_DATA_LANGUAGE:
        monDataBlockA->originLanguage = *u8Value;
        break;

    case MON_DATA_HP_EV:
        monDataBlockA->hpEV = *u8Value;
        break;

    case MON_DATA_ATK_EV:
        monDataBlockA->atkEV = *u8Value;
        break;

    case MON_DATA_DEF_EV:
        monDataBlockA->defEV = *u8Value;
        break;

    case MON_DATA_SPEED_EV:
        monDataBlockA->speedEV = *u8Value;
        break;

    case MON_DATA_SPATK_EV:
        monDataBlockA->spAtkEV = *u8Value;
        break;

    case MON_DATA_SPDEF_EV:
        monDataBlockA->spDefEV = *u8Value;
        break;

    case MON_DATA_COOL:
        monDataBlockA->cool = *u8Value;
        break;

    case MON_DATA_BEAUTY:
        monDataBlockA->beauty = *u8Value;
        break;

    case MON_DATA_CUTE:
        monDataBlockA->cute = *u8Value;
        break;

    case MON_DATA_SMART:
        monDataBlockA->smart = *u8Value;
        break;

    case MON_DATA_TOUGH:
        monDataBlockA->tough = *u8Value;
        break;

    case MON_DATA_SHEEN:
        monDataBlockA->sheen = *u8Value;
        break;

    case MON_DATA_SINNOH_CHAMP_RIBBON:
    case MON_DATA_ABILITY_RIBBON:
    case MON_DATA_GREAT_ABILITY_RIBBON:
    case MON_DATA_DOUBLE_ABILITY_RIBBON:
    case MON_DATA_MULTI_ABILITY_RIBBON:
    case MON_DATA_PAIR_ABILITY_RIBBON:
    case MON_DATA_WORLD_ABILITY_RIBBON:
    case MON_DATA_ALERT_RIBBON:
    case MON_DATA_SHOCK_RIBBON:
    case MON_DATA_DOWNCAST_RIBBON:
    case MON_DATA_CARELESS_RIBBON:
    case MON_DATA_RELAX_RIBBON:
    case MON_DATA_SNOOZE_RIBBON:
    case MON_DATA_SMILE_RIBBON:
    case MON_DATA_GORGEOUS_RIBBON:
    case MON_DATA_ROYAL_RIBBON:
    case MON_DATA_GORGEOUS_ROYAL_RIBBON:
    case MON_DATA_FOOTPRINT_RIBBON:
    case MON_DATA_RECORD_RIBBON:
    case MON_DATA_HISTORY_RIBBON:
    case MON_DATA_LEGEND_RIBBON:
    case MON_DATA_RED_RIBBON:
    case MON_DATA_GREEN_RIBBON:
    case MON_DATA_BLUE_RIBBON:
    case MON_DATA_FESTIVAL_RIBBON:
    case MON_DATA_CARNIVAL_RIBBON:
    case MON_DATA_CLASSIC_RIBBON:
    case MON_DATA_PREMIER_RIBBON:
    case MON_DATA_UNUSED_RIBBON_53: {
        u64 bit = 1 << (param - MON_DATA_SINNOH_CHAMP_RIBBON);

        if (*u8Value) {
            monDataBlockA->ribbonsDS1 |= bit;
        } else {
            monDataBlockA->ribbonsDS1 &= (bit ^ 0xFFFFFFFF);
        }

        break;
    }

    case MON_DATA_MOVE1:
    case MON_DATA_MOVE2:
    case MON_DATA_MOVE3:
    case MON_DATA_MOVE4:
        monDataBlockB->moves[param - MON_DATA_MOVE1] = *u16Value;
        break;

    case MON_DATA_MOVE1_PP:
    case MON_DATA_MOVE2_PP:
    case MON_DATA_MOVE3_PP:
    case MON_DATA_MOVE4_PP:
        monDataBlockB->moveCurrentPPs[param - MON_DATA_MOVE1_PP] = *u8Value;
        break;

    case MON_DATA_MOVE1_PP_UPS:
    case MON_DATA_MOVE2_PP_UPS:
    case MON_DATA_MOVE3_PP_UPS:
    case MON_DATA_MOVE4_PP_UPS:
        monDataBlockB->movePPUps[param - MON_DATA_MOVE1_PP_UPS] = *u8Value;
        break;

    case MON_DATA_MOVE1_MAX_PP:
    case MON_DATA_MOVE2_MAX_PP:
    case MON_DATA_MOVE3_MAX_PP:
    case MON_DATA_MOVE4_MAX_PP:
        break;

    case MON_DATA_HP_IV:
        monDataBlockB->hpIV = *u8Value;
        break;

    case MON_DATA_ATK_IV:
        monDataBlockB->atkIV = *u8Value;
        break;

    case MON_DATA_DEF_IV:
        monDataBlockB->defIV = *u8Value;
        break;

    case MON_DATA_SPEED_IV:
        monDataBlockB->speedIV = *u8Value;
        break;

    case MON_DATA_SPATK_IV:
        monDataBlockB->spAtkIV = *u8Value;
        break;

    case MON_DATA_SPDEF_IV:
        monDataBlockB->spDefIV = *u8Value;
        break;

    case MON_DATA_IS_EGG:
        monDataBlockB->isEgg = *u8Value;
        break;

    case MON_DATA_HAS_NICKNAME:
        monDataBlockB->hasNickname = *u8Value;
        break;

    case MON_DATA_COOL_RIBBON:
    case MON_DATA_COOL_RIBBON_SUPER:
    case MON_DATA_COOL_RIBBON_HYPER:
    case MON_DATA_COOL_RIBBON_MASTER:
    case MON_DATA_BEAUTY_RIBBON:
    case MON_DATA_BEAUTY_RIBBON_SUPER:
    case MON_DATA_BEAUTY_RIBBON_HYPER:
    case MON_DATA_BEAUTY_RIBBON_MASTER:
    case MON_DATA_CUTE_RIBBON:
    case MON_DATA_CUTE_RIBBON_SUPER:
    case MON_DATA_CUTE_RIBBON_HYPER:
    case MON_DATA_CUTE_RIBBON_MASTER:
    case MON_DATA_SMART_RIBBON:
    case MON_DATA_SMART_RIBBON_SUPER:
    case MON_DATA_SMART_RIBBON_HYPER:
    case MON_DATA_SMART_RIBBON_MASTER:
    case MON_DATA_TOUGH_RIBBON:
    case MON_DATA_TOUGH_RIBBON_SUPER:
    case MON_DATA_TOUGH_RIBBON_HYPER:
    case MON_DATA_TOUGH_RIBBON_MASTER:
    case MON_DATA_CHAMPION_RIBBON:
    case MON_DATA_WINNING_RIBBON:
    case MON_DATA_VICTORY_RIBBON:
    case MON_DATA_ARTIST_RIBBON:
    case MON_DATA_EFFORT_RIBBON:
    case MON_DATA_MARINE_RIBBON:
    case MON_DATA_LAND_RIBBON:
    case MON_DATA_SKY_RIBBON:
    case MON_DATA_COUNTRY_RIBBON:
    case MON_DATA_NATIONAL_RIBBON:
    case MON_DATA_EARTH_RIBBON:
    case MON_DATA_WORLD_RIBBON: {
        u64 bit = 1 << (param - MON_DATA_COOL_RIBBON);

        if (*u8Value) {
            monDataBlockB->ribbonsGBA |= bit;
        } else {
            monDataBlockB->ribbonsGBA &= (bit ^ 0xFFFFFFFF);
        }

        break;
    }

    case MON_DATA_FATEFUL_ENCOUNTER:
        monDataBlockB->fatefulEncounter = *u8Value;
        break;

    case MON_DATA_GENDER:
        monDataBlockB->gender = Pokemon_GetGenderOf(monDataBlockA->species, boxMon->personality);
        break;

    case MON_DATA_FORM:
        monDataBlockB->form = *u8Value;
        break;

    case MON_DATA_UNUSED_113:
        monDataBlockB->unused1 = *u8Value;
        break;

    case MON_DATA_UNUSED_114:
        monDataBlockB->unused2 = *u16Value;
        break;

    case MON_DATA_NICKNAME_AND_FLAG: {
        charcode_t baseName[MON_NAME_LEN + 1];

        MessageLoader_GetSpeciesName(monDataBlockA->species, HEAP_ID_SYSTEM, baseName);
        monDataBlockB->hasNickname = CharCode_Compare(baseName, u16Value);
    }
        // fall-through
    case MON_DATA_NICKNAME:
        for (int i = 0; i < NELEMS(monDataBlockC->nickname); i++) {
            monDataBlockC->nickname[i] = u16Value[i];
        }
        break;

    case MON_DATA_NICKNAME_STRING_AND_FLAG: {
        charcode_t baseName[MON_NAME_LEN + 1];
        charcode_t nickname[MON_NAME_LEN + 1];

        MessageLoader_GetSpeciesName(monDataBlockA->species, HEAP_ID_SYSTEM, baseName);
        String_ToChars(value, nickname, NELEMS(nickname));

        monDataBlockB->hasNickname = CharCode_Compare(baseName, nickname);
    }
        // fall-through
    case MON_DATA_NICKNAME_STRING:
        String_ToChars(value, monDataBlockC->nickname, NELEMS(monDataBlockC->nickname));
        break;

    case MON_DATA_UNUSED_121:
        monDataBlockC->unused = *u8Value;
        break;

    case MON_DATA_MET_GAME:
        monDataBlockC->originGame = *u8Value;
        break;

    case MON_DATA_SUPER_COOL_RIBBON:
    case MON_DATA_SUPER_COOL_RIBBON_GREAT:
    case MON_DATA_SUPER_COOL_RIBBON_ULTRA:
    case MON_DATA_SUPER_COOL_RIBBON_MASTER:
    case MON_DATA_SUPER_BEAUTY_RIBBON:
    case MON_DATA_SUPER_BEAUTY_RIBBON_GREAT:
    case MON_DATA_SUPER_BEAUTY_RIBBON_ULTRA:
    case MON_DATA_SUPER_BEAUTY_RIBBON_MASTER:
    case MON_DATA_SUPER_CUTE_RIBBON:
    case MON_DATA_SUPER_CUTE_RIBBON_GREAT:
    case MON_DATA_SUPER_CUTE_RIBBON_ULTRA:
    case MON_DATA_SUPER_CUTE_RIBBON_MASTER:
    case MON_DATA_SUPER_SMART_RIBBON:
    case MON_DATA_SUPER_SMART_RIBBON_GREAT:
    case MON_DATA_SUPER_SMART_RIBBON_ULTRA:
    case MON_DATA_SUPER_SMART_RIBBON_MASTER:
    case MON_DATA_SUPER_TOUGH_RIBBON:
    case MON_DATA_SUPER_TOUGH_RIBBON_GREAT:
    case MON_DATA_SUPER_TOUGH_RIBBON_ULTRA:
    case MON_DATA_SUPER_TOUGH_RIBBON_MASTER:
    case MON_DATA_UNUSED_RIBBON_143: {
        u64 bit = 1 << (param - MON_DATA_SUPER_COOL_RIBBON);

        if (*u8Value) {
            monDataBlockC->ribbonsDS2 |= bit;
        } else {
            monDataBlockC->ribbonsDS2 &= (bit ^ 0xFFFFFFFFFFFFFFFF);
        }

        break;
    }

    case MON_DATA_OT_NAME:
        for (int i = 0; i < NELEMS(monDataBlockD->otName); i++) {
            monDataBlockD->otName[i] = u16Value[i];
        }
        break;

    case MON_DATA_OT_NAME_STRING:
        String_ToChars(value, monDataBlockD->otName, NELEMS(monDataBlockD->otName));
        break;

    case MON_DATA_EGG_YEAR:
        monDataBlockD->eggYear = *u8Value;
        break;

    case MON_DATA_EGG_MONTH:
        monDataBlockD->eggMonth = *u8Value;
        break;

    case MON_DATA_EGG_DAY:
        monDataBlockD->eggDay = *u8Value;
        break;

    case MON_DATA_MET_YEAR:
        monDataBlockD->metYear = *u8Value;
        break;

    case MON_DATA_MET_MONTH:
        monDataBlockD->metMonth = *u8Value;
        break;

    case MON_DATA_MET_DAY:
        monDataBlockD->metDay = *u8Value;
        break;

    case MON_DATA_EGG_LOCATION:
    case MON_DATA_EGG_LOCATION_PTHGSS:
        if (*u16Value == 0 || sub_0201708C(*u16Value) == TRUE) {
            monDataBlockD->EggLocation_DP = *u16Value;
            monDataBlockB->EggLocation_PtHGSS = *u16Value;
        } else {
            monDataBlockD->EggLocation_DP = FATEFUL_ENCOUNTER_LOCATION;
            monDataBlockB->EggLocation_PtHGSS = *u16Value;
        }
        break;

    case MON_DATA_MET_LOCATION:
    case MON_DATA_MET_LOCATION_PTHGSS:
        if (*u16Value == 0 || sub_0201708C(*u16Value) == TRUE) {
            monDataBlockD->MetLocation_DP = *u16Value;
            monDataBlockB->MetLocation_PtHGSS = *u16Value;
        } else {
            monDataBlockD->MetLocation_DP = FATEFUL_ENCOUNTER_LOCATION;
            monDataBlockB->MetLocation_PtHGSS = *u16Value;
        }
        break;

    case MON_DATA_POKERUS:
        monDataBlockD->pokerus = *u8Value;
        break;

    case MON_DATA_POKEBALL:
        monDataBlockD->pokeball = *u8Value;
        break;

    case MON_DATA_MET_LEVEL:
        monDataBlockD->metLevel = *u8Value;
        break;

    case MON_DATA_OT_GENDER:
        monDataBlockD->otGender = *u8Value;
        break;

    case MON_DATA_MET_TERRAIN:
        monDataBlockD->metTerrain = *u8Value;
        break;

    case MON_DATA_UNUSED_159:
        monDataBlockD->unused = *u16Value;
        break;

    case MON_DATA_COMBINED_IVS:
        monDataBlockB->hpIV = (*u32Value >> 0) & 0x1F;
        monDataBlockB->atkIV = (*u32Value >> 5) & 0x1F;
        monDataBlockB->defIV = (*u32Value >> 10) & 0x1F;
        monDataBlockB->speedIV = (*u32Value >> 15) & 0x1F;
        monDataBlockB->spAtkIV = (*u32Value >> 20) & 0x1F;
        monDataBlockB->spDefIV = (*u32Value >> 25) & 0x1F;
        break;

    case MON_DATA_NO_PRINT_GENDER:
    case MON_DATA_TYPE_1:
    case MON_DATA_TYPE_2:
        break;

    case MON_DATA_SPECIES_NAME: {
        String *string = MessageUtil_SpeciesName(monDataBlockA->species, HEAP_ID_SYSTEM);

        String_ToChars(string, monDataBlockC->nickname, NELEMS(monDataBlockC->nickname));
        String_Free(string);

        break;
    }
    }
}

void Pokemon_IncreaseValue(Pokemon *mon, enum PokemonDataParam param, int value)
{
    if (param == MON_DATA_HP && RunGame_Exhausted(&mon->box)) return;
    if (param == MON_DATA_EXPERIENCE && value > 0 && RunSession_Configured()
        && (RunSession_Config()->flags & RUN_LEVEL_CAPS)) {
        u32 experience = RunGame_Experience(&mon->box, Pokemon_GetValue(mon, MON_DATA_EXPERIENCE, NULL), value);
        Pokemon_SetValue(mon, MON_DATA_EXPERIENCE, &experience);
        return;
    }
    if (mon->box.partyDecrypted == FALSE) {
        Pokemon_DecryptData(&mon->party, sizeof(PartyPokemon), mon->box.personality);
        Pokemon_DecryptData(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4, mon->box.checksum);

        u16 checksum = Pokemon_GetDataChecksum(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4);

        if (checksum != mon->box.checksum) {
            GF_ASSERT(checksum == mon->box.checksum);
            Pokemon_EncryptData(mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4, mon->box.checksum);
            return;
        }
    }

    Pokemon_IncreaseDataInternal(mon, param, value);

    if (mon->box.partyDecrypted == FALSE) {
        Pokemon_EncryptData(&mon->party, sizeof(PartyPokemon), mon->box.personality);
        mon->box.checksum = Pokemon_GetDataChecksum(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4);
        Pokemon_EncryptData(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4, mon->box.checksum);
    }
}

static void Pokemon_IncreaseDataInternal(Pokemon *mon, enum PokemonDataParam param, int value)
{
    switch (param) {
    case MON_DATA_HP:
        if (mon->party.hp + value > mon->party.maxHP) {
            mon->party.hp = mon->party.maxHP;
        } else {
            mon->party.hp += value;
        }
        break;
    case MON_DATA_STATUS:
    case MON_DATA_LEVEL:
    case MON_DATA_BALL_CAPSULE_ID:
    case MON_DATA_MAX_HP:
    case MON_DATA_ATK:
    case MON_DATA_DEF:
    case MON_DATA_SPEED:
    case MON_DATA_SP_ATK:
    case MON_DATA_SP_DEF:
    case MON_DATA_MAIL:
        GF_ASSERT(FALSE);
        break;
    default:
        BoxPokemon_IncreaseDataInternal(&mon->box, param, value);
        break;
    }
}

static void BoxPokemon_IncreaseDataInternal(BoxPokemon *boxMon, enum PokemonDataParam param, int value)
{
    PokemonDataBlockA *monDataBlockA = BoxPokemon_GetDataBlock(boxMon, boxMon->personality, DATA_BLOCK_A);
    PokemonDataBlockB *monDataBlockB = BoxPokemon_GetDataBlock(boxMon, boxMon->personality, DATA_BLOCK_B);
    PokemonDataBlockC *monDataBlockC = BoxPokemon_GetDataBlock(boxMon, boxMon->personality, DATA_BLOCK_C);
    PokemonDataBlockD *monDataBlockD = BoxPokemon_GetDataBlock(boxMon, boxMon->personality, DATA_BLOCK_D);

    switch (param) {
    case MON_DATA_EXPERIENCE:
        if (monDataBlockA->exp + value > Pokemon_GetSpeciesBaseExpAt(monDataBlockA->species, MAX_POKEMON_LEVEL)) {
            monDataBlockA->exp = Pokemon_GetSpeciesBaseExpAt(monDataBlockA->species, MAX_POKEMON_LEVEL);
        } else {
            monDataBlockA->exp += value;
        }
        break;
    case MON_DATA_FRIENDSHIP:
        int newValue;

        newValue = monDataBlockA->friendship;

        if (newValue + value > MAX_FRIENDSHIP_VALUE) {
            newValue = MAX_FRIENDSHIP_VALUE;
        }

        if (newValue + value < 0) {
            newValue = 0;
        } else {
            newValue += value;
        }

        monDataBlockA->friendship = newValue;
        break;
    case MON_DATA_HP_EV:
        monDataBlockA->hpEV += value;
        break;
    case MON_DATA_ATK_EV:
        monDataBlockA->atkEV += value;
        break;
    case MON_DATA_DEF_EV:
        monDataBlockA->defEV += value;
        break;
    case MON_DATA_SPEED_EV:
        monDataBlockA->speedEV += value;
        break;
    case MON_DATA_SPATK_EV:
        monDataBlockA->spAtkEV += value;
        break;
    case MON_DATA_SPDEF_EV:
        monDataBlockA->spDefEV += value;
        break;
    case MON_DATA_COOL:
        if (monDataBlockA->cool + value > MAX_CONTEST_STAT) {
            monDataBlockA->cool = MAX_CONTEST_STAT;
        } else {
            monDataBlockA->cool += value;
        }
        break;
    case MON_DATA_BEAUTY:
        if (monDataBlockA->beauty + value > MAX_CONTEST_STAT) {
            monDataBlockA->beauty = MAX_CONTEST_STAT;
        } else {
            monDataBlockA->beauty += value;
        }
        break;
    case MON_DATA_CUTE:
        if (monDataBlockA->cute + value > MAX_CONTEST_STAT) {
            monDataBlockA->cute = MAX_CONTEST_STAT;
        } else {
            monDataBlockA->cute += value;
        }
        break;
    case MON_DATA_SMART:
        if (monDataBlockA->smart + value > MAX_CONTEST_STAT) {
            monDataBlockA->smart = MAX_CONTEST_STAT;
        } else {
            monDataBlockA->smart += value;
        }
        break;
    case MON_DATA_TOUGH:
        if (monDataBlockA->tough + value > MAX_CONTEST_STAT) {
            monDataBlockA->tough = MAX_CONTEST_STAT;
        } else {
            monDataBlockA->tough += value;
        }
        break;
    case MON_DATA_SHEEN:
        if (monDataBlockA->sheen + value > MAX_POKEMON_SHEEN) {
            monDataBlockA->sheen = MAX_POKEMON_SHEEN;
        } else {
            monDataBlockA->sheen += value;
        }
        break;
    case MON_DATA_MOVE1_PP:
    case MON_DATA_MOVE2_PP:
    case MON_DATA_MOVE3_PP:
    case MON_DATA_MOVE4_PP:
        if (monDataBlockB->moveCurrentPPs[param - MON_DATA_MOVE1_PP] + value > MoveTable_CalcMaxPP(monDataBlockB->moves[param - MON_DATA_MOVE1_PP], monDataBlockB->movePPUps[param - MON_DATA_MOVE1_PP])) {
            monDataBlockB->moveCurrentPPs[param - MON_DATA_MOVE1_PP] = MoveTable_CalcMaxPP(monDataBlockB->moves[param - MON_DATA_MOVE1_PP], monDataBlockB->movePPUps[param - MON_DATA_MOVE1_PP]);
        } else {
            monDataBlockB->moveCurrentPPs[param - MON_DATA_MOVE1_PP] += value;
        }
        break;
    case MON_DATA_MOVE1_PP_UPS:
    case MON_DATA_MOVE2_PP_UPS:
    case MON_DATA_MOVE3_PP_UPS:
    case MON_DATA_MOVE4_PP_UPS:
        if (monDataBlockB->movePPUps[param - MON_DATA_MOVE1_PP_UPS] + value > MAX_PP_UP_BONUSES) {
            monDataBlockB->movePPUps[param - MON_DATA_MOVE1_PP_UPS] = MAX_PP_UP_BONUSES;
        } else {
            monDataBlockB->movePPUps[param - MON_DATA_MOVE1_PP_UPS] += value;
        }
        break;
    case MON_DATA_MOVE1_MAX_PP:
    case MON_DATA_MOVE2_MAX_PP:
    case MON_DATA_MOVE3_MAX_PP:
    case MON_DATA_MOVE4_MAX_PP:
        break;
    case MON_DATA_HP_IV:
        if (monDataBlockB->hpIV + value > MAX_IVS_SINGLE_STAT) {
            monDataBlockB->hpIV = MAX_IVS_SINGLE_STAT;
        } else {
            monDataBlockB->hpIV += value;
        }
        break;
    case MON_DATA_ATK_IV:
        if (monDataBlockB->atkIV + value > MAX_IVS_SINGLE_STAT) {
            monDataBlockB->atkIV = MAX_IVS_SINGLE_STAT;
        } else {
            monDataBlockB->atkIV += value;
        }
        break;
    case MON_DATA_DEF_IV:
        if (monDataBlockB->defIV + value > MAX_IVS_SINGLE_STAT) {
            monDataBlockB->defIV = MAX_IVS_SINGLE_STAT;
        } else {
            monDataBlockB->defIV += value;
        }
        break;
    case MON_DATA_SPEED_IV:
        if (monDataBlockB->speedIV + value > MAX_IVS_SINGLE_STAT) {
            monDataBlockB->speedIV = MAX_IVS_SINGLE_STAT;
        } else {
            monDataBlockB->speedIV += value;
        }
        break;
    case MON_DATA_SPATK_IV:
        if (monDataBlockB->spAtkIV + value > MAX_IVS_SINGLE_STAT) {
            monDataBlockB->spAtkIV = MAX_IVS_SINGLE_STAT;
        } else {
            monDataBlockB->spAtkIV += value;
        }
        break;
    case MON_DATA_SPDEF_IV:
        if (monDataBlockB->spDefIV + value > MAX_IVS_SINGLE_STAT) {
            monDataBlockB->spDefIV = MAX_IVS_SINGLE_STAT;
        } else {
            monDataBlockB->spDefIV += value;
        }
        break;
    case MON_DATA_PERSONALITY:
    case MON_DATA_IS_PARTY_DECRYPTED:
    case MON_DATA_IS_BOX_DECRYPTED:
    case MON_DATA_CHECKSUM:
    case MON_DATA_SPECIES:
    case MON_DATA_HELD_ITEM:
    case MON_DATA_OT_ID:
    case MON_DATA_ABILITY:
    case MON_DATA_MARKINGS:
    case MON_DATA_LANGUAGE:
    case MON_DATA_SINNOH_CHAMP_RIBBON:
    case MON_DATA_ABILITY_RIBBON:
    case MON_DATA_GREAT_ABILITY_RIBBON:
    case MON_DATA_DOUBLE_ABILITY_RIBBON:
    case MON_DATA_MULTI_ABILITY_RIBBON:
    case MON_DATA_PAIR_ABILITY_RIBBON:
    case MON_DATA_WORLD_ABILITY_RIBBON:
    case MON_DATA_ALERT_RIBBON:
    case MON_DATA_SHOCK_RIBBON:
    case MON_DATA_DOWNCAST_RIBBON:
    case MON_DATA_CARELESS_RIBBON:
    case MON_DATA_RELAX_RIBBON:
    case MON_DATA_SNOOZE_RIBBON:
    case MON_DATA_SMILE_RIBBON:
    case MON_DATA_GORGEOUS_RIBBON:
    case MON_DATA_ROYAL_RIBBON:
    case MON_DATA_GORGEOUS_ROYAL_RIBBON:
    case MON_DATA_FOOTPRINT_RIBBON:
    case MON_DATA_RECORD_RIBBON:
    case MON_DATA_HISTORY_RIBBON:
    case MON_DATA_LEGEND_RIBBON:
    case MON_DATA_RED_RIBBON:
    case MON_DATA_GREEN_RIBBON:
    case MON_DATA_BLUE_RIBBON:
    case MON_DATA_FESTIVAL_RIBBON:
    case MON_DATA_CARNIVAL_RIBBON:
    case MON_DATA_CLASSIC_RIBBON:
    case MON_DATA_PREMIER_RIBBON:
    case MON_DATA_UNUSED_RIBBON_53:
    case MON_DATA_MOVE1:
    case MON_DATA_MOVE2:
    case MON_DATA_MOVE3:
    case MON_DATA_MOVE4:
    case MON_DATA_IS_EGG:
    case MON_DATA_HAS_NICKNAME:
    case MON_DATA_COOL_RIBBON:
    case MON_DATA_COOL_RIBBON_SUPER:
    case MON_DATA_COOL_RIBBON_HYPER:
    case MON_DATA_COOL_RIBBON_MASTER:
    case MON_DATA_BEAUTY_RIBBON:
    case MON_DATA_BEAUTY_RIBBON_SUPER:
    case MON_DATA_BEAUTY_RIBBON_HYPER:
    case MON_DATA_BEAUTY_RIBBON_MASTER:
    case MON_DATA_CUTE_RIBBON:
    case MON_DATA_CUTE_RIBBON_SUPER:
    case MON_DATA_CUTE_RIBBON_HYPER:
    case MON_DATA_CUTE_RIBBON_MASTER:
    case MON_DATA_SMART_RIBBON:
    case MON_DATA_SMART_RIBBON_SUPER:
    case MON_DATA_SMART_RIBBON_HYPER:
    case MON_DATA_SMART_RIBBON_MASTER:
    case MON_DATA_TOUGH_RIBBON:
    case MON_DATA_TOUGH_RIBBON_SUPER:
    case MON_DATA_TOUGH_RIBBON_HYPER:
    case MON_DATA_TOUGH_RIBBON_MASTER:
    case MON_DATA_CHAMPION_RIBBON:
    case MON_DATA_WINNING_RIBBON:
    case MON_DATA_VICTORY_RIBBON:
    case MON_DATA_ARTIST_RIBBON:
    case MON_DATA_EFFORT_RIBBON:
    case MON_DATA_MARINE_RIBBON:
    case MON_DATA_LAND_RIBBON:
    case MON_DATA_SKY_RIBBON:
    case MON_DATA_COUNTRY_RIBBON:
    case MON_DATA_NATIONAL_RIBBON:
    case MON_DATA_EARTH_RIBBON:
    case MON_DATA_WORLD_RIBBON:
    case MON_DATA_FATEFUL_ENCOUNTER:
    case MON_DATA_GENDER:
    case MON_DATA_FORM:
    case MON_DATA_UNUSED_113:
    case MON_DATA_UNUSED_114:
    case MON_DATA_NICKNAME:
    case MON_DATA_NICKNAME_STRING:
    case MON_DATA_NICKNAME_STRING_AND_FLAG:
    case MON_DATA_UNUSED_121:
    case MON_DATA_MET_GAME:
    case MON_DATA_SUPER_COOL_RIBBON:
    case MON_DATA_SUPER_COOL_RIBBON_GREAT:
    case MON_DATA_SUPER_COOL_RIBBON_ULTRA:
    case MON_DATA_SUPER_COOL_RIBBON_MASTER:
    case MON_DATA_SUPER_BEAUTY_RIBBON:
    case MON_DATA_SUPER_BEAUTY_RIBBON_GREAT:
    case MON_DATA_SUPER_BEAUTY_RIBBON_ULTRA:
    case MON_DATA_SUPER_BEAUTY_RIBBON_MASTER:
    case MON_DATA_SUPER_CUTE_RIBBON:
    case MON_DATA_SUPER_CUTE_RIBBON_GREAT:
    case MON_DATA_SUPER_CUTE_RIBBON_ULTRA:
    case MON_DATA_SUPER_CUTE_RIBBON_MASTER:
    case MON_DATA_SUPER_SMART_RIBBON:
    case MON_DATA_SUPER_SMART_RIBBON_GREAT:
    case MON_DATA_SUPER_SMART_RIBBON_ULTRA:
    case MON_DATA_SUPER_SMART_RIBBON_MASTER:
    case MON_DATA_SUPER_TOUGH_RIBBON:
    case MON_DATA_SUPER_TOUGH_RIBBON_GREAT:
    case MON_DATA_SUPER_TOUGH_RIBBON_ULTRA:
    case MON_DATA_SUPER_TOUGH_RIBBON_MASTER:
    case MON_DATA_UNUSED_RIBBON_143:
    case MON_DATA_OT_NAME:
    case MON_DATA_OT_NAME_STRING:
    case MON_DATA_EGG_YEAR:
    case MON_DATA_EGG_MONTH:
    case MON_DATA_EGG_DAY:
    case MON_DATA_MET_YEAR:
    case MON_DATA_MET_MONTH:
    case MON_DATA_MET_DAY:
    case MON_DATA_EGG_LOCATION:
    case MON_DATA_MET_LOCATION:
    case MON_DATA_POKERUS:
    case MON_DATA_POKEBALL:
    case MON_DATA_MET_LEVEL:
    case MON_DATA_OT_GENDER:
    case MON_DATA_MET_TERRAIN:
    case MON_DATA_UNUSED_159:
    case MON_DATA_COMBINED_IVS:
    case MON_DATA_NO_PRINT_GENDER:
    case MON_DATA_TYPE_1:
    case MON_DATA_TYPE_2:
    case MON_DATA_SPECIES_NAME:
    default:
        GF_ASSERT(FALSE);
        break;
    }
}

SpeciesData *SpeciesData_FromMonForm(int monSpecies, int monForm, enum HeapID heapID)
{
    SpeciesData *speciesData = Heap_Alloc(heapID, sizeof(SpeciesData));
    SpeciesData_LoadForm(monSpecies, monForm, speciesData);

    return speciesData;
}

SpeciesData *SpeciesData_FromMonSpecies(int monSpecies, enum HeapID heapID)
{
    SpeciesData *speciesData = Heap_Alloc(heapID, sizeof(SpeciesData));
    SpeciesData_LoadSpecies(monSpecies, speciesData);

    return speciesData;
}

u32 SpeciesData_GetValue(SpeciesData *speciesData, enum SpeciesDataParam param)
{
    u32 result;

    GF_ASSERT(speciesData);

    switch (param) {
    case SPECIES_DATA_BASE_HP:
        result = speciesData->baseStats.hp;
        break;
    case SPECIES_DATA_BASE_ATK:
        result = speciesData->baseStats.attack;
        break;
    case SPECIES_DATA_BASE_DEF:
        result = speciesData->baseStats.defense;
        break;
    case SPECIES_DATA_BASE_SPEED:
        result = speciesData->baseStats.speed;
        break;
    case SPECIES_DATA_BASE_SP_ATK:
        result = speciesData->baseStats.spAttack;
        break;
    case SPECIES_DATA_BASE_SP_DEF:
        result = speciesData->baseStats.spDefense;
        break;
    case SPECIES_DATA_TYPE_1:
        result = speciesData->types[0];
        break;
    case SPECIES_DATA_TYPE_2:
        result = speciesData->types[1];
        break;
    case SPECIES_DATA_CATCH_RATE:
        result = speciesData->catchRate;
        break;
    case SPECIES_DATA_BASE_EXP_REWARD:
        result = speciesData->baseExpReward;
        break;
    case SPECIES_DATA_EV_HP_YIELD:
        result = speciesData->evYields.hp;
        break;
    case SPECIES_DATA_EV_ATK_YIELD:
        result = speciesData->evYields.attack;
        break;
    case SPECIES_DATA_EV_DEF_YIELD:
        result = speciesData->evYields.defense;
        break;
    case SPECIES_DATA_EV_SPEED_YIELD:
        result = speciesData->evYields.speed;
        break;
    case SPECIES_DATA_EV_SP_ATK_YIELD:
        result = speciesData->evYields.spAttack;
        break;
    case SPECIES_DATA_EV_SP_DEF_YIELD:
        result = speciesData->evYields.spDefense;
        break;
    case SPECIES_DATA_HELD_ITEM_COMMON:
        result = speciesData->wildHeldItems.common;
        break;
    case SPECIES_DATA_HELD_ITEM_RARE:
        result = speciesData->wildHeldItems.rare;
        break;
    case SPECIES_DATA_GENDER_RATIO:
        result = speciesData->genderRatio;
        break;
    case SPECIES_DATA_HATCH_CYCLES:
        result = speciesData->hatchCycles;
        break;
    case SPECIES_DATA_BASE_FRIENDSHIP:
        result = speciesData->baseFriendship;
        break;
    case SPECIES_DATA_EXP_RATE:
        result = speciesData->expRate;
        break;
    case SPECIES_DATA_EGG_GROUP_1:
        result = speciesData->eggGroups[0];
        break;
    case SPECIES_DATA_EGG_GROUP_2:
        result = speciesData->eggGroups[1];
        break;
    case SPECIES_DATA_ABILITY_1:
        result = speciesData->abilities[0];
        break;
    case SPECIES_DATA_ABILITY_2:
        result = speciesData->abilities[1];
        break;
    case SPECIES_DATA_SAFARI_FLEE_RATE:
        result = speciesData->safariFleeRate;
        break;
    case SPECIES_DATA_BODY_COLOR:
        result = speciesData->bodyColor;
        break;
    case SPECIES_DATA_FLIP_SPRITE:
        result = speciesData->flipSprite;
        break;
    case SPECIES_DATA_TM_LEARNSET_MASK_1:
        result = speciesData->tmLearnsetMasks[0];
        break;
    case SPECIES_DATA_TM_LEARNSET_MASK_2:
        result = speciesData->tmLearnsetMasks[1];
        break;
    case SPECIES_DATA_TM_LEARNSET_MASK_3:
        result = speciesData->tmLearnsetMasks[2];
        break;
    case SPECIES_DATA_TM_LEARNSET_MASK_4:
        result = speciesData->tmLearnsetMasks[3];
        break;
    }

    return result;
}

void SpeciesData_Free(SpeciesData *speciesData)
{
    GF_ASSERT(speciesData);
    Heap_Free(speciesData);
}

u32 SpeciesData_GetFormValue(int monSpecies, int monForm, enum SpeciesDataParam param)
{
    monSpecies = Pokemon_GetFormNarcIndex(monSpecies, monForm);

    SpeciesData *speciesData = SpeciesData_FromMonSpecies(monSpecies, HEAP_ID_SYSTEM);
    u32 result = SpeciesData_GetValue(speciesData, param);

    SpeciesData_Free(speciesData);

    return result;
}

u32 SpeciesData_GetSpeciesValue(int monSpecies, enum SpeciesDataParam param)
{
    SpeciesData *speciesData = SpeciesData_FromMonSpecies(monSpecies, HEAP_ID_SYSTEM);
    u32 result = SpeciesData_GetValue(speciesData, param);

    SpeciesData_Free(speciesData);

    return result;
}

u8 Pokemon_GetPercentToNextLevel(Pokemon *mon)
{
    BOOL recrypt = Pokemon_EnterDecryptionContext(mon);
    u16 monSpecies = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
    u8 monLevel = Pokemon_GetValue(mon, MON_DATA_LEVEL, NULL);
    u32 monCurrentLevelExp = Pokemon_GetSpeciesBaseExpAt(monSpecies, monLevel);
    u32 monNextLevelExp = Pokemon_GetSpeciesBaseExpAt(monSpecies, monLevel + 1);
    u32 monExp = Pokemon_GetValue(mon, MON_DATA_EXPERIENCE, NULL);

    Pokemon_ExitDecryptionContext(mon, recrypt);
    u8 monPercentToNextLevel = ((monExp - monCurrentLevelExp) * 100) / (monNextLevelExp - monCurrentLevelExp);

    return monPercentToNextLevel;
}

u32 Pokemon_GetExpToNextLevel(Pokemon *mon)
{
    return BoxPokemon_GetExpToNextLevel(&mon->box);
}

static u32 BoxPokemon_GetExpToNextLevel(BoxPokemon *boxMon)
{
    u16 monSpecies = BoxPokemon_GetValue(boxMon, MON_DATA_SPECIES, NULL);
    u16 monNextlevel = BoxPokemon_GetLevel(boxMon) + 1;
    u32 monExp = BoxPokemon_GetValue(boxMon, MON_DATA_EXPERIENCE, NULL);

    return Pokemon_GetSpeciesBaseExpAt(monSpecies, monNextlevel) - monExp;
}

u32 Pokemon_GetCurrentLevelBaseExp(Pokemon *mon)
{
    return Pokemon_GetSpeciesBaseExpAt(Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL), Pokemon_GetValue(mon, MON_DATA_LEVEL, NULL));
}

u32 Pokemon_GetSpeciesBaseExpAt(int monSpecies, int monLevel)
{
    return Pokemon_GetExpRateBaseExpAt(SpeciesData_GetSpeciesValue(monSpecies, SPECIES_DATA_EXP_RATE), monLevel);
}

static void Pokemon_LoadExperienceTableOf(enum ExpRate monExpRate, u32 *monExpTable)
{
    GF_ASSERT(monExpRate < EXP_RATE_COUNT);
    NARC_ReadWholeMemberByIndexPair(monExpTable, NARC_INDEX_POKETOOL__PERSONAL__PL_GROWTBL, monExpRate);
}

static u32 Pokemon_GetExpRateBaseExpAt(enum ExpRate monExpRate, int monLevel)
{
    GF_ASSERT(monExpRate < EXP_RATE_COUNT);
    GF_ASSERT(monLevel <= MAX_POKEMON_LEVEL + 1);

    u32 *expTable = Heap_Alloc(HEAP_ID_SYSTEM, (MAX_POKEMON_LEVEL + 1) * 4);
    Pokemon_LoadExperienceTableOf(monExpRate, expTable);

    u32 result = expTable[monLevel];
    Heap_Free(expTable);

    return result;
}

u32 Pokemon_GetLevel(Pokemon *mon)
{
    return BoxPokemon_GetLevel(&mon->box);
}

u32 BoxPokemon_GetLevel(BoxPokemon *boxMon)
{
    BOOL reencrypt = BoxPokemon_EnterDecryptionContext(boxMon);
    int monSpecies = BoxPokemon_GetValue(boxMon, MON_DATA_SPECIES, NULL);
    u32 monExp = BoxPokemon_GetValue(boxMon, MON_DATA_EXPERIENCE, NULL);

    BoxPokemon_ExitDecryptionContext(boxMon, reencrypt);

    return Pokemon_GetSpeciesLevelAt(monSpecies, monExp);
}

u32 Pokemon_GetSpeciesLevelAt(u16 monSpecies, u32 monExp)
{
    SpeciesData *speciesData = SpeciesData_FromMonSpecies(monSpecies, HEAP_ID_SYSTEM);

    u32 monLevel = SpeciesData_GetLevelAt(speciesData, monSpecies, monExp);
    SpeciesData_Free(speciesData);

    return monLevel;
}

u32 SpeciesData_GetLevelAt(SpeciesData *speciesData, u16 unused_monSpecies, u32 monExp)
{
    static u32 monExpTable[MAX_POKEMON_LEVEL + 1];

    enum ExpRate monExpRate = SpeciesData_GetValue(speciesData, SPECIES_DATA_EXP_RATE);
    Pokemon_LoadExperienceTableOf(monExpRate, monExpTable);

    int i;
    for (i = 1; i < MAX_POKEMON_LEVEL + 1; i++) {
        if (monExpTable[i] > monExp) {
            break;
        }
    }

    return i - 1;
}

u8 Pokemon_GetNature(Pokemon *mon)
{
    return BoxPokemon_GetNature(&mon->box);
}

u8 BoxPokemon_GetNature(BoxPokemon *boxMon)
{
    BOOL reencrypt = BoxPokemon_EnterDecryptionContext(boxMon);
    u32 monPersonality = BoxPokemon_GetValue(boxMon, MON_DATA_PERSONALITY, NULL);

    BoxPokemon_ExitDecryptionContext(boxMon, reencrypt);

    return Pokemon_GetNatureOf(monPersonality);
}

u8 Pokemon_GetNatureOf(u32 monPersonality)
{
    return (u8)(monPersonality % NATURE_COUNT);
}

static const s8 sNatureStatAffinities[][5] = {
    [NATURE_HARDY] = {
        [STAT_ATTACK - 1] = 0,
        [STAT_DEFENSE - 1] = 0,
        [STAT_SPEED - 1] = 0,
        [STAT_SPECIAL_ATTACK - 1] = 0,
        [STAT_SPECIAL_DEFENSE - 1] = 0,
    },
    [NATURE_LONELY] = {
        [STAT_ATTACK - 1] = 1,
        [STAT_DEFENSE - 1] = -1,
        [STAT_SPEED - 1] = 0,
        [STAT_SPECIAL_ATTACK - 1] = 0,
        [STAT_SPECIAL_DEFENSE - 1] = 0,
    },
    [NATURE_BRAVE] = {
        [STAT_ATTACK - 1] = 1,
        [STAT_DEFENSE - 1] = 0,
        [STAT_SPEED - 1] = -1,
        [STAT_SPECIAL_ATTACK - 1] = 0,
        [STAT_SPECIAL_DEFENSE - 1] = 0,
    },
    [NATURE_ADAMANT] = {
        [STAT_ATTACK - 1] = 1,
        [STAT_DEFENSE - 1] = 0,
        [STAT_SPEED - 1] = 0,
        [STAT_SPECIAL_ATTACK - 1] = -1,
        [STAT_SPECIAL_DEFENSE - 1] = 0,
    },
    [NATURE_NAUGHTY] = {
        [STAT_ATTACK - 1] = 1,
        [STAT_DEFENSE - 1] = 0,
        [STAT_SPEED - 1] = 0,
        [STAT_SPECIAL_ATTACK - 1] = 0,
        [STAT_SPECIAL_DEFENSE - 1] = -1,
    },
    [NATURE_BOLD] = {
        [STAT_ATTACK - 1] = -1,
        [STAT_DEFENSE - 1] = 1,
        [STAT_SPEED - 1] = 0,
        [STAT_SPECIAL_ATTACK - 1] = 0,
        [STAT_SPECIAL_DEFENSE - 1] = 0,
    },
    [NATURE_DOCILE] = {
        [STAT_ATTACK - 1] = 0,
        [STAT_DEFENSE - 1] = 0,
        [STAT_SPEED - 1] = 0,
        [STAT_SPECIAL_ATTACK - 1] = 0,
        [STAT_SPECIAL_DEFENSE - 1] = 0,
    },
    [NATURE_RELAXED] = {
        [STAT_ATTACK - 1] = 0,
        [STAT_DEFENSE - 1] = 1,
        [STAT_SPEED - 1] = -1,
        [STAT_SPECIAL_ATTACK - 1] = 0,
        [STAT_SPECIAL_DEFENSE - 1] = 0,
    },
    [NATURE_IMPISH] = {
        [STAT_ATTACK - 1] = 0,
        [STAT_DEFENSE - 1] = 1,
        [STAT_SPEED - 1] = 0,
        [STAT_SPECIAL_ATTACK - 1] = -1,
        [STAT_SPECIAL_DEFENSE - 1] = 0,
    },
    [NATURE_LAX] = {
        [STAT_ATTACK - 1] = 0,
        [STAT_DEFENSE - 1] = 1,
        [STAT_SPEED - 1] = 0,
        [STAT_SPECIAL_ATTACK - 1] = 0,
        [STAT_SPECIAL_DEFENSE - 1] = -1,
    },
    [NATURE_TIMID] = {
        [STAT_ATTACK - 1] = -1,
        [STAT_DEFENSE - 1] = 0,
        [STAT_SPEED - 1] = 1,
        [STAT_SPECIAL_ATTACK - 1] = 0,
        [STAT_SPECIAL_DEFENSE - 1] = 0,
    },
    [NATURE_HASTY] = {
        [STAT_ATTACK - 1] = 0,
        [STAT_DEFENSE - 1] = -1,
        [STAT_SPEED - 1] = 1,
        [STAT_SPECIAL_ATTACK - 1] = 0,
        [STAT_SPECIAL_DEFENSE - 1] = 0,
    },
    [NATURE_SERIOUS] = {
        [STAT_ATTACK - 1] = 0,
        [STAT_DEFENSE - 1] = 0,
        [STAT_SPEED - 1] = 0,
        [STAT_SPECIAL_ATTACK - 1] = 0,
        [STAT_SPECIAL_DEFENSE - 1] = 0,
    },
    [NATURE_JOLLY] = {
        [STAT_ATTACK - 1] = 0,
        [STAT_DEFENSE - 1] = 0,
        [STAT_SPEED - 1] = 1,
        [STAT_SPECIAL_ATTACK - 1] = -1,
        [STAT_SPECIAL_DEFENSE - 1] = 0,
    },
    [NATURE_NAIVE] = {
        [STAT_ATTACK - 1] = 0,
        [STAT_DEFENSE - 1] = 0,
        [STAT_SPEED - 1] = 1,
        [STAT_SPECIAL_ATTACK - 1] = 0,
        [STAT_SPECIAL_DEFENSE - 1] = -1,
    },
    [NATURE_MODEST] = {
        [STAT_ATTACK - 1] = -1,
        [STAT_DEFENSE - 1] = 0,
        [STAT_SPEED - 1] = 0,
        [STAT_SPECIAL_ATTACK - 1] = 1,
        [STAT_SPECIAL_DEFENSE - 1] = 0,
    },
    [NATURE_MILD] = {
        [STAT_ATTACK - 1] = 0,
        [STAT_DEFENSE - 1] = -1,
        [STAT_SPEED - 1] = 0,
        [STAT_SPECIAL_ATTACK - 1] = 1,
        [STAT_SPECIAL_DEFENSE - 1] = 0,
    },
    [NATURE_QUIET] = {
        [STAT_ATTACK - 1] = 0,
        [STAT_DEFENSE - 1] = 0,
        [STAT_SPEED - 1] = -1,
        [STAT_SPECIAL_ATTACK - 1] = 1,
        [STAT_SPECIAL_DEFENSE - 1] = 0,
    },
    [NATURE_BASHFUL] = {
        [STAT_ATTACK - 1] = 0,
        [STAT_DEFENSE - 1] = 0,
        [STAT_SPEED - 1] = 0,
        [STAT_SPECIAL_ATTACK - 1] = 0,
        [STAT_SPECIAL_DEFENSE - 1] = 0,
    },
    [NATURE_RASH] = {
        [STAT_ATTACK - 1] = 0,
        [STAT_DEFENSE - 1] = 0,
        [STAT_SPEED - 1] = 0,
        [STAT_SPECIAL_ATTACK - 1] = 1,
        [STAT_SPECIAL_DEFENSE - 1] = -1,
    },
    [NATURE_CALM] = {
        [STAT_ATTACK - 1] = -1,
        [STAT_DEFENSE - 1] = 0,
        [STAT_SPEED - 1] = 0,
        [STAT_SPECIAL_ATTACK - 1] = 0,
        [STAT_SPECIAL_DEFENSE - 1] = 1,
    },
    [NATURE_GENTLE] = {
        [STAT_ATTACK - 1] = 0,
        [STAT_DEFENSE - 1] = -1,
        [STAT_SPEED - 1] = 0,
        [STAT_SPECIAL_ATTACK - 1] = 0,
        [STAT_SPECIAL_DEFENSE - 1] = 1,
    },
    [NATURE_SASSY] = {
        [STAT_ATTACK - 1] = 0,
        [STAT_DEFENSE - 1] = 0,
        [STAT_SPEED - 1] = -1,
        [STAT_SPECIAL_ATTACK - 1] = 0,
        [STAT_SPECIAL_DEFENSE - 1] = 1,
    },
    [NATURE_CAREFUL] = {
        [STAT_ATTACK - 1] = 0,
        [STAT_DEFENSE - 1] = 0,
        [STAT_SPEED - 1] = 0,
        [STAT_SPECIAL_ATTACK - 1] = -1,
        [STAT_SPECIAL_DEFENSE - 1] = 1,
    },
    [NATURE_QUIRKY] = {
        [STAT_ATTACK - 1] = 0,
        [STAT_DEFENSE - 1] = 0,
        [STAT_SPEED - 1] = 0,
        [STAT_SPECIAL_ATTACK - 1] = 0,
        [STAT_SPECIAL_DEFENSE - 1] = 0,
    }
};

static u16 Pokemon_GetNatureStatValue(u8 monNature, u16 monStatValue, u8 statType)
{
    if (statType < STAT_ATTACK || statType > STAT_SPECIAL_DEFENSE) {
        return monStatValue;
    }

    u16 result;
    switch (sNatureStatAffinities[monNature][statType - 1]) {
    case 1:
        result = monStatValue * 110;
        result /= 100;
        break;
    case -1:
        result = monStatValue * 90;
        result /= 100;
        break;
    default:
        result = monStatValue;
        break;
    }

    return result;
}

s8 Pokemon_GetStatAffinityOf(u8 monNature, u8 statType)
{
    return sNatureStatAffinities[monNature][statType - 1];
}

static const s8 sFriendshipChangeTable[FRIENDSHIP_EVENT_MAX][3] = {
    [FRIENDSHIP_EVENT_LEVEL_UP] = { 5, 3, 2 },
    [FRIENDSHIP_EVENT_UNK_1] = { 5, 3, 2 },
    [FRIENDSHIP_EVENT_UNK_2] = { 1, 1, 0 },
    [FRIENDSHIP_EVENT_BEAT_GYM_LEADER_E4_OR_CHAMPION] = { 3, 2, 1 },
    [FRIENDSHIP_EVENT_LEARN_TMHM] = { 1, 1, 0 },
    [FRIENDSHIP_EVENT_WALK_CYCLE] = { 1, 1, 1 },
    [FRIENDSHIP_EVENT_BATTLE_FAINT] = { -1, -1, -1 },
    [FRIENDSHIP_EVENT_POISON_SURVIVE] = { -5, -5, -10 },
    [FRIENDSHIP_EVENT_BATTLE_FAINT_HIGH_LVL_DIFF] = { -5, -5, -10 },
    [FRIENDSHIP_EVENT_CONTEST_WIN] = { 3, 2, 1 }
};

void Pokemon_UpdateFriendship(Pokemon *mon, u8 friendshipEvent, u16 mapID)
{
    if (friendshipEvent == FRIENDSHIP_EVENT_WALK_CYCLE) {
        if (LCRNG_Next() & 1) {
            return;
        }
    }

    u16 monSpeciesEgg = Pokemon_GetValue(mon, MON_DATA_SPECIES_OR_EGG, NULL);

    if (monSpeciesEgg == SPECIES_NONE || monSpeciesEgg == SPECIES_EGG) {
        return;
    }

    u16 monHeldItem = Pokemon_GetValue(mon, MON_DATA_HELD_ITEM, NULL);
    u8 itemHoldEffect = Item_LoadParam(monHeldItem, ITEM_PARAM_HOLD_EFFECT, HEAP_ID_SYSTEM);
    u8 friendshipTier = 0;
    s16 monFriendship = Pokemon_GetValue(mon, MON_DATA_FRIENDSHIP, NULL);

    if (monFriendship >= LOW_FRIENDSHIP_LIMIT) {
        friendshipTier++;
    }

    if (monFriendship >= MED_FRIENDSHIP_LIMIT) {
        friendshipTier++;
    }

    s8 friendshipChange = sFriendshipChangeTable[friendshipEvent][friendshipTier];

    if (friendshipChange > 0 && Pokemon_GetValue(mon, MON_DATA_POKEBALL, NULL) == ITEM_LUXURY_BALL) {
        friendshipChange++;
    }

    if (friendshipChange > 0 && Pokemon_GetValue(mon, MON_DATA_EGG_LOCATION, NULL) == mapID) {
        friendshipChange++;
    }

    if (friendshipChange > 0) {
        if (itemHoldEffect == HOLD_EFFECT_FRIENDSHIP_UP) {
            friendshipChange = friendshipChange * 150 / 100;
        }
    }

    monFriendship += friendshipChange;

    if (monFriendship < 0) {
        monFriendship = 0;
    }

    if (monFriendship > MAX_FRIENDSHIP_VALUE) {
        monFriendship = MAX_FRIENDSHIP_VALUE;
    }

    Pokemon_SetValue(mon, MON_DATA_FRIENDSHIP, &monFriendship);
}

u8 Pokemon_GetGender(Pokemon *mon)
{
    return BoxPokemon_GetGender(&mon->box);
}

u8 BoxPokemon_GetGender(BoxPokemon *boxMon)
{
    BOOL reencrypt = BoxPokemon_EnterDecryptionContext(boxMon);
    u16 monSpecies = BoxPokemon_GetValue(boxMon, MON_DATA_SPECIES, NULL);
    u32 monPersonality = BoxPokemon_GetValue(boxMon, MON_DATA_PERSONALITY, NULL);

    BoxPokemon_ExitDecryptionContext(boxMon, reencrypt);

    return Pokemon_GetGenderOf(monSpecies, monPersonality);
}

u8 Pokemon_GetGenderOf(u16 monSpecies, u32 monPersonality)
{
    SpeciesData *speciesData = SpeciesData_FromMonSpecies(monSpecies, HEAP_ID_SYSTEM);
    u8 monGender = SpeciesData_GetGenderOf(speciesData, monSpecies, monPersonality);

    SpeciesData_Free(speciesData);

    return monGender;
}

u8 SpeciesData_GetGenderOf(SpeciesData *speciesData, u16 unused_monSpecies, u32 monPersonality)
{
    u8 monGender = SpeciesData_GetValue(speciesData, SPECIES_DATA_GENDER_RATIO);

    switch (monGender) {
    case GENDER_RATIO_MALE_ONLY:
        return GENDER_MALE;
    case GENDER_RATIO_FEMALE_ONLY:
        return GENDER_FEMALE;
    case GENDER_RATIO_NO_GENDER:
        return GENDER_NONE;
    }

    if (monGender > (monPersonality & 0xff)) {
        return GENDER_FEMALE;
    } else {
        return GENDER_MALE;
    }
}

// TODO return bool
u8 Pokemon_IsShiny(Pokemon *mon)
{
    return BoxPokemon_IsShiny(&mon->box);
}

// TODO return bool
static u8 BoxPokemon_IsShiny(BoxPokemon *boxMon)
{
    u32 monOTID = BoxPokemon_GetValue(boxMon, MON_DATA_OT_ID, NULL);
    u32 monPersonality = BoxPokemon_GetValue(boxMon, MON_DATA_PERSONALITY, NULL);

    return Pokemon_IsPersonalityShiny(monOTID, monPersonality);
}

static inline BOOL Pokemon_InlineIsPersonalityShiny(u32 monOTID, u32 monPersonality)
{
    // COLOR MOD (port of PokeHeartGold ENTRY AD item 1): was a hard-coded "< 8" (vanilla 1/8192 odds).
    // SHINY_ODDS = 32768 = exactly half of 65536, i.e. 50% of Pokemon are shiny.
    return (((monOTID & 0xFFFF0000) >> 16) ^ (monOTID & 0xFFFF) ^ ((monPersonality & 0xFFFF0000) >> 16) ^ (monPersonality & 0xFFFF)) < SHINY_ODDS;
}

u8 Pokemon_IsPersonalityShiny(u32 monOTID, u32 monPersonality)
{
    return Pokemon_InlineIsPersonalityShiny(monOTID, monPersonality);
}

u32 Pokemon_FindShinyPersonality(u32 monOTID)
{
    // 1. Pre-compute the XOR of the two halves of the trainer ID. We only
    // care about the most-significant 13 bits, so truncate the last 3.
    monOTID = (((monOTID & 0xFFFF0000) >> 16) ^ (monOTID & 0xFFFF)) >> 3;

    int i;

    // 2. Randomize the least-significant 3-bits of each half of the
    // generated personality.
    u16 rndLow = LCRNG_Next() & 0x7;
    u16 rndHigh = LCRNG_Next() & 0x7;

    // 3. For each of the remaining 13 bits, pick some permutation of them
    // across both halves to be set to 1 such that the XOR of their bits
    // will XOR with the monOTID to 0.
    for (i = 0; i < 13; i++) {
        if (monOTID & FlagIndex(i)) {
            // Trainer ID XORs to 1; set one of the two personality bits to 1
            if (LCRNG_Next() & 1) {
                rndLow |= FlagIndex(i + 3);
            } else {
                rndHigh |= FlagIndex(i + 3);
            }
        } else if (LCRNG_Next() & 1) {
            // Trainer ID XORs to 0; set both of the two bits to either 0 or 1
            rndLow |= FlagIndex(i + 3);
            rndHigh |= FlagIndex(i + 3);
        }
    }

    return rndLow | (rndHigh << 16);
}

void Pokemon_BuildSpriteTemplate(PokemonSpriteTemplate *spriteTemplate, Pokemon *mon, u8 face)
{
    BoxPokemon_BuildSpriteTemplate(spriteTemplate, &mon->box, face, FALSE);
}

void Pokemon_BuildSpriteTemplateDP(PokemonSpriteTemplate *spriteTemplate, Pokemon *mon, u8 face)
{
    BoxPokemon_BuildSpriteTemplate(spriteTemplate, &mon->box, face, TRUE);
}

void BoxPokemon_BuildSpriteTemplate(PokemonSpriteTemplate *spriteTemplate, BoxPokemon *mon, u8 face, BOOL preferDP)
{
    BOOL reencrypt = BoxPokemon_EnterDecryptionContext(mon);

    u16 monSpeciesEgg = BoxPokemon_GetValue(mon, MON_DATA_SPECIES_OR_EGG, NULL);
    u8 monGender = BoxPokemon_GetGender(mon);
    u8 monShininess = BoxPokemon_IsShiny(mon);
    u32 monPersonality = BoxPokemon_GetValue(mon, MON_DATA_PERSONALITY, NULL);

    u8 monForm;
    if (monSpeciesEgg == SPECIES_EGG) {
        if (BoxPokemon_GetValue(mon, MON_DATA_SPECIES, NULL) == SPECIES_MANAPHY) {
            monForm = 1;
        } else {
            monForm = 0;
        }
    } else {
        monForm = BoxPokemon_GetValue(mon, MON_DATA_FORM, NULL);
    }

    if (preferDP == TRUE) {
        BuildPokemonSpriteTemplateDP(spriteTemplate, monSpeciesEgg, monGender, face, monShininess, monForm, monPersonality);
    } else {
        BuildPokemonSpriteTemplate(spriteTemplate, monSpeciesEgg, monGender, face, monShininess, monForm, monPersonality);
    }

    BoxPokemon_ExitDecryptionContext(mon, reencrypt);
}

void BuildPokemonSpriteTemplate(PokemonSpriteTemplate *spriteTemplate, u16 species, u8 gender, u8 face, u8 shiny, u8 form, u32 personality)
{
    spriteTemplate->spindaSpots = 0;
    spriteTemplate->dummy = 0;
    // COLOR MOD FIX (2026-09-23): this used to be hardcoded to 0 here, with the real personality value
    // only ever assigned further down for SPECIES_SPINDA (its spot pattern is personality-derived in
    // vanilla too). ENTRY P5's color-mod hook (BufferPokemonSpritePlttData, src/pokemon_sprite.c) reads
    // this same field for EVERY species assuming it was already universally populated - it wasn't, so
    // every non-Spinda Pokemon's personality-based palette rotation was always computed from personality
    // 0, producing no individual color variation at all (this is what the user reported as "the color
    // mod isn't working"). Safe to populate for every species: the only other consumer of this field,
    // TryDrawSpindaSpots (src/pokemon_sprite.c), gates on spindaSpots, not personality, so this doesn't
    // change Spinda's own behavior at all.
    spriteTemplate->personality = personality;
    form = Pokemon_SanitizeFormId(species, form);

    switch (species) {
    case SPECIES_BURMY:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__PL_OTHERPOKE;
        spriteTemplate->character = 72 + (face / 2) + form * 2;
        spriteTemplate->palette = 166 + shiny + form * 2;
        break;

    case SPECIES_WORMADAM:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__PL_OTHERPOKE;
        spriteTemplate->character = 78 + (face / 2) + form * 2;
        spriteTemplate->palette = 172 + shiny + form * 2;
        break;

    case SPECIES_SHELLOS:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__PL_OTHERPOKE;
        spriteTemplate->character = 84 + face + form;
        spriteTemplate->palette = 178 + shiny + form * 2;
        break;

    case SPECIES_GASTRODON:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__PL_OTHERPOKE;
        spriteTemplate->character = 88 + face + form;
        spriteTemplate->palette = 182 + shiny + form * 2;
        break;

    case SPECIES_CHERRIM:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__PL_OTHERPOKE;
        spriteTemplate->character = 92 + face + form;
        spriteTemplate->palette = 186 + (shiny * 2) + form;
        break;

    case SPECIES_ARCEUS:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__PL_OTHERPOKE;
        spriteTemplate->character = 96 + (face / 2) + form * 2;
        spriteTemplate->palette = 190 + shiny + form * 2;
        break;

    case SPECIES_CASTFORM:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__PL_OTHERPOKE;
        spriteTemplate->character = 64 + (face * 2) + form;
        spriteTemplate->palette = 158 + (shiny * 4) + form;
        break;

    case SPECIES_DEOXYS:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__PL_OTHERPOKE;
        spriteTemplate->character = 0 + (face / 2) + form * 2;
        spriteTemplate->palette = 154 + shiny;
        break;

    case SPECIES_UNOWN:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__PL_OTHERPOKE;
        spriteTemplate->character = 8 + (face / 2) + form * 2;
        spriteTemplate->palette = 156 + shiny;
        break;

    case SPECIES_EGG:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__PL_OTHERPOKE;
        spriteTemplate->character = 132 + form;
        spriteTemplate->palette = 226 + form;
        break;

    case SPECIES_BAD_EGG:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__PL_OTHERPOKE;
        spriteTemplate->character = 132;
        spriteTemplate->palette = 226;
        break;

    case SPECIES_SHAYMIN:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__PL_OTHERPOKE;
        spriteTemplate->character = 134 + (face / 2) + form * 2;
        spriteTemplate->palette = 228 + shiny + form * 2;
        break;

    case SPECIES_ROTOM:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__PL_OTHERPOKE;
        spriteTemplate->character = 138 + (face / 2) + form * 2;
        spriteTemplate->palette = 232 + shiny + form * 2;
        break;

    case SPECIES_GIRATINA:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__PL_OTHERPOKE;
        spriteTemplate->character = 150 + (face / 2) + form * 2;
        spriteTemplate->palette = 244 + shiny + form * 2;
        break;

    default:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__PL_POKEGRA;
        spriteTemplate->character = species * 6 + face + (gender != GENDER_FEMALE ? 1 : 0); // ternary must remain to match
        spriteTemplate->palette = species * 6 + 4 + shiny;

        if (species == SPECIES_SPINDA && face == FACE_FRONT) {
            spriteTemplate->spindaSpots = SPECIES_SPINDA;
            spriteTemplate->dummy = 0;
            spriteTemplate->personality = personality;
        }
    }
}

u8 Pokemon_SanitizeFormId(u16 monSpecies, u8 monForm)
{
    switch (monSpecies) {
    case SPECIES_BURMY:
        if (monForm > BURMY_FORM_COUNT - 1) {
            monForm = 0;
        }
        break;
    case SPECIES_WORMADAM:
        if (monForm > WORMADAM_FORM_COUNT - 1) {
            monForm = 0;
        }
        break;
    case SPECIES_SHELLOS:
        if (monForm > SHELLOS_FORM_COUNT - 1) {
            monForm = 0;
        }
        break;
    case SPECIES_GASTRODON:
        if (monForm > GASTRODON_FORM_COUNT - 1) {
            monForm = 0;
        }
        break;
    case SPECIES_CHERRIM:
        if (monForm > CHERRIM_FORM_COUNT - 1) {
            monForm = 0;
        }
        break;
    case SPECIES_ARCEUS:
        if (monForm > ARCEUS_FORM_COUNT - 1) {
            monForm = 0;
        }
        break;
    case SPECIES_CASTFORM:
        if (monForm > CASTFORM_FORM_COUNT - 1) {
            monForm = 0;
        }
        break;
    case SPECIES_DEOXYS:
        if (monForm > DEOXYS_FORM_COUNT - 1) {
            monForm = 0;
        }
        break;
    case SPECIES_UNOWN:
        if (monForm >= UNOWN_FORM_COUNT) {
            monForm = 0;
        }
        break;
    case SPECIES_EGG:
        if (monForm > EGG_FORM_COUNT - 1) {
            monForm = 0;
        }
        break;
    case SPECIES_SHAYMIN:
        if (monForm > SHAYMIN_FORM_COUNT - 1) {
            monForm = 0;
        }
        break;
    case SPECIES_ROTOM:
        if (monForm > ROTOM_FORM_COUNT - 1) {
            monForm = 0;
        }
        break;
    case SPECIES_GIRATINA:
        if (monForm > GIRATINA_FORM_COUNT - 1) {
            monForm = 0;
        }
        break;
    }

    return monForm;
}

/**
 * @brief Build a PokemonSpriteTemplate for a Pokemon sprite, preferring sprites from
 * Diamond/Pearl over Platinum.
 *
 * This routine will still use sprites from Platinum for Pokemon variants which
 * did not exist in Diamond/Pearl, namely:
 * - Giratina-Origin
 * - Shaymin-Sky
 * - Rotom appliances
 *
 * @param spriteTemplate Pointer to the sprite template to be populated
 * @param species        The Pokemon's species
 * @param gender         The Pokemon's gender
 * @param face           Which face of the Pokemon the player sees
 * @param shiny          1 if the Pokemon is shiny, 0 if not
 * @param form           The Pokemon's form
 * @param personality    The Pokemon's personality value
 */
static void BuildPokemonSpriteTemplateDP(PokemonSpriteTemplate *spriteTemplate, u16 species, u8 gender, u8 face, u8 shiny, u8 form, u32 personality)
{
    spriteTemplate->spindaSpots = 0;
    spriteTemplate->dummy = 0;
    // COLOR MOD FIX (2026-09-23): see the identical fix/comment in BuildPokemonSpriteTemplate above.
    spriteTemplate->personality = personality;

    form = Pokemon_SanitizeFormId(species, form);

    switch (species) {
    case SPECIES_BURMY:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__OTHERPOKE;
        spriteTemplate->character = 72 + (face / 2) + form * 2;
        spriteTemplate->palette = 146 + shiny + form * 2;
        break;

    case SPECIES_WORMADAM:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__OTHERPOKE;
        spriteTemplate->character = 78 + (face / 2) + form * 2;
        spriteTemplate->palette = 152 + shiny + form * 2;
        break;

    case SPECIES_SHELLOS:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__OTHERPOKE;
        spriteTemplate->character = 84 + face + form;
        spriteTemplate->palette = 158 + shiny + form * 2;
        break;

    case SPECIES_GASTRODON:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__OTHERPOKE;
        spriteTemplate->character = 88 + face + form;
        spriteTemplate->palette = 162 + shiny + form * 2;
        break;

    case SPECIES_CHERRIM:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__OTHERPOKE;
        spriteTemplate->character = 92 + face + form;
        spriteTemplate->palette = 166 + (shiny * 2) + form;
        break;

    case SPECIES_ARCEUS:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__OTHERPOKE;
        spriteTemplate->character = 96 + (face / 2) + form * 2;
        spriteTemplate->palette = 170 + shiny + form * 2;
        break;

    case SPECIES_CASTFORM:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__OTHERPOKE;
        spriteTemplate->character = 64 + (face * 2) + form;
        spriteTemplate->palette = 138 + (shiny * 4) + form;
        break;

    case SPECIES_DEOXYS:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__OTHERPOKE;
        spriteTemplate->character = 0 + (face / 2) + form * 2;
        spriteTemplate->palette = 134 + shiny;
        break;

    case SPECIES_UNOWN:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__OTHERPOKE;
        spriteTemplate->character = 8 + (face / 2) + form * 2;
        spriteTemplate->palette = 136 + shiny;
        break;

    case SPECIES_EGG:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__OTHERPOKE;
        spriteTemplate->character = 132 + form;
        spriteTemplate->palette = 206 + form;
        break;

    case SPECIES_BAD_EGG:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__OTHERPOKE;
        spriteTemplate->character = 132;
        spriteTemplate->palette = 206;
        break;

    case SPECIES_SHAYMIN:
        if (form > 0) {
            spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__PL_OTHERPOKE;
            spriteTemplate->character = 134 + (face / 2) + form * 2;
            spriteTemplate->palette = 230 + shiny;
        } else {
            spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__POKEGRA;
            spriteTemplate->character = species * 6 + face + (gender != GENDER_FEMALE ? 1 : 0);
            spriteTemplate->palette = species * 6 + 4 + shiny;
        }
        break;

    case SPECIES_ROTOM:
        if (form > 0) {
            spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__PL_OTHERPOKE;
            spriteTemplate->character = 138 + (face / 2) + form * 2;
            spriteTemplate->palette = 232 + shiny + form * 2;
        } else {
            spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__POKEGRA;
            spriteTemplate->character = species * 6 + face + (gender != GENDER_FEMALE ? 1 : 0);
            spriteTemplate->palette = species * 6 + 4 + shiny;
        }
        break;

    case SPECIES_GIRATINA:
        if (form > 0) {
            spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__PL_OTHERPOKE;
            spriteTemplate->character = 150 + (face / 2) + form * 2;
            spriteTemplate->palette = 244 + shiny + form * 2;
        } else {
            spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__POKEGRA;
            spriteTemplate->character = species * 6 + face + (gender != GENDER_FEMALE ? 1 : 0);
            spriteTemplate->palette = species * 6 + 4 + shiny;
        }
        break;

    default:
        spriteTemplate->narcID = NARC_INDEX_POKETOOL__POKEGRA__POKEGRA;
        spriteTemplate->character = species * 6 + face + (gender != GENDER_FEMALE ? 1 : 0);
        spriteTemplate->palette = species * 6 + 4 + shiny;

        if (species == SPECIES_SPINDA && face == FACE_FRONT) {
            spriteTemplate->spindaSpots = SPECIES_SPINDA;
            spriteTemplate->dummy = 0;
            spriteTemplate->personality = personality;
        }

        break;
    }
}

u8 Pokemon_SpriteYOffset(Pokemon *mon, u8 face)
{
    return BoxPokemon_SpriteYOffset(&mon->box, face, FALSE);
}

u8 Pokemon_DPSpriteYOffset(Pokemon *mon, u8 face)
{
    return BoxPokemon_SpriteYOffset(&mon->box, face, TRUE);
}

u8 BoxPokemon_SpriteYOffset(BoxPokemon *boxMon, u8 face, BOOL preferDP)
{
    u16 species = BoxPokemon_GetValue(boxMon, MON_DATA_SPECIES_OR_EGG, NULL);
    u8 gender = BoxPokemon_GetGender(boxMon);
    u32 personality = BoxPokemon_GetValue(boxMon, MON_DATA_PERSONALITY, NULL);

    u8 form;
    if (species == SPECIES_EGG) {
        if (BoxPokemon_GetValue(boxMon, MON_DATA_SPECIES, NULL) == SPECIES_MANAPHY) {
            form = 1;
        } else {
            form = 0;
        }
    } else {
        form = BoxPokemon_GetValue(boxMon, MON_DATA_FORM, NULL);
    }

    if (preferDP == TRUE) {
        return LoadPokemonDPSpriteHeight(species, gender, face, form, personality);
    }

    return LoadPokemonSpriteYOffset(species, gender, face, form, personality);
}

u8 LoadPokemonSpriteYOffset(u16 species, u8 gender, u8 face, u8 form, u32 personality)
{
    form = Pokemon_SanitizeFormId(species, form);

    enum NarcID narcID;
    int memberIndex;
    switch (species) {
    case SPECIES_BURMY:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__HEIGHT_O;
        memberIndex = 72 + (face / 2) + form * 2;
        break;

    case SPECIES_WORMADAM:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__HEIGHT_O;
        memberIndex = 78 + (face / 2) + form * 2;
        break;

    case SPECIES_SHELLOS:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__HEIGHT_O;
        memberIndex = 84 + face + form;
        break;

    case SPECIES_GASTRODON:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__HEIGHT_O;
        memberIndex = 88 + face + form;
        break;

    case SPECIES_CHERRIM:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__HEIGHT_O;
        memberIndex = 92 + face + form;
        break;

    case SPECIES_ARCEUS:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__HEIGHT_O;
        memberIndex = 96 + (face / 2) + form * 2;
        break;

    case SPECIES_CASTFORM:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__HEIGHT_O;
        memberIndex = 64 + face * 2 + form;
        break;

    case SPECIES_DEOXYS:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__HEIGHT_O;
        memberIndex = 0 + (face / 2) + form * 2;
        break;

    case SPECIES_UNOWN:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__HEIGHT_O;
        memberIndex = 8 + (face / 2) + form * 2;
        break;

    case SPECIES_EGG:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__HEIGHT_O;
        memberIndex = 132 + form;
        break;

    case SPECIES_BAD_EGG:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__HEIGHT_O;
        memberIndex = 132;
        break;

    case SPECIES_SHAYMIN:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__HEIGHT_O;
        memberIndex = 136 + (face / 2) + form * 2;
        break;

    case SPECIES_ROTOM:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__HEIGHT_O;
        memberIndex = 140 + (face / 2) + form * 2;
        break;

    case SPECIES_GIRATINA:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__HEIGHT_O;
        memberIndex = 152 + (face / 2) + form * 2;
        break;

    default:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__HEIGHT;
        memberIndex = species * 4 + face + (gender != GENDER_FEMALE ? 1 : 0);
        break;
    }

    u8 result;
    NARC_ReadWholeMemberByIndexPair(&result, narcID, memberIndex);
    return result;
}

static u8 LoadPokemonDPSpriteHeight(u16 species, u8 gender, u8 face, u8 form, u32 personality)
{
    // TODO enum values?
    form = Pokemon_SanitizeFormId(species, form);

    enum NarcID narcID;
    int memberIndex;
    switch (species) {
    case SPECIES_BURMY:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__DP_HEIGHT_O;
        memberIndex = 72 + (face / 2) + form * 2;
        break;

    case SPECIES_WORMADAM:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__DP_HEIGHT_O;
        memberIndex = 78 + (face / 2) + form * 2;
        break;

    case SPECIES_SHELLOS:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__DP_HEIGHT_O;
        memberIndex = 84 + face + form;
        break;

    case SPECIES_GASTRODON:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__DP_HEIGHT_O;
        memberIndex = 88 + face + form;
        break;

    case SPECIES_CHERRIM:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__DP_HEIGHT_O;
        memberIndex = 92 + face + form;
        break;

    case SPECIES_ARCEUS:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__DP_HEIGHT_O;
        memberIndex = 96 + (face / 2) + form * 2;
        break;

    case SPECIES_CASTFORM:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__DP_HEIGHT_O;
        memberIndex = 64 + face * 2 + form;
        break;

    case SPECIES_DEOXYS:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__DP_HEIGHT_O;
        memberIndex = 0 + (face / 2) + form * 2;
        break;

    case SPECIES_UNOWN:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__DP_HEIGHT_O;
        memberIndex = 8 + (face / 2) + form * 2;
        break;

    case SPECIES_EGG:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__DP_HEIGHT_O;
        memberIndex = 132 + form;
        break;

    case SPECIES_BAD_EGG:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__DP_HEIGHT_O;
        memberIndex = 132;
        break;

    case SPECIES_SHAYMIN:
        if (form > 0) {
            narcID = NARC_INDEX_POKETOOL__POKEGRA__HEIGHT_O;
            memberIndex = 136 + (face / 2) + form * 2;
        } else {
            narcID = NARC_INDEX_POKETOOL__POKEGRA__DP_HEIGHT;
            memberIndex = species * 4 + face + (gender != GENDER_FEMALE ? 1 : 0);
        }
        break;

    case SPECIES_ROTOM:
        if (form > 0) {
            narcID = NARC_INDEX_POKETOOL__POKEGRA__HEIGHT_O;
            memberIndex = 140 + (face / 2) + form * 2;
        } else {
            narcID = NARC_INDEX_POKETOOL__POKEGRA__DP_HEIGHT;
            memberIndex = species * 4 + face + (gender != GENDER_FEMALE ? 1 : 0);
        }
        break;

    case SPECIES_GIRATINA:
        if (form > 0) {
            narcID = NARC_INDEX_POKETOOL__POKEGRA__HEIGHT_O;
            memberIndex = 152 + (face / 2) + form * 2;
        } else {
            narcID = NARC_INDEX_POKETOOL__POKEGRA__DP_HEIGHT;
            memberIndex = species * 4 + face + (gender != GENDER_FEMALE ? 1 : 0);
        }
        break;

    default:
        narcID = NARC_INDEX_POKETOOL__POKEGRA__DP_HEIGHT;
        memberIndex = species * 4 + face + (gender != GENDER_FEMALE ? 1 : 0);
        break;
    }

    u8 result;
    NARC_ReadWholeMemberByIndexPair(&result, narcID, memberIndex);
    return result;
}

void SpriteSystem_SetTrainerFrontSpriteTemplate(PokemonSpriteTemplate *spriteTemplate, u16 param1)
{
    spriteTemplate->narcID = NARC_INDEX_POKETOOL__TRGRA__TRFGRA;
    spriteTemplate->character = param1 * 2;
    spriteTemplate->palette = param1 * 2 + 1;
    spriteTemplate->spindaSpots = 0;
    spriteTemplate->dummy = 0;
    spriteTemplate->personality = 0;
}

static const SpriteTemplate sSpriteTemplateTrainer = {
    .x = 0,
    .y = 0,
    .z = 0,
    .animIdx = 0,
    .priority = 0,
    .plttIdx = 0,
    .vramType = NNS_G2D_VRAM_TYPE_2DMAIN,
    .resources = { 0x4E2F, 0x4E2A, 0x4E27, 0x4E27, 0xffffffff, 0xffffffff },
    .bgPriority = 2,
    .vramTransfer = TRUE,
};

static const int Unk_020F0588[] = {
    0x0,
    0x0,
    0x1,
    0x0,
    0x0,
    0x1
};

ManagedSprite *SpriteSystem_NewManagedSpriteTrainer(SpriteSystem *spriteSys, SpriteManager *spriteMan, PaletteData *paletteData, int x, int y, enum TrainerClass trainerClass, int face, int battlerType, enum HeapID heapID)
{
    SpriteTemplate spriteTemplate;
    ManagedSprite *managedSprite;
    NARC *narc;
    TrainerClassGraphicIndex trainerClassGraphicIndex;
    int paletteIdx = 1;

    SpriteSystem_SetTrainerClassGraphicsIndex(trainerClass, face, &trainerClassGraphicIndex);

    if (trainerClass == TRAINER_CLASS_CASTLE_VALET) {
        paletteIdx = 2;
    }

    narc = NARC_ctor(trainerClassGraphicIndex.narcID, heapID);

    SpriteSystem_LoadCharResObjFromOpenNarc(spriteSys, spriteMan, narc, trainerClassGraphicIndex.tiles, FALSE, NNS_G2D_VRAM_TYPE_2DMAIN, 20015 + battlerType);
    SpriteSystem_LoadPaletteBufferFromOpenNarc(paletteData, PLTTBUF_MAIN_OBJ, spriteSys, spriteMan, narc, trainerClassGraphicIndex.palette, FALSE, paletteIdx, NNS_G2D_VRAM_TYPE_2DMAIN, 20010 + battlerType);
    SpriteSystem_LoadCellResObjFromOpenNarc(spriteSys, spriteMan, narc, trainerClassGraphicIndex.cells, FALSE, 20007 + battlerType);
    SpriteSystem_LoadAnimResObjFromOpenNarc(spriteSys, spriteMan, narc, trainerClassGraphicIndex.anims, FALSE, 20007 + battlerType);
    NARC_dtor(narc);

    spriteTemplate = sSpriteTemplateTrainer;

    // TODO enum values?
    spriteTemplate.resources[0] = 20015 + battlerType;
    spriteTemplate.resources[1] = 20010 + battlerType;
    spriteTemplate.resources[2] = 20007 + battlerType;
    spriteTemplate.resources[3] = 20007 + battlerType;
    spriteTemplate.priority = Unk_020F0588[battlerType];

    managedSprite = SpriteSystem_NewSprite(spriteSys, spriteMan, &spriteTemplate);

    Sprite_SetExplicitPaletteOffsetAutoAdjust(managedSprite->sprite, 0);
    ManagedSprite_SetPositionXY(managedSprite, x, y);
    ManagedSprite_TickFrame(managedSprite);
    ManagedSprite_SetAnimateFlag(managedSprite, TRUE);

    return managedSprite;
}

void SpriteSystem_SetTrainerClassGraphicsIndex(enum TrainerClass trainerClass, int face, TrainerClassGraphicIndex *trainerClassGraphicIndex)
{
    if (face == FACE_FRONT) {
        trainerClassGraphicIndex->narcID = NARC_INDEX_POKETOOL__TRGRA__TRFGRA;
        trainerClassGraphicIndex->tiles = TCFT_TILES + trainerClass * TRAINER_CLASS_NUM_FILETYPES;
        trainerClassGraphicIndex->palette = TCFT_PALETTE + trainerClass * TRAINER_CLASS_NUM_FILETYPES;
        trainerClassGraphicIndex->cells = TCFT_CELLS + trainerClass * TRAINER_CLASS_NUM_FILETYPES;
        trainerClassGraphicIndex->anims = TCFT_ANIMS + trainerClass * TRAINER_CLASS_NUM_FILETYPES;
        trainerClassGraphicIndex->scan = TCFT_SCAN + trainerClass * TRAINER_CLASS_NUM_FILETYPES;
    } else {
        trainerClassGraphicIndex->narcID = NARC_INDEX_POKETOOL__TRGRA__TRBGRA;
        trainerClass = SpriteSystem_TrainerClassBackSpriteIndex(trainerClass);
        trainerClassGraphicIndex->tiles = TCFT_TILES + trainerClass * TRAINER_CLASS_NUM_FILETYPES;
        trainerClassGraphicIndex->palette = TCFT_PALETTE + trainerClass * TRAINER_CLASS_NUM_FILETYPES;
        trainerClassGraphicIndex->cells = TCFT_CELLS + trainerClass * TRAINER_CLASS_NUM_FILETYPES;
        trainerClassGraphicIndex->anims = TCFT_ANIMS + trainerClass * TRAINER_CLASS_NUM_FILETYPES;
        trainerClassGraphicIndex->scan = TCFT_SCAN + trainerClass * TRAINER_CLASS_NUM_FILETYPES;
    }
}

u32 Pokemon_GetStructSize(void)
{
    return (u32)sizeof(Pokemon);
}

u32 BoxPokemon_GetStructSize(void)
{
    return (u32)sizeof(BoxPokemon);
}

u8 Pokemon_GetForm(Pokemon *mon)
{
    return BoxPokemon_GetForm(&mon->box);
}

u8 BoxPokemon_GetForm(BoxPokemon *boxMon)
{
    return BoxPokemon_GetValue(boxMon, MON_DATA_FORM, NULL);
}

BoxPokemon *Pokemon_GetBoxPokemon(Pokemon *mon)
{
    return &mon->box;
}

BOOL Pokemon_ShouldLevelUp(Pokemon *mon)
{
    u16 monSpecies = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
    u8 monNextLevel = Pokemon_GetValue(mon, MON_DATA_LEVEL, NULL) + 1;
    u32 monExp = Pokemon_GetValue(mon, MON_DATA_EXPERIENCE, NULL);
    int monExpRate = SpeciesData_GetSpeciesValue(monSpecies, SPECIES_DATA_EXP_RATE);
    u32 maxExp = Pokemon_GetExpRateBaseExpAt(monExpRate, MAX_POKEMON_LEVEL);

    if (monExp > maxExp) {
        monExp = maxExp;
        Pokemon_SetValue(mon, MON_DATA_EXPERIENCE, &monExp);
    }

    if (monNextLevel > MAX_POKEMON_LEVEL) {
        return FALSE;
    }

    maxExp = Pokemon_GetExpRateBaseExpAt(monExpRate, monNextLevel);

    if (monExp >= maxExp) {
        Pokemon_SetValue(mon, MON_DATA_LEVEL, &monNextLevel);
        return TRUE;
    }

    return FALSE;
}

u16 Pokemon_GetEvolutionTargetSpecies(Party *party, Pokemon *mon, u8 evoClass, u16 evoParam, int *evoTypeResult)
{
    u16 targetSpecies = SPECIES_NONE;

    u16 monSpecies = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
    u16 monHeldItem = Pokemon_GetValue(mon, MON_DATA_HELD_ITEM, NULL);
    u32 monPersonality = Pokemon_GetValue(mon, MON_DATA_PERSONALITY, NULL);
    u8 monBeauty = Pokemon_GetValue(mon, MON_DATA_BEAUTY, NULL);

    int i;
    u16 monFriendship;

    u16 monPersonalityUpper = (monPersonality & 0xFFFF0000) >> 16;
    u8 itemHoldEffect = Item_LoadParam(monHeldItem, ITEM_PARAM_HOLD_EFFECT, HEAP_ID_SYSTEM);

    if (monSpecies != SPECIES_KADABRA
        && itemHoldEffect == HOLD_EFFECT_NO_EVOLVE
        && evoClass != EVO_CLASS_BY_ITEM) {
        return SPECIES_NONE;
    }

    int stackVar;
    if (evoTypeResult == NULL) {
        evoTypeResult = &stackVar;
    }

    SpeciesEvolution *speciesEvolutions = Heap_Alloc(HEAP_ID_SYSTEM, sizeof(SpeciesEvolution) * MAX_EVOLUTIONS);
    LoadSpeciesEvolutions(monSpecies, speciesEvolutions);

    switch (evoClass) {
    case EVO_CLASS_BY_LEVEL: {
        u8 monLevel = Pokemon_GetValue(mon, MON_DATA_LEVEL, NULL);
        monFriendship = Pokemon_GetValue(mon, MON_DATA_FRIENDSHIP, NULL);

        for (i = 0; i < MAX_EVOLUTIONS; i++) {
            switch (speciesEvolutions[i].method) {
            case EVO_LEVEL_HAPPINESS:
                if (EVOLVE_FRIENDSHIP_THRESHOLD <= monFriendship) {
                    targetSpecies = speciesEvolutions[i].targetSpecies;
                    *evoTypeResult = EVO_LEVEL_HAPPINESS;
                }
                break;

            case EVO_LEVEL_HAPPINESS_DAY:
                if (IsNight() == FALSE && EVOLVE_FRIENDSHIP_THRESHOLD <= monFriendship) {
                    targetSpecies = speciesEvolutions[i].targetSpecies;
                    *evoTypeResult = EVO_LEVEL_HAPPINESS_DAY;
                }
                break;

            case EVO_LEVEL_HAPPINESS_NIGHT:
                if (IsNight() == TRUE && EVOLVE_FRIENDSHIP_THRESHOLD <= monFriendship) {
                    targetSpecies = speciesEvolutions[i].targetSpecies;
                    *evoTypeResult = EVO_LEVEL_HAPPINESS_NIGHT;
                }
                break;

            case EVO_LEVEL:
                if (speciesEvolutions[i].param <= monLevel) {
                    targetSpecies = speciesEvolutions[i].targetSpecies;
                    *evoTypeResult = EVO_LEVEL;
                }
                break;

            case EVO_LEVEL_ATK_GT_DEF:
                if (speciesEvolutions[i].param <= monLevel) {
                    if (Pokemon_GetValue(mon, MON_DATA_ATK, NULL) > Pokemon_GetValue(mon, MON_DATA_DEF, NULL)) {
                        targetSpecies = speciesEvolutions[i].targetSpecies;
                        *evoTypeResult = EVO_LEVEL_ATK_GT_DEF;
                    }
                }
                break;

            case EVO_LEVEL_ATK_EQ_DEF:
                if (speciesEvolutions[i].param <= monLevel) {
                    if (Pokemon_GetValue(mon, MON_DATA_ATK, NULL) == Pokemon_GetValue(mon, MON_DATA_DEF, NULL)) {
                        targetSpecies = speciesEvolutions[i].targetSpecies;
                        *evoTypeResult = EVO_LEVEL_ATK_EQ_DEF;
                    }
                }
                break;

            case EVO_LEVEL_ATK_LT_DEF:
                if (speciesEvolutions[i].param <= monLevel) {
                    if (Pokemon_GetValue(mon, MON_DATA_ATK, NULL) < Pokemon_GetValue(mon, MON_DATA_DEF, NULL)) {
                        targetSpecies = speciesEvolutions[i].targetSpecies;
                        *evoTypeResult = EVO_LEVEL_ATK_LT_DEF;
                    }
                }
                break;

            case EVO_LEVEL_PID_LOW:
                if (speciesEvolutions[i].param <= monLevel) {
                    if (monPersonalityUpper % 10 < 5) {
                        targetSpecies = speciesEvolutions[i].targetSpecies;
                        *evoTypeResult = EVO_LEVEL_PID_LOW;
                    }
                }
                break;

            case EVO_LEVEL_PID_HIGH:
                if (speciesEvolutions[i].param <= monLevel) {
                    if (monPersonalityUpper % 10 >= 5) {
                        targetSpecies = speciesEvolutions[i].targetSpecies;
                        *evoTypeResult = EVO_LEVEL_PID_HIGH;
                    }
                }
                break;

            case EVO_LEVEL_NINJASK:
                if (speciesEvolutions[i].param <= monLevel) {
                    targetSpecies = speciesEvolutions[i].targetSpecies;
                    *evoTypeResult = EVO_LEVEL_NINJASK;
                }
                break;

            case EVO_LEVEL_SHEDINJA:
                *evoTypeResult = EVO_LEVEL_SHEDINJA;
                break;

            case EVO_LEVEL_BEAUTY:
                if (speciesEvolutions[i].param <= monBeauty) {
                    targetSpecies = speciesEvolutions[i].targetSpecies;
                    *evoTypeResult = EVO_LEVEL_BEAUTY;
                }
                break;

            case EVO_LEVEL_WITH_HELD_ITEM_DAY:
                if (IsNight() == FALSE && speciesEvolutions[i].param == monHeldItem) {
                    targetSpecies = speciesEvolutions[i].targetSpecies;
                    *evoTypeResult = EVO_LEVEL_WITH_HELD_ITEM_DAY;
                }
                break;

            case EVO_LEVEL_WITH_HELD_ITEM_NIGHT:
                if (IsNight() == TRUE && speciesEvolutions[i].param == monHeldItem) {
                    targetSpecies = speciesEvolutions[i].targetSpecies;
                    *evoTypeResult = EVO_LEVEL_WITH_HELD_ITEM_NIGHT;
                }
                break;

            case EVO_LEVEL_KNOW_MOVE:
                if (Pokemon_HasMove(mon, speciesEvolutions[i].param) == TRUE) {
                    targetSpecies = speciesEvolutions[i].targetSpecies;
                    *evoTypeResult = EVO_LEVEL_KNOW_MOVE;
                }
                break;

            case EVO_LEVEL_SPECIES_IN_PARTY:
                if (party != NULL) {
                    if (Party_HasSpecies(party, speciesEvolutions[i].param) == TRUE) {
                        targetSpecies = speciesEvolutions[i].targetSpecies;
                        *evoTypeResult = EVO_LEVEL_SPECIES_IN_PARTY;
                    }
                }
                break;

            case EVO_LEVEL_MALE:
                if (Pokemon_GetValue(mon, MON_DATA_GENDER, NULL) == GENDER_MALE && speciesEvolutions[i].param <= monLevel) {
                    targetSpecies = speciesEvolutions[i].targetSpecies;
                    *evoTypeResult = EVO_LEVEL_MALE;
                }
                break;

            case EVO_LEVEL_FEMALE:
                if (Pokemon_GetValue(mon, MON_DATA_GENDER, NULL) == GENDER_FEMALE && speciesEvolutions[i].param <= monLevel) {
                    targetSpecies = speciesEvolutions[i].targetSpecies;
                    *evoTypeResult = EVO_LEVEL_FEMALE;
                }
                break;

            case EVO_LEVEL_MAGNETIC_FIELD:
                if (speciesEvolutions[i].method == evoParam) {
                    targetSpecies = speciesEvolutions[i].targetSpecies;
                    *evoTypeResult = EVO_LEVEL_MAGNETIC_FIELD;
                }
                break;

            case EVO_LEVEL_MOSS_ROCK:
                if (speciesEvolutions[i].method == evoParam) {
                    targetSpecies = speciesEvolutions[i].targetSpecies;
                    *evoTypeResult = EVO_LEVEL_MOSS_ROCK;
                }
                break;

            case EVO_LEVEL_ICE_ROCK:
                if (speciesEvolutions[i].method == evoParam) {
                    targetSpecies = speciesEvolutions[i].targetSpecies;
                    *evoTypeResult = EVO_LEVEL_ICE_ROCK;
                }
                break;
            }

            if (targetSpecies) {
                break;
            }
        }
    } break;

    case EVO_CLASS_BY_TRADE:
        for (i = 0; i < MAX_EVOLUTIONS; i++) {
            switch (speciesEvolutions[i].method) {
            case EVO_TRADE:
                targetSpecies = speciesEvolutions[i].targetSpecies;
                *evoTypeResult = EVO_TRADE;
                break;

            case EVO_TRADE_WITH_HELD_ITEM:
                if (speciesEvolutions[i].param == monHeldItem) {
                    targetSpecies = speciesEvolutions[i].targetSpecies;
                    *evoTypeResult = EVO_TRADE_WITH_HELD_ITEM;
                }
                break;
            }

            if (targetSpecies) {
                break;
            }
        }
        break;

    case EVO_CLASS_UNUSED_02:
    case EVO_CLASS_BY_ITEM:
        for (i = 0; i < MAX_EVOLUTIONS; i++) {
            if (speciesEvolutions[i].method == EVO_USE_ITEM && speciesEvolutions[i].param == evoParam) {
                targetSpecies = speciesEvolutions[i].targetSpecies;
                *evoTypeResult = EVO_NONE;
                break;
            }

            if (speciesEvolutions[i].method == EVO_USE_ITEM_MALE
                && Pokemon_GetValue(mon, MON_DATA_GENDER, NULL) == GENDER_MALE
                && speciesEvolutions[i].param == evoParam) {
                targetSpecies = speciesEvolutions[i].targetSpecies;
                *evoTypeResult = EVO_NONE;
                break;
            }

            if (speciesEvolutions[i].method == EVO_USE_ITEM_FEMALE
                && Pokemon_GetValue(mon, MON_DATA_GENDER, NULL) == GENDER_FEMALE
                && speciesEvolutions[i].param == evoParam) {
                targetSpecies = speciesEvolutions[i].targetSpecies;
                *evoTypeResult = EVO_NONE;
                break;
            }

            if (targetSpecies) {
                break;
            }
        }
        break;
    }

    Heap_Free(speciesEvolutions);
    return targetSpecies;
}

u16 Pokemon_GetBaseSpeciesFromPersonalData(const u16 species)
{
    u16 result = 0;
    GF_ASSERT(NATIONAL_DEX_COUNT + 1 > species);

    FSFile file;
    FS_InitFile(&file);
    FS_OpenFile(&file, "poketool/personal/pms.narc");
    FS_SeekFile(&file, species * 2, FS_SEEK_SET);
    FS_ReadFile(&file, &result, 2);
    FS_CloseFile(&file);

    return result;
}

u16 Pokemon_GetBaseSpeciesForBattle(const u16 species)
{
    switch (species) {
    case SPECIES_WOBBUFFET:
    case SPECIES_MARILL:
    case SPECIES_MR_MIME:
    case SPECIES_SUDOWOODO:
    case SPECIES_SNORLAX:
    case SPECIES_MANTINE:
    case SPECIES_ROSELIA:
    case SPECIES_CHANSEY:
    case SPECIES_CHIMECHO:
        return species;
        break;
    }

    return Pokemon_GetBaseSpeciesFromPersonalData(species);
}

// ENTRY P108: PER-TYPE LEVEL-UP MOVES (user's "moves per type.txt", 2026-10-16). On top of its species learnset, a Pokemon
// learns these moves at these levels for each of its two types (MON_DATA_TYPE_1/2 - the effective types, custom type and
// Multitype included). Checked at every level-up (Pokemon_LevelUpMove: battle EXP, Rare Candy, evolution, Day Care) after
// the species moves, and merged into the default moveset (BoxPokemon_SetDefaultMoves: wild Pokemon, trainer Pokemon
// without a custom moveset), in level order, species move first at the same level. Ditto gets none (Transform only).
typedef struct TypeLevelUpMove {
    u8 level;
    u16 move;
} TypeLevelUpMove;

static const TypeLevelUpMove sTypeMovesNormal[] = { { 6, MOVE_QUICK_ATTACK }, { 12, MOVE_PAY_DAY }, { 18, MOVE_UPROAR }, { 20, MOVE_MEGA_PUNCH }, { 25, MOVE_FOCUS_ENERGY }, { 30, MOVE_SWIFT }, { 33, MOVE_PAY_DAY }, { 35, MOVE_MEGA_KICK }, { 40, MOVE_YAWN }, { 49, MOVE_BODY_SLAM }, { 50, MOVE_SKULL_BASH }, { 60, MOVE_THRASH }, { 0, MOVE_NONE } };
static const TypeLevelUpMove sTypeMovesFighting[] = { { 6, MOVE_VACUUM_WAVE }, { 12, MOVE_SEISMIC_TOSS }, { 20, MOVE_MACH_PUNCH }, { 28, MOVE_SUBMISSION }, { 35, MOVE_SKY_UPPERCUT }, { 40, MOVE_REVENGE }, { 45, MOVE_CLOSE_COMBAT }, { 55, MOVE_FOCUS_BLAST }, { 0, MOVE_NONE } };
static const TypeLevelUpMove sTypeMovesFlying[] = { { 4, MOVE_PECK }, { 10, MOVE_GUST }, { 14, MOVE_BOUNCE }, { 22, MOVE_AERIAL_ACE }, { 28, MOVE_AIR_CUTTER }, { 35, MOVE_TAILWIND }, { 44, MOVE_AIR_SLASH }, { 54, MOVE_SKY_ATTACK }, { 0, MOVE_NONE } };
static const TypeLevelUpMove sTypeMovesPoison[] = { { 2, MOVE_POISON_STING }, { 9, MOVE_ACID }, { 15, MOVE_SLUDGE }, { 20, MOVE_POISON_FANG }, { 25, MOVE_TOXIC_SPIKES }, { 28, MOVE_TOXIC }, { 36, MOVE_CROSS_POISON }, { 50, MOVE_GUNK_SHOT }, { 0, MOVE_NONE } };
static const TypeLevelUpMove sTypeMovesGround[] = { { 6, MOVE_MUD_SLAP }, { 12, MOVE_MUD_SHOT }, { 20, MOVE_MAGNITUDE }, { 22, MOVE_SPIKES }, { 25, MOVE_DIG }, { 35, MOVE_MUD_BOMB }, { 44, MOVE_EARTH_POWER }, { 45, MOVE_EARTHQUAKE }, { 0, MOVE_NONE } };
static const TypeLevelUpMove sTypeMovesRock[] = { { 10, MOVE_ROCK_THROW }, { 15, MOVE_ROCK_TOMB }, { 20, MOVE_STEALTH_ROCK }, { 22, MOVE_POWER_GEM }, { 30, MOVE_ROCK_BLAST }, { 35, MOVE_ROCK_SLIDE }, { 49, MOVE_STONE_EDGE }, { 60, MOVE_ROCK_WRECKER }, { 0, MOVE_NONE } };
static const TypeLevelUpMove sTypeMovesBug[] = { { 6, MOVE_LEECH_LIFE }, { 10, MOVE_FURY_CUTTER }, { 15, MOVE_BUG_BITE }, { 25, MOVE_PIN_MISSILE }, { 30, MOVE_SIGNAL_BEAM }, { 42, MOVE_X_SCISSOR }, { 50, MOVE_BUG_BUZZ }, { 0, MOVE_NONE } };
static const TypeLevelUpMove sTypeMovesGhost[] = { { 6, MOVE_ASTONISH }, { 10, MOVE_SHADOW_SNEAK }, { 15, MOVE_NIGHT_SHADE }, { 18, MOVE_CURSE }, { 20, MOVE_SHADOW_PUNCH }, { 28, MOVE_SHADOW_BALL }, { 40, MOVE_DESTINY_BOND }, { 45, MOVE_OMINOUS_WIND }, { 0, MOVE_NONE } };
static const TypeLevelUpMove sTypeMovesSteel[] = { { 6, MOVE_METAL_BURST }, { 14, MOVE_BULLET_PUNCH }, { 24, MOVE_MAGNET_BOMB }, { 30, MOVE_IRON_DEFENSE }, { 35, MOVE_FLASH_CANNON }, { 45, MOVE_IRON_HEAD }, { 60, MOVE_GYRO_BALL }, { 0, MOVE_NONE } };
static const TypeLevelUpMove sTypeMovesFire[] = { { 6, MOVE_FIRE_SPIN }, { 9, MOVE_EMBER }, { 15, MOVE_FLAME_WHEEL }, { 30, MOVE_HEAT_WAVE }, { 40, MOVE_FLARE_BLITZ }, { 45, MOVE_FLAMETHROWER }, { 50, MOVE_FIRE_BLAST }, { 0, MOVE_NONE } };
static const TypeLevelUpMove sTypeMovesWater[] = { { 2, MOVE_BUBBLE }, { 6, MOVE_BUBBLE }, { 10, MOVE_AQUA_JET }, { 19, MOVE_BUBBLE_BEAM }, { 25, MOVE_RAIN_DANCE }, { 30, MOVE_AQUA_RING }, { 35, MOVE_MUDDY_WATER }, { 48, MOVE_HYDRO_PUMP }, { 0, MOVE_NONE } };
static const TypeLevelUpMove sTypeMovesGrass[] = { { 8, MOVE_ABSORB }, { 12, MOVE_VINE_WHIP }, { 16, MOVE_LEECH_SEED }, { 20, MOVE_SYNTHESIS }, { 24, MOVE_MAGICAL_LEAF }, { 30, MOVE_MEGA_DRAIN }, { 38, MOVE_RAZOR_LEAF }, { 45, MOVE_GIGA_DRAIN }, { 50, MOVE_SOLAR_BEAM }, { 55, MOVE_LEAF_STORM }, { 0, MOVE_NONE } };
static const TypeLevelUpMove sTypeMovesElectric[] = { { 6, MOVE_THUNDER_SHOCK }, { 15, MOVE_CHARGE_BEAM }, { 20, MOVE_SPARK }, { 30, MOVE_DISCHARGE }, { 40, MOVE_THUNDERBOLT }, { 50, MOVE_ZAP_CANNON }, { 55, MOVE_THUNDER }, { 0, MOVE_NONE } };
static const TypeLevelUpMove sTypeMovesPsychic[] = { { 8, MOVE_TELEPORT }, { 14, MOVE_CONFUSION }, { 20, MOVE_PSYBEAM }, { 30, MOVE_SKILL_SWAP }, { 34, MOVE_PSYCHO_CUT }, { 35, MOVE_CALM_MIND }, { 44, MOVE_MIST_BALL }, { 50, MOVE_PSYCHIC }, { 53, MOVE_FUTURE_SIGHT }, { 0, MOVE_NONE } };
static const TypeLevelUpMove sTypeMovesIce[] = { { 8, MOVE_POWDER_SNOW }, { 14, MOVE_ICE_SHARD }, { 25, MOVE_ICY_WIND }, { 40, MOVE_AURORA_BEAM }, { 45, MOVE_ICE_BEAM }, { 50, MOVE_ICICLE_SPEAR }, { 60, MOVE_BLIZZARD }, { 90, MOVE_SHEER_COLD }, { 0, MOVE_NONE } };
static const TypeLevelUpMove sTypeMovesDragon[] = { { 14, MOVE_TWISTER }, { 20, MOVE_DRAGON_RAGE }, { 28, MOVE_DRAGON_DANCE }, { 32, MOVE_DRAGON_BREATH }, { 44, MOVE_DRAGON_CLAW }, { 50, MOVE_DRAGON_PULSE }, { 65, MOVE_OUTRAGE }, { 0, MOVE_NONE } };
static const TypeLevelUpMove sTypeMovesDark[] = { { 8, MOVE_PURSUIT }, { 12, MOVE_THIEF }, { 20, MOVE_PAYBACK }, { 25, MOVE_NASTY_PLOT }, { 30, MOVE_FAINT_ATTACK }, { 40, MOVE_NIGHT_SLASH }, { 50, MOVE_DARK_PULSE }, { 60, MOVE_SUCKER_PUNCH }, { 0, MOVE_NONE } };

static const TypeLevelUpMove *GetTypeLevelUpMoves(u32 type)
{
    switch (type) {
    case TYPE_NORMAL:
        return sTypeMovesNormal;
    case TYPE_FIGHTING:
        return sTypeMovesFighting;
    case TYPE_FLYING:
        return sTypeMovesFlying;
    case TYPE_POISON:
        return sTypeMovesPoison;
    case TYPE_GROUND:
        return sTypeMovesGround;
    case TYPE_ROCK:
        return sTypeMovesRock;
    case TYPE_BUG:
        return sTypeMovesBug;
    case TYPE_GHOST:
        return sTypeMovesGhost;
    case TYPE_STEEL:
        return sTypeMovesSteel;
    case TYPE_FIRE:
        return sTypeMovesFire;
    case TYPE_WATER:
        return sTypeMovesWater;
    case TYPE_GRASS:
        return sTypeMovesGrass;
    case TYPE_ELECTRIC:
        return sTypeMovesElectric;
    case TYPE_PSYCHIC:
        return sTypeMovesPsychic;
    case TYPE_ICE:
        return sTypeMovesIce;
    case TYPE_DRAGON:
        return sTypeMovesDragon;
    case TYPE_DARK:
        return sTypeMovesDark;
    }

    return NULL;
}

// ENTRY P109: TRUE if the move is in that type's per-type level-up list (the trainer AI tells from it which type taught it).
BOOL Pokemon_IsTypeLevelUpMove(u32 type, u16 move)
{
    const TypeLevelUpMove *moves = GetTypeLevelUpMoves(type);

    if (moves == NULL) {
        return FALSE;
    }

    for (int i = 0; moves[i].move != MOVE_NONE; i++) {
        if (moves[i].move == move) {
            return TRUE;
        }
    }

    return FALSE;
}

// ENTRY P109: TRUE if the species (form) learns the move by level-up.
BOOL Pokemon_LevelUpLearnsetHasMove(int monSpecies, int monForm, u16 move)
{
    u16 *monLevelUpMoves = Heap_Alloc(HEAP_ID_SYSTEM, sizeof(SpeciesLearnset));
    BOOL result = FALSE;

    Pokemon_LoadLevelUpMovesOf(monSpecies, monForm, monLevelUpMoves);

    for (int i = 0; monLevelUpMoves[i] != LEARNSET_SENTINEL_ENTRY; i++) {
        if ((monLevelUpMoves[i] & 0x1FF) == move) {
            result = TRUE;
            break;
        }
    }

    Heap_Free(monLevelUpMoves);
    return result;
}

// The (up to two) per-type move lists of this Pokemon: its second type only if it differs from the first. Ditto has none.
static int BoxPokemon_GetTypeLevelUpMoveLists(BoxPokemon *boxMon, const TypeLevelUpMove **lists)
{
    int count = 0;

    if (BoxPokemon_GetValue(boxMon, MON_DATA_SPECIES, NULL) == SPECIES_DITTO) {
        return 0;
    }

    u32 type1 = BoxPokemon_GetValue(boxMon, MON_DATA_TYPE_1, NULL);
    u32 type2 = BoxPokemon_GetValue(boxMon, MON_DATA_TYPE_2, NULL);

    if ((lists[count] = GetTypeLevelUpMoves(type1)) != NULL) {
        count++;
    }

    if (type2 != type1 && (lists[count] = GetTypeLevelUpMoves(type2)) != NULL) {
        count++;
    }

    return count;
}

static void BoxPokemon_SetDefaultMoves(BoxPokemon *boxMon)
{
    BOOL reencrypt; // must pre-declare to match
    u16 *monLevelUpMoves = Heap_Alloc(HEAP_ID_SYSTEM, sizeof(SpeciesLearnset));
    reencrypt = BoxPokemon_EnterDecryptionContext(boxMon);

    u16 monSpecies = BoxPokemon_GetValue(boxMon, MON_DATA_SPECIES, NULL);
    int monForm = BoxPokemon_GetValue(boxMon, MON_DATA_FORM, NULL);
    u8 monLevel = BoxPokemon_GetLevel(boxMon);

    Pokemon_LoadLevelUpMovesOf(monSpecies, monForm, monLevelUpMoves);

    // ENTRY P108: species moves and per-type moves together, level by level (species first at the same level); with all
    // four slots used the oldest move is pushed out, as before.
    const TypeLevelUpMove *typeLists[2];
    int numTypeLists = BoxPokemon_GetTypeLevelUpMoveLists(boxMon, typeLists);

    for (int level = 0; level <= monLevel; level++) {
        for (int i = 0; monLevelUpMoves[i] != LEARNSET_SENTINEL_ENTRY; i++) {
            if ((monLevelUpMoves[i] & 0xFE00) == level << 9) {
                u16 monLevelUpMoveID = monLevelUpMoves[i] & 0x1FF;
                if (BoxPokemon_AddMove(boxMon, monLevelUpMoveID) == LEARNSET_ALL_SLOTS_FILLED) {
                    BoxPokemon_ReplaceMove(boxMon, monLevelUpMoveID);
                }
            }
        }

        for (int t = 0; t < numTypeLists; t++) {
            for (int i = 0; typeLists[t][i].move != MOVE_NONE; i++) {
                if (typeLists[t][i].level == level && BoxPokemon_AddMove(boxMon, typeLists[t][i].move) == LEARNSET_ALL_SLOTS_FILLED) {
                    BoxPokemon_ReplaceMove(boxMon, typeLists[t][i].move);
                }
            }
        }
    }

    Heap_Free(monLevelUpMoves);
    BoxPokemon_ExitDecryptionContext(boxMon, reencrypt);
}

u16 Pokemon_AddMove(Pokemon *mon, u16 moveID)
{
    BoxPokemon *boxMon = Pokemon_GetBoxPokemon(mon);
    return BoxPokemon_AddMove(boxMon, moveID);
}

static u16 BoxPokemon_AddMove(BoxPokemon *boxMon, u16 moveID)
{
    u16 result = LEARNSET_ALL_SLOTS_FILLED;
    BOOL reencrypt = BoxPokemon_EnterDecryptionContext(boxMon);

    for (int i = 0; i < LEARNED_MOVES_MAX; i++) {
        u16 slotMove;
        if ((slotMove = BoxPokemon_GetValue(boxMon, MON_DATA_MOVE1 + i, NULL)) == MOVE_NONE) {
            BoxPokemon_SetMoveSlot(boxMon, moveID, i);
            result = moveID;
            break;
        } else {
            if (slotMove == moveID) {
                result = LEARNSET_MOVE_ALREADY_KNOWN;
                break;
            }
        }
    }

    BoxPokemon_ExitDecryptionContext(boxMon, reencrypt);
    return result;
}

void Pokemon_ReplaceMove(Pokemon *mon, u16 moveID)
{
    BoxPokemon *boxMon = Pokemon_GetBoxPokemon(mon);
    BoxPokemon_ReplaceMove(boxMon, moveID);
}

static void BoxPokemon_ReplaceMove(BoxPokemon *boxMon, u16 moveID)
{
    BOOL reencrypt = BoxPokemon_EnterDecryptionContext(boxMon);

    u16 moveIDs[LEARNED_MOVES_MAX];
    u8 movePPs[LEARNED_MOVES_MAX];
    u8 movePPUps[LEARNED_MOVES_MAX];

    // Bubble move slots 2 through 4 upwards
    for (int i = 0; i < LEARNED_MOVES_MAX - 1; i++) {
        moveIDs[i] = BoxPokemon_GetValue(boxMon, MON_DATA_MOVE2 + i, NULL);
        movePPs[i] = BoxPokemon_GetValue(boxMon, MON_DATA_MOVE2_PP + i, NULL);
        movePPUps[i] = BoxPokemon_GetValue(boxMon, MON_DATA_MOVE2_PP_UPS + i, NULL);
    }

    moveIDs[LEARNED_MOVES_MAX - 1] = moveID;
    movePPs[LEARNED_MOVES_MAX - 1] = MoveTable_LoadParam(moveID, MOVEATTRIBUTE_PP);
    movePPUps[LEARNED_MOVES_MAX - 1] = 0;

    for (int i = 0; i < LEARNED_MOVES_MAX; i++) {
        BoxPokemon_SetValue(boxMon, MON_DATA_MOVE1 + i, &moveIDs[i]);
        BoxPokemon_SetValue(boxMon, MON_DATA_MOVE1_PP + i, &movePPs[i]);
        BoxPokemon_SetValue(boxMon, MON_DATA_MOVE1_PP_UPS + i, &movePPUps[i]);
    }

    BoxPokemon_ExitDecryptionContext(boxMon, reencrypt);
}

void Pokemon_ResetMoveSlot(Pokemon *mon, u16 moveID, u8 moveSlot)
{
    Pokemon_SetMoveSlot(mon, moveID, moveSlot);

    u32 moveMaxPP, movePPUps = 0;
    Pokemon_SetValue(mon, MON_DATA_MOVE1_PP_UPS + moveSlot, &movePPUps);

    moveMaxPP = MoveTable_CalcMaxPP(moveID, 0);
    Pokemon_SetValue(mon, MON_DATA_MOVE1_PP + moveSlot, &moveMaxPP);
}

void Pokemon_SetMoveSlot(Pokemon *mon, u16 moveID, u8 moveSlot)
{
    BoxPokemon_SetMoveSlot(&mon->box, moveID, moveSlot);
}

static void BoxPokemon_SetMoveSlot(BoxPokemon *boxMon, u16 moveID, u8 moveSlot)
{
    BoxPokemon_SetValue(boxMon, MON_DATA_MOVE1 + moveSlot, &moveID);

    u8 movePPUps = BoxPokemon_GetValue(boxMon, MON_DATA_MOVE1_PP_UPS + moveSlot, NULL);
    u8 moveMaxPP = MoveTable_CalcMaxPP(moveID, movePPUps);

    BoxPokemon_SetValue(boxMon, MON_DATA_MOVE1_PP + moveSlot, &moveMaxPP);
}

// ENTRY P108: indices from here on walk the per-type moves (first type's list, then the second's) after the species moves.
#define TYPE_LEVEL_UP_MOVE_INDEX_BASE 0x400

static u16 Pokemon_TypeLevelUpMove(Pokemon *mon, int *index, u16 *moveID, u8 monLevel)
{
    const TypeLevelUpMove *typeLists[2];
    int numTypeLists = BoxPokemon_GetTypeLevelUpMoveLists(&mon->box, typeLists);
    int pos = 0;

    for (int t = 0; t < numTypeLists; t++) {
        for (int i = 0; typeLists[t][i].move != MOVE_NONE; i++, pos++) {
            if (pos < *index - TYPE_LEVEL_UP_MOVE_INDEX_BASE || typeLists[t][i].level != monLevel) {
                continue;
            }

            *moveID = typeLists[t][i].move;
            *index = TYPE_LEVEL_UP_MOVE_INDEX_BASE + pos + 1;
            return Pokemon_AddMove(mon, *moveID);
        }
    }

    *index = TYPE_LEVEL_UP_MOVE_INDEX_BASE + pos;
    return MOVE_NONE;
}

u16 Pokemon_LevelUpMove(Pokemon *mon, int *index, u16 *moveID)
{
    u16 result = MOVE_NONE;
    u8 monLevel = Pokemon_GetValue(mon, MON_DATA_LEVEL, NULL);

    if (*index >= TYPE_LEVEL_UP_MOVE_INDEX_BASE) {
        return Pokemon_TypeLevelUpMove(mon, index, moveID, monLevel);
    }

    u16 *monLevelUpMoves = Heap_Alloc(HEAP_ID_SYSTEM, sizeof(SpeciesLearnset));
    u16 monSpecies = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
    int monForm = Pokemon_GetValue(mon, MON_DATA_FORM, NULL);

    Pokemon_LoadLevelUpMovesOf(monSpecies, monForm, monLevelUpMoves);

    if (monLevelUpMoves[*index] == LEARNSET_SENTINEL_ENTRY) {
        Heap_Free(monLevelUpMoves);
        *index = TYPE_LEVEL_UP_MOVE_INDEX_BASE; // ENTRY P108: species moves done, now the type moves
        return Pokemon_TypeLevelUpMove(mon, index, moveID, monLevel);
    }

    while ((monLevelUpMoves[*index] & 0xFE00) != monLevel << 9) {
        (*index)++;
        if (monLevelUpMoves[*index] == LEARNSET_SENTINEL_ENTRY) {
            Heap_Free(monLevelUpMoves);
            *index = TYPE_LEVEL_UP_MOVE_INDEX_BASE; // ENTRY P108
            return Pokemon_TypeLevelUpMove(mon, index, moveID, monLevel);
        }
    }

    if ((monLevelUpMoves[*index] & 0xFE00) == monLevel << 9) {
        *moveID = monLevelUpMoves[*index] & 0x1FF;
        (*index)++;
        result = Pokemon_AddMove(mon, *moveID);
    }

    Heap_Free(monLevelUpMoves);
    return result;
}

void Pokemon_SwapMoveSlots(Pokemon *mon, int moveSlot1, int moveSlot2)
{
    BoxPokemon_SwapMoveSlots(&mon->box, moveSlot1, moveSlot2);
}

void BoxPokemon_SwapMoveSlots(BoxPokemon *boxMon, int moveSlot1, int moveSlot2)
{
    u16 moveIDs[2];
    u8 movePPs[2];
    u8 movePPUps[2];

    moveIDs[0] = BoxPokemon_GetValue(boxMon, MON_DATA_MOVE1 + moveSlot1, NULL);
    movePPs[0] = BoxPokemon_GetValue(boxMon, MON_DATA_MOVE1_PP + moveSlot1, NULL);
    movePPUps[0] = BoxPokemon_GetValue(boxMon, MON_DATA_MOVE1_PP_UPS + moveSlot1, NULL);
    moveIDs[1] = BoxPokemon_GetValue(boxMon, MON_DATA_MOVE1 + moveSlot2, NULL);
    movePPs[1] = BoxPokemon_GetValue(boxMon, MON_DATA_MOVE1_PP + moveSlot2, NULL);
    movePPUps[1] = BoxPokemon_GetValue(boxMon, MON_DATA_MOVE1_PP_UPS + moveSlot2, NULL);

    BoxPokemon_SetValue(boxMon, MON_DATA_MOVE1 + moveSlot1, &moveIDs[1]);
    BoxPokemon_SetValue(boxMon, MON_DATA_MOVE1_PP + moveSlot1, &movePPs[1]);
    BoxPokemon_SetValue(boxMon, MON_DATA_MOVE1_PP_UPS + moveSlot1, &movePPUps[1]);
    BoxPokemon_SetValue(boxMon, MON_DATA_MOVE1 + moveSlot2, &moveIDs[0]);
    BoxPokemon_SetValue(boxMon, MON_DATA_MOVE1_PP + moveSlot2, &movePPs[0]);
    BoxPokemon_SetValue(boxMon, MON_DATA_MOVE1_PP_UPS + moveSlot2, &movePPUps[0]);
}

void Pokemon_ClearMoveSlot(Pokemon *mon, u32 moveSlot)
{
    u16 moveID;
    u8 movePP;
    u8 movePPUps;

    for (u32 i = moveSlot; i < 3; i++) {
        moveID = Pokemon_GetValue(mon, MON_DATA_MOVE1 + i + 1, NULL);
        movePP = Pokemon_GetValue(mon, MON_DATA_MOVE1_PP + i + 1, NULL);
        movePPUps = Pokemon_GetValue(mon, MON_DATA_MOVE1_PP_UPS + i + 1, NULL);

        Pokemon_SetValue(mon, MON_DATA_MOVE1 + i, &moveID);
        Pokemon_SetValue(mon, MON_DATA_MOVE1_PP + i, &movePP);
        Pokemon_SetValue(mon, MON_DATA_MOVE1_PP_UPS + i, &movePPUps);
    }

    moveID = 0;
    movePP = 0;
    movePPUps = 0;

    Pokemon_SetValue(mon, MON_DATA_MOVE4, &moveID);
    Pokemon_SetValue(mon, MON_DATA_MOVE4_PP, &movePP);
    Pokemon_SetValue(mon, MON_DATA_MOVE4_PP_UPS, &movePPUps);
}

static BOOL Pokemon_HasMove(Pokemon *mon, u16 moveID)
{
    int i;
    for (i = 0; i < LEARNED_MOVES_MAX; i++) {
        if (Pokemon_GetValue(mon, MON_DATA_MOVE1 + i, NULL) == moveID) {
            break;
        }
    }

    return i != 4;
}

void Pokemon_FromBoxPokemon(BoxPokemon *boxMon, Pokemon *mon)
{
    u32 zero = 0;

    mon->box = *boxMon;
    if (mon->box.boxDecrypted) {
        mon->box.partyDecrypted = TRUE;
    }

    Pokemon_SetValue(mon, MON_DATA_STATUS, &zero);
    Pokemon_SetValue(mon, MON_DATA_HP, &zero);
    Pokemon_SetValue(mon, MON_DATA_MAX_HP, &zero);

    Mail *mail = Mail_New(HEAP_ID_SYSTEM);
    Pokemon_SetValue(mon, MON_DATA_MAIL, mail);
    Heap_Free(mail);

    Pokemon_SetValue(mon, MON_DATA_BALL_CAPSULE_ID, &zero);

    BallCapsule v2;
    MI_CpuClearFast(&v2, sizeof(BallCapsule));
    Pokemon_SetValue(mon, MON_DATA_BALL_CAPSULE, &v2);

    Pokemon_CalcLevelAndStats(mon);
}

u8 Party_GetMaxLevel(Party *party)
{
    int currentPartyCount = Party_GetCurrentCount(party);

    u8 result = 1;

    for (int i = 0; i < currentPartyCount; i++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(party, i);

        if (Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL) && Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL) == FALSE) {
            u8 monLevel = Pokemon_GetValue(mon, MON_DATA_LEVEL, NULL);

            if (monLevel > result) {
                result = monLevel;
            }
        }
    }

    return result;
}

u16 Pokemon_SinnohDexNumber(u16 species)
{
    u16 result;

    NARC_ReadFromMemberByIndexPair(&result, NARC_INDEX_POKETOOL__PL_POKEZUKAN, 0, species * 2, 2);

    return result;
}

u16 Pokemon_NationalDexNumber(u16 sinnohDexNumber)
{
    u16 result = 0;

    if (sinnohDexNumber <= REGIONAL_DEX_COUNT) {
        NARC_ReadFromMemberByIndexPair(&result, NARC_INDEX_POKETOOL__SHINZUKAN, 0, sinnohDexNumber * 2, 2);
    }

    return result;
}

void Pokemon_Copy(Pokemon *src, Pokemon *dest)
{
    *dest = *src;
}

void BoxPokemon_Copy(BoxPokemon *src, BoxPokemon *dest)
{
    *dest = *src;
}

void BoxPokemon_FromPokemon(Pokemon *src, BoxPokemon *dest)
{
    *dest = src->box;
}

s8 Pokemon_GetFlavorAffinity(Pokemon *mon, enum Flavor flavor)
{
    return BoxPokemon_GetFlavorAffinity(&mon->box, flavor);
}

static s8 BoxPokemon_GetFlavorAffinity(BoxPokemon *boxMon, enum Flavor flavor)
{
    return Pokemon_GetFlavorAffinityOf(BoxPokemon_GetValue(boxMon, MON_DATA_PERSONALITY, NULL), flavor);
}

s8 Pokemon_GetFlavorAffinityOf(u32 monPersonality, enum Flavor flavor)
{
    u8 monNature = Pokemon_GetNatureOf(monPersonality);
    return sNatureFlavorAffinities[monNature][flavor];
}

int Pokemon_LoadLevelUpMoveIdsOf(int monSpecies, int monForm, u16 *monLevelUpMoveIDs)
{
    u16 *monLevelUpMoves = Heap_Alloc(HEAP_ID_SYSTEM, sizeof(SpeciesLearnset));

    Pokemon_LoadLevelUpMovesOf(monSpecies, monForm, monLevelUpMoves);

    int result = 0;

    while (monLevelUpMoves[result] != LEARNSET_ALL_SLOTS_FILLED) {
        monLevelUpMoveIDs[result] = monLevelUpMoves[result] & 0x1FF;
        result++;
    }

    Heap_Free(monLevelUpMoves);
    return result;
}

void Pokemon_ApplyPokerus(Party *party)
{
    int currentPartyCount = Party_GetCurrentCount(party);
    u16 rand = LCRNG_Next();

    if (rand == 16384 || rand == 32768 || rand == 49152) {
        int partySlot;
        Pokemon *mon;
        do {
            partySlot = LCRNG_Next() % currentPartyCount;
            mon = Party_GetPokemonBySlotIndex(party, partySlot);

            if (Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL) && Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL) == FALSE) {
                break;
            }

            partySlot = currentPartyCount;
        } while (partySlot == currentPartyCount);

        if (Pokemon_HasPokerus(party, FlagIndex(partySlot)) == 0) {
            u8 monPokerus;
            do {
                monPokerus = LCRNG_Next() & 0xff;
            } while ((monPokerus & 0x7) == 0);

            if (monPokerus & 0xf0) {
                monPokerus &= 0x7;
            }

            monPokerus |= (monPokerus << 4);
            monPokerus &= 0xf3;
            monPokerus++;

            Pokemon_SetValue(mon, MON_DATA_POKERUS, &monPokerus);
        }
    }
}

u8 Pokemon_HasPokerus(Party *party, u8 param1)
{
    int partySlot = 0;
    int v1 = 1;

    u8 result = 0;
    if (param1) {
        do {
            if (param1 & 1) {
                Pokemon *mon = Party_GetPokemonBySlotIndex(party, partySlot);

                if (Pokemon_GetValue(mon, MON_DATA_POKERUS, NULL)) {
                    result |= v1;
                }
            }

            partySlot++;
            v1 = v1 << 1;
            param1 = param1 >> 1;
        } while (param1 != 0);
    } else {
        Pokemon *mon = Party_GetPokemonBySlotIndex(party, partySlot);

        if (Pokemon_GetValue(mon, MON_DATA_POKERUS, NULL)) {
            result++;
        }
    }

    return result;
}

void Party_UpdatePokerusStatus(Party *party, s32 daysPassed)
{
    int currentPartyCount = Party_GetCurrentCount(party);

    for (int i = 0; i < currentPartyCount; i++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(party, i);

        if (Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL)) {
            u8 monPokerus = Pokemon_GetValue(mon, MON_DATA_POKERUS, NULL);

            if (monPokerus & 0xf) {
                if (((monPokerus & 0xf) < daysPassed) || (daysPassed > 4)) {
                    monPokerus &= 0xf0;
                } else {
                    monPokerus -= daysPassed;
                }

                if (monPokerus == 0) {
                    monPokerus = 0x10;
                }

                Pokemon_SetValue(mon, MON_DATA_POKERUS, &monPokerus);
            }
        }
    }
}

void Pokemon_ValidatePokerus(Party *party)
{
    int currentPartyCount = Party_GetCurrentCount(party);

    if (LCRNG_Next() % 3 == 0) {
        for (int i = 0; i < currentPartyCount; i++) {
            Pokemon *mon = Party_GetPokemonBySlotIndex(party, i);

            if (Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL)) {
                u8 monPokerus = Pokemon_GetValue(mon, MON_DATA_POKERUS, NULL);

                if (monPokerus & 0xf) {
                    if (i != 0) {
                        mon = Party_GetPokemonBySlotIndex(party, i - 1);

                        if ((Pokemon_GetValue(mon, MON_DATA_POKERUS, NULL) & 0xf0) == 0) {
                            Pokemon_SetValue(mon, MON_DATA_POKERUS, &monPokerus);
                        }
                    }

                    if (i < currentPartyCount - 1) {
                        mon = Party_GetPokemonBySlotIndex(party, i + 1);

                        if ((Pokemon_GetValue(mon, MON_DATA_POKERUS, NULL) & 0xf0) == 0) {
                            Pokemon_SetValue(mon, MON_DATA_POKERUS, &monPokerus);
                            i++;
                        }
                    }
                }
            }
        }
    }
}

BOOL Pokemon_InfectedWithPokerus(Pokemon *mon)
{
    return IsBoxPokemonInfectedWithPokerus(&mon->box);
}

static BOOL IsBoxPokemonInfectedWithPokerus(BoxPokemon *boxMon)
{
    return (BoxPokemon_GetValue(boxMon, MON_DATA_POKERUS, NULL) & 0xf) != 0;
}

BOOL Pokemon_HasCuredPokerus(Pokemon *mon)
{
    return BoxPokemonHasCuredPokerus(&mon->box);
}

static BOOL BoxPokemonHasCuredPokerus(BoxPokemon *boxMon)
{
    u8 monPokerus = BoxPokemon_GetValue(boxMon, MON_DATA_POKERUS, NULL);

    if (monPokerus & 0xF) {
        return FALSE;
    }

    return (monPokerus & 0xF0) != 0;
}

void Pokemon_SetArceusForm(Pokemon *mon)
{
    BoxPokemon_SetArceusForm(&mon->box);
}

void BoxPokemon_SetArceusForm(BoxPokemon *boxMon)
{
    int monSpecies = BoxPokemon_GetValue(boxMon, MON_DATA_SPECIES, NULL);
    int monAbility = BoxPokemon_GetValue(boxMon, MON_DATA_ABILITY, NULL);
    int monHeldItem = BoxPokemon_GetValue(boxMon, MON_DATA_HELD_ITEM, NULL);

    if (monSpecies == SPECIES_ARCEUS && monAbility == ABILITY_MULTITYPE) {
        int monForm = Pokemon_GetArceusTypeOf(Item_LoadParam(monHeldItem, ITEM_PARAM_HOLD_EFFECT, HEAP_ID_SYSTEM));
        BoxPokemon_SetValue(boxMon, MON_DATA_FORM, &monForm);
    }
}

u8 Pokemon_GetArceusTypeOf(u16 itemHoldEffect)
{
    u8 type;

    switch (itemHoldEffect) {
    case HOLD_EFFECT_ARCEUS_FIRE:
        type = TYPE_FIRE;
        break;
    case HOLD_EFFECT_ARCEUS_WATER:
        type = TYPE_WATER;
        break;
    case HOLD_EFFECT_ARCEUS_ELECTRIC:
        type = TYPE_ELECTRIC;
        break;
    case HOLD_EFFECT_ARCEUS_GRASS:
        type = TYPE_GRASS;
        break;
    case HOLD_EFFECT_ARCEUS_ICE:
        type = TYPE_ICE;
        break;
    case HOLD_EFFECT_ARCEUS_FIGHTING:
        type = TYPE_FIGHTING;
        break;
    case HOLD_EFFECT_ARCEUS_POISON:
        type = TYPE_POISON;
        break;
    case HOLD_EFFECT_ARCEUS_GROUND:
        type = TYPE_GROUND;
        break;
    case HOLD_EFFECT_ARCEUS_FLYING:
        type = TYPE_FLYING;
        break;
    case HOLD_EFFECT_ARCEUS_PSYCHIC:
        type = TYPE_PSYCHIC;
        break;
    case HOLD_EFFECT_ARCEUS_BUG:
        type = TYPE_BUG;
        break;
    case HOLD_EFFECT_ARCEUS_ROCK:
        type = TYPE_ROCK;
        break;
    case HOLD_EFFECT_ARCEUS_GHOST:
        type = TYPE_GHOST;
        break;
    case HOLD_EFFECT_ARCEUS_DRAGON:
        type = TYPE_DRAGON;
        break;
    case HOLD_EFFECT_ARCEUS_DARK:
        type = TYPE_DARK;
        break;
    case HOLD_EFFECT_ARCEUS_STEEL:
        type = TYPE_STEEL;
        break;
    default:
        type = TYPE_NORMAL;
        break;
    }

    return type;
}

int Pokemon_SetGiratinaFormByHeldItem(Pokemon *mon)
{
    int result = BoxPokemon_SetGiratinaForm(&mon->box);

    if (result != -1) {
        Pokemon_CalcLevelAndStats(mon);
    }

    return result;
}

int BoxPokemon_SetGiratinaForm(BoxPokemon *boxMon)
{
    int species = BoxPokemon_GetValue(boxMon, MON_DATA_SPECIES, NULL);
    int item = BoxPokemon_GetValue(boxMon, MON_DATA_HELD_ITEM, NULL);

    if (species == SPECIES_GIRATINA) {
        int form = (item == ITEM_GRISEOUS_ORB) ? GIRATINA_FORM_ORIGIN : GIRATINA_FORM_ALTERED;

        BoxPokemon_SetValue(boxMon, MON_DATA_FORM, &form);
        BoxPokemon_CalcAbility(boxMon);

        return form;
    }

    return -1;
}

void Pokemon_SetGiratinaOriginForm(Pokemon *mon)
{
    int form = GIRATINA_FORM_ORIGIN;

    if (Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL) == SPECIES_GIRATINA) {
        BoxPokemon_SetValue(&mon->box, MON_DATA_FORM, &form);
        BoxPokemon_CalcAbility(&mon->box);
        Pokemon_CalcLevelAndStats(mon);
    }
}

void Party_SetGiratinaForm(Party *party, int form)
{
    int currentPartyCount = Party_GetCurrentCount(party);

    for (int i = 0; i < currentPartyCount; i++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(party, i);

        if (form) {
            Pokemon_SetGiratinaOriginForm(mon);
        } else {
            Pokemon_SetGiratinaFormByHeldItem(mon);
        }
    }
}

void Pokemon_SetShayminForm(Pokemon *mon, int form)
{
    BoxPokemon_SetShayminForm(&mon->box, form);
    Pokemon_CalcLevelAndStats(mon);
}

void BoxPokemon_SetShayminForm(BoxPokemon *boxMon, int form)
{
    int species = BoxPokemon_GetValue(boxMon, MON_DATA_SPECIES, NULL);

    if (species == SPECIES_SHAYMIN) {
        GF_ASSERT(form <= SHAYMIN_FORM_COUNT - 1);

        BoxPokemon_SetValue(boxMon, MON_DATA_FORM, &form);
        BoxPokemon_CalcAbility(boxMon);
    }
}

BOOL Pokemon_CanShayminSkyForm(Pokemon *mon)
{
    u32 species = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
    u32 monForm = Pokemon_GetValue(mon, MON_DATA_FORM, NULL);
    u32 condition = Pokemon_GetValue(mon, MON_DATA_STATUS, NULL);
    u32 currentHP = Pokemon_GetValue(mon, MON_DATA_HP, NULL);
    u32 fatefulEncounter = Pokemon_GetValue(mon, MON_DATA_FATEFUL_ENCOUNTER, NULL);

    RTCTime rtcTime;
    RTC_GetCurrentTime(&rtcTime);

    return species == SPECIES_SHAYMIN
        && monForm == SHAYMIN_FORM_LAND
        && currentHP > 0
        && fatefulEncounter == TRUE
        && (condition & MON_CONDITION_FREEZE) == FALSE
        && rtcTime.hour >= 4
        && rtcTime.hour < 20;
}

void Party_SetShayminLandForm(Party *party)
{
    int currentPartyCount = Party_GetCurrentCount(party);

    for (int i = 0; i < currentPartyCount; i++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(party, i);
        int species = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
        int form = Pokemon_GetValue(mon, MON_DATA_FORM, NULL);

        if (species == SPECIES_SHAYMIN && form == SHAYMIN_FORM_SKY) {
            int newForm = SHAYMIN_FORM_LAND;
            Pokemon_SetShayminForm(mon, newForm);
        }
    }
}

BOOL Party_SetShayminForm(Party *party, int param1, const RTCTime *rtcTime)
{
    if (rtcTime->hour >= 20 || rtcTime->hour < 4) {
        s32 hours = rtcTime->hour;

        if (hours < 4) {
            hours += 24;
        }

        hours -= 20;
        s32 minutes = rtcTime->minute + hours * 60;

        param1++;

        if (minutes < param1) {
            Party_SetShayminLandForm(party);
            return TRUE;
        }

        return FALSE;
    } else {
        s32 minutes = rtcTime->minute + (rtcTime->hour - 4) * 60;

        if (minutes < param1) {
            Party_SetShayminLandForm(party);
            return TRUE;
        }

        return FALSE;
    }
}

BOOL Pokemon_SetRotomForm(Pokemon *mon, int form, int moveSlot)
{
    int monSpecies = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);

    if (monSpecies != SPECIES_ROTOM) {
        return FALSE;
    }

    int currentMonForm = Pokemon_GetValue(mon, MON_DATA_FORM, NULL);

    static const u16 rotomFormMoves[] = {
        MOVE_NONE,
        MOVE_OVERHEAT,
        MOVE_HYDRO_PUMP,
        MOVE_BLIZZARD,
        MOVE_AIR_SLASH,
        MOVE_LEAF_STORM
    };

    int newFormMoveID = rotomFormMoves[form];

    int i;
    for (i = 0; i < LEARNED_MOVES_MAX; i++) {
        int j;
        int moveID = Pokemon_GetValue(mon, MON_DATA_MOVE1 + i, NULL);

        for (j = 1; j < NELEMS(rotomFormMoves); j++) {
            if (moveID != MOVE_NONE && moveID == rotomFormMoves[j]) {
                if (newFormMoveID != MOVE_NONE) {
                    Pokemon_ResetMoveSlot(mon, newFormMoveID, i);
                    newFormMoveID = MOVE_NONE;
                    break;
                } else {
                    Pokemon_ClearMoveSlot(mon, i);
                    i--;
                    break;
                }
            }
        }
    }

    if (newFormMoveID != MOVE_NONE) {
        for (i = 0; i < LEARNED_MOVES_MAX; i++) {
            if (Pokemon_GetValue(mon, MON_DATA_MOVE1 + i, NULL) == MOVE_NONE) {
                Pokemon_ResetMoveSlot(mon, newFormMoveID, i);
                break;
            }
        }

        if (i == 4) {
            Pokemon_ResetMoveSlot(mon, newFormMoveID, moveSlot);
        }
    }

    if (Pokemon_GetValue(mon, MON_DATA_MOVE1, NULL) == MOVE_NONE) {
        Pokemon_ResetMoveSlot(mon, MOVE_THUNDER_SHOCK, 0);
    }

    Pokemon_SetValue(mon, MON_DATA_FORM, &form);
    Pokemon_CalcAbility(mon);
    Pokemon_CalcLevelAndStats(mon);

    return TRUE;
}

void Pokemon_LoadLevelUpMovesOf(int monSpecies, int monForm, u16 *monLevelUpMoves)
{
    monSpecies = Pokemon_GetFormNarcIndex(monSpecies, monForm);
    NARC_ReadWholeMemberByIndexPair(monLevelUpMoves, NARC_INDEX_POKETOOL__PERSONAL__WOTBL, monSpecies);
}

void PlayCryWithParams(ChatotCry *chatotCry, enum PokemonCryMod cryMod, u16 species, int form, int pan, int volume, int forceDefaultChatot, enum HeapID heapID)
{
    if (species == SPECIES_CHATOT) {
        if (Sound_CanPlayChatotCry(cryMod) == FALSE) {
            Sound_SetUsingDefaultChatotCry(TRUE);
            Sound_PlayPokemonCryEx(cryMod, species, pan, volume, heapID, form);
        } else {
            if (forceDefaultChatot) {
                Sound_SetUsingDefaultChatotCry(TRUE);
            }

            Sound_PlayChatotCry(chatotCry, NULL, volume, pan);
        }

        return;
    }

    Sound_PlayPokemonCryEx(cryMod, species, pan, volume, heapID, form);
}

void Species_PlayDelayedCry(ChatotCry *chatotCry, enum PokemonCryMod cryMod, u16 species, int form, int pan, int volume, int forceDefaultChatot, enum HeapID heapID, u8 delay)
{
    if (species == SPECIES_CHATOT) {
        if (Sound_CanPlayChatotCry(cryMod) == FALSE) {
            Sound_SetUsingDefaultChatotCry(TRUE);
            Sound_PlayDelayedPokemonCryEx(cryMod, species, pan, volume, heapID, delay, form);
        } else {
            if (forceDefaultChatot) {
                Sound_SetUsingDefaultChatotCry(TRUE);
            }

            Sound_PlayDelayedChatotCry(chatotCry, NULL, volume, pan, delay);
        }

        return;
    }

    Sound_PlayDelayedPokemonCryEx(cryMod, species, pan, volume, heapID, delay, form);
}

BOOL Pokemon_PlayCry(Pokemon *mon)
{
    int species = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
    int form = Pokemon_GetValue(mon, MON_DATA_FORM, NULL);

    return Sound_PlayPokemonCry(species, form);
}

void Pokemon_SetCatchData(Pokemon *mon, TrainerInfo *trainerInfo, int monPokeball, int metLocation, int metTerrain, enum HeapID heapID)
{
    InitializeBoxPokemonAfterCapture(&mon->box, trainerInfo, monPokeball, metLocation, metTerrain, heapID);

    if (monPokeball == ITEM_HEAL_BALL) {
        int monMaxHP = Pokemon_GetValue(mon, MON_DATA_MAX_HP, NULL);
        Pokemon_SetValue(mon, MON_DATA_HP, &monMaxHP);

        monMaxHP = 0;
        Pokemon_SetValue(mon, MON_DATA_STATUS, &monMaxHP);
    }
}

static void InitializeBoxPokemonAfterCapture(BoxPokemon *boxMon, TrainerInfo *trainer, int monPokeball, int metLocation, int metTerrain, enum HeapID heapID)
{
    UpdateBoxMonStatusAndTrainerInfo(boxMon, trainer, 0, metLocation, heapID);
    BoxPokemon_SetValue(boxMon, MON_DATA_MET_GAME, &gGameVersion);
    BoxPokemon_SetValue(boxMon, MON_DATA_POKEBALL, &monPokeball);
    BoxPokemon_SetValue(boxMon, MON_DATA_MET_TERRAIN, &metTerrain);
}

void Pokemon_UpdateAfterCatch(Pokemon *mon, TrainerInfo *trainer, int monPokeball, int metLocation, int metTerrain, int heapID)
{
    PostCaptureBoxPokemonProcessing(&mon->box, trainer, monPokeball, metLocation, metTerrain, heapID);
}

static void PostCaptureBoxPokemonProcessing(BoxPokemon *boxMon, TrainerInfo *trainer, int monPokeball, int metLocation, int metTerrain, int heapID)
{
    InitializeBoxPokemonAfterCapture(boxMon, trainer, monPokeball, metLocation, metTerrain, heapID);
}

static const u16 sHeldItemChance[][2] = {
    { 45, 95 },
    { 20, 80 }
};

void Pokemon_GiveHeldItem(Pokemon *mon, u32 battleType, int itemRates)
{
    if (battleType & (BATTLE_TYPE_TRAINER | BATTLE_TYPE_FRONTIER)) {
        return;
    }

    u32 rand = LCRNG_Next() % 100;
    u16 monSpecies = Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
    u16 monForm = Pokemon_GetValue(mon, MON_DATA_FORM, NULL);
    u16 monItem1 = SpeciesData_GetFormValue(monSpecies, monForm, SPECIES_DATA_HELD_ITEM_COMMON);
    u16 monItem2 = SpeciesData_GetFormValue(monSpecies, monForm, SPECIES_DATA_HELD_ITEM_RARE);

    if (monItem1 == monItem2 && monItem1 != ITEM_NONE) {
        Pokemon_SetValue(mon, MON_DATA_HELD_ITEM, &monItem1);
        return;
    }

    // Without CompoundEyes (itemRates == 0) 45% no item, 50% common item, 5% rare item
    // With CompoundEyes (itemRates == 1) 20% no item, 60% common item, 20% rare item
    if (rand < sHeldItemChance[itemRates][0]) {
        return;
    } else if (rand < sHeldItemChance[itemRates][1]) {
        Pokemon_SetValue(mon, MON_DATA_HELD_ITEM, &monItem1);
    } else {
        Pokemon_SetValue(mon, MON_DATA_HELD_ITEM, &monItem2);
    }
}

BOOL Pokemon_CanLearnTM(Pokemon *mon, u8 tmID)
{
    return CanBoxPokemonLearnTM(&mon->box, tmID);
}

static BOOL CanBoxPokemonLearnTM(BoxPokemon *boxMon, u8 tmID)
{
    u16 monSpeciesEgg = BoxPokemon_GetValue(boxMon, MON_DATA_SPECIES_OR_EGG, NULL);
    int monForm = BoxPokemon_GetValue(boxMon, MON_DATA_FORM, NULL);

    // ENTRY P108: Ditto learns no TM or HM at all - not even through the type rule below.
    if (monSpeciesEgg == SPECIES_DITTO) {
        return FALSE;
    }

    // COLOR MOD (port of PokeHeartGold ENTRY AD item 4): a mon can also learn a TM/HM whose move type
    // matches either of its types (MON_DATA_TYPE_1/2, which already include the custom type override -
    // see ApplyCustomType above), even if its species normally cannot learn that TM/HM.
    if (monSpeciesEgg != SPECIES_EGG && tmID < NUM_TMS + NUM_HMS) {
        u16 tmhmMove = Item_MoveForTMHM(ITEM_TM01 + tmID);
        if (tmhmMove != MOVE_NONE) {
            u32 moveType = MoveTable_LoadParam(tmhmMove, MOVEATTRIBUTE_TYPE);
            if (moveType == BoxPokemon_GetValue(boxMon, MON_DATA_TYPE_1, NULL) || moveType == BoxPokemon_GetValue(boxMon, MON_DATA_TYPE_2, NULL)) {
                return TRUE;
            }
        }
    }

    return CanPokemonFormLearnTM(monSpeciesEgg, monForm, tmID);
}

BOOL CanPokemonFormLearnTM(u16 monSpecies, int monForm, u8 tmID)
{
    if (monSpecies == SPECIES_EGG) {
        return FALSE;
    }

    u32 tmFlag;
    u8 speciesDataAttribute;
    if (tmID < 32) {
        tmFlag = (1 << tmID);
        speciesDataAttribute = SPECIES_DATA_TM_LEARNSET_MASK_1;
    } else if (tmID < 64) {
        tmFlag = (1 << (tmID - 32));
        speciesDataAttribute = SPECIES_DATA_TM_LEARNSET_MASK_2;
    } else if (tmID < 96) {
        tmFlag = (1 << (tmID - 64));
        speciesDataAttribute = SPECIES_DATA_TM_LEARNSET_MASK_3;
    } else {
        tmFlag = (1 << (tmID - 96));
        speciesDataAttribute = SPECIES_DATA_TM_LEARNSET_MASK_4;
    }

    return (SpeciesData_GetFormValue(monSpecies, monForm, speciesDataAttribute) & tmFlag) != 0;
}

void Pokemon_CalcAbility(Pokemon *mon)
{
    BoxPokemon_CalcAbility(&mon->box);
}

static void BoxPokemon_CalcAbility(BoxPokemon *boxMon)
{
    BOOL reencrypt = BoxPokemon_EnterDecryptionContext(boxMon);
    int monSpecies = BoxPokemon_GetValue(boxMon, MON_DATA_SPECIES, NULL);
    u32 monPersonality = BoxPokemon_GetValue(boxMon, MON_DATA_PERSONALITY, NULL);
    int monForm = BoxPokemon_GetValue(boxMon, MON_DATA_FORM, NULL);
    int monAbility1 = SpeciesData_GetFormValue(monSpecies, monForm, SPECIES_DATA_ABILITY_1);
    int monAbility2 = SpeciesData_GetFormValue(monSpecies, monForm, SPECIES_DATA_ABILITY_2);

    if (monAbility2 != ABILITY_NONE) {
        if (monPersonality & 1) {
            BoxPokemon_SetValue(boxMon, MON_DATA_ABILITY, &monAbility2);
        } else {
            BoxPokemon_SetValue(boxMon, MON_DATA_ABILITY, &monAbility1);
        }
    } else {
        BoxPokemon_SetValue(boxMon, MON_DATA_ABILITY, &monAbility1);
    }

    BoxPokemon_ExitDecryptionContext(boxMon, reencrypt);
}

void sub_020780C4(Pokemon *mon, u32 monPersonality)
{
    Pokemon *newMon = Pokemon_New(HEAP_ID_SYSTEM);

    Pokemon_Copy(mon, newMon);

    PokemonDataBlockA *newMonBlockA = BoxPokemon_GetDataBlock(&newMon->box, mon->box.personality, DATA_BLOCK_A);
    PokemonDataBlockB *newMonBlockB = BoxPokemon_GetDataBlock(&newMon->box, mon->box.personality, DATA_BLOCK_B);
    PokemonDataBlockC *newMonBlockC = BoxPokemon_GetDataBlock(&newMon->box, mon->box.personality, DATA_BLOCK_C);
    PokemonDataBlockD *newMonBlockD = BoxPokemon_GetDataBlock(&newMon->box, mon->box.personality, DATA_BLOCK_D);
    PokemonDataBlockA *monBlockA = BoxPokemon_GetDataBlock(&mon->box, monPersonality, DATA_BLOCK_A);
    PokemonDataBlockB *monBlockB = BoxPokemon_GetDataBlock(&mon->box, monPersonality, DATA_BLOCK_B);
    PokemonDataBlockC *monBlockC = BoxPokemon_GetDataBlock(&mon->box, monPersonality, DATA_BLOCK_C);
    PokemonDataBlockD *monBlockD = BoxPokemon_GetDataBlock(&mon->box, monPersonality, DATA_BLOCK_D);

    Pokemon_DecryptData(&newMon->box.dataBlocks, sizeof(PokemonDataBlock) * 4, newMon->box.checksum);
    Pokemon_DecryptData(&mon->party, sizeof(PartyPokemon), mon->box.personality);
    Pokemon_DecryptData(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4, mon->box.checksum);

    mon->box.personality = monPersonality;

    *monBlockA = *newMonBlockA;
    *monBlockB = *newMonBlockB;
    *monBlockC = *newMonBlockC;
    *monBlockD = *newMonBlockD;

    mon->box.checksum = Pokemon_GetDataChecksum(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4);

    Pokemon_EncryptData(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4, mon->box.checksum);
    Pokemon_EncryptData(&mon->party, sizeof(PartyPokemon), mon->box.personality);
    Heap_Free(newMon);
}

static void SpeciesData_LoadSpecies(int monSpecies, SpeciesData *speciesData)
{
    NARC_ReadWholeMemberByIndexPair(speciesData, NARC_INDEX_POKETOOL__PERSONAL__PL_PERSONAL, monSpecies);
}

static void SpeciesData_LoadForm(int monSpecies, int monForm, SpeciesData *speciesData)
{
    monSpecies = Pokemon_GetFormNarcIndex(monSpecies, monForm);
    NARC_ReadWholeMemberByIndexPair(speciesData, NARC_INDEX_POKETOOL__PERSONAL__PL_PERSONAL, monSpecies);
}

static void LoadSpeciesEvolutions(int monSpecies, SpeciesEvolution speciesEvolutions[MAX_EVOLUTIONS])
{
    NARC_ReadWholeMemberByIndexPair(speciesEvolutions, NARC_INDEX_POKETOOL__PERSONAL__EVO, monSpecies);
}

static void Pokemon_EncryptData(void *data, u32 bytes, u32 seed)
{
    EncodeData(data, bytes, seed);
}

static void Pokemon_DecryptData(void *data, u32 bytes, u32 seed)
{
    DecodeData(data, bytes, seed);
}

static u16 Pokemon_GetDataChecksum(void *data, u32 bytes)
{
    int i;
    u16 checksum = 0;

    u16 *dataWords = data;

    for (i = 0; i < bytes / 2; i++) {
        checksum += dataWords[i];
    }

    return checksum;
}

#define DATA_BLOCK_SHUFFLE_CASE(v1, v2, v3, v4)            \
    {                                                      \
        PokemonDataBlock *dataBlocks = boxMon->dataBlocks; \
        switch (dataBlockID) {                             \
        case DATA_BLOCK_A:                                 \
            result = &dataBlocks[v1];                      \
            break;                                         \
        case DATA_BLOCK_B:                                 \
            result = &dataBlocks[v2];                      \
            break;                                         \
        case DATA_BLOCK_C:                                 \
            result = &dataBlocks[v3];                      \
            break;                                         \
        case DATA_BLOCK_D:                                 \
            result = &dataBlocks[v4];                      \
            break;                                         \
        }                                                  \
        break;                                             \
    }

static void *BoxPokemon_GetDataBlock(BoxPokemon *boxMon, u32 personality, enum PokemonDataBlockID dataBlockID)
{
    personality = (personality & 0x3e000) >> 13;
    GF_ASSERT(personality <= 31);

    void *result;
    switch (personality) {
    case 0:
    case 24:
        DATA_BLOCK_SHUFFLE_CASE(0, 1, 2, 3)
    case 1:
    case 25:
        DATA_BLOCK_SHUFFLE_CASE(0, 1, 3, 2)
    case 2:
    case 26:
        DATA_BLOCK_SHUFFLE_CASE(0, 2, 1, 3)
    case 3:
    case 27:
        DATA_BLOCK_SHUFFLE_CASE(0, 3, 1, 2)
    case 4:
    case 28:
        DATA_BLOCK_SHUFFLE_CASE(0, 2, 3, 1)
    case 5:
    case 29:
        DATA_BLOCK_SHUFFLE_CASE(0, 3, 2, 1)
    case 6:
    case 30:
        DATA_BLOCK_SHUFFLE_CASE(1, 0, 2, 3)
    case 7:
    case 31:
        DATA_BLOCK_SHUFFLE_CASE(1, 0, 3, 2)
    case 8:
        DATA_BLOCK_SHUFFLE_CASE(2, 0, 1, 3)
    case 9:
        DATA_BLOCK_SHUFFLE_CASE(3, 0, 1, 2)
    case 10:
        DATA_BLOCK_SHUFFLE_CASE(2, 0, 3, 1)
    case 11:
        DATA_BLOCK_SHUFFLE_CASE(3, 0, 2, 1)
    case 12:
        DATA_BLOCK_SHUFFLE_CASE(1, 2, 0, 3)
    case 13:
        DATA_BLOCK_SHUFFLE_CASE(1, 3, 0, 2)
    case 14:
        DATA_BLOCK_SHUFFLE_CASE(2, 1, 0, 3)
    case 15:
        DATA_BLOCK_SHUFFLE_CASE(3, 1, 0, 2)
    case 16:
        DATA_BLOCK_SHUFFLE_CASE(2, 3, 0, 1)
    case 17:
        DATA_BLOCK_SHUFFLE_CASE(3, 2, 0, 1)
    case 18:
        DATA_BLOCK_SHUFFLE_CASE(1, 2, 3, 0)
    case 19:
        DATA_BLOCK_SHUFFLE_CASE(1, 3, 2, 0)
    case 20:
        DATA_BLOCK_SHUFFLE_CASE(2, 1, 3, 0)
    case 21:
        DATA_BLOCK_SHUFFLE_CASE(3, 1, 2, 0)
    case 22:
        DATA_BLOCK_SHUFFLE_CASE(2, 3, 1, 0)
    case 23:
        DATA_BLOCK_SHUFFLE_CASE(3, 2, 1, 0)
    }

    return result;
}

static int Pokemon_GetFormNarcIndex(int monSpecies, int monForm)
{
    // TODO enum values?
    switch (monSpecies) {
    case SPECIES_DEOXYS:
        if (monForm && monForm <= DEOXYS_FORM_COUNT - 1) {
            monSpecies = (496 - 1) + monForm;
        }
        break;
    case SPECIES_WORMADAM:
        if (monForm && monForm <= WORMADAM_FORM_COUNT - 1) {
            monSpecies = (499 - 1) + monForm;
        }
        break;
    case SPECIES_GIRATINA:
        if (monForm && monForm <= GIRATINA_FORM_COUNT - 1) {
            monSpecies = (501 - 1) + monForm;
        }
        break;
    case SPECIES_SHAYMIN:
        if (monForm && monForm <= SHAYMIN_FORM_COUNT - 1) {
            monSpecies = (502 - 1) + monForm;
        }
        break;
    case SPECIES_ROTOM:
        if (monForm && monForm <= ROTOM_FORM_COUNT - 1) {
            monSpecies = (503 - 1) + monForm;
        }
        break;
    default:
        break;
    }

    return monSpecies;
}

u32 FlagIndex(int num)
{
    int i;
    u32 flag = 1;

    GF_ASSERT(num < 32);

    for (i = 0; i < num; i++) {
        flag <<= 1;
    }

    return flag;
}

int LowestBit(u32 flag)
{
    int i;
    u32 mask = 1;

    for (i = 0; i < 32; i++) {
        if (flag & mask) {
            break;
        }

        mask <<= 1;
    }

    return i;
}

static const u16 sBattleFrontierBanlist[BATTLE_FRONTIER_BANLIST_SIZE] = {
    SPECIES_MEWTWO,
    SPECIES_MEW,
    SPECIES_LUGIA,
    SPECIES_HO_OH,
    SPECIES_CELEBI,
    SPECIES_KYOGRE,
    SPECIES_GROUDON,
    SPECIES_RAYQUAZA,
    SPECIES_JIRACHI,
    SPECIES_DEOXYS,
    SPECIES_DIALGA,
    SPECIES_PALKIA,
    SPECIES_GIRATINA,
    SPECIES_PHIONE,
    SPECIES_MANAPHY,
    SPECIES_DARKRAI,
    SPECIES_SHAYMIN,
    SPECIES_ARCEUS
};

BOOL Pokemon_IsOnBattleFrontierBanlist(u16 species)
{
    for (u32 i = 0; i < BATTLE_FRONTIER_BANLIST_SIZE; i++) {
        if (species == sBattleFrontierBanlist[i]) {
            return TRUE;
        }
    }

    return FALSE;
}

u16 Pokemon_GetBattleFrontierBanlistEntry(u8 index)
{
    if (index >= BATTLE_FRONTIER_BANLIST_SIZE) {
        index = 0;
    }

    return sBattleFrontierBanlist[index];
}

BOOL Pokemon_IsBannedFromBattleFrontier(Pokemon *mon)
{
    u16 species = (u16)Pokemon_GetValue(mon, MON_DATA_SPECIES, NULL);
    return Pokemon_IsOnBattleFrontierBanlist(species);
}

BOOL sub_0207884C(BoxPokemon *boxMon, TrainerInfo *param1, enum HeapID heapID)
{
    u32 v0 = TrainerInfo_ID(param1);
    u32 monOTID = BoxPokemon_GetValue(boxMon, MON_DATA_OT_ID, NULL);
    u32 v2 = TrainerInfo_Gender(param1);
    u32 monOtGender = BoxPokemon_GetValue(boxMon, MON_DATA_OT_GENDER, NULL);
    String *v4 = TrainerInfo_NameNewString(param1, heapID);
    // TODO enum/const value?
    String *v5 = String_Init(8, heapID);
    BOOL v6 = FALSE;

    BoxPokemon_GetValue(boxMon, MON_DATA_OT_NAME_STRING, v5);

    if (v0 == monOTID && v2 == monOtGender && String_Compare(v4, v5) == 0) {
        v6 = TRUE;
    }

    String_Free(v5);
    String_Free(v4);

    return v6;
}

// Mapping to Back Sprites order. Returns player if not otherwise defined
int SpriteSystem_TrainerClassBackSpriteIndex(enum TrainerClass trainerClass)
{
    switch (trainerClass) {
    case TRAINER_CLASS_PLAYER_MALE:
    case TRAINER_CLASS_PLAYER_FEMALE:
        break;
    case TRAINER_CLASS_RIVAL:
        trainerClass = rival_back_NCGR / TRAINER_CLASS_NUM_FILETYPES;
        break;
    case TRAINER_CLASS_TRAINER_CHERYL:
    case TRAINER_CLASS_TRAINER_RILEY:
    case TRAINER_CLASS_TRAINER_MARLEY:
    case TRAINER_CLASS_TRAINER_BUCK:
    case TRAINER_CLASS_TRAINER_MIRA:
        trainerClass = trainer_cheryl_back_NCGR / TRAINER_CLASS_NUM_FILETYPES + trainerClass - TRAINER_CLASS_TRAINER_CHERYL;
        break;
    case TRAINER_CLASS_DP_PLAYER_MALE_2:
    case TRAINER_CLASS_DP_PLAYER_FEMALE_2:
        trainerClass = dp_player_male_back_NCGR / TRAINER_CLASS_NUM_FILETYPES + trainerClass - TRAINER_CLASS_DP_PLAYER_MALE_2;
        break;
    default:
        if (TrainerClass_Gender(trainerClass) == GENDER_FEMALE) {
            trainerClass = player_female_back_NCGR / TRAINER_CLASS_NUM_FILETYPES;
        } else {
            trainerClass = player_male_back_NCGR / TRAINER_CLASS_NUM_FILETYPES;
        }
        break;
    }

    return trainerClass;
}

void Pokemon_ClearBallCapsuleData(Pokemon *mon)
{
    u8 zero = 0;

    BallCapsule v1;
    MI_CpuClearFast(&v1, sizeof(BallCapsule));

    Pokemon_SetValue(mon, MON_DATA_BALL_CAPSULE_ID, &zero);
    Pokemon_SetValue(mon, MON_DATA_BALL_CAPSULE, &v1);
}

void BoxPokemon_RestorePP(BoxPokemon *boxMon)
{
    if (RunGame_Exhausted(boxMon)) return;
    BOOL reencrypt = BoxPokemon_EnterDecryptionContext(boxMon);

    for (int i = 0; i < LEARNED_MOVES_MAX; i++) {
        if (BoxPokemon_GetValue(boxMon, MON_DATA_MOVE1 + i, NULL)) {
            u8 moveMaxPP = BoxPokemon_GetValue(boxMon, MON_DATA_MOVE1_MAX_PP + i, NULL);

            BoxPokemon_SetValue(boxMon, MON_DATA_MOVE1_PP + i, &moveMaxPP);
        }
    }

    BoxPokemon_ExitDecryptionContext(boxMon, reencrypt);
}

static inline int Pokemon_Face(int clientType)
{
    return (clientType & 1) ? 0 : 1;
}

void PokemonSprite_LoadAnimFrames(NARC *narc, SpriteAnimFrame *frames, u16 species, u16 clientType)
{
    int face = Pokemon_Face(clientType);

    SpeciesSpriteData data;
    NARC_ReadFromMember(narc, 0, species * sizeof(SpeciesSpriteData), sizeof(SpeciesSpriteData), &data);
    MI_CpuCopy8(data.faceAnims[face].frames, frames, sizeof(SpriteAnimFrame) * MAX_ANIMATION_FRAMES);
}

void PokemonSprite_LoadAnim(NARC *narc, PokemonAnimManager *monAnimMan, PokemonSprite *sprite, u16 species, int face, int flipSprite, int frame)
{
    int faceType = (face == FACE_FRONT) ? 0 : 1;

    SpeciesSpriteData spriteData;
    NARC_ReadFromMember(narc, 0, species * sizeof(SpeciesSpriteData), sizeof(SpeciesSpriteData), &spriteData);

    PokemonAnimTemplate animTemplate;
    animTemplate.animation = spriteData.faceAnims[faceType].animation;
    animTemplate.startDelay = spriteData.faceAnims[faceType].startDelay;
    animTemplate.flipSprite = flipSprite;

    PokemonAnimManager_InitAnim(monAnimMan, sprite, &animTemplate, frame);
}

void PokemonSprite_LoadCryDelay(NARC *narc, u8 *cryDelay, u16 species, u16 clientType)
{
    int face = Pokemon_Face(clientType);

    SpeciesSpriteData data;
    NARC_ReadFromMember(narc, 0, species * sizeof(SpeciesSpriteData), sizeof(SpeciesSpriteData), &data);

    *cryDelay = data.faceAnims[face].cryDelay;
}

void PokemonSprite_LoadYOffset(NARC *narc, s8 *yOffset, u16 species)
{
    SpeciesSpriteData data;

    NARC_ReadFromMember(narc, 0, species * sizeof(SpeciesSpriteData), sizeof(SpeciesSpriteData), &data);
    *yOffset = data.yOffset;
}

void PokemonSprite_LoadXOffsetShadow(NARC *narc, s8 *xOffsetShadow, u16 species)
{
    SpeciesSpriteData data;

    NARC_ReadFromMember(narc, 0, species * sizeof(SpeciesSpriteData), sizeof(SpeciesSpriteData), &data);
    *xOffsetShadow = data.xOffsetShadow;
}

void PokemonSprite_LoadShadowSize(NARC *narc, u8 *shadowSize, u16 species)
{
    SpeciesSpriteData data;

    NARC_ReadFromMember(narc, 0, species * sizeof(SpeciesSpriteData), sizeof(SpeciesSpriteData), &data);
    *shadowSize = data.shadowSize;
}

BOOL Pokemon_SetBallSeal(int param0, Pokemon *mon, enum HeapID heapID)
{
    int v0 = param0;

    if (v0 == 0) {
        return FALSE;
    }

    NARC *narc = NARC_ctor(NARC_INDEX_APPLICATION__CUSTOM_BALL__EDIT__PL_CB_DATA, heapID);
    v0 = v0 - 1;
    int one = 1;

    BallCapsule v2;
    NARC_ReadFromMember(narc, 0, v0 * sizeof(BallCapsule), sizeof(BallCapsule), &v2);
    Pokemon_SetValue(mon, MON_DATA_BALL_CAPSULE_ID, &one);
    Pokemon_SetValue(mon, MON_DATA_BALL_CAPSULE, &v2);
    NARC_dtor(narc);

    return TRUE;
}

void sub_02078B40(Pokemon *mon, UnkStruct_02078B40 *param1)
{
    if (mon->box.partyDecrypted == FALSE) {
        Pokemon_DecryptData(&mon->party, sizeof(PartyPokemon), mon->box.personality);
        Pokemon_DecryptData(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4, mon->box.checksum);
    }

    BoxPokemon *boxMon = Pokemon_GetBoxPokemon(mon);

    PokemonDataBlockA *monDataBlockA = BoxPokemon_GetDataBlock(boxMon, boxMon->personality, DATA_BLOCK_A);
    PokemonDataBlockB *monDataBlockB = BoxPokemon_GetDataBlock(boxMon, boxMon->personality, DATA_BLOCK_B);
    PokemonDataBlockC *monDataBlockC = BoxPokemon_GetDataBlock(boxMon, boxMon->personality, DATA_BLOCK_C);
    PokemonDataBlockD *monDataBlockD = BoxPokemon_GetDataBlock(boxMon, boxMon->personality, DATA_BLOCK_D);

    param1->personality = boxMon->personality;
    param1->partyDecrypted = FALSE;
    param1->boxDecrypted = FALSE;
    param1->checksumFailed = boxMon->checksumFailed;
    param1->species = monDataBlockA->species;
    param1->heldItem = monDataBlockA->heldItem;
    param1->otID = monDataBlockA->otID;
    param1->exp = monDataBlockA->exp;
    param1->friendship = monDataBlockA->friendship;
    param1->ability = monDataBlockA->ability;
    param1->hpEV = monDataBlockA->hpEV;
    param1->atkEV = monDataBlockA->atkEV;
    param1->defEV = monDataBlockA->defEV;
    param1->speedEV = monDataBlockA->speedEV;
    param1->spAtkEV = monDataBlockA->spAtkEV;
    param1->spDefEV = monDataBlockA->spDefEV;
    param1->originLanguage = monDataBlockA->originLanguage;

    int i;
    for (i = 0; i < 4; i++) {
        param1->unk_1C[i] = monDataBlockB->moves[i];
        param1->unk_24[i] = monDataBlockB->moveCurrentPPs[i];
        param1->unk_28[i] = monDataBlockB->movePPUps[i];
    }

    param1->hpIV = monDataBlockB->hpIV;
    param1->atkIV = monDataBlockB->atkIV;
    param1->defIV = monDataBlockB->defIV;
    param1->speedIV = monDataBlockB->speedIV;
    param1->spAtkIV = monDataBlockB->spAtkIV;
    param1->spDefIV = monDataBlockB->spDefIV;
    param1->isEgg = monDataBlockB->isEgg;
    param1->unk_2C_31 = monDataBlockB->hasNickname;
    param1->fatefulEncounter = monDataBlockB->fatefulEncounter;
    param1->gender = monDataBlockB->gender;
    param1->form = monDataBlockB->form;

    for (i = 0; i < 10 + 1; i++) {
        param1->unk_32[i] = monDataBlockC->nickname[i];
    }

    for (i = 0; i < 7 + 1; i++) {
        param1->unk_48[i] = monDataBlockD->otName[i];
    }

    param1->pokeball = monDataBlockD->pokeball;

    param1->unk_5C = mon->party.status;
    param1->level = mon->party.level;
    param1->unk_61 = mon->party.ballCapsuleID;
    param1->unk_62 = mon->party.hp;
    param1->unk_64 = mon->party.maxHP;
    param1->unk_66 = mon->party.attack;
    param1->unk_68 = mon->party.defense;
    param1->unk_6A = mon->party.speed;
    param1->unk_6C = mon->party.spAtk;
    param1->unk_6E = mon->party.spDef;

    if (mon->box.partyDecrypted == FALSE) {
        Pokemon_EncryptData(&mon->party, sizeof(PartyPokemon), mon->box.personality);
        Pokemon_EncryptData(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4, mon->box.checksum);
    }
}

void sub_02078E0C(UnkStruct_02078B40 *param0, Pokemon *mon)
{
    MI_CpuClearFast(mon, sizeof(Pokemon));

    BoxPokemon *boxMon = Pokemon_GetBoxPokemon(mon);

    PokemonDataBlockA *monDataBlockA = BoxPokemon_GetDataBlock(boxMon, param0->personality, DATA_BLOCK_A);
    PokemonDataBlockB *monDataBlockB = BoxPokemon_GetDataBlock(boxMon, param0->personality, DATA_BLOCK_B);
    PokemonDataBlockC *monDataBlockC = BoxPokemon_GetDataBlock(boxMon, param0->personality, DATA_BLOCK_C);
    PokemonDataBlockD *monDataBlockD = BoxPokemon_GetDataBlock(boxMon, param0->personality, DATA_BLOCK_D);

    boxMon->personality = param0->personality;
    boxMon->partyDecrypted = FALSE;
    boxMon->boxDecrypted = FALSE;
    boxMon->checksumFailed = param0->checksumFailed;

    monDataBlockA->species = param0->species;
    monDataBlockA->heldItem = param0->heldItem;
    monDataBlockA->otID = param0->otID;
    monDataBlockA->exp = param0->exp;
    monDataBlockA->friendship = param0->friendship;
    monDataBlockA->ability = param0->ability;
    monDataBlockA->hpEV = param0->hpEV;
    monDataBlockA->atkEV = param0->atkEV;
    monDataBlockA->defEV = param0->defEV;
    monDataBlockA->speedEV = param0->speedEV;
    monDataBlockA->spAtkEV = param0->spAtkEV;
    monDataBlockA->spDefEV = param0->spDefEV;
    monDataBlockA->originLanguage = param0->originLanguage;

    int i;
    for (i = 0; i < 4; i++) {
        monDataBlockB->moves[i] = param0->unk_1C[i];
        monDataBlockB->moveCurrentPPs[i] = param0->unk_24[i];
        monDataBlockB->movePPUps[i] = param0->unk_28[i];
    }

    monDataBlockB->hpIV = param0->hpIV;
    monDataBlockB->atkIV = param0->atkIV;
    monDataBlockB->defIV = param0->defIV;
    monDataBlockB->speedIV = param0->speedIV;
    monDataBlockB->spAtkIV = param0->spAtkIV;
    monDataBlockB->spDefIV = param0->spDefIV;
    monDataBlockB->isEgg = param0->isEgg;
    monDataBlockB->hasNickname = param0->unk_2C_31;
    monDataBlockB->fatefulEncounter = param0->fatefulEncounter;
    monDataBlockB->gender = param0->gender;
    monDataBlockB->form = param0->form;

    for (i = 0; i < 10 + 1; i++) {
        monDataBlockC->nickname[i] = param0->unk_32[i];
    }

    for (i = 0; i < 7 + 1; i++) {
        monDataBlockD->otName[i] = param0->unk_48[i];
    }

    monDataBlockD->pokeball = param0->pokeball;

    mon->party.status = param0->unk_5C;
    mon->party.level = param0->level;
    mon->party.ballCapsuleID = param0->unk_61;
    mon->party.hp = param0->unk_62;
    mon->party.maxHP = param0->unk_64;
    mon->party.attack = param0->unk_66;
    mon->party.defense = param0->unk_68;
    mon->party.speed = param0->unk_6A;
    mon->party.spAtk = param0->unk_6C;
    mon->party.spDef = param0->unk_6E;

    Pokemon_EncryptData(&mon->party, sizeof(PartyPokemon), mon->box.personality);
    mon->box.checksum = Pokemon_GetDataChecksum(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4);
    Pokemon_EncryptData(&mon->box.dataBlocks, sizeof(PokemonDataBlock) * 4, mon->box.checksum);
}
