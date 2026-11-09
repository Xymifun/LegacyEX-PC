#ifndef GAME_COMMAND_EGAMECOMMAND_HPP
#define GAME_COMMAND_EGAMECOMMAND_HPP

enum EGameCommand
{
    eGameCommand_DefaultGameMode = 0,
    eGameCommand_Effect = 1,
    eGameCommand_EnchantItem = 2,
    eGameCommand_Experience = 3,
    eGameCommand_GameMode = 4,
    eGameCommand_GiveItem = 5,
    eGameCommand_Kill = 6,
    eGameCommand_Time = 7,
    eGameCommand_ToggleDownfall = 8,
    eGameCommand_Teleport = 9,
    eGameCommand_GameDifficulty = 10,
    eGameCommand_SetDefaultSpawnpoint = 11,
    eGameCommand_Weather = 12,
    eGameCommand_SetSpawnpoint = 13,
    eGameCommand_Achievement = 15,
    eGameCommand_BlockData = 16,
    eGameCommand_Clone = 17,
    eGameCommand_Compare = 18,
    eGameCommand_EntityData = 19,
    eGameCommand_ExecuteAt = 20,
    eGameCommand_Fill = 21,
    eGameCommand_MessageRaw = 22,
    eGameCommand_Particle = 23,
    eGameCommand_Say = 24,
    eGameCommand_ReplaceItem = 25,
    eGameCommand_SetBlock = 26,
    eGameCommand_Stats = 27,
    eGameCommand_Summon = 28,
    eGameCommand_TestForBlock = 29,
    eGameCommand_Title = 30,
    eGameCommand_Trigger = 31,
    eGameCommand_WorldBorder = 32,
    eGameCommand_Checkpoint = 33,
    eGameCommand_ChangeCollisionType = 34
};

#endif
