#include "macros/btlcmd.inc"

    // ENTRY P99: Arceus transforms into the type BattleSystem_TryArceusTransform picked (its battle types and form are
    // already changed). Same effect as subscript_form_change (Transform's sound, pixelate out, new form, pixelate in),
    // for the defender, after "<attacker> used <move>!" has finished.

_000:
    Wait
    CompareMonDataToValue OPCODE_FLAG_NOT, BTLSCR_DEFENDER, BATTLEMON_VOLATILE_STATUS, VOLATILE_CONDITION_SUBSTITUTE, _017
    PlayBattleAnimation BTLSCR_DEFENDER, BATTLE_ANIMATION_SUB_OUT
    Wait
    RestoreSprite BTLSCR_DEFENDER
    Wait
    PlayBattleAnimation BTLSCR_DEFENDER, BATTLE_ANIMATION_SUB_IN
    Wait

_017:
    PlaySound BTLSCR_DEFENDER, 1980
    SetMosaic BTLSCR_DEFENDER, 8, 1
    Wait
    ChangeForm BTLSCR_DEFENDER
    PlaySound BTLSCR_DEFENDER, 1984
    SetMosaic BTLSCR_DEFENDER, 0, 1
    Wait
    // {0} transformed!
    PrintMessage BattleStrings_Text_PokemonTransformed_Ally, TAG_NICKNAME, BTLSCR_DEFENDER
    Wait
    WaitButtonABTime 30
    CompareMonDataToValue OPCODE_FLAG_NOT, BTLSCR_DEFENDER, BATTLEMON_VOLATILE_STATUS, VOLATILE_CONDITION_SUBSTITUTE, _059
    PlayBattleAnimation BTLSCR_DEFENDER, BATTLE_ANIMATION_SUB_OUT
    Wait
    RefreshSprite BTLSCR_DEFENDER
    Wait
    PlayBattleAnimation BTLSCR_DEFENDER, BATTLE_ANIMATION_SUB_IN
    Wait

_059:
    End
