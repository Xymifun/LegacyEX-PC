#ifndef GAME_UI_FJ_TOOLTIP_HPP
#define GAME_UI_FJ_TOOLTIP_HPP

#include <stdint.h>
#include <string>

#include "game/ui/FJ_Base.hpp"

class fuiRenderNode;
class FJ_FuiNode;
class fuiRenderNodeEditText;

class FJ_ToolTip : public FJ_Base
{
public:
    explicit FJ_ToolTip(fuiRenderNode* renderNode);
    virtual ~FJ_ToolTip();

    fuiRenderNodeEditText* GetSecondaryTextField();
    void SetSecondaryText(const std::wstring& text);
    void SetType(int type, int subtype);
    void Init(const std::wstring& label, int id, bool visible, int type, int subtype);
    void UpdateControllerType(int controllerType);
    uint8_t IsSingleButtonToolTip();
    void Animate();
    virtual void SetLabel(const std::wstring& label);
    void ChangeType(int type, int subtype);

private:
    int m_controllerType;
    FJ_FuiNode* m_primaryText;
    FJ_FuiNode* m_secondaryText;
    FJ_FuiNode* m_textContainer;
    FJ_FuiNode* m_secondaryPanel;
    int m_primaryType;
    int m_secondaryType;
    int m_textContainerX;
};

#endif
