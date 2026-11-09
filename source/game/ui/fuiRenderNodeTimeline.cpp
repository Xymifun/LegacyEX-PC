#include "game/ui/fuiRenderNodeTimeline.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>

#include "game/client/render/Renderer.hpp"
#include "game/ui/FJ_Base.hpp"
#include "game/ui/FJ_DynamicHtmlText.hpp"
#include "game/ui/FJ_Event.hpp"
#include "game/ui/FJ_Factory.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_HtmlText.hpp"
#include "game/ui/FJ_HtmlText_WithPanel.hpp"
#include "game/ui/FJ_Label.hpp"
#include "game/ui/FJ_Label_Blink.hpp"
#include "game/ui/FJ_Label_HUD_Splash.hpp"
#include "game/ui/FJ_Label_HUD_SplashBig.hpp"
#include "game/ui/FJ_Label_HUD_SplashVita.hpp"
#include "game/ui/FJ_LeaderboardList.hpp"
#include "game/ui/FJ_List.hpp"
#include "game/ui/FJ_List2D.hpp"
#include "game/ui/FJ_ListButton.hpp"
#include "game/ui/FJ_List_ListButton.hpp"
#include "game/ui/FJ_List_ListButtonIconLeft.hpp"
#include "game/ui/FJ_List_MenuButton.hpp"
#include "game/ui/FJ_SignInput.hpp"
#include "game/ui/FJ_SlotList.hpp"
#include "game/ui/FJ_TextInput.hpp"
#include "game/ui/FJ_TexturePackList.hpp"
#include "game/ui/fui.hpp"
#include "game/ui/fuiBitmap.hpp"
#include "game/ui/fuiEdittext.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiReference.hpp"
#include "game/ui/fuiRenderNodeBitmap.hpp"
#include "game/ui/fuiRenderNodeEditText.hpp"
#include "game/ui/fuiRenderNodeShape.hpp"
#include "game/ui/fuiRenderNodeStage.hpp"
#include "game/ui/fuiShape.hpp"
#include "game/ui/fuiSymbol.hpp"
#include "game/ui/fuiTimeline.hpp"
#include "game/ui/fuiTimelineAction.hpp"
#include "game/ui/fuiTimelineEvent.hpp"
#include "game/ui/fuiTimelineFrame.hpp"

extern Renderer* gRenderer;

fuiRenderNodeTimeline::frameAction::frameAction(const frameAction& other)
    : frame(other.frame),
      type(other.type),
      value(other.value)
{
}

fuiRenderNodeTimeline::fuiRenderNodeTimeline(fuiRenderNode* stage, fuiRenderNode* parent, fuiObject* object,
                                             unsigned int symbolIndex, unsigned char depth, fuiRGBA* color,
                                             unsigned char clipDepth, bool visible, fuiFile* file)
    : fuiRenderNode(stage, parent, object, symbolIndex, eFuiObjectType_Timeline, depth, color, clipDepth, visible, file)
{
    m_flags |= 4;
    m_currentFrame = -1;
    m_previousFrame = 0;
    m_mask = 0;
    if (object != 0)
    {
        fuiTimeline* timeline = static_cast<fuiTimeline*>(object);
        if (timeline->bounds.xMin != 0.0f || timeline->bounds.xMax != 0.0f || timeline->bounds.yMin != 0.0f
            || timeline->bounds.yMax != 0.0f)
            m_flags |= 0x20;
    }
}

fuiRenderNodeTimeline::~fuiRenderNodeTimeline()
{
    for (unsigned int i = 0; i < m_children.size(); ++i)
        delete m_children[i];
    delete m_mask;
}

void fuiRenderNodeTimeline::setPlaying(bool playing)
{
    if (playing)
    {
        m_flags |= 6;
        return;
    }
    m_flags &= ~4u;
    if ((m_flags & 0x10) == 0)
        m_previousFrame = m_currentFrame;
}

void fuiRenderNodeTimeline::setFrame(unsigned int frame)
{
    unsigned int last = static_cast<fuiTimeline*>(m_object)->timelineFrameCount - 1;
    m_flags |= 0x12;
    m_previousFrame = std::min(frame - 1, last);
}

void fuiRenderNodeTimeline::addFrameAction(int frame, eFrameActionType type, const std::string& value)
{
    frameAction action;
    action.frame = frame - 1;
    action.type = type;
    action.value = value;
    m_frameActions.push_back(action);
}

void fuiRenderNodeTimeline::expandBoundsFromChild(fuiRect& bounds)
{
    m_bounds.xMin = m_bounds.xMin > bounds.xMin ? bounds.xMin : m_bounds.xMin;
    m_bounds.xMax = m_bounds.xMax < bounds.xMax ? bounds.xMax : m_bounds.xMax;
    m_bounds.yMin = m_bounds.yMin > bounds.yMin ? bounds.yMin : m_bounds.yMin;
    m_bounds.yMax = m_bounds.yMax < bounds.yMax ? bounds.yMax : m_bounds.yMax;
}

