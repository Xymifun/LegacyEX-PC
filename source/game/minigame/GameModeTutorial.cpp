#include "game/minigame/GameModeTutorial.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/GameModeConstraints.hpp"
#include "game/core/GameModeHint.hpp"

GameModeTutorial::GameModeTutorial(int playerId, GameModeConstraints* constraints, int gameMode)
    : Tutorial(playerId, constraints, false, true)
{
    if (gameMode == 1)
    {
        if (!isHintCompleted(eTutorial_Hint_BattleLobby))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode(eTutorial_GameMode_Lobby);
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_BattleLobby, this, (int)0xe18d25cc, modes, 1, true,
                                                  -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_BattleSelectMaps))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode(eTutorial_GameMode_Lobby);
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_BattleSelectMaps, this, (int)0x7a0c9966, modes, 1,
                                                  true, -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_BattleScoreboard))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode(eTutorial_GameMode_Scoreboard);
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_BattleScoreboard, this, (int)0x1015193c, modes, 1,
                                                  true, -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_BattleGracePeriod))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode(eTutorial_GameMode_GracePeriod);
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_BattleGracePeriod, this, (int)0xea36abc3, modes, 1,
                                                  true, -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_BattleExplore))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode(eTutorial_GameMode_RoundPlaying);
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_BattleExplore, this, (int)0x79e5667b, modes, 1, true,
                                                  -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_BattleShowdown))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode(eTutorial_GameMode_Showdown);
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_BattleShowdown, this, (int)0x2653f950, modes, 1, true,
                                                  -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_BattleDied))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[2];
            modes[0] = eTutorial_GameMode_SpectatingInvisible;
            modes[1] = eTutorial_GameMode_SpectatingAsMob;
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_BattleDied, this, (int)0xb5800810, modes, 2, true, -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_BattleSpectateAsMob))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[2];
            modes[0] = eTutorial_GameMode_SpectatingInvisible;
            modes[1] = eTutorial_GameMode_SpectatingAsMob;
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_BattleSpectateAsMob, this, (int)0xe28e0a7c, modes, 2,
                                                  true, -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_BattleFollowMode))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode(eTutorial_GameMode_Following);
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_BattleFollowMode, this, (int)0xec671466, modes, 1,
                                                  true, -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_BattleLastSurvivor))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[3];
            modes[0] = eTutorial_GameMode_SpectatingAsMob;
            modes[1] = eTutorial_GameMode_RoundStarting;
            modes[2] = eTutorial_GameMode_Following;
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_BattleLastSurvivor, this, (int)0x2272922b, modes, 3,
                                                  true, -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_BattleHunger))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[3];
            modes[0] = eTutorial_GameMode_SpectatingAsMob;
            modes[1] = eTutorial_GameMode_RoundPlaying;
            modes[2] = eTutorial_GameMode_Following;
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_BattleHunger, this, (int)0x6f523bbe, modes, 3, true,
                                                  -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_BattleLowHunger))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[3];
            modes[0] = eTutorial_GameMode_SpectatingAsMob;
            modes[1] = eTutorial_GameMode_RoundPlaying;
            modes[2] = eTutorial_GameMode_Following;
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_BattleLowHunger, this, (int)0xa85a4173, modes, 3, true,
                                                  -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_BattleSprint))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[3];
            modes[0] = eTutorial_GameMode_SpectatingAsMob;
            modes[1] = eTutorial_GameMode_RoundPlaying;
            modes[2] = eTutorial_GameMode_Following;
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_BattleSprint, this, (int)0x8257c70a, modes, 3, true,
                                                  -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_BattleChestRefill))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode(eTutorial_GameMode_ChestRefill);
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_BattleChestRefill, this, (int)0x92614a33, modes, 1,
                                                  true, -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_BattleWaitingForLoad))
        {
            TutorialHint* hint = new TutorialHint(eTutorial_Hint_BattleWaitingForLoad, this,
                                                  (int)IDS_MG_WAITING_FOR_LOAD, TutorialHint::HINT_CREATE_ITEM_SELECTED,
                                                  true, true, -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_BattleWaitingForLoad2))
        {
            TutorialHint* hint = new TutorialHint(eTutorial_Hint_BattleWaitingForLoad2, this,
                                                  (int)IDS_MG_WAITING_FOR_LOAD, TutorialHint::HINT_CREATE_ITEM_SELECTED,
                                                  true, true, -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_BattleWaitingForLoad3))
        {
            TutorialHint* hint = new TutorialHint(eTutorial_Hint_BattleWaitingForLoad3, this,
                                                  (int)IDS_MG_WAITING_FOR_LOAD, TutorialHint::HINT_CREATE_ITEM_SELECTED,
                                                  true, true, -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
    }
    else if (gameMode == 2)
    {
        if (!isHintCompleted(eTutorial_Hint_TumbleLobby))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode(eTutorial_GameMode_Lobby);
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_TumbleLobby, this, (int)0xe18d25cc, modes, 1, true,
                                                  -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_TumbleScoreboard))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode(eTutorial_GameMode_Scoreboard);
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_TumbleScoreboard, this, (int)0x1015193c, modes, 1,
                                                  true, -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_TumbleDied))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[2];
            modes[0] = eTutorial_GameMode_SpectatingInvisible;
            modes[1] = eTutorial_GameMode_SpectatingAsMob;
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_TumbleDied, this, (int)0xb5800810, modes, 2, true, -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_TumbleSpectateAsMob))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[2];
            modes[0] = eTutorial_GameMode_SpectatingInvisible;
            modes[1] = eTutorial_GameMode_SpectatingAsMob;
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_TumbleSpectateAsMob, this, (int)0xe28e0a7c, modes, 2,
                                                  true, -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_TumbleFollowMode))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode(eTutorial_GameMode_Following);
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_TumbleFollowMode, this, (int)0xec671466, modes, 1,
                                                  true, -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        if (!isHintCompleted(eTutorial_Hint_TumbleBreakBlocks))
        {
            eTutorial_GameMode* modes = new eTutorial_GameMode[3];
            modes[0] = eTutorial_GameMode_SpectatingAsMob;
            modes[1] = eTutorial_GameMode_RoundPlaying;
            modes[2] = eTutorial_GameMode_Following;
            GameModeHint* hint = new GameModeHint(eTutorial_Hint_TumbleBreakBlocks, this, (int)0xa086b6b3, modes, 3,
                                                  true, -1);
            addHint(eTutorial_State_Gameplay, hint);
        }
        eTutorial_GameMode* modes = new eTutorial_GameMode(eTutorial_GameMode_Fireworks);
        GameModeHint* hint = new GameModeHint(eTutorial_Hint_General, this, (int)0xd3c3dc8f, modes, 1, false, 15000);
        addHint(eTutorial_State_Gameplay, hint);
    }
}

unsigned char GameModeTutorial::isStateCompleted(eTutorial_State state)
{
    return stateCompleted[(int)state];
}

void GameModeTutorial::setStateCompleted(eTutorial_State state)
{
    stateCompleted[(int)state] = 1;
    Tutorial::setStateCompleted(state);
}

void GameModeTutorial::changeTutorialState(eTutorial_State state, UIScene* scene)
{
    gameModeConstraints->SetCurrentState(0);
}
