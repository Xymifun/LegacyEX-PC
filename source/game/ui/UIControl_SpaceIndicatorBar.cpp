#include "game/ui/UIControl_SpaceIndicatorBar.hpp"
#include "game/ui/FJ_SpaceIndicatorBar.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_SpaceIndicatorBar::UIControl_SpaceIndicatorBar()
{
    m_startOffset = 0;
    m_endOffset = 100;
    m_saveSize = 0;
    m_totalSize = 0;
    m_saveGameOffset = 0.0f;
}

bool UIControl_SpaceIndicatorBar::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_Progress;
    return UIControl_Base::setupControl(scene, node, name);
}

void UIControl_SpaceIndicatorBar::setSaveSize(long long size)
{
    m_saveSize = size;
    float ratio = (float)(m_saveSize - m_startOffset) / (float)(m_endOffset - m_startOffset);
    ((FJ_SpaceIndicatorBar*)m_fuiRenderNode->getFuiNode())->setSaveGameSize(ratio);
}

void UIControl_SpaceIndicatorBar::setSaveGameOffset(float offset)
{
    m_saveGameOffset = offset;
    ((FJ_SpaceIndicatorBar*)m_fuiRenderNode->getFuiNode())->setSaveGameOffset(offset);
}

void UIControl_SpaceIndicatorBar::selectSave(int index)
{
    if (index >= 0 && (unsigned int)index < m_saves.size())
    {
        std::pair<long long, float> save = m_saves[index];
        setSaveSize(save.first);
        setSaveGameOffset(save.second);
    }
    else
    {
        setSaveSize(0);
        setSaveGameOffset(0.0f);
    }
}

void UIControl_SpaceIndicatorBar::setTotalSize(long long size)
{
    float ratio = (float)(m_totalSize - m_startOffset) / (float)(m_endOffset - m_startOffset);
    ((FJ_SpaceIndicatorBar*)m_fuiRenderNode->getFuiNode())->setTotalSize(ratio);
}

void UIControl_SpaceIndicatorBar::reset()
{
    m_saves.clear();
    m_totalSize = 0;
    setTotalSize(0);
    setSaveSize(0);
    setSaveGameOffset(0.0f);
}

void UIControl_SpaceIndicatorBar::addSave(long long size)
{
    float offset = (float)(m_totalSize - m_startOffset) / (float)(m_endOffset - m_startOffset);
    m_saves.push_back(std::pair<long long, float>(size, offset));
    m_totalSize += size;
    setTotalSize(m_totalSize);
}

void UIControl_SpaceIndicatorBar::init(UIString label, int controlId, long long startOffset, long long totalOffset)
{
    m_label = label;
    m_controlId = controlId;
    m_startOffset = startOffset;
    m_endOffset = totalOffset;
    std::wstring& text = label.getString();
    ((FJ_SpaceIndicatorBar*)m_fuiRenderNode->getFuiNode())->Init(text);
}

void UIControl_SpaceIndicatorBar::ReInit()
{
    UIControl::ReInit();
    init(m_label, m_controlId, m_startOffset, m_endOffset);
    setSaveSize(m_saveSize);
    setTotalSize(m_totalSize);
    setSaveGameOffset(m_saveGameOffset);
}
