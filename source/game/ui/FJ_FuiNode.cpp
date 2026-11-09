#include "game/ui/FJ_FuiNode.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_BitmapIcon.hpp"
#include "game/ui/FJ_Button.hpp"
#include "game/ui/FJ_CheckBox.hpp"
#include "game/ui/FJ_Cursor.hpp"
#include "game/ui/FJ_Document.hpp"
#include "game/ui/FJ_DragonHealth.hpp"
#include "game/ui/FJ_DynamicHtmlText.hpp"
#include "game/ui/FJ_ExperienceBar.hpp"
#include "game/ui/FJ_HorseJumpBar.hpp"
#include "game/ui/FJ_HtmlText.hpp"
#include "game/ui/FJ_HtmlTextPanel.hpp"
#include "game/ui/FJ_HtmlTextPanelLarge.hpp"
#include "game/ui/FJ_HtmlText_WithPanel.hpp"
#include "game/ui/FJ_Hud.hpp"
#include "game/ui/FJ_IconSlot.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_LabelOutline.hpp"
#include "game/ui/FJ_Label_Blink.hpp"
#include "game/ui/FJ_Label_Book.hpp"
#include "game/ui/FJ_Label_HUD_Splash.hpp"
#include "game/ui/FJ_Label_HUD_SplashBig.hpp"
#include "game/ui/FJ_Label_HUD_SplashVita.hpp"
#include "game/ui/FJ_LeaderboardList.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/FJ_List2D.hpp"
#include "game/ui/FJ_ListButton.hpp"
#include "game/ui/FJ_ListButtonAchievement.hpp"
#include "game/ui/FJ_ListButtonController.hpp"
#include "game/ui/FJ_List_ListButton.hpp"
#include "game/ui/FJ_List_ListButtonAchievement.hpp"
#include "game/ui/FJ_List_ListButtonPlayerList.hpp"
#include "game/ui/FJ_List_ListButtonScoreBoard.hpp"
#include "game/ui/FJ_List_MenuButton.hpp"
#include "game/ui/FJ_List_MultiList.hpp"
#include "game/ui/FJ_MenuButton.hpp"
#include "game/ui/FJ_MobEffect.hpp"
#include "game/ui/FJ_MobEffect_Tile.hpp"
#include "game/ui/FJ_ProgressBar.hpp"
#include "game/ui/FJ_ScrollArrow.hpp"
#include "game/ui/FJ_SignInput.hpp"
#include "game/ui/FJ_Slider.hpp"
#include "game/ui/FJ_SlotList.hpp"
#include "game/ui/FJ_TextInput.hpp"
#include "game/ui/FJ_UpsellButton.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRenderNodeStage.hpp"
#include "game/ui/fuiRenderNodeTimeline.hpp"

FJ_FuiNode::FJ_FuiNode(fuiRenderNode* pRenderNode)
{
    m_pRenderNode = pRenderNode;
    m_pCallbackScene = NULL;
    m_iTabIndex = -1;
}

FJ_FuiNode* FJ_FuiNode::Create(fuiRenderNode* pRenderNode)
{
    return new FJ_FuiNode(pRenderNode);
}

FJ_FuiNodeStage* FJ_FuiNode::getStage()
{
    return (FJ_FuiNodeStage*)m_pRenderNode->getStage()->getFuiNode();
}

FJ_FuiNode::~FJ_FuiNode()
{
    gFui->removeEventListenerForNode(this);

    if (m_pRenderNode != NULL)
    {
        if (m_pRenderNode->m_stage != NULL)
            getStage()->removeFocus(this);
    }
}

void FJ_FuiNode::ASConstructor()
{
}

float FJ_FuiNode::getX()
{
    return m_pRenderNode->getX();
}

float FJ_FuiNode::getY()
{
    return m_pRenderNode->getY();
}

float FJ_FuiNode::getWidth()
{
    return m_pRenderNode->getWidth();
}

float FJ_FuiNode::getHeight()
{
    return m_pRenderNode->getHeight();
}

void FJ_FuiNode::setScaleX(float fScaleX)
{
    m_pRenderNode->setScaleX(fScaleX);
}

void FJ_FuiNode::setScaleY(float fScaleY)
{
    m_pRenderNode->setScaleY(fScaleY);
}

float FJ_FuiNode::getScaleX()
{
    return m_pRenderNode->getScaleX();
}

float FJ_FuiNode::getScaleY()
{
    return m_pRenderNode->getScaleY();
}

const char* FJ_FuiNode::getName()
{
    return m_pRenderNode->getName();
}

