#ifndef GAME_UI_FJ_BEACONMENU_HPP
#define GAME_UI_FJ_BEACONMENU_HPP

#include "game/ui/FJ_AbstractContainerDocument.hpp"

class fuiRenderNode;

class FJ_BeaconMenu : public FJ_AbstractContainerDocument
{
public:
    FJ_BeaconMenu(fuiRenderNode* renderNode);
    static FJ_BeaconMenu* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void SetSafeZone(int top, int bottom, int left, int right);
};

#endif
