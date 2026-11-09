#ifndef GAME_UI_FJ_TRADINGMENU_HPP
#define GAME_UI_FJ_TRADINGMENU_HPP

#include <string>

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;

class FJ_TradingMenu : public FJ_Document
{
public:
    FJ_TradingMenu(fuiRenderNode* renderNode);

    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void init(FJ_Event* event);
    virtual void SetSafeZone(int top, int bottom, int left, int right);
    void ScrollRight();
    void ScrollLeft();
    void PositionOfferDescription();
    void MoveSelector(bool right);
    void SetSelectorSlot(int slot);
    void SetOfferDescription(const std::wstring& description);
    void ShowScrollRightArrow(bool show);
    void ShowScrollLeftArrow(bool show);
    int getTotalTradingSlots();

private:
    FJ_FuiNode* m_scrollRight;
    FJ_FuiNode* m_scrollLeft;
    int m_slotWidth;
    int m_visibleSlots;
    int m_selectorSlot;
};

#endif