void fuiRenderNodeTimeline::updateBoundsLocal()
{
    for (unsigned int i = 0; i < m_children.size(); ++i)
    {
        fuiRect bounds;
        m_children[i]->getParentSpaceBounds(&bounds);
        if (i == 0)
            m_bounds = bounds;
        else
            expandBoundsFromChild(bounds);
    }
}

void fuiRenderNodeTimeline::progogateBoundsFromChild()
{
    updateBoundsLocal();
    if (m_parent != 0)
    {
        fuiRenderNode* timeline = m_parent->asTimeline();
        if (timeline != 0)
            static_cast<fuiRenderNodeTimeline*>(timeline)->progogateBoundsFromChild();
    }
}

void fuiRenderNodeTimeline::shutdown()
{
    for (unsigned int i = 0; i < m_children.size(); ++i)
        m_children[i]->shutdown();
}

fuiObject* fuiRenderNodeTimeline::getReferencedObject(fuiObject* object)
{
    fuiReference* reference = static_cast<fuiReference*>(object);
    fui* gui = gFui;
    for (;;)
    {
        fuiFile* file = reinterpret_cast<fuiFile*>(gui->getFuiFileFromIndex(reference->index));
        fuiSymbol* symbol;
        do
        {
            while (static_cast<int>(reference->symbolIndex) == -1)
            {
            }
            symbol = &file->symbolData[reference->symbolIndex];
            switch (symbol->objectType)
            {
            case eFuiObjectType_Shape:
                return &file->shapeData[symbol->objectIndex];
            case eFuiObjectType_Timeline:
                return &file->timelineData[symbol->objectIndex];
            case eFuiObjectType_Bitmap:
                return &file->bitmaps[symbol->objectIndex];
            case eFuiObjectType_EditText:
                return &file->editTextData[symbol->objectIndex];
            default:
                break;
            }
        } while (symbol->objectType != eFuiObjectType_Reference);
        reference = &file->referenceData[symbol->objectIndex];
    }
}

fuiRenderNode* fuiRenderNodeTimeline::createReferencedNode(fuiObject* object, unsigned int depth,
                                                           unsigned char renderFlags, fuiRGBA* color,
                                                           unsigned char clipDepth, bool visible)
{
    for (;;)
    {
        fuiReference* reference = static_cast<fuiReference*>(object);
        int symbolIndex = reference->symbolIndex;
        fuiFile* file;
        fuiSymbol* symbol;
        do
        {
            if (symbolIndex == -1)
                return 0;
            file = reinterpret_cast<fuiFile*>(gFui->getFuiFileFromIndex(reference->index));
            symbol = &file->symbolData[symbolIndex];
            switch (symbol->objectType)
            {
            case eFuiObjectType_Shape:
                return new fuiRenderNodeShape(m_stage, this, &file->shapeData[symbol->objectIndex], depth, renderFlags,
                                              color, clipDepth, visible, file);
            case eFuiObjectType_Timeline:
                return new fuiRenderNodeTimeline(m_stage, this, &file->timelineData[symbol->objectIndex], depth,
                                                 renderFlags, color, clipDepth, visible, file);
            case eFuiObjectType_Bitmap:
                return new fuiRenderNodeBitmap(m_stage, this, &file->bitmaps[symbol->objectIndex], depth, renderFlags,
                                               color, clipDepth, visible, file);
            case eFuiObjectType_EditText:
                return new fuiRenderNodeEditText(m_stage, this, &file->editTextData[symbol->objectIndex], depth,
                                                 renderFlags, color, clipDepth, visible, file);
            default:
                break;
            }
        } while (symbol->objectType != eFuiObjectType_Reference);
        object = &file->referenceData[symbol->objectIndex];
    }
}

bool fuiRenderNodeTimeline::depthComparison(const fuiRenderNode* first, const fuiRenderNode* second)
{
    return first->m_symbolIndex < second->m_symbolIndex;
}

void fuiRenderNodeTimeline::depthSort()
{
    std::sort(m_children.begin(), m_children.end(), depthComparison);
}

