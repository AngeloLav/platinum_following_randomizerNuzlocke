#include "run_extras.h"
#include <stddef.h>
#include "run_randomizer.h"
#include "generated/items.h"
#include "generated/species.h"

static u32 Mix(u32 x)
{
    x ^= x >> 16; x *= 0x7FEB352Du; x ^= x >> 15; x *= 0x846CA68Bu;
    return x ^ (x >> 16);
}

BOOL RunExtras_UsableBall(u16 item)
{
    return item >= ITEM_MASTER_BALL && item <= ITEM_CHERISH_BALL && item != ITEM_SAFARI_BALL;
}

BOOL RunExtras_SafeFieldItem(u16 item)
{
    // Explicit Gen-IV ordinary ranges; protect quests, fossils, orbs, unused
    // IDs, mail, all TMs/HMs and Key Items. Only pickup callers use this.
    return item >= ITEM_MASTER_BALL && item <= ITEM_RAZOR_FANG
        && item != ITEM_SAFARI_BALL
        && !(item >= ITEM_ROOT_FOSSIL && item <= ITEM_SKULL_FOSSIL)
        && !(item >= ITEM_ODD_KEYSTONE && item <= ITEM_UNUSED_134)
        && item != ITEM_ADAMANT_ORB && item != ITEM_LUSTROUS_ORB
        && !(item >= ITEM_GRASS_MAIL && item <= ITEM_BRICK_MAIL);
}

u16 RunExtras_FieldItem(const RunConfig *config, u32 locationKey, u16 original)
{
    static const u16 pool[] = {
        ITEM_ULTRA_BALL, ITEM_GREAT_BALL, ITEM_POKE_BALL, ITEM_NET_BALL,
        ITEM_NEST_BALL, ITEM_DUSK_BALL, ITEM_HEAL_BALL, ITEM_QUICK_BALL,
        ITEM_POTION, ITEM_SUPER_POTION, ITEM_HYPER_POTION, ITEM_MAX_POTION,
        ITEM_FULL_RESTORE, ITEM_ANTIDOTE, ITEM_BURN_HEAL, ITEM_ICE_HEAL,
        ITEM_AWAKENING, ITEM_PARLYZ_HEAL, ITEM_FULL_HEAL, ITEM_REVIVE,
        ITEM_ETHER, ITEM_MAX_ETHER, ITEM_ELIXIR, ITEM_MAX_ELIXIR,
        ITEM_HP_UP, ITEM_PROTEIN, ITEM_IRON, ITEM_CARBOS, ITEM_CALCIUM, ITEM_ZINC,
        ITEM_PP_UP, ITEM_RARE_CANDY, ITEM_REPEL, ITEM_SUPER_REPEL, ITEM_MAX_REPEL,
        ITEM_ESCAPE_ROPE, ITEM_RED_SHARD, ITEM_BLUE_SHARD, ITEM_YELLOW_SHARD,
        ITEM_GREEN_SHARD, ITEM_STARDUST, ITEM_STAR_PIECE, ITEM_NUGGET,
        ITEM_HEART_SCALE, ITEM_HONEY, ITEM_CHERI_BERRY, ITEM_CHESTO_BERRY,
        ITEM_PECHA_BERRY, ITEM_RAWST_BERRY, ITEM_ASPEAR_BERRY, ITEM_LEPPA_BERRY,
        ITEM_ORAN_BERRY, ITEM_SITRUS_BERRY, ITEM_LUM_BERRY
    };
    if (!RunConfig_IsValid(config) || !(config->flags & RUN_RANDOMIZE_FIELD_ITEMS)
        || !RunExtras_SafeFieldItem(original)) return original;
    u32 value = Mix(config->seed ^ 0x4954454Du ^ config->speciesDataVersion);
    value = Mix(value ^ locationKey);
    value = Mix(value ^ original);
    u32 count = sizeof(pool) / sizeof(pool[0]);
    u32 state = value;
    do {
        state += 0x9E3779B9u;
        value = Mix(state);
    } while (value < (0u - count) % count);
    return pool[value % count];
}

u16 RunExtras_IntroSpecies(const RunConfig *config)
{
    if (!RunConfig_IsValid(config)) return SPECIES_BUNEARY;
    return 1 + Mix(config->seed ^ config->speciesDataVersion ^ 0x524F574Eu) % RunSpecies_Count();
}

u16 RunExtras_Starter(const RunConfig *config, u8 slot)
{
    static const u16 originals[3] = { SPECIES_TURTWIG, SPECIES_CHIMCHAR, SPECIES_PIPLUP };
    if (slot >= 3) return SPECIES_NONE;
    if (config != NULL && config->rulesVersion == 2) {
        return RunRandomizer_SelectSpecies(config, RUN_DOMAIN_GIFT,
            RUN_LEGACY_STARTER_EVENT_KEY, 0, originals[slot]);
    }
    return RunRandomizer_SelectSpecies(config, RUN_DOMAIN_GIFT, 0x53544152u, slot, originals[slot]);
}

static u8 Unown(u32 p)
{
    return (((p & 0x03000000u) >> 18) | ((p & 0x00030000u) >> 12)
        | ((p & 0x00000300u) >> 6) | (p & 3)) % 28;
}

u32 RunExtras_ShinyPID(u32 personality, u32 trainerID, u8 rate, u8 constraint)
{
    u16 x = (personality >> 16) ^ personality ^ (trainerID >> 16) ^ trainerID;
    // 8/16/32/64-wide XOR intervals implement the four probabilities.
    // Existing/forced shinies and all ineligible draws are untouched.
    if (rate == 0 || rate > 3 || x < 8 || x >= (8u << rate)) return personality;
    u16 ot = trainerID ^ (trainerID >> 16);
    u32 nature = personality % 25;
    // Keep low byte (gender/ability bit). Bounded construction, no game RNG.
    for (u32 i = 0; i < 256; i++) {
        u16 low = (personality & 255) | (((personality >> 8) + i) & 255) << 8;
        for (u32 shiny = 0; shiny < 8; shiny++) {
            u32 candidate = low | ((u32)(low ^ ot ^ shiny) << 16);
            if (candidate % 25 != nature) continue;
            if (constraint == 1 && Unown(candidate) != Unown(personality)) continue;
            if (constraint == 2 && ((candidate >> 16) % 10 < 5) != ((personality >> 16) % 10 < 5)) continue;
            return candidate;
        }
    }
    return personality; // Never sacrifice a required PID constraint.
}
