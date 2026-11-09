#ifndef GAME_UI_FJ_MAPSELECTMENU_HPP
#define GAME_UI_FJ_MAPSELECTMENU_HPP

#include "game/ui/FJ_Document.hpp"

class fuiRenderNode;

class FJ_MapSelectMenu : public FJ_Document
{
public:
    explicit FJ_MapSelectMenu(fuiRenderNode* renderNode);

    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void SetSafeZone(int top, int bottom, int left, int right);
};

#endif
