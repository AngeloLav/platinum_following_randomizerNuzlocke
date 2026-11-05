#ifndef POKEPLATINUM_RUN_STORE_H
#define POKEPLATINUM_RUN_STORE_H

#include "run_state.h"

// Retail extras end at sector 43 in each 64-sector half. Reserve 44..59 and
// 108..123 for the two run snapshots; leave all native tables/footers in place.
#define RUN_STORE_SECTOR_SIZE 4096
#define RUN_STORE_FIRST_SECTOR 44
#define RUN_STORE_HALF_SECTORS 64

typedef struct RunStoreIO {
    void *context;
    BOOL (*read)(void *context, u32 address, void *data, u32 size);
    BOOL (*write)(void *context, u32 address, const void *data, u32 size);
} RunStoreIO;

u32 RunStore_Address(u8 slot);
BOOL RunStore_Prepare(const RunStoreIO *io, RunState *state, u32 nextNormalCounter, u8 targetSlot);
BOOL RunStore_Load(const RunStoreIO *io, RunState *destination, RunState *scratch, u32 nonce, u32 normalCounter);

#endif // POKEPLATINUM_RUN_STORE_H
