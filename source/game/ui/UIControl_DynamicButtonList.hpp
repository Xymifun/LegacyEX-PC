#ifndef GAME_UI_UICONTROL_DYNAMICBUTTONLIST_HPP
#define GAME_UI_UICONTROL_DYNAMICBUTTONLIST_HPP

#include <vector>

#include "game/ui/UIControl_ButtonList.hpp"
#include "game/ui/UIString.hpp"

class UIControl_DynamicButtonList : public UIControl_ButtonList
{
public:
    virtual ~UIControl_DynamicButtonList() {}
    virtual void tick();
    void addItem(UIString label, int index);
    virtual void removeItem(int index, bool removeNow);

private:
    std::vector<UIString> m_labels;
};

#endif
