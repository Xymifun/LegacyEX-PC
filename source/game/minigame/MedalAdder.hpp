#ifndef GAME_MINIGAME_MEDALADDER_HPP
#define GAME_MINIGAME_MEDALADDER_HPP

#include <vector>

#include "game/minigame/GameMedal.hpp"

class MedalAdder
{
public:
    MedalAdder(int* nextMedalId, std::vector<const GameMedal*>* medals, std::vector<const GameMedal*>* allMedals)
        : mNextMedalId(nextMedalId),
          mMedals(medals),
          mAllMedals(allMedals)
    {
    }

    void addMedal(GameMedal* medal)
    {
        medal->SetId(*mNextMedalId);
        mMedals->push_back(medal);
        mAllMedals->push_back(medal);
        ++(*mNextMedalId);
    }

private:
    int* mNextMedalId;
    std::vector<const GameMedal*>* mMedals;
    std::vector<const GameMedal*>* mAllMedals;
};

#endif
