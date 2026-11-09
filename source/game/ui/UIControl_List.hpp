#ifndef GAME_UI_UICONTROL_LIST_HPP
#define GAME_UI_UICONTROL_LIST_HPP

#include <boost/unordered_map.hpp>
#include <string>
#include <vector>

#include "game/ui/UIControl_Base.hpp"
#include "game/ui/UIString.hpp"

class UIControl_List : public UIControl_Base
{
public:
    class GenericPendingUpdate
    {
    public:
        GenericPendingUpdate(int item) : m_item(item) {}
        virtual ~GenericPendingUpdate() {}
        virtual unsigned int OnTick() = 0;

    protected:
        int m_item;
    };

    template <class T>
    class PendingUpdate : public GenericPendingUpdate
    {
    public:
        PendingUpdate(T* list, int item) : GenericPendingUpdate(item), m_list(list) {}
        virtual ~PendingUpdate() {}

    protected:
        T* m_list;
    };

    class PendingRemoveItem : public PendingUpdate<UIControl_List>
    {
    public:
        PendingRemoveItem(UIControl_List* list, int item) : PendingUpdate<UIControl_List>(list, item) {}
        virtual ~PendingRemoveItem();
        virtual unsigned int OnTick();
    };

    class PendingSetCurrentSelection : public PendingUpdate<UIControl_List>
    {
    public:
        PendingSetCurrentSelection(UIControl_List* list, int item, bool update)
            : PendingUpdate<UIControl_List>(list, item),
              m_update(update)
        {
        }
        virtual ~PendingSetCurrentSelection();
        virtual unsigned int OnTick();

    private:
        bool m_update;
    };

    class PendingSetEnable : public PendingUpdate<UIControl_List>
    {
    public:
        PendingSetEnable(UIControl_List* list, int item, bool enabled)
            : PendingUpdate<UIControl_List>(list, item),
              m_enabled(enabled)
        {
        }
        virtual ~PendingSetEnable();
        virtual unsigned int OnTick();

    private:
        bool m_enabled;
    };

    class PendingSetLabel : public PendingUpdate<UIControl_List>
    {
    public:
        PendingSetLabel(UIControl_List* list, int item, UIString label) : PendingUpdate<UIControl_List>(list, item)
        {
            m_label = label;
        }
        virtual ~PendingSetLabel() {}
        virtual unsigned int OnTick();

    private:
        UIString m_label;
    };

    UIControl_List();
    virtual ~UIControl_List();
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    virtual void UpdateControl();
    virtual void tick();
    virtual void ReInit();
    virtual void clearList();
    virtual void removeItem(int item, bool immediate);
    void addElement(UIControl::eUIControlType type, int index);
    unsigned int GetTopItem();
    void setCurrentSelection(int selection, bool update, bool immediate);
    void updateChildFocus(int index);
    int getItemCount();
    unsigned int getCurrentSelection() const;
    void init(int controlId);
    int getListIndex(int index);
    void EnableItem(int item, bool enabled, bool immediate);
    bool istype(UIControl::eUIControlType type, int index);
    void clearElements();
    void SetItemLabel(int item, UIString label, bool immediate);
    void remElement(int index);
    unsigned char IsEnabled(int index);
    bool CheckElementExists(int index);
    int GetActiveItem();
    void SetTouchFocus(int x, int y, bool focus);
#ifdef MINECRAFT_PC
    void PcPressSlider(int x, int y);
    void PcHoverItem(int y);
#endif
    unsigned int CanTouchFocus(int x, int y);
    unsigned int CanTouchTrigger(int x, int y);
    int getListDataValue(int index);
    UIString GetItemLabel(int index);
    void SetVerticalSpacing(int spacing);
    int GetVerticalSpacing();

protected:
    std::vector<GenericPendingUpdate*> m_pendingUpdates;
    std::vector<int> m_elements;
    int m_currentSelection;
    int m_itemIndex;
    boost::unordered_map<unsigned int, UIControl::eUIControlType> m_elementTypes;
    boost::unordered_map<unsigned int, bool> m_elementEnabled;
};

#endif