fuiRect FJ_FuiNode::getRect(FJ_FuiNode* pNode)
{
    return m_pRenderNode->getBounds();
}

int FJ_FuiNode::getTabIndex()
{
    return m_iTabIndex;
}

void FJ_FuiNode::setScrollRect(fuiRect& rect)
{
    m_pRenderNode->setScrollRect(rect);
}

int FJ_FuiNode::getNumChildren()
{
    if (m_pRenderNode == NULL)
        return 0;

    if (m_pRenderNode->m_type != eFuiObjectType_Timeline)
        return 0;

    return static_cast<fuiRenderNodeTimeline*>(m_pRenderNode)->getNumChildren();
}

FJ_FuiNode* FJ_FuiNode::getChildByName(const std::string& name)
{
    if (m_pRenderNode->m_type == eFuiObjectType_Timeline)
    {
        fuiRenderNode* pChild = static_cast<fuiRenderNodeTimeline*>(m_pRenderNode)->findNode(name.c_str());
        if (pChild != NULL)
            return pChild->getFuiNode();
    }

    return NULL;
}

fuiRenderNode* FJ_FuiNode::getRenderNodeChildByName(const std::string& name)
{
    if (m_pRenderNode->m_type == eFuiObjectType_Timeline)
    {
        fuiRenderNode* pChild = static_cast<fuiRenderNodeTimeline*>(m_pRenderNode)->findNode(name.c_str());
        if (pChild != NULL)
            return pChild;
    }

    return NULL;
}

fuiRenderNode* FJ_FuiNode::getRenderNodeChildAt(int iIndex)
{
    if (m_pRenderNode->m_type != eFuiObjectType_Timeline)
        return NULL;

    return static_cast<fuiRenderNodeTimeline*>(m_pRenderNode)->getChildAt(iIndex);
}

fuiRenderNode* FJ_FuiNode::getRenderNodeParent()
{
    return m_pRenderNode->getParent();
}

FJ_FuiNode* FJ_FuiNode::getChildAt(int iIndex)
{
    if (m_pRenderNode->m_type != eFuiObjectType_Timeline)
        return NULL;

    return static_cast<fuiRenderNodeTimeline*>(m_pRenderNode)->getChildAt(iIndex)->getFuiNode();
}

FJ_FuiNode* FJ_FuiNode::getParent()
{
    fuiRenderNode* pParent = m_pRenderNode->getParent();
    if (pParent == NULL)
        return NULL;

    return pParent->getFuiNode();
}

void FJ_FuiNode::removeChild(FJ_FuiNode* pNode, bool bDestroy)
{
    static_cast<fuiRenderNodeTimeline*>(m_pRenderNode->asTimeline())->removeChild(pNode->m_pRenderNode, bDestroy);
}

void FJ_FuiNode::removeChild(fuiRenderNode* pRenderNode, bool bDestroy)
{
    static_cast<fuiRenderNodeTimeline*>(m_pRenderNode->asTimeline())->removeChild(pRenderNode, bDestroy);
}

void FJ_FuiNode::removeChildAt(int iIndex, bool bDestroy)
{
    static_cast<fuiRenderNodeTimeline*>(m_pRenderNode->asTimeline())->removeChildAt(iIndex, bDestroy);
}

void FJ_FuiNode::addMask(FJ_FuiNode* pNode)
{
    static_cast<fuiRenderNodeTimeline*>(m_pRenderNode->asTimeline())->addMask(pNode->m_pRenderNode);
}

void FJ_FuiNode::addChild(FJ_FuiNode* pNode)
{
    static_cast<fuiRenderNodeTimeline*>(m_pRenderNode->asTimeline())->addChild(pNode->m_pRenderNode);
}

void FJ_FuiNode::addChildAt(FJ_FuiNode* pNode, int iIndex)
{
    static_cast<fuiRenderNodeTimeline*>(m_pRenderNode->asTimeline())->addChildAt(pNode->m_pRenderNode, iIndex);
}

void FJ_FuiNode::addChild(fuiRenderNode* pRenderNode)
{
    static_cast<fuiRenderNodeTimeline*>(m_pRenderNode->asTimeline())->addChild(pRenderNode);
}

void FJ_FuiNode::addChildAt(fuiRenderNode* pRenderNode, int iIndex)
{
    static_cast<fuiRenderNodeTimeline*>(m_pRenderNode->asTimeline())->addChildAt(pRenderNode, iIndex);
}

