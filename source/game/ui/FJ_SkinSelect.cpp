#include "game/ui/FJ_SkinSelect.hpp"

#include <string>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_KeyboardEvent.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/UIScene.hpp"

FJ_SkinSelect::FJ_SkinSelect(fuiRenderNode* renderNode) : FJ_Document(renderNode), m_animationPlaying(false)
{
    SetPlayerCharacterSelected(false, false);
    SetCharacterLocked(false);
    SetCharacterFavourite(false);
}

void FJ_SkinSelect::SetPlayerCharacterSelected(bool selected, bool visible)
{
    FJ_FuiNode* node = getChildByName(std::string("Selected"));
    node->setVisible(selected);
}

void FJ_SkinSelect::SetCharacterLocked(bool locked)
{
    FJ_FuiNode* lockedNode = getChildByName(std::string("Locked"));
    FJ_FuiNode* blockedNode = getChildByName(std::string("Blocked"));
    FJ_FuiNode* selectedNode = getChildByName(std::string("Selected"));
    lockedNode->setVisible(locked);
    if (locked)
    {
        blockedNode->setVisible(false);
        selectedNode->setVisible(false);
    }
}

void FJ_SkinSelect::SetCharacterFavourite(bool favourite)
{
    FJ_FuiNode* node = getChildByName(std::string("Favourite"));
    node->setVisible(favourite);
}

FJ_SkinSelect* FJ_SkinSelect::Create(fuiRenderNode* renderNode)
{
    return new FJ_SkinSelect(renderNode);
}

unsigned int FJ_SkinSelect::getType()
{
    return 43;
}

void FJ_SkinSelect::handleAnimationEnd(FJ_Event*)
{
    m_animationPlaying = false;
    m_pCallbackScene->handleAnimationEnd();
}

void FJ_SkinSelect::_handleAnimationEnd(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_SkinSelect*>(node)->handleAnimationEnd(event);
}

void FJ_SkinSelect::init(FJ_Event* event)
{
    FJ_FuiNode* characters = getChildByName(std::string("IggyCharacters"));
    FJ_FuiNode* buttonList = getChildByName(std::string("SkinButtonList"));
    characters->addEventListener(eFJEventType_AnimationEnd, this, _handleAnimationEnd, false);
    buttonList->asFJ_List()->BlockTouchTrigger(true);
    FJ_Document::init(event);
}

void FJ_SkinSelect::SetSafeZone(int top, int bottom, int left, int right)
{
    FJ_Document::SetSafeZone(top, bottom, left, right);
    if (getStage()->getStageHeight() == 360.0f || getStage()->getStageHeight() == 540.0f)
        setY(top);
}

void FJ_SkinSelect::CycleLeft()
{
    FJ_FuiNode* characters = getChildByName(std::string("IggyCharacters"));
    if (!m_animationPlaying)
    {
        characters->gotoAndPlay(std::string("Left"));
        m_animationPlaying = true;
    }
}

void FJ_SkinSelect::CycleRight()
{
    FJ_FuiNode* characters = getChildByName(std::string("IggyCharacters"));
    if (!m_animationPlaying)
    {
        characters->gotoAndPlay(std::string("Right"));
        m_animationPlaying = true;
    }
}

void FJ_SkinSelect::SetCharacterBlocked(bool blocked)
{
    FJ_FuiNode* blockedNode = getChildByName(std::string("Blocked"));
    FJ_FuiNode* selectedNode = getChildByName(std::string("Selected"));
    blockedNode->setVisible(blocked);
    if (blocked)
        selectedNode->setVisible(false);
}

void FJ_SkinSelect::keyDownHandler(FJ_Event* event)
{
    if (static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode == 5)
        CycleLeft();
    else if (static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode == 6)
        CycleRight();
    FJ_Document::keyDownHandler(event);
}
