#include "game/ui/FJ_ListButtonLobby_Small.hpp"

FJ_ListButtonLobby_Small::FJ_ListButtonLobby_Small(fuiRenderNode* renderNode) : FJ_ListButtonLobby(renderNode)
{
}

FJ_ListButtonLobby* FJ_ListButtonLobby_Small::Create(fuiRenderNode* renderNode)
{
    return new FJ_ListButtonLobby_Small(renderNode);
}

uint32_t FJ_ListButtonLobby_Small::getType()
{
    return 0x8027;
}
