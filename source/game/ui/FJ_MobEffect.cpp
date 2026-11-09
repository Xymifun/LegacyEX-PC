#include "game/ui/FJ_MobEffect.hpp"

#include <string>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"

template <typename T> std::wstring _toString(T value);

FJ_MobEffect::FJ_MobEffect(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode)
{
}

void FJ_MobEffect::SetupEffectType(int iEffectType)
{
    getChildByName("Icon")->gotoAndStop(iEffectType);
}

std::wstring FJ_MobEffect::GetFormattedTime(int iDuration)
{
    int iMinutes = (iDuration % 3600) / 60;
    int iSeconds = (iDuration % 3600) % 60;
    return _toString(iMinutes) + L":" + (iSeconds < 10 ? L"0" : L"") + _toString(iSeconds);
}

void FJ_MobEffect::SetDuration(int iDuration)
{
    m_iDuration = iDuration;
    m_pDurationText->setText(GetFormattedTime(iDuration));
}

void FJ_MobEffect::Init(const std::wstring& effectName, int iDuration)
{
    FJ_FuiNode* pPointerTextPanel = getChildByName("PointerTextPanel")->asMovieClip();
    int iWidth = (int)pPointerTextPanel->getWidth();
    int iHeight = (int)pPointerTextPanel->getHeight();

    pPointerTextPanel->setScaleX(1.0f);
    pPointerTextPanel->setScaleY(1.0f);
    pPointerTextPanel->setX(0.0f);
    pPointerTextPanel->setY(0.0f);
    pPointerTextPanel->getChildAt(0)->setWidth((float)iWidth);
    pPointerTextPanel->getChildAt(0)->setHeight((float)iHeight);

    FJ_FuiNode* pTextNode = getChildByName("EffectName")->asMovieClip();
    fuiRenderNodeEditText* pEffectNameText =
        pTextNode->getChildAt(0)->asMovieClip()->getRenderNodeChildAt(0)->asEditText();
    pEffectNameText->setScaleX(1.0f / pTextNode->getScaleX());
    pEffectNameText->setScaleY(1.0f / pTextNode->getScaleY());
    pEffectNameText->setWidth((float)iWidth);
    pEffectNameText->setText(effectName);

    pTextNode = getChildByName("EffectDuration")->asMovieClip();
    m_pDurationText = pTextNode->getChildAt(0)->asMovieClip()->getRenderNodeChildAt(0)->asEditText();
    m_pDurationText->setScaleX(1.0f / pTextNode->getScaleX());
    m_pDurationText->setScaleY(1.0f / pTextNode->getScaleY());
    m_pDurationText->setWidth((float)iWidth);
    m_iDuration = iDuration;
    SetDuration(iDuration);
}

unsigned int FJ_MobEffect::getTimeLeft()
{
    return (unsigned int)m_iDuration;
}
