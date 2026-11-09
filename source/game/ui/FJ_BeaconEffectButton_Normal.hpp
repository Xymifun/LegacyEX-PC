#ifndef GAME_UI_FJ_BEACONEFFECTBUTTON_NORMAL_HPP
#define GAME_UI_FJ_BEACONEFFECTBUTTON_NORMAL_HPP

#include "game/ui/FJ_BeaconEffectButton.hpp"

class fuiRenderNode;

class FJ_BeaconEffectButton_Normal : public FJ_BeaconEffectButton
{
public:
    FJ_BeaconEffectButton_Normal(fuiRenderNode* pRenderNode);
    static FJ_BeaconEffectButton* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
