#include "macros/btlcmd.inc"

    // Psywave. ENTRY P113: 1x to 2x the user's level in steps of 0.1, rounded up ((level * (10..20) + 9) / 10), was 0.5x to
    // 1.5x rounded down.

_000:
    UpdateVar OPCODE_FLAG_ON, BTLVAR_BATTLE_CTX_STATUS, SYSCTL_IGNORE_TYPE_CHECKS
    Random 10, 10
    UpdateMonDataFromVar OPCODE_GET, BTLSCR_ATTACKER, BATTLEMON_LEVEL, BTLVAR_DAMAGE
    UpdateVarFromVar OPCODE_MUL, BTLVAR_DAMAGE, BTLVAR_CALC_TEMP
    UpdateVar OPCODE_ADD, BTLVAR_DAMAGE, 9
    UpdateVar OPCODE_DIV, BTLVAR_DAMAGE, 10
    CompareVarToValue OPCODE_NEQ, BTLVAR_DAMAGE, 0, _028
    UpdateVar OPCODE_SET, BTLVAR_DAMAGE, 1

_028:
    UpdateVar OPCODE_MUL, BTLVAR_DAMAGE, -1
    End
