#ifndef GAME_UI_FJ_BIOMESELECTMENU_HPP
#define GAME_UI_FJ_BIOMESELECTMENU_HPP

#include "game/ui/FJ_AbstractContainerDocument.hpp"

class fuiRenderNode;

class FJ_BiomeSelectMenu : public FJ_AbstractContainerDocument
{
public:
    explicit FJ_BiomeSelectMenu(fuiRenderNode* renderNode);
    static FJ_AbstractContainerDocument* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void SetMapNavigator(float scale, int x, int z);

private:
    float mNavigatorWidth;
    float mNavigatorHeight;
    float mNavigatorX;
    float mNavigatorY;
};

#endif
