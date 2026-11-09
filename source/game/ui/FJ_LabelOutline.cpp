#include "game/ui/FJ_LabelOutline.hpp"

#include <cstdio>

#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"

FJ_LabelOutline::FJ_LabelOutline(fuiRenderNode* renderNode) : FJ_Base(renderNode)
{
    m_centreLabel = true;
    FJ_FuiNode* labelContainer = getChildAt(0)->asMovieClip();
    labelContainer->setX(0.0f);
    labelContainer->setY(0.0f);

    m_labelText = labelContainer->getRenderNodeChildByName(std::string("FJ_Label"))->asEditText();
    m_labelText->setAutoSize(TextFieldAutoSizeLeft);

    int i;
    for (i = 0; i < 8; ++i)
    {
        char name[68];
        std::sprintf(name, "FJ_Outline_%d", i);
        fuiRenderNode* outline = labelContainer->getRenderNodeChildByName(std::string(name));
        if (outline != 0)
        {
            m_outlineText.push_back(outline->asEditText());
            m_outlineText.back()->setAutoSize(TextFieldAutoSizeLeft);
            m_outlineX.push_back(m_outlineText[i]->getX());
        }
    }

    m_halfWidth = static_cast<int>(getWidth() * 0.5f);
    m_labelX = static_cast<int>(m_labelText->getX());
    m_centreLabel = true;
}

void FJ_LabelOutline::Init(const std::wstring& label)
{
    SetLabel(label);
}

void FJ_LabelOutline::SetLabel(const std::wstring& label)
{
    m_label.assign(label, 0, static_cast<std::wstring::size_type>(-1));
    m_labelText->setX(static_cast<float>(m_labelX));
    m_labelText->setText(label);

    int offset = 0;
    if (m_centreLabel)
    {
        offset = static_cast<int>(m_labelText->m_textWidth * 0.5f - static_cast<float>(m_halfWidth));
        m_labelText->setX(m_labelText->getX() - static_cast<float>(offset));
    }

    for (int i = 0; i < m_outlineText.size(); ++i)
    {
        m_outlineText[i]->setText(label);
        m_outlineText[i]->setX(m_outlineX[i]);
        if (m_centreLabel)
            m_outlineText[i]->setX(m_outlineText[i]->getX() - static_cast<float>(offset));
    }
}
