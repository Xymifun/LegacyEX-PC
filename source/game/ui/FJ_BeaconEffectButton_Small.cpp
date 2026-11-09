#include "game/ui/FJ_BeaconEffectButton_Small.hpp"

FJ_BeaconEffectButton_Small::FJ_BeaconEffectButton_Small(fuiRenderNode* pRenderNode)
    : FJ_BeaconEffectButton(pRenderNode)
{
}

FJ_BeaconEffectButton* FJ_BeaconEffectButton_Small::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_BeaconEffectButton_Small(pRenderNode);
}

unsigned int FJ_BeaconEffectButton_Small::getType()
{
    return 0x54;
}