int FJ_FuiNode::getChildIndex(FJ_FuiNode* pNode)
{
    return static_cast<fuiRenderNodeTimeline*>(m_pRenderNode->asTimeline())->getChildIndex(pNode->m_pRenderNode);
}

FJ_FuiNode* FJ_FuiNode::asMovieClip()
{
    if (m_pRenderNode->m_type == eFuiObjectType_Timeline)
        return this;

    return NULL;
}

FJ_FuiNode* FJ_FuiNode::asMovieClip(FJ_FuiNode* pNode)
{
    if (pNode != NULL)
        return pNode->asMovieClip();

    return NULL;
}

FJ_Label* FJ_FuiNode::asFJ_Label()
{
    return dynamic_cast<FJ_Label*>(this);
}

FJ_Base* FJ_FuiNode::asFJ_Base()
{
    return dynamic_cast<FJ_Base*>(this);
}

bool FJ_FuiNode::isFJ_Base()
{
    return asFJ_Base() != NULL;
}

FJ_Button* FJ_FuiNode::asFJ_Button()
{
    return dynamic_cast<FJ_Button*>(this);
}

bool FJ_FuiNode::isFJ_Button()
{
    return asFJ_Button() != NULL;
}

FJ_MenuButton* FJ_FuiNode::asFJ_MenuButton()
{
    return dynamic_cast<FJ_MenuButton*>(this);
}


bool FJ_FuiNode::isFJ_Label()
{
    return asFJ_Label() != NULL;
}

FJ_LabelOutline* FJ_FuiNode::asFJ_LabelOutline()
{
    return dynamic_cast<FJ_LabelOutline*>(this);
}

bool FJ_FuiNode::isFJ_LabelOutline()
{
    return asFJ_LabelOutline() != NULL;
}

FJ_Label_HUD_Splash* FJ_FuiNode::asFJ_Label_HUD_Splash()
{
    return dynamic_cast<FJ_Label_HUD_Splash*>(this);
}

bool FJ_FuiNode::isFJ_Label_HUD_Splash()
{
    return asFJ_Label_HUD_Splash() != NULL;
}

FJ_Label_HUD_SplashBig* FJ_FuiNode::asFJ_Label_HUD_SplashBig()
{
    return dynamic_cast<FJ_Label_HUD_SplashBig*>(this);
}

bool FJ_FuiNode::isFJ_Label_HUD_SplashBig()
{
    return asFJ_Label_HUD_SplashBig() != NULL;
}

FJ_Label_HUD_SplashVita* FJ_FuiNode::asFJ_Label_HUD_SplashVita()
{
    return dynamic_cast<FJ_Label_HUD_SplashVita*>(this);
}


FJ_Label_Blink* FJ_FuiNode::asFJ_Label_Blink()
{
    return dynamic_cast<FJ_Label_Blink*>(this);
}

bool FJ_FuiNode::isFJ_Label_Blink()
{
    return asFJ_Label_Blink() != NULL;
}

FJ_List* FJ_FuiNode::asFJ_List()
{
    return dynamic_cast<FJ_List*>(this);
}

bool FJ_FuiNode::isFJ_List()
{
    return asFJ_List() != NULL;
}

FJ_List2D* FJ_FuiNode::asFJ_List2D()
{
    return dynamic_cast<FJ_List2D*>(this);
}

bool FJ_FuiNode::isFJ_List2D()
{
    return asFJ_List2D() != NULL;
}

FJ_List_MenuButton* FJ_FuiNode::asFJ_List_MenuButton()
{
    return dynamic_cast<FJ_List_MenuButton*>(this);
}


FJ_ListButton* FJ_FuiNode::asFJ_ListButton()
{
    return dynamic_cast<FJ_ListButton*>(this);
}

bool FJ_FuiNode::isFJ_ListButton()
{
    return asFJ_ListButton() != NULL;
}


FJ_ListButtonController* FJ_FuiNode::asFJ_ListButtonController()
{
    return dynamic_cast<FJ_ListButtonController*>(this);
}

bool FJ_FuiNode::isFJ_ListButtonController()
{
    return asFJ_ListButtonController() != NULL;
}

FJ_List_ListButton* FJ_FuiNode::asFJ_List_ListButton()
{
    return dynamic_cast<FJ_List_ListButton*>(this);
}

bool FJ_FuiNode::isFJ_List_ListButton()
{
    return asFJ_List_ListButton() != NULL;
}


FJ_List_ListButtonPlayerList* FJ_FuiNode::asFJ_List_ListButtonPlayerList()
{
    return dynamic_cast<FJ_List_ListButtonPlayerList*>(this);
}