void fuiRenderNodeTimeline::handleConstruction()
{
    FJ_FuiNode* created;
    if (static_cast<int>(m_object->symbolIndex) != -1)
    {
        created = FJ_Factory::createObject(m_file->symbolData[static_cast<int>(m_object->symbolIndex)].name, this,
                                           false, 0);
    }
    else
    {
        created = FJ_Factory::createObject("", this, false, 0);
    }

    fuiTimeline* timeline = static_cast<fuiTimeline*>(m_object);
    fuiTimelineAction* action = m_file->timelineActionData + timeline->timelineActionIndex;
    m_node = created;

    for (unsigned int index = 0; index < timeline->timelineActionCount; ++index, ++action)
    {
        switch (action->actionType)
        {
        case 0x1e:
            addFrameAction(action->frameIndex, eFrameActionType_Stop, "");
            continue;
        case 0x1f:
            addFrameAction(action->frameIndex, eFrameActionType_GotoAndStopLabel, action->actionValue);
            continue;
        case 0x20:
            addFrameAction(action->frameIndex, eFrameActionType_GotoAndPlay, action->actionValue);
            continue;
        case 0x21:
            addFrameAction(action->frameIndex, eFrameActionType_GotoAndStop, action->actionValue);
            continue;
        default:
            break;
        }

        fuiRenderNode* target = findNode(action->targetName);
        if (target != 0)
        {
            FJ_FuiNode* node = target->getFuiNode();
            if (node != 0)
            {
                switch (action->actionType)
                {
                case 0:
                {
                    FJ_Base* base = node->asFJ_Base();
                    if (base != 0)
                        base->m_navUpName = action->actionValue;
                    break;
                }
                case 1:
                {
                    FJ_Base* base = node->asFJ_Base();
                    if (base != 0)
                        base->m_navDownName = action->actionValue;
                    break;
                }
                case 2:
                {
                    FJ_Base* base = node->asFJ_Base();
                    if (base != 0)
                        base->m_navLeftName = action->actionValue;
                    break;
                }
                case 3:
                {
                    FJ_Base* base = node->asFJ_Base();
                    if (base != 0)
                        base->m_navRightName = action->actionValue;
                    break;
                }
                case 4:
                {
                    FJ_Base* base = node->asFJ_Base();
                    if (base != 0)
                        base->setTabIndex(std::atoi(action->actionValue));
                    break;
                }
                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                {
                    FJ_Label* label = node->asFJ_Label();
                    if (label == 0)
                    {
                        FJ_List_MenuButton* menuButton = node->asFJ_List_MenuButton();
                        if (menuButton == 0)
                        {
                            FJ_Label_Blink* blink = node->asFJ_Label_Blink();
                            if (blink != 0)
                                blink->setAction(action->actionType, action->actionValue);
                        }
                        else
                        {
                            menuButton->setAction(action->actionType, action->actionValue);
                        }
                    }
                    else
                    {
                        label->setAction(action->actionType, action->actionValue);
                    }
                    break;
                }
                case 10:
                case 0xb:
                case 0xc:
                case 0xd:
                case 0x1d:
                {
                    FJ_List* list = node->asFJ_List();
                    if (list != 0)
                        list->setAction(action->actionType, action->actionValue);
                    FJ_List2D* list2d = node->asFJ_List2D();
                    if (list2d != 0)
                        list2d->setAction(action->actionType, action->actionValue);
                    break;
                }
                case 0xe:
                {
                    if (node->getType() == 0x8009)
                    {
                        FJ_TexturePackList* texturePackList = static_cast<FJ_TexturePackList*>(node);
                        texturePackList->m_disabledAlpha = static_cast<float>(std::atof(action->actionValue));
                    }
                    break;
                }
                case 0xf:
                {
                    if (node->getType() == 0x8032)
                    {
                        FJ_List_ListButtonIconLeft* iconLeft = static_cast<FJ_List_ListButtonIconLeft*>(node);
                        iconLeft->m_useLayerIcon = std::strcmp(action->actionValue, "true") == 0;
                    }
                    break;
                }
                case 0x10:
                {
                    if (node->getType() == 0x5e)
                    {
                        FJ_SlotList* slotList = static_cast<FJ_SlotList*>(node);
                        slotList->m_slotSize = std::atoi(action->actionValue);
                    }
                    break;
                }
                case 0x11:
                {
                    if (node->getType() == 0x5e)
                    {
                        FJ_SlotList* slotList = static_cast<FJ_SlotList*>(node);
                        slotList->m_slotVisible = std::strcmp(action->actionValue, "true") == 0;
                    }
                    break;
                }
                case 0x12:
                {
                    if (node->getType() == 0x8013 || node->getType() == 0x8014 || node->getType() == 0x8015)
                    {
                        FJ_ListButton* listButton = static_cast<FJ_ListButton*>(node);
                        listButton->m_listButtonFlag = std::strcmp(action->actionValue, "true") == 0;
                    }
                    break;
                }
                case 0x13:
                {
                    if (node->isFJ_HtmlText())
                    {
                        FJ_HtmlText* html = node->asFJ_HtmlText();
                        html->m_bUseScrollRect = std::strcmp(action->actionValue, "true") == 0;
                    }
                    break;
                }
                case 0x14:
                {
                    if (node->isFJ_HtmlText())
                    {
                        FJ_HtmlText* html = node->asFJ_HtmlText();
                        html->m_bNoBackground = std::strcmp(action->actionValue, "true") == 0;
                    }
                    break;
                }
                case 0x15:
                {
                    if (node->isFJ_DynamicHtmlText())
                    {
                        FJ_DynamicHtmlText* dynamicText = node->asFJ_DynamicHtmlText();
                        dynamicText->m_iTopMargin = std::atoi(action->actionValue);
                    }
                    if (node->isFJ_HtmlText_WithPanel())
                    {
                        FJ_HtmlText_WithPanel* htmlTextWithPanel = node->asFJ_HtmlText_WithPanel();
                        htmlTextWithPanel->m_panelOffset = std::atoi(action->actionValue);
                    }
                    break;
                }
                case 0x16:
                {
                    if (node->isFJ_TextInput())
                    {
                        FJ_TextInput* textInput = node->asFJ_TextInput();
                        textInput->SetCharLimit(std::atoi(action->actionValue));
                    }
                    if (node->isFJ_SignInput())
                    {
                        FJ_SignInput* signInput = node->asFJ_SignInput();
                        signInput->m_maxLength = std::atoi(action->actionValue);
                    }
                    break;
                }
                case 0x17:
                {
                    if (node->isFJ_TextInput())
                    {
                        FJ_TextInput* textInput = node->asFJ_TextInput();
                        textInput->m_scrollWidth = std::atoi(action->actionValue);
                    }
                    if (node->isFJ_DynamicHtmlText())
                    {
                        FJ_DynamicHtmlText* dynamicText = node->asFJ_DynamicHtmlText();
                        dynamicText->m_iCharacterHeight = std::atoi(action->actionValue);
                    }
                    break;
                }
                case 0x18:
                {
                    if (node->asFJ_Label_HUD_Splash() != 0)
                        node->asFJ_Label_HUD_Splash()->m_animated = std::strcmp(action->actionValue, "true") == 0;
                    if (node->asFJ_Label_HUD_SplashBig() != 0)
                        node->asFJ_Label_HUD_SplashBig()->m_animated = std::strcmp(action->actionValue, "true") == 0;
                    if (node->asFJ_Label_HUD_SplashVita() != 0)
                        node->asFJ_Label_HUD_SplashVita()->m_animated = std::strcmp(action->actionValue, "true") == 0;
                    break;
                }
                case 0x19:
                {
                    if (node->isFJ_LeaderboardList())
                    {
                        FJ_LeaderboardList* leaderboardList = node->asFJ_LeaderboardList();
                        leaderboardList->m_small = std::strcmp(action->actionValue, "true") == 0;
                    }
                    break;
                }
                case 0x1a:
                {
                    if (node->isFJ_LeaderboardList())
                    {
                        FJ_LeaderboardList* leaderboardList = node->asFJ_LeaderboardList();
                        leaderboardList->m_visibleEntries = std::atoi(action->actionValue);
                    }
                    break;
                }
                case 0x1b:
                {
                    if (node->isFJ_ListButton())
                    {
                        FJ_ListButton* listButton = node->asFJ_ListButton();
                        listButton->m_labelAlignment = std::atoi(action->actionValue);
                    }
                    break;
                }
                case 0x1c:
                {
                    if (node->isFJ_List_ListButton())
                    {
                        FJ_List_ListButton* listListButton = node->asFJ_List_ListButton();
                        listListButton->m_labelAlignment = std::atoi(action->actionValue);
                    }
                    break;
                }
                default:
                    break;
                }
            }
        }
    }
}

