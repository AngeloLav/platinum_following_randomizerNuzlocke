#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "run_state.h"
#include "run_store.h"
#include "run_world.h"

static RunState state, scratch, restored, before;
static u8 flash[512 * 1024];
static u8 backup[512 * 1024];
static int writes, failAt;

static BOOL Read(void *context, u32 address, void *data, u32 size)
{
    (void)context;
    assert(address >= 44 * 4096 && address + size <= 124 * 4096);
    memcpy(data, flash + address, size);
    return TRUE;
}

static BOOL Write(void *context, u32 address, const void *data, u32 size)
{
    (void)context;
    assert((address >= 44 * 4096 && address + size <= 60 * 4096)
        || (address >= 108 * 4096 && address + size <= 124 * 4096));
    if (failAt >= 0 && writes++ == failAt) {
        // Simulate power loss in the middle of a backup write.
        memcpy(flash + address, data, size / 2);
        return FALSE;
    }
    memcpy(flash + address, data, size);
    return TRUE;
}

static void Reset(u8 opportunities)
{
    RunConfig config;
    assert(RunConfig_Init(&config, RUN_PRESET_HARDCORE_RANDOMIZER, 1));
    config.captureAttempts = opportunities;
    assert(RunState_Init(&state, &config, 0xABCDEF01u));
}

static void TestOpportunities(void)
{
    for (u8 limit = 1; limit <= 3; limit++) {
        Reset(limit);
        u8 slot = 99;
        assert(RunState_Reserve(&state, 201, 399, RUN_SOURCE_WILD, FALSE, &slot) == RUN_CAPTURE_INACTIVE);
        assert(RunState_Visited(&state, 201));
        assert(RunState_Remaining(&state, 201) == limit);
        RunState_ActivateCapture(&state);
        for (u8 i = 0; i < limit; i++) {
            assert(RunState_Reserve(&state, 201, 399, RUN_SOURCE_WILD, FALSE, &slot) == RUN_CAPTURE_ALLOWED);
            assert(slot == i);
            assert(RunState_Remaining(&state, 201) == limit - i - 1);
            enum RunEncounterOutcome outcome = i == 0 ? RUN_OUTCOME_DEFEATED : RUN_OUTCOME_FLED;
            assert(RunState_Finish(&state, 201, slot, outcome, 0));
            assert(RunState_Finish(&state, 201, slot, outcome, 0));
            assert(!RunState_Finish(&state, 201, slot, RUN_OUTCOME_FAILED, 0));
        }
        assert(RunState_Reserve(&state, 201, 399, RUN_SOURCE_WILD, FALSE, &slot) == RUN_CAPTURE_EXHAUSTED);
        assert(RunState_Reserve(&state, 201, 150, RUN_SOURCE_STATIC, FALSE, &slot) == RUN_CAPTURE_ALLOWED);
        assert(slot == 0xFF && RunState_Remaining(&state, 201) == 0);
        assert(RunState_Reserve(&state, 201, 399, RUN_SOURCE_WILD, TRUE, &slot) == RUN_CAPTURE_ALLOWED);
        assert(RunState_Reserve(&state, 201, 133, RUN_SOURCE_GIFT, FALSE, &slot) == RUN_CAPTURE_ALLOWED);
        assert(RunState_Reserve(&state, 201, 175, RUN_SOURCE_EGG, FALSE, &slot) == RUN_CAPTURE_ALLOWED);
        assert(RunState_Remaining(&state, 202) == limit);
    }
    Reset(1);
    RunState_ActivateCapture(&state);
    RunState_MarkOwned(&state, 133);
    u8 slot;
    assert(RunState_Reserve(&state, 203, 471, RUN_SOURCE_WILD, FALSE, &slot) == RUN_CAPTURE_DUPLICATE);
    assert(RunState_DuplicatesSeen(&state, 203) == 1);
    assert(RunState_Remaining(&state, 203) == 1);
    assert(RunState_Reserve(&state, 203, 471, RUN_SOURCE_STATIC, FALSE, &slot) == RUN_CAPTURE_ALLOWED);
    assert(RunState_Reserve(&state, 203, 471, RUN_SOURCE_WILD, TRUE, &slot) == RUN_CAPTURE_ALLOWED);
    assert(RunState_Reserve(&state, 203, 399, RUN_SOURCE_WILD, FALSE, &slot) == RUN_CAPTURE_ALLOWED);
    RunState_RecoverInterrupted(&state);
    RunOpportunity opportunity;
    assert(RunState_GetOpportunity(&state, 203, slot, &opportunity));
    assert(opportunity.outcome == RUN_OUTCOME_INTERRUPTED && RunState_Remaining(&state, 203) == 0);
    RunConfig shared;
    RunConfig_Init(&shared, RUN_PRESET_HARDCORE_RANDOMIZER, 1);
    shared.flags &= ~(RUN_SEPARATE_GIFTS | RUN_SEPARATE_EGGS);
    RunState_Init(&state, &shared, 1);
    RunState_ActivateCapture(&state);
    RunState_MarkOwned(&state, 133);
    assert(RunState_Reserve(&state, 203, 471, RUN_SOURCE_GIFT, FALSE, &slot) == RUN_CAPTURE_ALLOWED);
    assert(slot == 0 && RunState_Remaining(&state, 203) == 0);
    assert(RunState_Reserve(&state, 203, 175, RUN_SOURCE_EGG, FALSE, &slot) == RUN_CAPTURE_EXHAUSTED);
    Reset(0);
    RunState_ActivateCapture(&state);
    assert(RunState_Reserve(&state, 201, 399, RUN_SOURCE_WILD, FALSE, &slot) == RUN_CAPTURE_INACTIVE);
}

