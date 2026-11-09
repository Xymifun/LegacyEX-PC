#ifndef GAME_UI_FJ_MOBEFFECT_HPP
#define GAME_UI_FJ_MOBEFFECT_HPP

#include <string>

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNodeEditText;

class FJ_MobEffect : public FJ_FuiNode
{
public:
    FJ_MobEffect(fuiRenderNode* pRenderNode);
    virtual ~FJ_MobEffect() {}

    void SetupEffectType(int iEffectType);
    std::wstring GetFormattedTime(int iDuration);
    void SetDuration(int iDuration);
    void Init(const std::wstring& effectName, int iDuration);
    unsigned int getTimeLeft();

private:
    int m_iDuration;
    fuiRenderNodeEditText* m_pDurationText;
};

#endif