int fuiRenderNodeTimeline::getFrame(const char* name)
{
    fuiTimeline* timeline = static_cast<fuiTimeline*>(m_object);
    fuiTimelineFrame* frame = m_file->frameData + timeline->timelineFrameIndex;
    for (unsigned int i = 0; i < timeline->timelineFrameCount; ++i, ++frame)
        if (std::strcmp(frame->name, name) == 0)
            return static_cast<int>(i + 1);
    return 0;
}

void fuiRenderNodeTimeline::setFrame(const char* name)
{
    int frame = getFrame(name);
    if (frame != 0)
        setFrame(static_cast<unsigned int>(frame));
}

void fuiRenderNodeTimeline::handleFrameTransition(int frame)
{
    for (unsigned int i = 0; i < m_frameActions.size(); ++i)
    {
        if (m_frameActions[i].frame == frame)
        {
            switch (m_frameActions[i].type)
            {
            case eFrameActionType_GotoAndStop:
                if (m_frameActions[i].value == "handleAnimationStep")
                    m_node->dispatchEvent(new FJ_Event(eFJEventType_AnimationStep, false, false));
                else if (m_frameActions[i].value == "UpdateLabel")
                    m_node->dispatchEvent(new FJ_Event(eFJEventType_UpdateLabel, false, false));
                else if (m_frameActions[i].value == "InitHud")
                    m_node->dispatchEvent(new FJ_Event(eFJEventType_InitHud, false, false));
                else if (m_frameActions[i].value == "AnimationEnd")
                    m_node->dispatchEvent(new FJ_Event(eFJEventType_AnimationEnd, false, false));
                else if (m_frameActions[i].value == "ShowTimerAnimation")
                    m_node->dispatchEvent(new FJ_Event(eFJEventType_ShowTimerAnimation, false, false));
                else if (m_frameActions[i].value == "SlideComplete")
                    m_node->dispatchEvent(new FJ_Event(eFJEventType_SlideComplete, false, false));
                else if (m_frameActions[i].value == "ShowAnimatedLogoText")
                    m_node->dispatchEvent(new FJ_Event(eFJEventType_ShowAnimatedLogoText, true, false));
                break;
            case eFrameActionType_Stop:
                setPlaying(false);
                break;
            case eFrameActionType_GotoAndStopLabel:
                setFrame(m_frameActions[i].value.c_str());
                setPlaying(false);
                break;
            case eFrameActionType_GotoAndPlay:
                setFrame(m_frameActions[i].value.c_str());
                setPlaying(true);
                break;
            }
        }
    }
}

