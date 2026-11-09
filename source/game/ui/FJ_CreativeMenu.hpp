#ifndef GAME_UI_FJ_CREATIVEMENU_HPP
#define GAME_UI_FJ_CREATIVEMENU_HPP

#include "game/ui/FJ_AbstractContainerDocument.hpp"

class FJ_Event;
class FJ_FuiNode;
class FJ_ScrollArrow;
class fuiRenderNode;

class FJ_CreativeMenu : public FJ_AbstractContainerDocument
{
public:
    explicit FJ_CreativeMenu(fuiRenderNode* renderNode);

    void handleSlideComplete(FJ_Event* event);
    static void _handleSlideComplete(FJ_FuiNode* node, FJ_Event* event);
    static FJ_CreativeMenu* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void SetSafeZone(int top, int bottom, int left, int right);
    void SetActiveTab(int tab);
    void SetScrollBar(int pageCount, int currentPage);

private:
    int m_activeTab;
    int m_currentPage;
    FJ_ScrollArrow* m_scrollUpArrow;
    FJ_ScrollArrow* m_scrollDownArrow;
    int m_scrollBarButtonPosition;
    int m_scrollBarEndPosition;
};

#endif
