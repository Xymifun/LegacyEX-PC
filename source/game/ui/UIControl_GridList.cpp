#include "game/ui/UIControl_GridList.hpp"

#include "game/core/ConsoleUIController.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/FJ_List2D.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIControl_GridList::UIControl_GridList()
{
    m_itemIndex = 0;
    m_currentSelection = 0;
}

UIControl_GridList::~UIControl_GridList()
{
    std::vector<GenericPendingUpdate*>::iterator update = m_pendingUpdates.begin();
    while (update != m_pendingUpdates.end())
    {
        delete *update;
        ++update;
    }
}

unsigned int UIControl_GridList::CanTouchFocus(int x, int y)
{
    unsigned int result = 0;
    if (!m_fuiRenderNode->getFuiNode()->isFJ_List2D())
        return result;
    result = m_fuiRenderNode->getFuiNode()->asFJ_List2D()->CanTouchFocus(x, y);
    return result;
}

void UIControl_GridList::SetTouchFocus(int x, int y, bool focus)
{
    if (!m_fuiRenderNode->getFuiNode()->isFJ_List2D())
        return;
    m_fuiRenderNode->getFuiNode()->asFJ_List2D()->SetTouchFocus(x, y, focus);
}

unsigned int UIControl_GridList::CanTouchTrigger(int x, int y)
{
    unsigned int result = 0;
    if (!m_fuiRenderNode->getFuiNode()->isFJ_List2D())
        return result;
    result = m_fuiRenderNode->getFuiNode()->asFJ_List2D()->CanTouchTrigger(x, y);
    return result;
}

void UIControl_GridList::addElement(UIControl::eUIControlType type, int index)
{
    if (index < 0)
        index = m_itemIndex;
    ++m_itemIndex;
    m_elementTypes[(unsigned int)index] = type;
    m_elementEnabled[(unsigned int)index] = true;
    m_elements.push_back(index);
}

void UIControl_GridList::clearElements()
{
    m_currentSelection = 0;
    m_itemIndex = 0;
    m_elementTypes.clear();
    m_elementEnabled.clear();
    m_elements.clear();
}

int UIControl_GridList::getListIndex(int index)
{
    std::vector<int>::iterator element = m_elements.begin();
    std::vector<int>::iterator end = m_elements.end();
    int listIndex = 0;
    while (element != end && *element != index)
    {
        ++element;
        ++listIndex;
    }
    if (element == end)
        return -1;
    return listIndex;
}

bool UIControl_GridList::setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name)
{
    return UIControl_Base::setupControl(scene, node, name);
}

void UIControl_GridList::clearList()
{
    static_cast<FJ_List2D*>(m_fuiRenderNode->getFuiNode())->removeAllItems();
    clearElements();
}

void UIControl_GridList::init(int controlId)
{
    m_controlId = controlId;
    static_cast<FJ_List2D*>(m_fuiRenderNode->getFuiNode())->Init(controlId);
    int layerId = m_scene->m_layer->m_layerId;
    switch (layerId)
    {
    case 4:
    case 6:
    case 7:
        gConsoleUIController.TouchBoxAdd(this, m_scene);
        break;
    default:
        break;
    }
}

void UIControl_GridList::ReInit()
{
    UIControl::ReInit();
    init(m_controlId);
    clearElements();
    m_pendingUpdates.clear();
}

void UIControl_GridList::tick()
{
    UIControl_Base::tick();
    std::vector<GenericPendingUpdate*>::iterator update = m_pendingUpdates.begin();
    while (update != m_pendingUpdates.end())
    {
        (*update)->OnTick();
        delete *update;
        ++update;
    }
    m_pendingUpdates.clear();
}

void UIControl_GridList::setCurrentSelection(int selection, bool immediate)
{
    if (immediate)
    {
        static_cast<FJ_List2D*>(m_fuiRenderNode->getFuiNode())->HighlightItem(selection);
        return;
    }
    m_pendingUpdates.push_back(new PendingSetCurrentSelection(this, selection));
}

void UIControl_GridList::EnableItem(int item, bool enabled, bool immediate)
{
    if (immediate)
    {
        int listIndex = getListIndex(item);
        static_cast<FJ_List2D*>(m_fuiRenderNode->getFuiNode())->EnableItem(listIndex, enabled);
        return;
    }
    m_pendingUpdates.push_back(new PendingSetEnable(this, item, enabled));
}

UIControl_GridList::GenericPendingUpdate::~GenericPendingUpdate()
{
}

UIControl_GridList::PendingSetCurrentSelection::~PendingSetCurrentSelection()
{
}

unsigned int UIControl_GridList::PendingSetCurrentSelection::OnTick()
{
    m_list->setCurrentSelection(m_value, true);
    return 1;
}

UIControl_GridList::PendingSetEnable::~PendingSetEnable()
{
}

unsigned int UIControl_GridList::PendingSetEnable::OnTick()
{
    m_list->EnableItem(m_value, m_enabled, true);
    return 1;
}
