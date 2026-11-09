#include "game/ui/FJ_ProgressBar.hpp"

#include <string>

#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/fuiRenderNodeCodeGenRect.hpp"

FJ_ProgressBar::FJ_ProgressBar(fuiRenderNode* renderNode)
    : FJ_Base(renderNode),
      m_pBarFiller(NULL),
      m_pFiller(NULL),
      m_progress(0.0f)
{
    InitBarType();
}

void FJ_ProgressBar::InitBarType()
{
    m_barType = true;
    m_reverse = false;
}

void FJ_ProgressBar::initFiller()
{
    GetBarFiller();

    m_fillerX = m_pFiller->getX();
    m_fillerY = m_pFiller->getY();
    m_fillerWidth = m_pFiller->getWidth();
    m_fillerHeight = m_pFiller->getHeight();

    fuiRect rect;
    rect.xMin = 0.0f;
    rect.yMin = 0.0f;
    rect.xMax = m_pFiller->getWidth();
    rect.yMax = m_pFiller->getHeight();
    fuiRenderNodeCodeGenRect* renderNode = new fuiRenderNodeCodeGenRect(getStage()->m_pRenderNode, &rect, "fillerMask");
    m_pBarFiller = new FJ_FuiNode(renderNode);
    renderNode->setFuiNode(m_pBarFiller);
    m_pFiller->addMask(m_pBarFiller);
    setProgress(0.0f);
}

void FJ_ProgressBar::GetBarFiller()
{
    m_pFiller = getChildByName("ProgressBarFiller")->asMovieClip();
}

void FJ_ProgressBar::Init(const std::wstring& label)
{
    SetLabelAlignment(0);
    InitBase(label, -1);
}

void FJ_ProgressBar::ShowBar(bool show)
{
    if (m_pFiller != NULL)
        m_pFiller->setVisible(show);

    FJ_FuiNode* movieClip = FJ_FuiNode::asMovieClip(getChildByName("ProgressBarBackground"));
    if (movieClip != NULL)
        movieClip->setVisible(show);
}

float FJ_ProgressBar::getProgress()
{
    return m_progress;
}

void FJ_ProgressBar::setProgress(float progress)
{
    m_progress = progress;
    if (m_progress > 1.0f)
        m_progress = 1.0f;
    else if (m_progress < 0.0f)
        m_progress = 0.0f;

    if (m_barType)
    {
        m_pBarFiller->setWidth((m_fillerWidth / m_pFiller->getScaleX()) * m_progress);
        if (m_reverse)
            m_pBarFiller->setX((m_fillerWidth / m_pFiller->getScaleX()) * (1.0f - m_progress) + m_fillerX);
    }
    else
    {
        m_pBarFiller->setHeight((m_fillerHeight / m_pFiller->getScaleY()) * m_progress);
        if (m_reverse)
            m_pBarFiller->setY((m_fillerHeight / m_pFiller->getScaleY()) * (1.0f - m_progress) + m_fillerY);
    }
}
