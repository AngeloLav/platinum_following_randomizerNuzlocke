#include "run_store.h"

#include <string.h>

u32 RunStore_Address(u8 slot)
{
    return (RUN_STORE_FIRST_SECTOR + slot * RUN_STORE_HALF_SECTORS) * RUN_STORE_SECTOR_SIZE;
}

BOOL RunStore_Prepare(const RunStoreIO *io, RunState *state, u32 nextNormalCounter, u8 targetSlot)
{
    if (io == NULL || io->write == NULL || state == NULL || targetSlot > 1) {
        return FALSE;
    }
    RunState_Seal(state, nextNormalCounter);
    if (!RunState_Validate(state, RunState_Nonce(state), nextNormalCounter)) {
        return FALSE;
    }
    u32 base = RunStore_Address(targetSlot);
    u8 invalidHeader[32] = { 0 };
    if (!io->write(io->context, base, invalidHeader, sizeof(invalidHeader))) {
        return FALSE;
    }
    // Commit the extension before the corresponding native normal-block footer.
    // Never overwrite the slot paired with the currently committed native block.
    for (u32 offset = 32; offset < RUN_STATE_SIZE; offset += RUN_STORE_SECTOR_SIZE) {
        u32 size = RUN_STATE_SIZE - offset;
        if (size > RUN_STORE_SECTOR_SIZE) {
            size = RUN_STORE_SECTOR_SIZE;
        }
        if (!io->write(io->context, base + offset, state->bytes + offset, size)) {
            return FALSE;
        }
    }
    return io->write(io->context, base, state->bytes, 32);
}

BOOL RunStore_Load(const RunStoreIO *io, RunState *destination, RunState *scratch, u32 nonce, u32 normalCounter)
{
    if (io == NULL || io->read == NULL || destination == NULL || scratch == NULL || destination == scratch) {
        return FALSE;
    }
    for (u8 slot = 0; slot < 2; slot++) {
        if (io->read(io->context, RunStore_Address(slot), scratch, sizeof(*scratch))
            && RunState_Validate(scratch, nonce, normalCounter)) {
            memcpy(destination, scratch, sizeof(*destination));
            RunState_RecoverInterrupted(destination);
            return TRUE;
        }
    }
    // Never replace a live run with zeroes/Classic after a failed read.
    return FALSE;
}
