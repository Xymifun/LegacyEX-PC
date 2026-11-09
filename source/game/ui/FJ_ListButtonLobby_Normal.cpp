#include "game/ui/FJ_ListButtonLobby_Normal.hpp"

FJ_ListButtonLobby_Normal::FJ_ListButtonLobby_Normal(fuiRenderNode* renderNode) : FJ_ListButtonLobby(renderNode)
{
}

FJ_ListButtonLobby* FJ_ListButtonLobby_Normal::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButtonLobby_Normal(renderNode);
}

unsigned int FJ_ListButtonLobby_Normal::getType()
{
    return 0x8026;
}
