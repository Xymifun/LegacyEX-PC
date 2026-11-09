#ifndef GAME_CORE_SLIDERLOGIC_HPP
#define GAME_CORE_SLIDERLOGIC_HPP

#include <string>
#include "game/core/GameModeOptions.hpp"
#include "game/minigame/EMiniGameId.hpp"

class UIControl_MultiList;

class SliderLogic
{
public:
    SliderLogic();
    SliderLogic(int id, int label, eCustomGameModeHostOption option, EMiniGameId miniGameId, bool includeRandom);
    void AddTo(UIControl_MultiList& list, int value) const;
    void Update(UIControl_MultiList& list, int value) const;
    virtual std::wstring GetLabel(int value) const;

private:
    int mId;
    int mLabel;
    bool mIncludeRandom;
    eCustomGameModeHostOption mOption;
    EMiniGameId mMiniGameId;
};

#endif