static void TestHistory(void)
{
    Reset(3);
    RunMonRecord mon;
    memset(&mon, 0, sizeof(mon));
    mon.originalSpecies = mon.species = 133;
    mon.originZone = 203;
    mon.deathZone = 0xFFFF;
    mon.personality = 123;
    mon.trainerID = 456;
    u16 special = RunState_SpecialAttempt(&state, 0xEE0011, 203, 133, RUN_SOURCE_GIFT);
    assert(special == 1);
    mon.specialID = special;
    mon.flags = RUN_SOURCE_GIFT;
    u16 id = RunState_AddMon(&state, &mon);
    assert(id == 1 && RunState_OwnedFamily(&state, 471));
    assert(RunState_FinishSpecial(&state, special, RUN_OUTCOME_CAUGHT, id));
    assert(RunState_FinishSpecial(&state, special, RUN_OUTCOME_CAUGHT, id));
    mon.species = 470;
    mon.evolutions[0] = 470;
    mon.evolutionCount = 1;
    mon.nickname[0] = 222;
    assert(RunState_PutMon(&state, id, &mon));
    assert(RunState_ExhaustMon(&state, id, 206));
    assert(RunState_ExhaustMon(&state, id, 207));
    assert(RunState_GetMon(&state, id, &mon));
    assert(mon.deathZone == 206 && mon.originalSpecies == 133 && mon.species == 470);
    assert(mon.flags & RUN_MON_EXHAUSTED);
    assert(mon.nickname[0] == 222 && mon.evolutions[0] == 470);
    RunSpecialRecord event;
    assert(RunState_GetSpecial(&state, special, &event));
    assert(event.caught == 1 && event.attempts == 1);
    assert(RunState_SpecialAttempt(&state, 0xEE0011, 203, 133, RUN_SOURCE_GIFT) == special);
    assert(RunState_FinishSpecial(&state, special, RUN_OUTCOME_FLED, 0));
    assert(RunState_GetSpecial(&state, special, &event));
    assert(event.attempts == 2 && event.caught == 1 && event.failed == 1);
    for (u16 i = 1; i < RUN_MON_CAPACITY; i++) {
        mon.personality++;
        assert(RunState_AddMon(&state, &mon) == i + 1);
    }
    assert(RunState_AddMon(&state, &mon) == 0);
    assert(RunState_MonCount(&state) == RUN_MON_CAPACITY);
    RunState_ActivateCapture(&state);
    u8 fullSlot;
    assert(RunState_Reserve(&state, 203, 399, RUN_SOURCE_WILD, FALSE, &fullSlot) == RUN_CAPTURE_HISTORY_FULL);
    assert(RunState_Reserve(&state, 203, 150, RUN_SOURCE_STATIC, FALSE, &fullSlot) == RUN_CAPTURE_ALLOWED);
    assert(RunState_Reserve(&state, 203, 399, RUN_SOURCE_WILD, TRUE, &fullSlot) == RUN_CAPTURE_ALLOWED);
    assert(RunState_Reserve(&state, 203, 133, RUN_SOURCE_GIFT, FALSE, &fullSlot) == RUN_CAPTURE_ALLOWED);
    u16 unlinkedGift = RunState_SpecialAttempt(&state, 0xEE0012, 203, 133, RUN_SOURCE_GIFT);
    assert(RunState_FinishSpecial(&state, unlinkedGift, RUN_OUTCOME_CAUGHT, 0));
    assert(RunState_GetSpecial(&state, unlinkedGift, &event));
    assert(event.caught == 1 && event.lastMonID == 0 && RunState_Remaining(&state, 203) == 3);
    RunState_Seal(&state, 99);
    assert(RunState_Validate(&state, 0xABCDEF01u, 99));
    assert(!RunState_Validate(&state, 0xABCDEF02u, 99));
    assert(!RunState_Validate(&state, 0xABCDEF01u, 98));
    RunState_SetGameOver(&state);
    RunState_Seal(&state, 100);
    assert(RunState_Status(&state) == RUN_STATUS_GAME_OVER);
    assert(RunState_Validate(&state, 0xABCDEF01u, 100));
}

