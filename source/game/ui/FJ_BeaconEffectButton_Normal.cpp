#include "game/ui/FJ_BeaconEffectButton_Normal.hpp"

FJ_BeaconEffectButton_Normal::FJ_BeaconEffectButton_Normal(fuiRenderNode* pRenderNode)
    : FJ_BeaconEffectButton(pRenderNode)
{
}

FJ_BeaconEffectButton* FJ_BeaconEffectButton_Normal::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_BeaconEffectButton_Normal(pRenderNode);
}

unsigned int FJ_BeaconEffectButton_Normal::getType()
{
    return 83;
}
