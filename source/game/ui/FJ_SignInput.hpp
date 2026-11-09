#ifndef GAME_UI_FJ_SIGNINPUT_HPP
#define GAME_UI_FJ_SIGNINPUT_HPP

#include <stdint.h>
#include <string>

#include "game/ui/FJ_Base.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;
class fuiRenderNodeEditText;

class FJ_SignInput : public FJ_Base
{
    friend class fuiRenderNodeTimeline;

public:
    explicit FJ_SignInput(fuiRenderNode* renderNode);
    virtual ~FJ_SignInput();

    static FJ_Base* Create(fuiRenderNode* renderNode);

    virtual unsigned int getType();
    void ChangeState(int state);
    void focusInHandler(FJ_Event* event);
    static void _focusInHandler(FJ_FuiNode* node, FJ_Event* event);
    void focusOutHandler(FJ_Event* event);
    static void _focusOutHandler(FJ_FuiNode* node, FJ_Event* event);
    void keyDownHandler(FJ_Event* event);
    static void _keyDownHandler(FJ_FuiNode* node, FJ_Event* event);
    void Init(std::wstring name, int id);
    virtual void SetLabel(const std::wstring& label);
    void HandleDirectInput(unsigned int character);
    virtual void HandleCharInput(unsigned int character);
    virtual void GetLabelTooWide(fuiRenderNodeEditText* textField);
    virtual void SetNewObjectWidth(int width);

private:
    int m_maxLength;
    FJ_FuiNode* m_leftArrow;
    FJ_FuiNode* m_rightArrow;
};

#endif
