#ifndef GAME_UI_FJ_KEYBOARDBUTTONS_HPP
#define GAME_UI_FJ_KEYBOARDBUTTONS_HPP

#include <string>
#include "game/ui/FJ_Button.hpp"

class fuiRenderNode;

class FJ_KeyboardButtons : public FJ_Button
{
public:
    explicit FJ_KeyboardButtons(fuiRenderNode* renderNode);
    virtual ~FJ_KeyboardButtons() {}

    virtual void SetLabel(const std::wstring& label);
    void Init(const std::wstring& label, int id);
    void RemotePressButton();
    virtual void UpdateLabel();
    bool getUseHtmlText();
    void setUseHtmlText(bool useHtmlText);

protected:
    int m_keyboardOffset;
    bool m_remotePressButton;
    int m_keyboardWidth;
    int m_keyboardHeight;
};

#endif
