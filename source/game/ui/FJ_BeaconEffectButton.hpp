#ifndef GAME_UI_FJ_BEACONEFFECTBUTTON_HPP
#define GAME_UI_FJ_BEACONEFFECTBUTTON_HPP

#include "game/ui/FJ_FuiNode.hpp"

class FJ_BeaconEffectButton : public FJ_FuiNode
{
private:
    int m_state;

public:
    FJ_BeaconEffectButton(fuiRenderNode* pRenderNode);
    virtual ~FJ_BeaconEffectButton() {}
    void ChangeState(int state);
    void SetIcon(int icon);
};

#endif