FJ_List_ListButtonScoreBoard* FJ_FuiNode::asFJ_List_ListButtonScoreBoard()
{
    return dynamic_cast<FJ_List_ListButtonScoreBoard*>(this);
}


FJ_HtmlText* FJ_FuiNode::asFJ_HtmlText()
{
    return dynamic_cast<FJ_HtmlText*>(this);
}

bool FJ_FuiNode::isFJ_HtmlText()
{
    return asFJ_HtmlText() != NULL;
}

FJ_DynamicHtmlText* FJ_FuiNode::asFJ_DynamicHtmlText()
{
    return dynamic_cast<FJ_DynamicHtmlText*>(this);
}

bool FJ_FuiNode::isFJ_DynamicHtmlText()
{
    return asFJ_DynamicHtmlText() != NULL;
}

FJ_HtmlText_WithPanel* FJ_FuiNode::asFJ_HtmlText_WithPanel()
{
    return dynamic_cast<FJ_HtmlText_WithPanel*>(this);
}

bool FJ_FuiNode::isFJ_HtmlText_WithPanel()
{
    return asFJ_HtmlText_WithPanel() != NULL;
}

FJ_Slider* FJ_FuiNode::asFJ_Slider()
{
    return dynamic_cast<FJ_Slider*>(this);
}

bool FJ_FuiNode::isFJ_Slider()
{
    return asFJ_Slider() != NULL;
}

FJ_CheckBox* FJ_FuiNode::asFJ_CheckBox()
{
    return dynamic_cast<FJ_CheckBox*>(this);
}

bool FJ_FuiNode::isFJ_CheckBox()
{
    return asFJ_CheckBox() != NULL;
}

FJ_TextInput* FJ_FuiNode::asFJ_TextInput()
{
    return dynamic_cast<FJ_TextInput*>(this);
}

bool FJ_FuiNode::isFJ_TextInput()
{
    return asFJ_TextInput() != NULL;
}


FJ_HorseJumpBar* FJ_FuiNode::asFJ_HorseJumpBar()
{
    return dynamic_cast<FJ_HorseJumpBar*>(this);
}


FJ_ExperienceBar* FJ_FuiNode::asFJ_ExperienceBar()
{
    return dynamic_cast<FJ_ExperienceBar*>(this);
}


FJ_Cursor* FJ_FuiNode::asFJ_Cursor()
{
    return dynamic_cast<FJ_Cursor*>(this);
}

bool FJ_FuiNode::isFJ_Cursor()
{
    return asFJ_Cursor() != NULL;
}

FJ_SlotList* FJ_FuiNode::asFJ_SlotList()
{
    return dynamic_cast<FJ_SlotList*>(this);
}


FJ_ScrollArrow* FJ_FuiNode::asFJ_ScrollArrow()
{
    return dynamic_cast<FJ_ScrollArrow*>(this);
}


FJ_MobEffect* FJ_FuiNode::asFJ_MobEffect()
{
    return dynamic_cast<FJ_MobEffect*>(this);
}


FJ_ProgressBar* FJ_FuiNode::asFJ_ProgressBar()
{
    return dynamic_cast<FJ_ProgressBar*>(this);
}


FJ_BitmapIcon* FJ_FuiNode::asFJ_BitmapIcon()
{
    return dynamic_cast<FJ_BitmapIcon*>(this);
}


FJ_IconSlot* FJ_FuiNode::asFJ_IconSlot()
{
    return dynamic_cast<FJ_IconSlot*>(this);
}


FJ_Label_Book* FJ_FuiNode::asFJ_Label_Book()
{
    return dynamic_cast<FJ_Label_Book*>(this);
}

bool FJ_FuiNode::isFJ_Label_Book()
{
    return asFJ_Label_Book() != NULL;
}

FJ_List_MultiList* FJ_FuiNode::asFJ_List_MultiList()
{
    return dynamic_cast<FJ_List_MultiList*>(this);
}


FJ_Document* FJ_FuiNode::asFJ_Document()
{
    return dynamic_cast<FJ_Document*>(this);
}

bool FJ_FuiNode::isFJ_Document()
{
    return asFJ_Document() != NULL;
}

FJ_HtmlTextPanel* FJ_FuiNode::asFJ_HtmlTextPanel()
{
    return dynamic_cast<FJ_HtmlTextPanel*>(this);
}

bool FJ_FuiNode::isFJ_HtmlTextPanel()
{
    return asFJ_HtmlTextPanel() != NULL;
}