void fuiRenderNodeTimeline::gather(bool force)
{
    unsigned int flags = m_flags;
    int enteredOnFrame = m_currentFrame;
    bool replayFrame = (flags & 0x10) != 0 && (flags & 4) != 0;

    if (m_object != 0 && (flags & 2) != 0)
    {
        fuiTimeline* timeline = static_cast<fuiTimeline*>(m_object);
        int frame = enteredOnFrame + 1;
        std::vector<fuiRenderNode*> removed;

        if (m_previousFrame < m_currentFrame)
        {
            removed = m_children;
            m_children.clear();
            std::vector<fuiRenderNode*>::iterator it = removed.begin();
            while (it != removed.end())
            {
                if (((*it)->m_flags & 0x80) != 0)
                {
                    m_children.push_back(*it);
                    it = removed.erase(it);
                }
                else
                {
                    ++it;
                }
            }
            frame = 0;
        }

        m_currentFrame = m_previousFrame;
        m_flags &= ~0x10u;

        for (; frame <= m_currentFrame; ++frame)
        {
            fuiTimelineFrame* timelineFrame = m_file->frameData + timeline->timelineFrameIndex + frame;
            fuiTimelineEvent* event = m_file->eventData + timelineFrame->timelineEventIndex;
            for (unsigned int e = 0; e < timelineFrame->timelineEventCount; ++e, ++event)
            {
                fuiRenderNode* node = 0;
                unsigned int index;

                if ((event->flags & 1) != 0)
                {
                    bool alreadyRemoved = false;
                    for (unsigned int r = 0; r < m_removedChildren.size(); ++r)
                    {
                        fuiObject* object = 0;
                        switch (event->objectType)
                        {
                        case eFuiObjectType_Shape:
                            object = &m_file->shapeData[event->objectIndex];
                            break;
                        case eFuiObjectType_Timeline:
                            object = &m_file->timelineData[event->objectIndex];
                            break;
                        case eFuiObjectType_Bitmap:
                            object = &m_file->bitmaps[event->objectIndex];
                            break;
                        case eFuiObjectType_Reference:
                            object = getReferencedObject(&m_file->referenceData[event->objectIndex]);
                            break;
                        case eFuiObjectType_EditText:
                            object = &m_file->editTextData[event->objectIndex];
                            break;
                        default:
                            break;
                        }
                        if (m_removedChildren[r].depth == event->depth && m_removedChildren[r].object == object)
                            alreadyRemoved = true;
                    }

                    if (alreadyRemoved)
                        continue;

                    unsigned char renderFlags = event->renderFlags;
                    bool visible = ((renderFlags >> 2) & 1) != 0;
                    switch (event->objectType)
                    {
                    case eFuiObjectType_Shape:
                        node = new fuiRenderNodeShape(m_stage, this, &m_file->shapeData[event->objectIndex],
                                                      event->depth, renderFlags, &event->color, event->blendMode,
                                                      visible, m_file);
                        break;
                    case eFuiObjectType_Timeline:
                        node = new fuiRenderNodeTimeline(m_stage, this, &m_file->timelineData[event->objectIndex],
                                                         event->depth, renderFlags, &event->color, event->blendMode,
                                                         visible, m_file);
                        break;
                    case eFuiObjectType_Bitmap:
                        node = new fuiRenderNodeBitmap(m_stage, this, &m_file->bitmaps[event->objectIndex],
                                                       event->depth, renderFlags, &event->color, event->blendMode,
                                                       visible, m_file);
                        break;
                    case eFuiObjectType_Reference:
                        node = createReferencedNode(&m_file->referenceData[event->objectIndex], event->depth,
                                                    renderFlags, &event->color, event->blendMode, visible);
                        break;
                    case eFuiObjectType_EditText:
                        node = new fuiRenderNodeEditText(m_stage, this, &m_file->editTextData[event->objectIndex],
                                                         event->depth, renderFlags, &event->color, event->blendMode,
                                                         visible, m_file);
                        break;
                    default:
                        continue;
                    }

                    if (node != 0)
                    {
                        if (event->nameIndex == -1)
                        {
                            node->m_name[0] = '\0';
                        }
                        else
                        {
                            const char* eventNames = reinterpret_cast<const char*>(m_file->eventNameData);
                            std::strncpy(node->m_name, eventNames + event->nameIndex * 0x40, 0x40);
                        }
                        node->m_frame = event->clipDepth;

                        index = m_children.size();
                        for (unsigned int c = 0; c < m_children.size(); ++c)
                        {
                            if (m_children[c]->m_symbolIndex == event->depth)
                            {
                                delete m_children[c];
                                m_children[c] = node;
                                index = c;
                                break;
                            }
                        }
                        if (index == m_children.size())
                            m_children.push_back(node);
                    }
                }
                else
                {
                    for (index = 0; index < m_children.size(); ++index)
                    {
                        if (m_children[index]->m_symbolIndex == event->depth)
                        {
                            node = m_children[index];
                            break;
                        }
                    }
                }

                if (node == 0)
                    continue;

                if ((event->flags & 2) != 0)
                {
                    std::vector<fuiRenderNode*>::iterator removedChild = m_children.begin() + index;
                    delete *removedChild;
                    m_children.erase(removedChild);
                    continue;
                }
                if ((event->flags & 4) != 0)
                {
                    node->m_localMatrix = event->matrix;
                    node->generateGlobalMatrix();
                }
                if ((event->flags & 8) != 0)
                {
                    node->m_redAdd = event->colorTransform.redMultiply;
                    node->m_greenAdd = event->colorTransform.greenMultiply;
                    node->m_blueAdd = event->colorTransform.blueMultiply;
                    node->m_alphaAdd = event->colorTransform.alphaMultiply;
                    node->m_redMultiply = event->colorTransform.redAdd;
                    node->m_greenMultiply = event->colorTransform.greenAdd;
                    node->m_blueMultiply = event->colorTransform.blueAdd;
                    node->m_alpha = event->colorTransform.alphaAdd;
                }
            }
        }

        depthSort();

        if (force && (m_flags & 4) != 0)
            m_previousFrame = (m_currentFrame + 1) % static_cast<int>(timeline->timelineFrameCount);

        if (!removed.empty())
        {
            unsigned int survivor = 0;
            while (survivor < removed.size())
            {
                bool reused = false;
                for (unsigned int c = 0; c < m_children.size(); ++c)
                {
                    if (m_children[c]->m_symbolIndex != removed[survivor]->m_symbolIndex)
                        continue;
                    if (m_children[c]->m_object != removed[survivor]->m_object)
                        continue;
                    if (std::strcmp(m_children[c]->m_name, removed[survivor]->m_name) != 0)
                        continue;

                    fuiRenderNode* node = removed[survivor];
                    fuiRenderNode* created = m_children[c];
                    m_children[c] = node;
                    unsigned char filterFlags = node->m_filterFlags;
                    if ((filterFlags & 0x80) != 0)
                    {
                        m_children[c]->m_localMatrix.translateX = created->m_localMatrix.translateX;
                        m_children[c]->m_localMatrix.translateY = created->m_localMatrix.translateY;
                    }
                    if ((filterFlags & 0x20) != 0)
                    {
                        m_children[c]->m_localMatrix.scaleX = created->m_localMatrix.scaleX;
                        m_children[c]->m_localMatrix.scaleY = created->m_localMatrix.scaleY;
                    }
                    if ((m_children[c]->m_filterFlags & 0x40) != 0)
                    {
                        m_children[c]->m_localMatrix.rotateSkew0 = created->m_localMatrix.rotateSkew0;
                        m_children[c]->m_localMatrix.rotateSkew1 = created->m_localMatrix.rotateSkew1;
                    }
                    m_children[c]->m_globalMatrix = created->m_globalMatrix;
                    if ((m_children[c]->m_filterFlags & 0x10) != 0)
                    {
                        m_children[c]->m_redAdd = created->m_redAdd;
                        m_children[c]->m_greenAdd = created->m_greenAdd;
                        m_children[c]->m_blueAdd = created->m_blueAdd;
                        m_children[c]->m_alphaAdd = created->m_alphaAdd;
                        m_children[c]->m_redMultiply = created->m_redMultiply;
                        m_children[c]->m_greenMultiply = created->m_greenMultiply;
                        m_children[c]->m_blueMultiply = created->m_blueMultiply;
                        m_children[c]->m_alpha = created->m_alpha;
                    }

                    removed[survivor] = removed[removed.size() - 1];
                    removed.pop_back();
                    delete created;
                    reused = true;
                    break;
                }
                if (!reused)
                    ++survivor;
            }

            for (unsigned int d = 0; d < removed.size(); ++d)
                delete removed[d];
        }
    }

    for (unsigned int i = 0; i < m_children.size(); ++i)
    {
        m_children[i]->gather(force);
        fuiRect bounds;
        m_children[i]->getParentSpaceBounds(&bounds);
        if (i == 0)
            m_bounds = bounds;
        else
            expandBoundsFromChild(bounds);
    }

    flags = m_flags;
    if ((flags & 1) != 0)
    {
        handleConstruction();
        flags = m_flags & ~1u;
        m_flags = flags;
    }
    if ((flags & 0x40) != 0 && m_node != 0)
    {
        m_node->ASConstructor();
        flags = m_flags & ~0x40u;
        m_flags = flags;
    }
    if (m_currentFrame != enteredOnFrame || replayFrame)
    {
        handleFrameTransition(m_currentFrame);
        flags = m_flags;
    }
    if ((flags & 4) == 0 && m_previousFrame == m_currentFrame)
        m_flags = flags & ~2u;
}

