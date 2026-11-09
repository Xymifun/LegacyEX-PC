#ifndef GAME_CORE_GAMEMODETRIALMANAGER_HPP
#define GAME_CORE_GAMEMODETRIALMANAGER_HPP

#include "game/minigame/EMiniGameId.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class GameModeTrialManager
{
public:
    class PlayerTrialsData
    {
    public:
        int GetTrialsUsed(EMiniGameId id);
        void AddTrialUse(EMiniGameId id);
    };

    class TrialLimitsData
    {
    public:
        TrialLimitsData(int value);
        bool IsTrialUnlimited(EMiniGameId id);
        int GetTrialLimit(EMiniGameId id);
    };

    class TrialsData
    {
    public:
        TrialsData();
        unsigned char GetValue(EMiniGameId id);
        TrialsData(unsigned char* data, int length);
        void Serialise(unsigned char** data, int* length);
        void SetValue(EMiniGameId id, unsigned char value);
    };

    GameModeTrialManager();
    int GetTrialsMax(EMiniGameId id);
    int GetGameModeTrialsRemaining(int player, EMiniGameId id);
    void AddGameModeTrialUse(int player, EMiniGameId id);
    bool HasPurchased(int player, EMiniGameId id);
    int GetPack(EMiniGameId id);
    void SetLimits(arrayWithLength<unsigned char> limits);
};

#endif
