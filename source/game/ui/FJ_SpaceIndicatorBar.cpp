#include "game/ui/FJ_SpaceIndicatorBar.hpp"

#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"

FJ_SpaceIndicatorBar::FJ_SpaceIndicatorBar(fuiRenderNode* renderNode) : FJ_Base(renderNode)
{
    GetBarFiller();

    m_barWidth = m_saveBar->getWidth();

    fuiRect rect;
    rect.xMin = 0.0f;
    rect.yMin = 0.0f;
    rect.xMax = m_saveBar->getWidth();
    rect.yMax = m_saveBar->getHeight();
    fuiRenderNodeCodeGenRect* greenNode = new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect,
                                                                       "fillerMask_Green");
    m_saveGameFiller = new FJ_FuiNode(greenNode);
    greenNode->setFuiNode(m_saveGameFiller);
    m_saveBar->addMask(m_saveGameFiller);

    rect.xMax = m_saveBarFiller->getWidth();
    rect.yMax = m_saveBarFiller->getHeight();
    fuiRenderNodeCodeGenRect* yellowNode = new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect,
                                                                        "fillerMask_Yellow");
    m_totalFiller = new FJ_FuiNode(yellowNode);
    yellowNode->setFuiNode(m_totalFiller);
    m_saveBarFiller->addMask(m_totalFiller);

    setSaveGameSize(0.0f);
    setTotalSize(0.0f);
    setSaveGameOffset(0.0f);
}

void FJ_SpaceIndicatorBar::GetBarFiller()
{
    m_saveBar = getChildByName("SaveBarFiller")->asMovieClip();
    m_saveBarFiller = getChildByName("SaveBarFiller2")->asMovieClip();
}

void FJ_SpaceIndicatorBar::setSaveGameSize(float size)
{
    if (size > 1.0f)
    {
        size = 1.0f;
    }
    else if (size < 0.0f)
    {
        size = 0.0f;
    }

    m_saveGameFiller->setWidth((m_barWidth / m_saveBar->getScaleX()) * size);

    if (m_saveGameFiller->getWidth() < 1.0f && size > 0.0f)
    {
        m_saveGameFiller->setWidth(1.0f);
    }
}

void FJ_SpaceIndicatorBar::setTotalSize(float size)
{
    if (size > 1.0f)
    {
        size = 1.0f;
    }
    else if (size < 0.0f)
    {
        size = 0.0f;
    }

    m_totalFiller->setWidth((m_barWidth / m_saveBarFiller->getScaleX()) * size);
}

void FJ_SpaceIndicatorBar::setSaveGameOffset(float offset)
{
    m_saveGameFiller->setX(m_barWidth * offset);
}

FJ_SpaceIndicatorBar* FJ_SpaceIndicatorBar::Create(fuiRenderNode* renderNode)
{
    return new FJ_SpaceIndicatorBar(renderNode);
}

unsigned int FJ_SpaceIndicatorBar::getType()
{
    return 0x800a;
}

void FJ_SpaceIndicatorBar::Init(const std::wstring& name)
{
    SetLabelAlignment(3);
    InitBase(name, -1);
}
