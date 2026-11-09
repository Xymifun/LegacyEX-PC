#include "game/ui/FJ_Label_HUD_Splash.hpp"

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"

FJ_Label_HUD_Splash::FJ_Label_HUD_Splash(fuiRenderNode* renderNode) : FJ_Label(renderNode), m_animated(false)
{
}

FJ_Label* FJ_Label_HUD_Splash::Create(fuiRenderNode* renderNode)
{
    return new FJ_Label_HUD_Splash(renderNode);
}

uint32_t FJ_Label_HUD_Splash::getType()
{
    return 0x8072;
}

void FJ_Label_HUD_Splash::Init(const std::wstring& label)
{
    FJ_Label::Init(label);
    if (m_animated)
    {
        gotoAndPlay("Animated");
    }
}

void FJ_Label_HUD_Splash::SetLabel(const std::wstring& label)
{
    FJ_Label::SetLabel(label);
    if (m_animated)
    {
        gotoAndPlay("Animated");
    }
}
