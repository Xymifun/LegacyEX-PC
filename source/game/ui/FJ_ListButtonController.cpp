#include "game/ui/FJ_ListButtonController.hpp"

FJ_ListButtonController::FJ_ListButtonController(fuiRenderNode* renderNode) : FJ_Button(renderNode)
{
}

float FJ_ListButtonController::getOriginalHeight()
{
    return m_originalHeight;
}

void FJ_ListButtonController::Init(const std::wstring& name, int id)
{
    SetLabelAlignment(4);
    m_textOnly = true;
    FJ_Button::Init(name, id);
    m_originalHeight = getHeight();
}