void fuiRenderNodeTimeline::preRender()
{
    if (!isVisible())
        return;
    for (unsigned int i = 0; i < m_children.size(); ++i)
        m_children[i]->preRender();
}

void fuiRenderNodeTimeline::render(fuiRect* bounds, fuiMatrix* matrix)
{
    if (!isVisible())
        return;

    if (std::strncmp(m_name, "iggy_", 5) == 0 && getRoot()->m_file->customDrawCallback != 0)
    {
        gRenderer->StateSetFaceCull(true);
        gRenderer->StateSetFaceCullCW(true);
        gRenderer->StateSetAlphaTestEnable(true);
        gRenderer->StateSetDepthTestEnable(true);
        gRenderer->StateSetDepthMask(true);
        gRenderer->StateSetColour(1.0f, 1.0f, 1.0f, 1.0f);
        pushMatrix();
        fuiFile* file = getRoot()->m_file;
        file->customDrawCallback(file->customDrawContext, m_name + 5, &m_bounds);
        popMatrix();
        gRenderer->StateSetFaceCull(false);
        gRenderer->StateSetBlendEnable(true);
        gRenderer->StateSetBlendFunc(1, 5);
        gRenderer->StateSetAlphaTestEnable(false);
        gRenderer->StateSetDepthTestEnable(false);
        gRenderer->StateSetDepthMask(false);
        return;
    }

    pushMatrix();
    pushColor();
    pushFilterFlags();
    setScrollRectViewport();

    fuiRect* childBounds = 0;
    if ((m_flags & 0x20) != 0)
    {
        matrix = &m_localMatrix;
        childBounds = &static_cast<fuiTimeline*>(m_object)->bounds;
    }

    if (m_mask != 0)
    {
        gFui->incrementStencilMask();
        gRenderer->StateSetStencil(7, gFui->getCurrentStencilMask(), 0, 255);
        m_mask->render(childBounds, matrix);
        gRenderer->StateSetStencil(2, gFui->getCurrentStencilMask(), 255, 0);
    }

    if ((m_flags & 0x100) != 0)
        gRenderer->StateSetBlendFunc(2, 5);

    unsigned int clipDepth = 0;
    for (unsigned int i = 0; i < m_children.size(); ++i)
    {
        if (m_children[i]->m_frame != 0)
        {
            gFui->incrementStencilMask();
            gRenderer->StateSetStencil(7, gFui->getCurrentStencilMask(), 0, 255);
            gRenderer->StateSetBlendFunc(0, 1);
            m_children[i]->render(childBounds, matrix);
            gRenderer->StateSetStencil(2, gFui->getCurrentStencilMask(), 255, 0);
            gRenderer->StateSetBlendFunc(1, 5);
            clipDepth = m_children[i]->m_frame;
        }
        else
        {
            m_children[i]->render(childBounds, matrix);
        }
        if (clipDepth != 0 && m_children[i]->m_symbolIndex > clipDepth)
        {
            gRenderer->StateSetStencil(7, 0, 0, 0);
            clipDepth = 0;
        }
    }

    if ((m_flags & 0x100) != 0)
        gRenderer->StateSetBlendFunc(1, 5);

    if (m_mask != 0 || clipDepth != 0)
        gRenderer->StateSetStencil(7, 0, 0, 0);

    clearScrollRectViewport();
    popFilterFlags();
    popColor();
    popMatrix();
}