FJ_HtmlTextPanelLarge* FJ_FuiNode::asFJ_HtmlTextPanelLarge()
{
    return dynamic_cast<FJ_HtmlTextPanelLarge*>(this);
}

bool FJ_FuiNode::isFJ_HtmlTextPanelLarge()
{
    return asFJ_HtmlTextPanelLarge() != NULL;
}

FJ_ListButtonAchievement* FJ_FuiNode::asFJ_ListButtonAchievement()
{
    return dynamic_cast<FJ_ListButtonAchievement*>(this);
}

bool FJ_FuiNode::isFJ_ListButtonAchievement()
{
    return asFJ_ListButtonAchievement() != NULL;
}

FJ_SignInput* FJ_FuiNode::asFJ_SignInput()
{
    return dynamic_cast<FJ_SignInput*>(this);
}

bool FJ_FuiNode::isFJ_SignInput()
{
    return asFJ_SignInput() != NULL;
}

FJ_LeaderboardList* FJ_FuiNode::asFJ_LeaderboardList()
{
    return dynamic_cast<FJ_LeaderboardList*>(this);
}

bool FJ_FuiNode::isFJ_LeaderboardList()
{
    return asFJ_LeaderboardList() != NULL;
}

FJ_Hud* FJ_FuiNode::asFJ_Hud()
{
    return dynamic_cast<FJ_Hud*>(this);
}

bool FJ_FuiNode::isFJ_Hud()
{
    return asFJ_Hud() != NULL;
}

void FJ_FuiNode::gotoAndPlay(int iFrame)
{
    if (m_pRenderNode->m_type == eFuiObjectType_Timeline)
    {
        fuiRenderNodeTimeline* pTimeline = static_cast<fuiRenderNodeTimeline*>(m_pRenderNode);
        pTimeline->setFrame(iFrame);
        pTimeline->setPlaying(true);
    }
}

void FJ_FuiNode::gotoAndPlay(const std::string& sLabel)
{
    if (m_pRenderNode->m_type == eFuiObjectType_Timeline)
    {
        fuiRenderNodeTimeline* pTimeline = static_cast<fuiRenderNodeTimeline*>(m_pRenderNode);
        pTimeline->setFrame(sLabel.c_str());
        pTimeline->setPlaying(true);
    }
}

void FJ_FuiNode::gotoAndStop(int iFrame)
{
    if (m_pRenderNode->m_type == eFuiObjectType_Timeline)
    {
        fuiRenderNodeTimeline* pTimeline = static_cast<fuiRenderNodeTimeline*>(m_pRenderNode);
        pTimeline->setFrame(iFrame);
        pTimeline->setPlaying(false);
    }
}

void FJ_FuiNode::gotoAndStop(const std::string& sLabel)
{
    if (m_pRenderNode->m_type == eFuiObjectType_Timeline)
    {
        fuiRenderNodeTimeline* pTimeline = static_cast<fuiRenderNodeTimeline*>(m_pRenderNode);
        pTimeline->setFrame(sLabel.c_str());
        pTimeline->setPlaying(false);
    }
}


std::string FJ_FuiNode::currentLabel()
{
    if (m_pRenderNode->m_type != eFuiObjectType_Timeline)
        return "";

    return static_cast<fuiRenderNodeTimeline*>(m_pRenderNode)->currentLabel();
}

unsigned int FJ_FuiNode::currentFrame()
{
    if (m_pRenderNode->m_type != eFuiObjectType_Timeline)
        return 1;

    return static_cast<fuiRenderNodeTimeline*>(m_pRenderNode)->currentFrame();
}

void FJ_FuiNode::setX(float fX)
{
    m_pRenderNode->setX(fX);
}

void FJ_FuiNode::decX(float fAmount)
{
    setX(getX() - fAmount);
}

void FJ_FuiNode::incX(float fAmount)
{
    setX(getX() + fAmount);
}

void FJ_FuiNode::setY(float fY)
{
    m_pRenderNode->setY(fY);
}

void FJ_FuiNode::decY(float fAmount)
{
    setY(getY() - fAmount);
}

void FJ_FuiNode::incY(float fAmount)
{
    setY(getY() + fAmount);
}

void FJ_FuiNode::setHeight(float fHeight)
{
    m_pRenderNode->setHeight(fHeight);
}

void FJ_FuiNode::decHeight(float fAmount)
{
    setHeight(getHeight() - fAmount);
}

