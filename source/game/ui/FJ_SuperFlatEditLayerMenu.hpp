#ifndef GAME_UI_FJ_SUPERFLATEDITLAYERMENU_HPP
#define GAME_UI_FJ_SUPERFLATEDITLAYERMENU_HPP

#include "game/ui/FJ_AbstractContainerDocument.hpp"

class FJ_ScrollArrow;
class fuiRenderNode;

class FJ_SuperFlatEditLayerMenu : public FJ_AbstractContainerDocument
{
public:
    explicit FJ_SuperFlatEditLayerMenu(fuiRenderNode* renderNode);
    void SetSlotID();
    static FJ_AbstractContainerDocument* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void SetScrollBar(int count, int position);

private:
    int mSlotID;
    FJ_ScrollArrow* mScrollUp;
    FJ_ScrollArrow* mScrollDown;
    int mScrollTop;
    int mScrollBottom;
};

#endif
