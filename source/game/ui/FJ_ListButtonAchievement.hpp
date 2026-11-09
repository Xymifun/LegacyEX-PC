#ifndef GAME_UI_FJ_LISTBUTTONACHIEVEMENT_HPP
#define GAME_UI_FJ_LISTBUTTONACHIEVEMENT_HPP

#include <string>

#include "game/ui/FJ_Base.hpp"

class FJ_BitmapIcon;
class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;

class FJ_ListButtonAchievement : public FJ_Base
{
public:
    FJ_ListButtonAchievement(fuiRenderNode* pRenderNode);
    virtual ~FJ_ListButtonAchievement() {}

    void EnableButton(bool enabled);
    void focusInHandler(FJ_Event* pEvent);
    static void _focusInHandler(FJ_FuiNode* pNode, FJ_Event* pEvent);
    void focusOutHandler(FJ_Event* pEvent);
    static void _focusOutHandler(FJ_FuiNode* pNode, FJ_Event* pEvent);
    void SetTextureName(const std::wstring& textureName);
    void Init(int id, const std::wstring& textureName);
    bool getEnabled();

private:
    FJ_BitmapIcon* m_bitmapIcon;
    bool m_enabled;
    FJ_FuiNode* m_highlight;
};

#endif
