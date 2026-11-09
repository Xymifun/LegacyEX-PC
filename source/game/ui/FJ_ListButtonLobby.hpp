#ifndef GAME_UI_FJ_LISTBUTTONLOBBY_HPP
#define GAME_UI_FJ_LISTBUTTONLOBBY_HPP

#include <string>
#include "game/ui/FJ_Button.hpp"

class FJ_FuiNode;
class fuiRenderNode;

class FJ_ListButtonLobby : public FJ_Button
{
public:
    explicit FJ_ListButtonLobby(fuiRenderNode* renderNode);
    virtual ~FJ_ListButtonLobby() {}

    virtual void Init(const std::wstring& name, int id);
    void SetPlayerIcon(int icon);
    void SetVOIPIcon(int icon);
    void ShowTick(bool show);

private:
    FJ_FuiNode* m_playerIcons;
    FJ_FuiNode* m_voipIcon;
    FJ_FuiNode* m_readyTick;
    FJ_FuiNode* m_readySlot;
};

#endif
