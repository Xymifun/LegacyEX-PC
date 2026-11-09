#ifndef GAME_UI_FJ_BEACONEFFECTBUTTON_SMALL_HPP
#define GAME_UI_FJ_BEACONEFFECTBUTTON_SMALL_HPP

#include "game/ui/FJ_BeaconEffectButton.hpp"

class FJ_BeaconEffectButton_Small : public FJ_BeaconEffectButton
{
public:
    FJ_BeaconEffectButton_Small(fuiRenderNode* pRenderNode);
    static FJ_BeaconEffectButton* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
};

#endif
