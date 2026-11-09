#include "game/ui/UIControl_Base.hpp"
#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_Cursor.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/core/StringConversion.hpp"

#include <string>
#include <vector>


UIControl_Base::UIControl_Base()
{
    m_labelChanged = false;
    m_controlId = 0;
}

UIControl_Base::~UIControl_Base()
{
}

bool UIControl_Base::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    return UIControl::setupControl(scene, node, name);
}

void UIControl_Base::setLabel(UIString label, bool update, bool force)
{
    if (force || ((!m_label.empty() || !label.empty()) && m_label.compare(label)))
        m_labelChanged = true;

    m_label = label;
    if (!m_labelChanged || !update)
        return;

    m_labelChanged = false;
    if (m_fuiRenderNode == 0)
        return;

    std::wstring text;
    if (label.isCharString() != 0)
        text = convStringToWstring(std::string(label.getCharString().c_str()));
    else
        text.assign(m_label.getString());

    if (m_fuiRenderNode->getFuiNode()->isFJ_Base())
        m_fuiRenderNode->getFuiNode()->asFJ_Base()->SetLabel(text);
    else if (m_fuiRenderNode->getFuiNode()->isFJ_Cursor())
        m_fuiRenderNode->getFuiNode()->asFJ_Cursor()->SetLabel(text);
}

void UIControl_Base::tick()
{
    if (m_label.needsUpdating() != 0 || m_labelChanged)
    {
        setLabel(m_label, true, true);
        m_label.setUpdated();
    }
}

const wchar_t* UIControl_Base::getLabel()
{
    if (m_fuiRenderNode->getFuiNode()->isFJ_Base())
    {
        const std::wstring& label = m_fuiRenderNode->getFuiNode()->asFJ_Base()->GetLabel();
        return label.c_str();
    }
    return L"";
}

void UIControl_Base::setAllPossibleLabels(int firstLabel, unsigned int labelCount, wchar_t (*labels)[256])
{
    std::vector<std::wstring> possibleLabels;
    for (unsigned int i = 0; i < labelCount; ++i)
        possibleLabels.push_back(std::wstring(labels[i]));

    m_fuiRenderNode->getFuiNode()->asFJ_Base()->CheckLabelWidths(possibleLabels);
}

bool UIControl_Base::hasFocus()
{
    return m_scene->controlHasFocus(this);
}
