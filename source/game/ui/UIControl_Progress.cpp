#include "game/ui/UIControl_Progress.hpp"

#include "game/ui/FJ_ProgressBar.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_Progress::UIControl_Progress()
    : UIControl_Base(),
      m_minimum(0),
      m_maximum(100),
      m_progress(0),
      m_progressRatio(0.0f),
      m_showBar(true)
{
}

bool UIControl_Progress::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    m_controlType = UIControl::eUIControlType_Progress;
    return UIControl_Base::setupControl(scene, node, name);
}

void UIControl_Progress::init(UIString label, int controlId, int minimum, int maximum, int progress)
{
    m_label = label;
    m_controlId = controlId;
    m_minimum = minimum;
    m_maximum = maximum;
    m_progress = progress;

    FJ_ProgressBar* progressBar = static_cast<FJ_ProgressBar*>(m_fuiRenderNode->getFuiNode());
    progressBar->Init(label.getString());
}

void UIControl_Progress::setProgress(int progress)
{
    m_progress = progress;
    float ratio = 0.0f;
    if (m_maximum != m_minimum)
        ratio = (float)(progress - m_minimum) / (float)(m_maximum - m_minimum);

    if (ratio == m_progressRatio)
        return;

    m_progressRatio = ratio;
    if (m_scene->m_movie != NULL)
    {
        FJ_ProgressBar* progressBar = static_cast<FJ_ProgressBar*>(m_fuiRenderNode->getFuiNode());
        progressBar->setProgress(m_progressRatio);
    }
}

void UIControl_Progress::showBar(bool show)
{
    if (show == m_showBar)
        return;

    m_showBar = show;
    if (m_scene->m_movie != NULL)
    {
        FJ_ProgressBar* progressBar = static_cast<FJ_ProgressBar*>(m_fuiRenderNode->getFuiNode());
        progressBar->ShowBar(show);
    }
}

void UIControl_Progress::ReInit()
{
    UIControl::ReInit();
    init(m_label, m_controlId, m_minimum, m_maximum, m_progress);
}

unsigned int UIControl_Progress::getMax()
{
    return (unsigned int)m_maximum;
}

void UIControl_Progress::setMax(int maximum)
{
    m_maximum = maximum;
}
