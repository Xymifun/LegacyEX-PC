#include "game/ui/FJ_ToolTips.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_KeyboardEvent.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_ToolTip.hpp"

static std::string s_toolTipNames[16] = {
    "ToolTip_0", "ToolTip_1", "ToolTip_2",  "ToolTip_3",  "ToolTip_4",  "ToolTip_5",  "ToolTip_6",  "ToolTip_7",
    "ToolTip_8", "ToolTip_9", "ToolTip_10", "ToolTip_11", "ToolTip_12", "ToolTip_13", "ToolTip_14", "ToolTip_15"};

FJ_ToolTips::FJ_ToolTips(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
    m_useTouchControls = 0;
    m_touchControlsAvailable = false;
}

FJ_Document* FJ_ToolTips::Create(fuiRenderNode* renderNode)
{
    return new FJ_ToolTips(renderNode);
}

unsigned int FJ_ToolTips::getType()
{
    return 6;
}

void FJ_ToolTips::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_ToolTips*>(node)->keyDownHandler(event);
}

void FJ_ToolTips::init(FJ_Event*)
{
    m_toolTipSuffix.assign(L"  ");
    for (int i = 0; i < 16; ++i)
    {
        m_toolTipNodes.push_back(getChildByName(s_toolTipNames[i]));
        static_cast<FJ_ToolTip*>(m_toolTipNodes[i])->Init(L"", i, false, -1, -1);
    }
    getStage()->addEventListener(eFJEventType_KeyDown, this, _keyDownHandler, false);
    m_touchControlsAvailable = false;
    if (getChildByName("Touch_0") != 0)
    {
        m_touchNodes.push_back(getChildByName("Touch_0"));
        m_touchNodes.push_back(getChildByName("Touch_1"));
        m_touchNodes.push_back(getChildByName("Touch_2"));
        m_touchNodes.push_back(getChildByName("Touch_3"));
        m_touchNodes.push_back(getChildByName("Touch_4"));
        m_touchNodes.push_back(getChildByName("Touch_5"));
        m_touchNodes.push_back(getChildByName("Touch_6"));
        m_touchNodes.push_back(getChildByName("Touch_7"));
        m_touchNodes.push_back(getChildByName("Touch_8"));
        m_touchNodes.push_back(getChildByName("Touch_9"));
        m_touchNodes.push_back(getChildByName("Touch_10"));
        m_touchNodes.push_back(getChildByName("Touch_11"));
        m_touchNodes.push_back(getChildByName("Touch_12"));
        m_touchNodes.push_back(getChildByName("Touch_13"));
        m_touchNodes.push_back(getChildByName("Touch_14"));
        m_touchNodes.push_back(getChildByName("Touch_15"));
        m_touchControlsAvailable = true;
    }
}

void FJ_ToolTips::UpdateControllerType(int controllerType)
{
    for (int i = 0; i < 16; ++i)
        static_cast<FJ_ToolTip*>(m_toolTipNodes[i])->UpdateControllerType(controllerType);
}

