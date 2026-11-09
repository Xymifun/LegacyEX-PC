#include "game/ui/FJ_ScrollArrow.hpp"

#include <string>

namespace
{
std::string g_scrollArrowStates[6] = {"Visible_A", "Animating_A", "AnimatingLoop_A",
                                      "Visible_B", "Animating_B", "AnimatingLoop_B"};
}

FJ_ScrollArrow::FJ_ScrollArrow(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode)
{
    m_iState = 0;
}

FJ_FuiNode* FJ_ScrollArrow::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_ScrollArrow(pRenderNode);
}

void FJ_ScrollArrow::ChangeState(int iState)
{
    const std::string& state = g_scrollArrowStates[iState];
    if (state.compare("Visible_A") == 0 || state.compare("Visible_B") == 0)
        gotoAndStop(state);
    else
        gotoAndPlay(state);
    m_iState = iState;
}

unsigned int FJ_ScrollArrow::getType()
{
    return 86;
}