static void TestStorage(void)
{
    RunStoreIO io = { NULL, Read, Write };
    Reset(3);
    memset(flash, 0xA5, sizeof(flash));
    failAt = -1;
    assert(RunStore_Prepare(&io, &state, 10, 0));
    memcpy(backup, flash, sizeof(flash));
    RunState_ActivateCapture(&state);
    u8 slot;
    assert(RunState_Reserve(&state, 203, 399, RUN_SOURCE_WILD, FALSE, &slot) == RUN_CAPTURE_ALLOWED);
    assert(RunState_Finish(&state, 203, slot, RUN_OUTCOME_FLED, 0));
    for (int cut = 0; cut < 18; cut++) {
        memcpy(flash, backup, sizeof(flash));
        failAt = cut;
        writes = 0;
        assert(!RunStore_Prepare(&io, &state, 11, 1));
        assert(RunStore_Load(&io, &restored, &scratch, 0xABCDEF01u, 10));
        assert(!RunState_CaptureActivated(&restored));
        assert(RunState_Remaining(&restored, 203) == 3);
        // No native or existing-extra bytes were touched.
        assert(memcmp(flash, backup, 44 * 4096) == 0);
        assert(memcmp(flash + 60 * 4096, backup + 60 * 4096, 48 * 4096) == 0);
        assert(memcmp(flash + 124 * 4096, backup + 124 * 4096, 4 * 4096) == 0);
    }
    failAt = -1;
    assert(RunStore_Prepare(&io, &state, 11, 1));
    // Old native footer still chooses the old run until its own commit.
    assert(RunStore_Load(&io, &restored, &scratch, 0xABCDEF01u, 10));
    assert(RunState_Remaining(&restored, 203) == 3);
    assert(RunStore_Load(&io, &restored, &scratch, 0xABCDEF01u, 11));
    assert(RunState_Remaining(&restored, 203) == 2);
    before = restored;
    flash[RunStore_Address(1) + 700] ^= 1;
    assert(!RunStore_Load(&io, &restored, &scratch, 0xABCDEF01u, 11));
    assert(memcmp(&before, &restored, sizeof(before)) == 0);
    assert(!RunStore_Load(&io, &restored, &scratch, 0xABCDEF01u, 12));
    assert(!RunStore_Prepare(&io, &state, 11, 2));
}

static void TestDifficulty(void)
{
    RunConfig config;
    assert(RunConfig_Init(&config, RUN_PRESET_HARDCORE_RANDOMIZER, 1));
    for (u8 badges = 0; badges <= 8; badges++) {
        assert(RunWorld_LevelCap(&config, badges) == RunWorld_BaseCap(badges));
        config.levelCapAllowance = 5;
        assert(RunWorld_LevelCap(&config, badges) == RunWorld_BaseCap(badges) + 5);
        config.levelCapAllowance = 0;
        for (u8 bonus = 1; bonus <= 3; bonus++) {
            config.trainerLevelsPerBadge = bonus;
            assert(RunWorld_TrainerLevel(&config, badges, 30, FALSE) == 30 + badges * bonus);
            assert(RunWorld_TrainerLevel(&config, badges, 99, FALSE) <= 100);
        }
    }
    config.flags &= ~RUN_SCALE_BOSSES;
    assert(RunWorld_TrainerLevel(&config, 8, 30, TRUE) == 30);
    config.flags &= ~RUN_TRAINER_SCALING;
    assert(RunWorld_TrainerLevel(&config, 8, 30, FALSE) == 30);
    config.flags &= ~RUN_LEVEL_CAPS;
    assert(RunWorld_LevelCap(&config, 0) == 100);
    assert(RunWorld_ClampExp(999, 500, 1000) == 1000);
    assert(RunWorld_ClampExp(999, 0xFFFFFFFFu, 1000) == 1000);
    assert(RunWorld_ClampExp(1000, 0xFFFFFFFFu, 1000) == 1000);
    assert(RunWorld_ClampExp(1001, 999, 1000) == 1001);
    assert(RunWorld_ClampExp(1, 1, 1000) == 2);
    assert(RunWorld_Zone(4095) == 0xFFFF);
    assert(!RunWorld_IsBoss(0xFFFFFFFFu));
}

int main(void)
{
    TestOpportunities();
    TestHistory();
    TestStorage();
    TestDifficulty();
    puts("PASS: capture opportunities, exemptions, dupes, history, exhaustion, CRC, 18 torn-write boundaries and native-sector preservation");
    return 0;
}
