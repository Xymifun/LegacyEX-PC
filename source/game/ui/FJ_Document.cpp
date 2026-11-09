#include "game/ui/FJ_Document.hpp"

#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_Button.hpp"
#include "game/ui/FJ_FuiNodeStage.hpp"
#include "game/ui/FJ_KeyboardEvent.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/FJ_List2D.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/util/Math.hpp"

#if defined(RECON_NATIVE)
extern "C" void OSReport(const char*, ...);
#endif

FJ_Document::FJ_Document(fuiRenderNode* renderNode)
    : FJ_FuiNode(renderNode),
      m_safeZoneTop(0),
      m_safeZoneBottom(0),
      m_safeZoneLeft(0),
      m_safeZoneRight(0),
      m_root(this),
      m_focus(NULL),
      m_focusParentId(0),
      m_focusChildId(0),
      m_focusChange(true)
{
    addEventListener(eFJEventType_Init, this, _init, false);
    getStage()->addEventListener(eFJEventType_FocusChangeInit, this, _HandleOnFocusChangeInit, false);
    getStage()->addEventListener(eFJEventType_FocusChangeUser, this, _HandleOnFocusChangeUser, false);
}

int FJ_Document::getISafeZoneTop()
{
    return m_safeZoneTop;
}
int FJ_Document::getISafeZoneBottom()
{
    return m_safeZoneBottom;
}
int FJ_Document::getISafeZoneLeft()
{
    return m_safeZoneLeft;
}
int FJ_Document::getISafeZoneRight()
{
    return m_safeZoneRight;
}

void FJ_Document::_init(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Document*>(node)->init(event);
}

void FJ_Document::HandleOnFocusChangeInit(FJ_Event* event)
{
    OnFocusChange(false);
}

void FJ_Document::_HandleOnFocusChangeInit(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Document*>(node)->HandleOnFocusChangeInit(event);
}

void FJ_Document::HandleOnFocusChangeUser(FJ_Event* event)
{
    OnFocusChange(true);
}

void FJ_Document::_HandleOnFocusChangeUser(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Document*>(node)->HandleOnFocusChangeUser(event);
}

void FJ_Document::_handlePress(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Document*>(node)->handlePress(event);
}

void FJ_Document::_keyDownHandler(FJ_FuiNode* node, FJ_Event* event)
{
    static_cast<FJ_Document*>(node)->keyDownHandler(event);
}

void FJ_Document::init(FJ_Event* event)
{
    m_root->addEventListener(eFJEventType_Press, this, FJ_Document::_handlePress, false);
    getStage()->addEventListener(eFJEventType_KeyDown, this, FJ_Document::_keyDownHandler, false);
}

void FJ_Document::SetFocus(int id)
{
    if (id >= 0)
    {
        for (int i = 0; i < m_root->getNumChildren(); ++i)
        {
            FJ_FuiNode* child = m_root->getChildAt(i);
            if (child != NULL)
            {
                FJ_Base* base = child->asFJ_Base();
                if (base != NULL && base->getObjListParent() == NULL && base->getId() == id)
                {
                    getStage()->setFocus(m_root->getChildAt(i));
                    return;
                }
            }
        }
    }

    if (m_focusChange)
    {
        FJ_FuiNode* focus = NULL;
        for (int i = 0; i < m_root->getNumChildren(); ++i)
        {
            FJ_FuiNode* child = m_root->getChildAt(i);
            if (child != NULL && child->getTabIndex() == 1)
            {
                focus = child;
                break;
            }
        }
        if (focus == NULL && m_root->getNumChildren() > 0)
            focus = m_root->getChildAt(0);
        if (focus != NULL)
            getStage()->setFocus(focus);
        OnFocusChange(false);
    }
}

void FJ_Document::SetSafeZone(int top, int bottom, int left, int right)
{
    m_safeZoneTop = top;
    m_safeZoneBottom = bottom;
    m_safeZoneLeft = left;
    m_safeZoneRight = right;
}