void FJ_FuiNode::incHeight(float fAmount)
{
    setHeight(getHeight() + fAmount);
}

void FJ_FuiNode::setWidth(float fWidth)
{
    m_pRenderNode->setWidth(fWidth);
}


void FJ_FuiNode::incWidth(float fAmount)
{
    setWidth(getWidth() + fAmount);
}

void FJ_FuiNode::setTabIndex(int iTabIndex)
{
    m_iTabIndex = iTabIndex;
}

void FJ_FuiNode::setVisible(bool bVisible)
{
    m_pRenderNode->setVisibility(bVisible);
}

bool FJ_FuiNode::isVisible()
{
    return m_pRenderNode->isVisible();
}

float FJ_FuiNode::getAlpha()
{
    return m_pRenderNode->getAlpha();
}

void FJ_FuiNode::setAlpha(float fAlpha)
{
    m_pRenderNode->setAlpha(fAlpha);
}


void FJ_FuiNode::incAlpha(float fAmount)
{
    setAlpha(getAlpha() + fAmount);
}

void FJ_FuiNode::eventListen(FJ_Event* pEvent, bool bCapture)
{
    for (unsigned int i = 0; i < m_eventListeners.size(); ++i)
    {
        const FJ_EventListener& listener = m_eventListeners[i];
        if (listener.m_bCapture == bCapture)
        {
            if (listener.m_eType == pEvent->m_eType)
                listener.m_pfnHandler(listener.m_pNode, pEvent);
        }
    }
}

void FJ_FuiNode::dispatchEvent(FJ_Event* pEvent)
{
    std::vector<FJ_FuiNode*> nodes;

    pEvent->m_pTarget = this;

    for (fuiRenderNode* pNode = m_pRenderNode; pNode != NULL; pNode = pNode->getParent())
    {
        if (pNode->getFuiNode() != NULL)
            nodes.push_back(pNode->getFuiNode());
    }

    if (nodes.size() != 0)
    {
        for (unsigned int i = nodes.size() - 1; i != 0; --i)
            nodes[i]->eventListen(pEvent, true);

        nodes[0]->eventListen(pEvent, false);

        if (pEvent->m_bBubbles)
        {
            for (unsigned int j = 1; j < nodes.size(); ++j)
                nodes[j]->eventListen(pEvent, false);
        }
    }

    delete pEvent;
}

void FJ_FuiNode::addEventListener(eFJEventType eType, FJ_FuiNode* pNode, void (*pfnHandler)(FJ_FuiNode*, FJ_Event*),
                                  bool bCapture)
{
    bool found = false;
    FJ_EventListener listener;
    listener.m_eType = eType;
    listener.m_bCapture = bCapture;
    listener.m_pNode = pNode;
    listener.m_pfnHandler = pfnHandler;

    if (eType == eFJEventType_EnterFrame)
    {
        gFui->addDirectEventListener(listener);
        return;
    }

    for (unsigned int i = 0; i < m_eventListeners.size(); ++i)
    {
        const FJ_EventListener& existing = m_eventListeners[i];
        if (existing.m_eType == listener.m_eType && existing.m_bCapture == listener.m_bCapture
            && existing.m_pNode == listener.m_pNode && existing.m_pfnHandler == listener.m_pfnHandler)
            found = true;
    }

    if (!found)
        m_eventListeners.push_back(listener);
}

void FJ_FuiNode::removeEventListener(eFJEventType eType, FJ_FuiNode* pNode, void (*pfnHandler)(FJ_FuiNode*, FJ_Event*))
{
    FJ_EventListener listener;
    listener.m_eType = eType;
    listener.m_bCapture = false;
    listener.m_pNode = pNode;
    listener.m_pfnHandler = pfnHandler;

    if (eType == eFJEventType_EnterFrame)
    {
        gFui->removeDirectEventListener(listener);
        return;
    }

    std::vector<FJ_EventListener>::iterator it = m_eventListeners.begin();
    while (it != m_eventListeners.end())
    {
        if (it->m_eType == listener.m_eType && it->m_pNode == listener.m_pNode
            && it->m_pfnHandler == listener.m_pfnHandler)
        {
            m_eventListeners.erase(it);
            return;
        }
        ++it;
    }
}

FJ_FuiNode* FJ_FuiNode::getRoot()
{
    return m_pRenderNode->getRoot()->getFuiNode();
}

unsigned int FJ_FuiNode::getType()
{
    return 0;
}

void FJ_FuiNode::setCallbackScene(UIScene* pScene)
{
    m_pCallbackScene = pScene;
}