void fuiRenderNodeTimeline::handleAddedToStage()
{
    fuiRenderNode::handleAddedToStage();
    for (unsigned int i = 0; i < m_children.size(); ++i)
        m_children[i]->handleAddedToStage();
}

fuiRenderNode* fuiRenderNodeTimeline::findNode(const char* name)
{
    for (unsigned int i = 0; i < m_children.size(); ++i)
    {
        fuiRenderNode* result = m_children[i]->getNodeFromPath(name);
        if (result != 0)
            return result;
    }
    return 0;
}

fuiRenderNode* fuiRenderNodeTimeline::getNodeFromPath(const char* path)
{
    const char* separator = path;
    while (*separator != 0 && *separator != '.')
        ++separator;
    if (*separator == 0)
        return fuiRenderNode::getNodeFromPath(path);
    if (std::strncmp(m_name, path, separator - path) == 0)
    {
        for (unsigned int i = 0; i < m_children.size(); ++i)
        {
            fuiRenderNode* result = m_children[i]->getNodeFromPath(separator + 1);
            if (result != 0)
                return result;
        }
    }
    return 0;
}

std::string fuiRenderNodeTimeline::currentLabel()
{
    fuiTimeline* timeline = static_cast<fuiTimeline*>(m_object);
    const char* name = m_file->frameData[timeline->timelineFrameIndex + m_currentFrame].name;
    return std::string(name);
}

