#ifndef GAME_CORE_MULTILISTSLIDERLOGIC_HPP
#define GAME_CORE_MULTILISTSLIDERLOGIC_HPP

#include <string>
#include "game/core/GameModeOptions.hpp"
#include "game/minigame/EMiniGameId.hpp"

class UIControl_MultiList;

class MultiListSliderLogic
{
public:
    MultiListSliderLogic();
    MultiListSliderLogic(int labelId, int descriptionId, EMiniGameId miniGameId, eCustomGameModeHostOption option);

    void AddTo(UIControl_MultiList& list, int itemIndex) const;
    void Update(UIControl_MultiList& list, int itemIndex) const;
    virtual std::wstring GetLabel(int value) const;

private:
    int mLabelId;
    int mDescriptionId;
    eCustomGameModeHostOption mOption;
    EMiniGameId mMiniGameId;
};

#endif
