#ifndef GAME_UI_FJ_LISTBUTTONSCOREBOARD_HPP
#define GAME_UI_FJ_LISTBUTTONSCOREBOARD_HPP

#include <string>

#include "game/ui/FJ_Button.hpp"

class FJ_FuiNode;
class fuiRenderNode;

class FJ_ListButtonScoreBoard : public FJ_Button
{
public:
    explicit FJ_ListButtonScoreBoard(fuiRenderNode* renderNode);
    virtual ~FJ_ListButtonScoreBoard() {}

    virtual void Init(const std::wstring& name, int id);
    void SetNumIcons(int numIcons, int width, int highlightWidth);
    void SetPlayerIcon(int frame);
    void SetVOIPIcon(int frame);
    void SetSlotText(int slot, const std::wstring& text);
    void SetSlotTextureName(int slot, const std::wstring& textureName);
    virtual void ChangeState(int state);

protected:
    bool m_small;

private:
    FJ_FuiNode* m_playerIcons;
    FJ_FuiNode* m_voipIcon;
    int m_numIcons;
    FJ_FuiNode* m_externalHighlight;
};

#endif
