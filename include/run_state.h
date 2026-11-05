#ifndef POKEPLATINUM_RUN_STATE_H
#define POKEPLATINUM_RUN_STATE_H

#include "run_config.h"

// Explicit little-endian storage. No compiler-layout structs are written to flash.
#define RUN_STATE_SIZE       65536
#define RUN_ZONE_COUNT       256
#define RUN_MON_CAPACITY     1024
#define RUN_SPECIAL_CAPACITY 256
#define RUN_HISTORY_NAME_LEN 11
#define RUN_STATE_SCHEMA     1

enum RunStatus {
    RUN_STATUS_UNCONFIGURED = 0,
    RUN_STATUS_ACTIVE,
    RUN_STATUS_GAME_OVER,
    RUN_STATUS_DAMAGED
};

enum RunEncounterSource {
    RUN_SOURCE_WILD = 0,
    RUN_SOURCE_STATIC,
    RUN_SOURCE_GIFT,
    RUN_SOURCE_EGG,
    RUN_SOURCE_SHINY
};

enum RunEncounterOutcome {
    RUN_OUTCOME_NONE = 0,
    RUN_OUTCOME_PENDING,
    RUN_OUTCOME_CAUGHT,
    RUN_OUTCOME_DEFEATED,
    RUN_OUTCOME_FLED,
    RUN_OUTCOME_FAILED,
    RUN_OUTCOME_INTERRUPTED
};

enum RunCapturePermission {
    RUN_CAPTURE_ALLOWED = 0,
    RUN_CAPTURE_INACTIVE,
    RUN_CAPTURE_DUPLICATE,
    RUN_CAPTURE_EXHAUSTED,
    RUN_CAPTURE_HISTORY_FULL,
    RUN_CAPTURE_INVALID
};

typedef struct RunState {
    u8 bytes[RUN_STATE_SIZE];
} RunState;

typedef struct RunOpportunity {
    u16 species;
    u16 monID;
    u8 outcome;
    u8 source;
} RunOpportunity;

typedef struct RunMonRecord {
    u32 personality;
    u32 trainerID;
    u16 originalSpecies;
    u16 species;
    u16 originZone;
    u16 deathZone;
    u16 specialID;
    u16 evolutions[3];
    u16 nickname[RUN_HISTORY_NAME_LEN];
    u8 flags;
    u8 evolutionCount;
} RunMonRecord;

typedef struct RunSpecialRecord {
    u32 eventKey;
    u16 zone;
    u16 species;
    u16 lastMonID;
    u16 attempts;
    u16 caught;
    u16 failed;
    u8 source;
    u8 outcome;
} RunSpecialRecord;

#define RUN_MON_EXHAUSTED 0x80
#define RUN_MON_SHINY     0x40
#define RUN_MON_EGG       0x20
#define RUN_MON_SOURCE_MASK 7

BOOL RunState_Init(RunState *state, const RunConfig *config, u32 nonce);
BOOL RunState_Config(const RunState *state, RunConfig *config);
enum RunStatus RunState_Status(const RunState *state);
void RunState_SetGameOver(RunState *state);
BOOL RunState_CaptureActivated(const RunState *state);
void RunState_ActivateCapture(RunState *state);
void RunState_Visit(RunState *state, u16 zone);
BOOL RunState_Visited(const RunState *state, u16 zone);
u16 RunState_DuplicatesSeen(const RunState *state, u16 zone);
u8 RunState_Remaining(const RunState *state, u16 zone);
BOOL RunState_GetOpportunity(const RunState *state, u16 zone, u8 slot, RunOpportunity *opportunity);
enum RunCapturePermission RunState_Reserve(RunState *state, u16 zone, u16 species, enum RunEncounterSource source, BOOL shiny, u8 *slot);
BOOL RunState_Finish(RunState *state, u16 zone, u8 slot, enum RunEncounterOutcome outcome, u16 monID);
BOOL RunState_OwnedFamily(const RunState *state, u16 species);
void RunState_MarkOwned(RunState *state, u16 species);
u16 RunState_MonCount(const RunState *state);
BOOL RunState_GetMon(const RunState *state, u16 id, RunMonRecord *record);
BOOL RunState_PutMon(RunState *state, u16 id, const RunMonRecord *record);
u16 RunState_AddMon(RunState *state, const RunMonRecord *record);
BOOL RunState_ExhaustMon(RunState *state, u16 id, u16 zone);
u16 RunState_SpecialCount(const RunState *state);
BOOL RunState_GetSpecial(const RunState *state, u16 id, RunSpecialRecord *record);
u16 RunState_SpecialAttempt(RunState *state, u32 eventKey, u16 zone, u16 species, enum RunEncounterSource source);
BOOL RunState_FinishSpecial(RunState *state, u16 id, enum RunEncounterOutcome outcome, u16 monID);
u32 RunState_Nonce(const RunState *state);
void RunState_Seal(RunState *state, u32 normalCounter);
BOOL RunState_Validate(const RunState *state, u32 nonce, u32 normalCounter);
void RunState_RecoverInterrupted(RunState *state);

#endif // POKEPLATINUM_RUN_STATE_H