int fuiRenderNodeTimeline::currentFrame()
{
    return m_currentFrame + 1;
}

void fuiRenderNodeTimeline::removeChild(fuiRenderNode* child, bool destroy)
{
    for (std::vector<fuiRenderNode*>::iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        if (*it == child)
        {
            m_children.erase(it);
            m_removedChildren.push_back(fuiRenderNodeRemoved(child->m_symbolIndex, child->m_object));
            if (destroy)
                delete child;
            else
                child->m_parent = 0;
            return;
        }
    }
}

void fuiRenderNodeTimeline::removeChildAt(int index, bool destroy)
{
    std::vector<fuiRenderNode*>::iterator it = m_children.begin() + index;
    fuiRenderNode* child = *it;
    m_children.erase(it);
    m_removedChildren.push_back(fuiRenderNodeRemoved(child->m_symbolIndex, child->m_object));
    if (destroy)
        delete child;
    else
        child->m_parent = 0;
}

void fuiRenderNodeTimeline::addMask(fuiRenderNode* mask)
{
    m_mask = mask;
}

void fuiRenderNodeTimeline::addChild(fuiRenderNode* child)
{
    unsigned int depth = 0;
    for (std::vector<fuiRenderNode*>::iterator it = m_children.begin(); it != m_children.end(); ++it)
        if ((*it)->m_symbolIndex > depth)
            depth = (*it)->m_symbolIndex;

    fuiRenderNode* node = child;
    if (child->m_parent != 0)
        static_cast<fuiRenderNodeTimeline*>(child->m_parent->asTimeline())->removeChild(child, false);

    node->m_parent = this;
    node->m_symbolIndex = depth + 1;
    node->m_stage = m_stage;
    m_children.push_back(node);
    node->generateGlobalMatrix();
    node->m_flags |= 0x80;
    depthSort();
}

void fuiRenderNodeTimeline::addChildAt(fuiRenderNode* child, int index)
{
    m_children.insert(m_children.begin() + index, child);

    int previousDepth = 0;
    int nextDepth = 0x7fffffff;
    if (index > 0)
        previousDepth = static_cast<int>(m_children[index - 1]->m_symbolIndex);
    if (static_cast<unsigned int>(index) < m_children.size() - 1)
        nextDepth = static_cast<int>(m_children[index + 1]->m_symbolIndex);
    int depth = previousDepth + 1;
    if (depth > nextDepth)
        depth = nextDepth;

    child->m_parent = this;
    child->m_stage = m_stage;
    child->m_symbolIndex = static_cast<unsigned int>(depth);
    child->generateGlobalMatrix();
    child->m_flags |= 0x80;
    depthSort();
}

unsigned int fuiRenderNodeTimeline::getChildIndex(fuiRenderNode* child)
{
    for (unsigned int i = 0; i < m_children.size(); ++i)
        if (m_children[i] == child)
            return i;
    return 0xffffffffu;
}

void fuiRenderNodeTimeline::generateGlobalMatrix()
{
    fuiRenderNode::generateGlobalMatrix();
    for (unsigned int i = 0; i < getNumChildren(); ++i)
        m_children[i]->generateGlobalMatrix();
}

void fuiRenderNodeTimeline::updateBoundsTree()
{
    for (unsigned int i = 0; i < m_children.size(); ++i)
    {
        fuiRenderNode* timeline = m_children[i]->asTimeline();
        if (timeline != 0)
            static_cast<fuiRenderNodeTimeline*>(timeline)->updateBoundsTree();
        fuiRect bounds;
        m_children[i]->getParentSpaceBounds(&bounds);
        if (i == 0)
            m_bounds = bounds;
        else
            expandBoundsFromChild(bounds);
    }
}