void FJ_Document::handlePress(FJ_Event* event)
{
    FJ_Base* base = event->m_pTarget->asFJ_Base();
    FJ_Button* button = event->m_pTarget->asFJ_Button();

#if defined(RECON_NATIVE)
    if (base != NULL && m_pCallbackScene != NULL)
        OSReport("[native-ui] press scene=%d node=%s id=%u\n", m_pCallbackScene->getSceneType(), base->getName(),
                 base->getId());
#endif

    if (button == NULL || button->getObjListParent() == NULL)
    {
        m_pCallbackScene->handlePress(base->getId(), 0);
        if (button == NULL)
            return;
    }
    else
    {
        if (button->getObjListParent()->isFJ_List())
        {
            m_pCallbackScene->handlePress(static_cast<FJ_Base*>(button->getObjListParent())->getId(), button->getId());
            button->setButtonPressed(false);
            return;
        }
        if (button->getObjListParent()->isFJ_List2D())
        {
            m_pCallbackScene->handlePress(static_cast<FJ_Base*>(button->getObjListParent())->getId(), button->getId());
            button->setButtonPressed(false);
            return;
        }
    }
    button->setButtonPressed(false);
}

void FJ_Document::keyDownHandler(FJ_Event* event)
{
    if (!getStage()->getFocus()->isFJ_Base())
        return;

    FJ_FuiNode* previous = getStage()->getFocus();
    if (getStage()->getFocus() == NULL)
        return;
    if (getStage()->getFocus()->asFJ_Base() == NULL)
        return;

    if (static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode == 3)
    {
        if (getStage()->getFocus()->asFJ_Base()->m_navUpName.length() != 0
            && m_root->getChildByName(getStage()->getFocus()->asFJ_Base()->m_navUpName) != NULL)
        {
            getStage()->setFocus(m_root->getChildByName(getStage()->getFocus()->asFJ_Base()->m_navUpName));
            if (getStage()->getFocus()->asFJ_Base()->getObjListParent() != NULL && getStage()->getFocus()->isFJ_Label())
                keyDownHandler(event);
        }
    }
    else if (static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode == 4)
    {
        if (getStage()->getFocus()->asFJ_Base()->m_navDownName.length() != 0
            && m_root->getChildByName(getStage()->getFocus()->asFJ_Base()->m_navDownName) != NULL)
        {
            getStage()->setFocus(m_root->getChildByName(getStage()->getFocus()->asFJ_Base()->m_navDownName));
            if (getStage()->getFocus()->asFJ_Base()->getObjListParent() != NULL && getStage()->getFocus()->isFJ_Label())
                keyDownHandler(event);
        }
    }
    else if (static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode == 5)
    {
        if (getStage()->getFocus()->asFJ_Base()->m_navLeftName.length() != 0
            && m_root->getChildByName(getStage()->getFocus()->asFJ_Base()->m_navLeftName) != NULL)
        {
            getStage()->setFocus(m_root->getChildByName(getStage()->getFocus()->asFJ_Base()->m_navLeftName));
        }
    }
    else if (static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode == 6)
    {
        if (getStage()->getFocus()->asFJ_Base()->m_navRightName.length() != 0
            && m_root->getChildByName(getStage()->getFocus()->asFJ_Base()->m_navRightName) != NULL)
        {
            getStage()->setFocus(m_root->getChildByName(getStage()->getFocus()->asFJ_Base()->m_navRightName));
        }
    }
    else if (static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode == 1)
    {
        if (getStage()->getFocus()->asFJ_Base()->getObjListParent() != NULL)
        {
            if (getStage()->getFocus()->asFJ_Base()->getObjListParent()->isFJ_List())
                getStage()->getFocus()->asFJ_Base()->getObjListParent()->asFJ_List()->HandlePageUpDownNavigation(true);
            else if (getStage()->getFocus()->asFJ_Base()->getObjListParent()->isFJ_List2D())
                getStage()->getFocus()->asFJ_Base()->getObjListParent()->asFJ_List2D()->HandlePageUpDownNavigation(
                    true);
        }
    }
    else if (static_cast<FJ_KeyboardEvent*>(event)->m_iKeyCode == 2)
    {
        if (getStage()->getFocus()->asFJ_Base()->getObjListParent() != NULL)
        {
            if (getStage()->getFocus()->asFJ_Base()->getObjListParent()->isFJ_List())
                getStage()->getFocus()->asFJ_Base()->getObjListParent()->asFJ_List()->HandlePageUpDownNavigation(false);
            else if (getStage()->getFocus()->asFJ_Base()->getObjListParent()->isFJ_List2D())
                getStage()->getFocus()->asFJ_Base()->getObjListParent()->asFJ_List2D()->HandlePageUpDownNavigation(
                    false);
        }
    }

    if (previous->isFJ_Base())
        previous->asFJ_Base()->LostFocus();
    OnFocusChange(true);
}

