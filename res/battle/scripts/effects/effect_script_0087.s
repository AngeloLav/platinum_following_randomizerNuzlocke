#include "macros/btlcmd.inc"

    // Seismic Toss, Night Shade. ENTRY P113: 1.5x the user's level, rounded up ((level * 3 + 1) / 2), was 1x.

_000:
    UpdateVar OPCODE_FLAG_ON, BTLVAR_BATTLE_CTX_STATUS, SYSCTL_IGNORE_TYPE_CHECKS
    UpdateMonDataFromVar OPCODE_GET, BTLSCR_ATTACKER, BATTLEMON_LEVEL, BTLVAR_DAMAGE
    UpdateVar OPCODE_MUL, BTLVAR_DAMAGE, 3
    UpdateVar OPCODE_ADD, BTLVAR_DAMAGE, 1
    UpdateVar OPCODE_DIV, BTLVAR_DAMAGE, 2
    UpdateVar OPCODE_MUL, BTLVAR_DAMAGE, -1
    End
