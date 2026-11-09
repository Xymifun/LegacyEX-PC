#ifndef GAME_UI_UICONTROL_BITMAPICON_HPP
#define GAME_UI_UICONTROL_BITMAPICON_HPP

#include <string>

#include "game/ui/UIControl.hpp"

class UIControl_BitmapIcon : public UIControl
{
protected:
    bool m_textureNamePending;
    std::wstring m_textureName;

public:
    UIControl_BitmapIcon();
    virtual ~UIControl_BitmapIcon() {}
    void setTextureName(const std::wstring& textureName, bool immediate);
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    virtual void tick();
    virtual void ReInit();
};

#endif