void FJ_Document::OnFocusChange(bool focused)
{
    if (getStage()->getFocus() == NULL)
        return;

    const bool changed = m_focus != getStage()->getFocus();
    m_focus = getStage()->getFocus();

    FJ_Base* base = getStage()->getFocus()->asFJ_Base();
    UIScene* scene = m_pCallbackScene;
    if (scene == NULL || base == NULL)
        return;

#if defined(RECON_NATIVE)
    if (changed)
        OSReport("[native-ui] focus scene=%d node=%s id=%u tab=%d up=%s down=%s left=%s right=%s\n",
                 m_pCallbackScene->getSceneType(), base->getName(), base->getId(), base->getTabIndex(),
                 base->m_navUpName.c_str(), base->m_navDownName.c_str(), base->m_navLeftName.c_str(),
                 base->m_navRightName.c_str());
#endif

    if (base->getObjListParent() == NULL)
    {
        const int id = base->getId();
        if (changed || id != m_focusChildId)
        {
            m_focusChildId = id;
            m_focusParentId = 0;
            if (focused)
            {
                scene->_handleFocusChange(base->getId(), 0);
                return;
            }
            scene->_handleInitFocus(base->getId(), 0);
        }
    }
    else
    {
        if (base->getObjListParent()->isFJ_List())
        {
            const int parentId = base->getObjListParent()->asFJ_List()->getId();
            const int childId = base->getId();
            if (changed || parentId != m_focusParentId || childId != m_focusChildId)
            {
                m_focusChildId = childId;
                m_focusParentId = parentId;
                if (focused)
                {
                    scene->_handleFocusChange(parentId, childId);
                    return;
                }
                scene->_handleInitFocus(parentId, childId);
                return;
            }
        }
        else if (base->getObjListParent()->isFJ_List2D())
        {
            const int parentId = base->getObjListParent()->asFJ_List2D()->getId();
            const int childId = base->getId();
            if (changed || parentId != m_focusParentId || childId != m_focusChildId)
            {
                m_focusChildId = childId;
                m_focusParentId = parentId;
                if (focused)
                {
                    scene->_handleFocusChange(parentId, childId);
                    return;
                }
                scene->_handleInitFocus(parentId, childId);
                return;
            }
        }
    }
}

FJ_FuiNode* FJ_Document::GetTopObject(FJ_FuiNode* node)
{
    int y = static_cast<int>(node->getY());
    for (;;)
    {
        if (node->asFJ_Base()->m_navUpName.length() == 0)
            return node;
        FJ_FuiNode* candidate = m_root->getChildByName(node->asFJ_Base()->m_navUpName)->asMovieClip();
        if (node->asFJ_Base()->m_navUpName == candidate->asFJ_Base()->m_navUpName)
            return node;
        if (candidate == NULL)
            return node;
        if (y < static_cast<int>(candidate->getY()))
            break;
        y = static_cast<int>(candidate->getY());
        node = candidate;
    }
    return node;
}

