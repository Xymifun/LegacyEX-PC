#include "game/ui/UIControl_DynamicButtonList.hpp"

void UIControl_DynamicButtonList::tick()
{
    UIControl_List::tick();

    int index = 0;
    std::vector<UIString>::iterator label = m_labels.begin();
    while (label != m_labels.end())
    {
        if (label->needsUpdating() != 0)
        {
            setButtonLabel(index, label->getString());
            label->setUpdated();
        }
        ++label;
        ++index;
    }
}

void UIControl_DynamicButtonList::addItem(UIString label, int index)
{
    if (index < 0)
    {
        index = m_itemIndex;
    }
    m_labels.push_back(label);
    UIControl_ButtonList::addItem(label.getString(), index);
}

void UIControl_DynamicButtonList::removeItem(int index, bool removeNow)
{
    m_labels.erase(m_labels.begin() + index);
    UIControl_List::removeItem(index, removeNow);
}
