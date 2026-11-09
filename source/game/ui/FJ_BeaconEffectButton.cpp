#include "game/ui/FJ_BeaconEffectButton.hpp"

#include <string>

static std::string s_stateLabels[4] = {std::string("Button_Disabled"), std::string("Button_Enabled_Unselected"),
                                       std::string("Button_Enabled_Selected"), std::string("Button_Pressed")};
static const int s_iconFrames[] = {-1, 12, 9, 2, 14, 11, 5, 3, 6, 13, 16, 7, 8, 1, 0, 4};

FJ_BeaconEffectButton::FJ_BeaconEffectButton(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode)
{
}

void FJ_BeaconEffectButton::ChangeState(int state)
{
    gotoAndStop(s_stateLabels[state]);
    m_state = state;
}

void FJ_BeaconEffectButton::SetIcon(int icon)
{
    if (icon > 0)
    {
        int frame = s_iconFrames[icon];
        FJ_FuiNode::asMovieClip(getChildByName(std::string("Icon")))->gotoAndStop(frame + 2);
        FJ_FuiNode::asMovieClip(getChildByName(std::string("BeaconEffectTick")))->setVisible(false);
    }
    else
    {
        FJ_FuiNode::asMovieClip(getChildByName(std::string("Icon")))->gotoAndStop(1);
        FJ_FuiNode::asMovieClip(getChildByName(std::string("BeaconEffectTick")))->setVisible(true);
    }
}
