#ifndef GAME_UI_FJ_ENDPOEM_HPP
#define GAME_UI_FJ_ENDPOEM_HPP

#include <string>
#include <vector>

#include "game/ui/FJ_Document.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;

class FJ_EndPoem : public FJ_Document
{
private:
    FJ_FuiNode* m_textNodes[20];
    FJ_FuiNode* m_tiles[48];
    FJ_FuiNode* m_tilePrototype;
    int m_tileCount;
    int m_tileColumns;
    int m_tileRows;
    bool m_moveTiles;
    int m_textNodeCount;
    std::vector<FJ_FuiNode*> m_visibleNodes;
    std::vector<int> m_visibleLabels;
    bool m_lastLabelShown;
    int m_lastLabel;
    int m_scrollSpeed;
    bool m_appending;

public:
    explicit FJ_EndPoem(fuiRenderNode* renderNode);
    virtual ~FJ_EndPoem();
    static FJ_Document* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    void RequestNextLabel(int label);
    void UpdateTileBackground();
    void UpdateScrolling(FJ_Event* event);
    static void _UpdateScrolling(FJ_FuiNode* node, FJ_Event* event);
    void keyUpHandler(FJ_Event* event);
    static void _keyUpHandler(FJ_FuiNode* node, FJ_Event* event);
    virtual void init(FJ_Event* event);
    unsigned int SetNextLabel(const std::wstring& label, int index, bool lastLabel);
    virtual void keyDownHandler(FJ_Event* event);
};

#endif
