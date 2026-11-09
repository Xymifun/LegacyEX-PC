#ifndef GAME_UI_FJ_LABEL_BLINK_HPP
#define GAME_UI_FJ_LABEL_BLINK_HPP

#include <stdint.h>
#include <string>

#include "game/ui/FJ_Base.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;
class fuiRenderNodeEditText;

class FJ_Label_Blink : public FJ_Base
{
public:
    explicit FJ_Label_Blink(fuiRenderNode* renderNode);
    virtual ~FJ_Label_Blink() {}

    void UpdateLabel();
    void HandleUpdateLabel(FJ_Event* event);
    static void _HandleUpdateLabel(FJ_FuiNode* node, FJ_Event* event);
    void Init(const std::wstring& label);
    void EnableBlink(bool enable);
    virtual void SetLabel(const std::wstring& label);
    virtual void GetLabelTooWide(fuiRenderNodeEditText* textField);
    void setAction(unsigned char action, char* value);

protected:
    int m_blinkAlignment;
    bool m_blinkEnabled;
};

#endif
