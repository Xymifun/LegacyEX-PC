#include "game/ui/FJ_ScrollArrowH.hpp"

#include <string>

namespace
{
std::string g_scrollArrowHStates[4] = {"Visible_A", "Animating_A", "Visible_B", "Animating_B"};
}

FJ_ScrollArrowH::FJ_ScrollArrowH(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode)
{
    m_iState = 0;
}

FJ_FuiNode* FJ_ScrollArrowH::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_ScrollArrowH(pRenderNode);
}

unsigned int FJ_ScrollArrowH::getType()
{
    return 87;
}

void FJ_ScrollArrowH::ChangeState(int iState)
{
    const std::string& state = g_scrollArrowHStates[iState];
    if (state.compare("Visible_A") == 0 || state.compare("Visible_B") == 0)
        gotoAndPlay(state);
    else
        gotoAndStop(state);
    m_iState = iState;
}