void FJ_Document::RepositionItems(FJ_FuiNode* first, FJ_FuiNode* second, bool horizontal)
{
    int prevY = 0;
    int prevHeight = 0;
    int bottomA = 0;
    int bottomB = 0;
    int shiftedCount = 0;
    FJ_FuiNode* node = second;
    for (;;)
    {
        bool hasListParent = false;
        if (node->isFJ_Base() && node->asFJ_Base()->getObjListParent() != NULL)
            hasListParent = true;

        if (!node->isFJ_List() && !node->isFJ_List2D() && !hasListParent)
        {
            if (node != second)
            {
                const int curY = Math::round(node->getY());
                node->decY(Math::round(second->getHeight() + ((curY - prevY) - prevHeight)));
                bottomB = Math::round(node->getY() + node->getHeight());
                prevHeight = Math::round(node->getHeight());
                ++shiftedCount;
                prevY = curY;
            }
            else
            {
                prevHeight = Math::round(node->getHeight());
                prevY = Math::round(node->getY());
            }
        }
        node = FJ_FuiNode::asMovieClip(m_root->getChildByName(node->asFJ_Base()->m_navDownName));

        if (node == NULL || node == first)
            break;
        bottomA = Math::round(node->getY() + node->getHeight());
    }

    if (bottomA == 0 && bottomB == 0)
    {
        FJ_FuiNode* candidate = FJ_FuiNode::asMovieClip(m_root->getChildByName(second->asFJ_Base()->m_navUpName));
        if (candidate != NULL)
        {
            bottomA = Math::round(second->getY() + second->getHeight());
            bottomB = Math::round(candidate->getY() + candidate->getHeight());
        }
    }

    if (shiftedCount < 1 && horizontal)
        horizontal = false;

    if (horizontal)
    {
        const float amount = static_cast<float>(bottomA - bottomB) * 0.5f;
        for (int i = 0; i < m_root->getNumChildren(); ++i)
        {
            if (m_root->getChildAt(i)->isFJ_HtmlText())
                continue;
            if (m_root->getChildAt(i)->isFJ_Base() && m_root->getChildAt(i)->asFJ_Base()->getObjListParent() != NULL)
                continue;

            bool move = true;
            if (!m_root->getChildAt(i)->isFJ_Base()
                && std::string(m_root->getChildAt(i)->getName()).compare("BackgroundPanel") != 0
                && std::string(m_root->getChildAt(i)->getName()).compare("PlayerIcon") != 0)
                move = false;
            if (move)
                m_root->getChildAt(i)->incY(Math::round(amount));
        }
    }

    FJ_FuiNode* background = m_root->getChildByName(std::string("BackgroundPanel"));
    if (background != NULL)
    {
        FJ_FuiNode* fill = background->getChildAt(0)->asMovieClip();
        if (fill != NULL)
            fill->decHeight(static_cast<float>(bottomA - bottomB));
    }
}

void FJ_Document::FixNavigation(FJ_FuiNode* node)
{
    if (node->asFJ_Base()->m_navDownName.length() != 0)
    {
        FJ_FuiNode* candidate = m_root->getChildByName(node->asFJ_Base()->m_navDownName)->asMovieClip();
        if (candidate != NULL)
        {
            candidate->asFJ_Base()->m_navUpName = node->asFJ_Base()->m_navUpName;
            if (getStage()->getFocus() == node)
                getStage()->setFocus(candidate);
            if (node->getTabIndex() == 1)
            {
                node->setTabIndex(2);
                candidate->setTabIndex(1);
            }
        }
    }
    if (node->asFJ_Base()->m_navUpName.length() != 0)
    {
        FJ_FuiNode* candidate = m_root->getChildByName(node->asFJ_Base()->m_navUpName)->asMovieClip();
        if (candidate != NULL)
        {
            candidate->asFJ_Base()->m_navDownName = node->asFJ_Base()->m_navDownName;
            if (getStage()->getFocus() == node)
                getStage()->setFocus(candidate);
            if (node->getTabIndex() == 1)
                candidate->setTabIndex(1);
        }
    }
}

