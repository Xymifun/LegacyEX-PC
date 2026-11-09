#include "game/core/SliderLogic.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/L10N.hpp"
#include "game/ui/UIControl_MultiList.hpp"
#include "game/ui/UIString.hpp"

SliderLogic::SliderLogic()
{
    mId = 0;
    mLabel = 0;
    mOption = eCustomGameModeHostOption_NoSmallSkins;
    mMiniGameId = eMiniGameId_Lobby;
    mIncludeRandom = false;
}

SliderLogic::SliderLogic(int id, int label, eCustomGameModeHostOption option, EMiniGameId miniGameId,
                         bool includeRandom)
{
    mId = id;
    mLabel = label;
    mOption = option;
    mMiniGameId = miniGameId;
    mIncludeRandom = includeRandom;
}

void SliderLogic::AddTo(UIControl_MultiList& list, int value) const
{
    std::wstring label = GetLabel(value);
    int maxValue = MasterGameMode::GetOptionMax(mMiniGameId, mOption);
    int minValue = MasterGameMode::GetOptionMin(mMiniGameId, mOption, mIncludeRandom);
    list.AddNewSlider(label, mId, minValue, maxValue, value, false, 7, 3);
}

void SliderLogic::Update(UIControl_MultiList& list, int value) const
{
    list.SetItemLabel(mId, UIString(GetLabel(value)), false);
}

std::wstring SliderLogic::GetLabel(int value) const
{
    std::wstring label(L"");
    std::wstring valueText = MasterGameMode::GetValueText(mMiniGameId, mOption, value);
    if (mLabel != -1 && mLabel != -2)
        label = std::wstring(L10N::GetString(mLabel)) + L": ";
    return label + valueText;
}
