#ifndef GAME_UI_FJ_LISTBUTTONPLAYERLIST_HPP
#define GAME_UI_FJ_LISTBUTTONPLAYERLIST_HPP

#include "game/ui/FJ_Button.hpp"

class FJ_FuiNode;
class fuiRenderNode;

class FJ_ListButtonPlayerList : public FJ_Button
{
public:
    explicit FJ_ListButtonPlayerList(fuiRenderNode* renderNode);
    virtual ~FJ_ListButtonPlayerList() {}

    void Init(const std::wstring& name, int id);
    void SetPlayerIcon(int frame);
    void SetVOIPIcon(int frame);

private:
    FJ_FuiNode* m_playerIcon;
    FJ_FuiNode* m_voipIcon;
};

#endif
