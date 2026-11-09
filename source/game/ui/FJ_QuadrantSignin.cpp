#include "game/ui/FJ_QuadrantSignin.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiRenderNode.hpp"

std::string s_controllerNames[4] = {"Controller1", "Controller2", "Controller3", "Controller4"};
const std::string s_controllerStatusNames[5] = {"ConnectController", "PressToJoin", "PlayerDetails",
                                                "PlayerLoggedIn_ConnectController", "PlayerLoggedIn_NoController"};

FJ_QuadrantSignin::FJ_QuadrantSignin(fuiRenderNode* renderNode) : FJ_Document(renderNode)
{
}

FJ_Document* FJ_QuadrantSignin::Create(fuiRenderNode* renderNode)
{
    return new FJ_QuadrantSignin(renderNode);
}

unsigned int FJ_QuadrantSignin::getType()
{
    return 24;
}

void FJ_QuadrantSignin::_init(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_QuadrantSignin*>(node)->init(event);
}

void FJ_QuadrantSignin::ShowControllerTimer(int controller, bool visible)
{
    mControllers[controller]->getChildByName("Timer")->setVisible(visible);
}

void FJ_QuadrantSignin::SetControllerStatus(int controller, int status, int frame)
{
    mControllers[controller]->gotoAndStop(s_controllerStatusNames[status]);
    mControllers[controller]->getChildByName("Controllers")->getChildAt(0)->gotoAndStop(frame + 1);
    if (status == 1)
    {
        if (mABSwap)
            mControllers[controller]->getChildByName("ButtonGraphic")->gotoAndStop("StopO");
        else
            mControllers[controller]->getChildByName("ButtonGraphic")->gotoAndStop("StopX");
    }
    ShowControllerTimer(controller, false);
}

void FJ_QuadrantSignin::ShowTimerAnimation(FJ_Event* event)
{
    event->m_pTarget->getParent()->getChildByName("Timer")->setVisible(true);
}

void FJ_QuadrantSignin::_ShowTimerAnimation(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_QuadrantSignin*>(node)->ShowTimerAnimation(event);
}

void FJ_QuadrantSignin::init(FJ_Event* event)
{
    for (int i = 0; i < 4; ++i)
    {
        mControllers[i] = getChildByName(s_controllerNames[i]);
        ShowControllerTimer(i, false);
        SetControllerStatus(i, 0, 1);
        mControllers[i]
            ->getChildByName("ButtonGraphic")
            ->addEventListener(eFJEventType_ShowTimerAnimation, this, _ShowTimerAnimation, false);
    }
    mABSwap = false;
}

void FJ_QuadrantSignin::SetABSwap(bool swap)
{
    mABSwap = swap;
    if (swap)
    {
        mControllers[0]->getChildByName("ButtonGraphic")->gotoAndStop("StopO");
        mControllers[1]->getChildByName("ButtonGraphic")->gotoAndStop("StopO");
        mControllers[2]->getChildByName("ButtonGraphic")->gotoAndStop("StopO");
        mControllers[3]->getChildByName("ButtonGraphic")->gotoAndStop("StopO");
    }
    else
    {
        mControllers[0]->getChildByName("ButtonGraphic")->gotoAndStop("StopX");
        mControllers[1]->getChildByName("ButtonGraphic")->gotoAndStop("StopX");
        mControllers[2]->getChildByName("ButtonGraphic")->gotoAndStop("StopX");
        mControllers[3]->getChildByName("ButtonGraphic")->gotoAndStop("StopX");
    }
}

void FJ_QuadrantSignin::JoinButtonPressed(int controller)
{
    SetControllerStatus(controller, 1, 1);
}
