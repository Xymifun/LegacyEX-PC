#ifndef GAME_UI_FJ_HOWTOPLAY_HPP
#define GAME_UI_FJ_HOWTOPLAY_HPP

#include <string>
#include <vector>

#include "game/ui/FJ_FuiNode.hpp"

class FJ_DynamicHtmlText;
class FJ_Event;
class fuiRenderNode;

class FJ_HowToPlay : public FJ_FuiNode
{
    int m_page;
    std::wstring m_text;
    bool m_initialized;
    FJ_DynamicHtmlText* m_dynamicHtmlText;

public:
    FJ_HowToPlay(fuiRenderNode* pRenderNode);

    static void _init(FJ_FuiNode* pNode, FJ_Event* pEvent);
    void init(FJ_Event* pEvent);
    bool LoadHowToPlayPage(int page, std::vector<std::wstring>& pages);
    void InitScene();
    static FJ_HowToPlay* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
    void SetSafeZone(int top, int bottom, int left, int right);
};

#endif
