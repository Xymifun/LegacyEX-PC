#include "game/ui/FJ_Label_HUD_SplashBig.hpp"

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_Label_HUD_SplashBig::FJ_Label_HUD_SplashBig(fuiRenderNode* renderNode) : FJ_Label(renderNode), m_animated(false)
{
}

FJ_Label* FJ_Label_HUD_SplashBig::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_HUD_SplashBig(renderNode);
}

uint32_t FJ_Label_HUD_SplashBig::getType()
{
    return 0x8073;
}

void FJ_Label_HUD_SplashBig::Init(const std::wstring& label)
{
    FJ_Label::Init(label);
    if (m_animated)
    {
        gotoAndPlay("Animated");
    }
}

void FJ_Label_HUD_SplashBig::SetLabel(const std::wstring& label)
{
    FJ_Label::SetLabel(label);
    if (m_animated)
    {
        gotoAndPlay("Animated");
    }
}