void FJ_Document::RemoveObject(const std::string& name, bool destroy)
{
    FJ_FuiNode* node = FJ_FuiNode::asMovieClip(m_root->getChildByName(name));
    if (node != NULL && node->isFJ_Base())
    {
        FJ_FuiNode* top = GetTopObject(node);
        RepositionItems(top, node, destroy);
        FixNavigation(node);
        node->getParent()->removeChild(node, true);
    }
}

void FJ_Document::DoHorizontalResizeCheck()
{
    bool resized = false;
    int left = 0;
    int widest = 0;
    int oldWidth = 0;
    FJ_FuiNode* background = m_root->getChildByName("BackgroundPanel");
    FJ_FuiNode* backgroundMovie = NULL;
    if (background != NULL)
        backgroundMovie = background->asMovieClip();

    for (int i = 0; i < m_root->getNumChildren(); ++i)
    {
        FJ_Base* base = m_root->getChildAt(i)->asFJ_Base();
        if (base != NULL && (!base->m_manualWrapFits || !base->m_multilineWrapFits))
        {
            int newWidth = base->getNewWidth();
            if (newWidth > widest)
            {
                widest = newWidth;
                oldWidth = base->getOldWidth();
                left = base->getX();
            }
            if (base->getLabelTooWide())
                resized = true;
        }
    }

    if (backgroundMovie != NULL && widest + (left - backgroundMovie->getX()) * 2 > backgroundMovie->getWidth())
    {
        oldWidth = backgroundMovie->getWidth() - (left - backgroundMovie->getX()) * 2;
        resized = true;
    }

    if (resized)
    {
        for (int i = 0; i < m_root->getNumChildren(); ++i)
        {
            FJ_Base* base = m_root->getChildAt(i)->asFJ_Base();
            if (base != NULL)
            {
                if (base->isFJ_Button() || base->isFJ_Slider() || base->isFJ_List() || base->isFJ_List2D())
                    base->SetNewObjectWidth(widest);
                FJ_FuiNode* parent = base->getObjListParent();
                if (parent != NULL)
                    base->setX(parent->getX());
                else
                    base->decX(Math::round((widest - oldWidth) * 0.5f));
            }
        }

        if (backgroundMovie != NULL)
        {
            backgroundMovie->getChildAt(0)->setWidth(Math::round(backgroundMovie->getWidth() + widest - oldWidth));
            backgroundMovie->decX(Math::round((widest - oldWidth) * 0.5f));
        }

        FJ_FuiNode* worldOptions = FJ_FuiNode::asMovieClip(m_root->getChildByName("WorldOptionsBox"));
        if (worldOptions != NULL)
        {
            worldOptions->incWidth(widest - oldWidth);
            worldOptions->decX(Math::round((widest - oldWidth) * 0.5f));
        }
    }

    for (int i = 0; i < m_root->getNumChildren(); ++i)
    {
        FJ_Base* base = m_root->getChildAt(i)->asFJ_Base();
        if (base != NULL)
            base->ResizingComplete();
    }
}

void FJ_Document::ResetHorizontalResizeCheck()
{
    for (int i = 0; i < m_root->getNumChildren(); ++i)
    {
        FJ_Base* base = m_root->getChildAt(i)->asFJ_Base();
        if (base != NULL)
            base->ResetResizing();
    }
}

void FJ_Document::SlideLeft()
{
    if (getChildByName(std::string("MainPanel")) != NULL)
        gotoAndPlay(std::string("Slide_Left"));
}

void FJ_Document::SlideRight()
{
    if (getChildByName(std::string("MainPanel")) != NULL)
        gotoAndPlay(std::string("Slide_Right"));
}
