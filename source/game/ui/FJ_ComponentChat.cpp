#include "game/ui/FJ_ComponentChat.hpp"

#include "game/ui/fuiRenderNode.hpp"

FJ_ComponentChat::FJ_ComponentChat(fuiRenderNode* pRenderNode) : FJ_FuiNode(pRenderNode)
{
}

FJ_FuiNode* FJ_ComponentChat::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_ComponentChat(pRenderNode);
}

unsigned int FJ_ComponentChat::getType()
{
    return 0x5b;
}

void FJ_ComponentChat::SetAlpha(const std::string& name, float alpha)
{
    FJ_FuiNode* pNode = getChildByName(name);
    pNode->setAlpha(alpha);
    pNode->setVisible(alpha > 0.0f);
}
