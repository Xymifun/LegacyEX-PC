#include "game/ui/FJ_Label_HUD_SplashVita.hpp"

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_Label_HUD_SplashVita::FJ_Label_HUD_SplashVita(fuiRenderNode* renderNode) : FJ_Label(renderNode), m_animated(false)
{
}

FJ_Label* FJ_Label_HUD_SplashVita::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_HUD_SplashVita(renderNode);
}

uint32_t FJ_Label_HUD_SplashVita::getType()
{
    return 0x8074;
}

void FJ_Label_HUD_SplashVita::Init(const std::wstring& label)
{
    FJ_Label::Init(label);
    if (m_animated)
    {
        gotoAndPlay("Animated");
    }
}

void FJ_Label_HUD_SplashVita::SetLabel(const std::wstring& label)
{
    FJ_Label::SetLabel(label);
    if (m_animated)
    {
        gotoAndPlay("Animated");
    }
}