void FJ_ToolTips::keyDownHandler(FJ_Event* event)
{
    switch (static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode)
    {
    case 0:
        if (static_cast<FJ_ToolTip*>(m_toolTipNodes[0])->IsSingleButtonToolTip())
            static_cast<FJ_ToolTip*>(m_toolTipNodes[0])->Animate();
        break;
    case 7:
        if (static_cast<FJ_ToolTip*>(m_toolTipNodes[1])->IsSingleButtonToolTip())
            static_cast<FJ_ToolTip*>(m_toolTipNodes[1])->Animate();
        break;
    case 8:
        if (static_cast<FJ_ToolTip*>(m_toolTipNodes[2])->IsSingleButtonToolTip())
            static_cast<FJ_ToolTip*>(m_toolTipNodes[2])->Animate();
        break;
    case 9:
        if (static_cast<FJ_ToolTip*>(m_toolTipNodes[3])->IsSingleButtonToolTip())
            static_cast<FJ_ToolTip*>(m_toolTipNodes[3])->Animate();
        break;
    case 3:
    case 4:
    case 5:
    case 6:
        if (static_cast<FJ_ToolTip*>(m_toolTipNodes[15])->IsSingleButtonToolTip())
            static_cast<FJ_ToolTip*>(m_toolTipNodes[15])->Animate();
        break;
    case 1:
        if (static_cast<FJ_ToolTip*>(m_toolTipNodes[4])->IsSingleButtonToolTip())
            static_cast<FJ_ToolTip*>(m_toolTipNodes[4])->Animate();
        break;
    case 2:
        if (static_cast<FJ_ToolTip*>(m_toolTipNodes[5])->IsSingleButtonToolTip())
            static_cast<FJ_ToolTip*>(m_toolTipNodes[5])->Animate();
        break;
    case 10:
        if (static_cast<FJ_ToolTip*>(m_toolTipNodes[7])->IsSingleButtonToolTip())
            static_cast<FJ_ToolTip*>(m_toolTipNodes[7])->Animate();
        break;
    case 11:
        if (static_cast<FJ_ToolTip*>(m_toolTipNodes[6])->IsSingleButtonToolTip())
            static_cast<FJ_ToolTip*>(m_toolTipNodes[6])->Animate();
        break;
    case 12:
        if (static_cast<FJ_ToolTip*>(m_toolTipNodes[9])->IsSingleButtonToolTip())
            static_cast<FJ_ToolTip*>(m_toolTipNodes[9])->Animate();
        break;
    case 13:
        if (static_cast<FJ_ToolTip*>(m_toolTipNodes[10])->IsSingleButtonToolTip())
            static_cast<FJ_ToolTip*>(m_toolTipNodes[10])->Animate();
        break;
    }
}

void FJ_ToolTips::SetToolTip(int index, int type, int subtype, const std::wstring& text, bool visible)
{
    if (index < 16)
    {
        m_toolTipNodes[index]->setVisible(visible);
        static_cast<FJ_ToolTip*>(m_toolTipNodes[index])->SetType(type, subtype);
        static_cast<FJ_ToolTip*>(m_toolTipNodes[index])->SetLabel(text + m_toolTipSuffix);
    }
    m_toolTipIndex = index;
}

void FJ_ToolTips::UpdateLayout()
{
    int bottom = getISafeZoneBottom();
    if (bottom == 0)
        bottom = 2;
    int left = getISafeZoneLeft();
    FJ_FuiNode** first = &m_toolTipNodes[0];
    fuiRect firstRect = (*first)->getRect(*first);
    int y = static_cast<int>(getStage()->getStageHeight() - (*first)->getHeight() - bottom - firstRect.yMin);
    if (left == 0)
        left = 2;

    for (int i = 0; i < 16; ++i)
    {
        if (m_toolTipNodes[i]->isVisible())
        {
            m_toolTipNodes[i]->setX(left);
            m_toolTipNodes[i]->setY(y);
            if (m_touchControlsAvailable)
            {
                m_touchNodes[i]->setVisible(true);
                m_touchNodes[i]->setX(left);
                m_touchNodes[i]->setY(y - m_toolTipNodes[i]->getHeight());
                m_touchNodes[i]->setWidth(m_toolTipNodes[i]->getWidth());
                FJ_FuiNode** node = &m_toolTipNodes[i];
                m_touchNodes[i]->setHeight((*node)->getRect(*node).yMin + (*node)->getHeight() * 2);
            }

            bool emptyLabel = false;
            if (static_cast<FJ_ToolTip*>(m_toolTipNodes[i])->GetLabel().length() == 2)
            {
                static_cast<FJ_ToolTip*>(m_toolTipNodes[i])->SetLabel(L"oo");
                emptyLabel = true;
            }
            left += static_cast<int>(m_toolTipNodes[i]->getWidth());
            if (emptyLabel)
                static_cast<FJ_ToolTip*>(m_toolTipNodes[i])->SetLabel(m_toolTipSuffix);
        }
        else if (m_touchControlsAvailable)
        {
            m_touchNodes[i]->setX(-1.0f);
            m_touchNodes[i]->setY(-1.0f);
            m_touchNodes[i]->setWidth(1.0f);
            m_touchNodes[i]->setHeight(1.0f);
        }
    }
}
