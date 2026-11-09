#include "game/ui/FJ_SignInput.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_KeyboardEvent.hpp"
#include "game/ui/UIScene.hpp"

FJ_SignInput::FJ_SignInput(fuiRenderNode* renderNode) : FJ_Base(renderNode), m_maxLength(15)
{
}

FJ_Base* FJ_SignInput::Create(fuiRenderNode* renderNode)
{
    return new FJ_SignInput(renderNode);
}

FJ_SignInput::~FJ_SignInput()
{
}

unsigned int FJ_SignInput::getType()
{
    return 0x8010;
}

void FJ_SignInput::ChangeState(int state)
{
    if (m_leftArrow != 0 && m_rightArrow != 0)
    {
        m_leftArrow->gotoAndPlay(state == 0 ? "Selected_Left" : "UnSelected");
        m_rightArrow->gotoAndPlay(state == 0 ? "Selected_Right" : "UnSelected");
    }
}

void FJ_SignInput::focusInHandler(FJ_Event*)
{
    ChangeState(0);
}

void FJ_SignInput::_focusInHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_SignInput*>(node)->focusInHandler(event);
}

void FJ_SignInput::focusOutHandler(FJ_Event*)
{
    ChangeState(1);
}

void FJ_SignInput::_focusOutHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_SignInput*>(node)->focusOutHandler(event);
}

void FJ_SignInput::keyDownHandler(FJ_Event* event)
{
    if (static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode == 0)
    {
        FJ_FuiNode* root = getRoot();
        root->m_pCallbackScene->handlePress(m_id, 0);
    }
}

void FJ_SignInput::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_SignInput*>(node)->keyDownHandler(event);
}

void FJ_SignInput::Init(std::wstring name, int id)
{
    SetLabelAlignment(4);
    m_isLabel = false;
    InitBase(name, id);
    m_leftArrow = getChildByName(std::string("ArrowLeft"));
    m_rightArrow = getChildByName(std::string("ArrowRight"));
    addEventListener(eFJEventType_FocusIn, this, &_focusInHandler, false);
    addEventListener(eFJEventType_FocusOut, this, &_focusOutHandler, false);
    addEventListener(eFJEventType_KeyDown, this, &_keyDownHandler, false);
    ChangeState(1);
}

void FJ_SignInput::SetLabel(const std::wstring& label)
{
    FJ_Base::SetLabel(label);
}

void FJ_SignInput::HandleDirectInput(unsigned int character)
{
    if (m_label.size() < static_cast<std::wstring::size_type>(m_maxLength) || m_maxLength == -1)
    {
        m_label = m_label.substr(0, m_label.size()) + static_cast<wchar_t>(character) + m_label.substr(m_label.size());
        SetLabel(m_label);
    }
}

void FJ_SignInput::HandleCharInput(unsigned int character)
{
    HandleDirectInput(character);
}

void FJ_SignInput::GetLabelTooWide(fuiRenderNodeEditText*)
{
}

void FJ_SignInput::SetNewObjectWidth(int)
{
}
