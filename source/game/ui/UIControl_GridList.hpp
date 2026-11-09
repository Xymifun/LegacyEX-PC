#ifndef GAME_UI_UICONTROL_GRIDLIST_HPP
#define GAME_UI_UICONTROL_GRIDLIST_HPP

#include <boost/unordered_map.hpp>
#include <string>
#include <vector>

#include "game/ui/UIControl_Base.hpp"

class UIControl_GridList : public UIControl_Base
{
public:
    class GenericPendingUpdate
    {
    public:
        GenericPendingUpdate(int value) : m_value(value) {}
        virtual ~GenericPendingUpdate();
        virtual unsigned int OnTick() = 0;

    protected:
        int m_value;
    };

    template <class T>
    class PendingUpdate : public GenericPendingUpdate
    {
    public:
        PendingUpdate(T* list, int value) : GenericPendingUpdate(value), m_list(list) {}
        virtual ~PendingUpdate() {}

    protected:
        T* m_list;
    };

    class PendingSetCurrentSelection : public PendingUpdate<UIControl_GridList>
    {
    public:
        PendingSetCurrentSelection(UIControl_GridList* list, int selection)
            : PendingUpdate<UIControl_GridList>(list, selection)
        {
        }
        virtual ~PendingSetCurrentSelection();
        virtual unsigned int OnTick();
    };

    class PendingSetEnable : public PendingUpdate<UIControl_GridList>
    {
    public:
        PendingSetEnable(UIControl_GridList* list, int item, bool enabled)
            : PendingUpdate<UIControl_GridList>(list, item),
              m_enabled(enabled)
        {
        }
        virtual ~PendingSetEnable();
        virtual unsigned int OnTick();

    private:
        bool m_enabled;
    };

    UIControl_GridList();
    virtual ~UIControl_GridList();
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    virtual void tick();
    virtual void ReInit();
    virtual void clearList();

    unsigned int CanTouchFocus(int x, int y);
    void SetTouchFocus(int x, int y, bool focus);
    unsigned int CanTouchTrigger(int x, int y);
    void addElement(UIControl::eUIControlType type, int index);
    void clearElements();
    int getListIndex(int index);
    void init(int controlId);
    void setCurrentSelection(int selection, bool immediate);
    void EnableItem(int item, bool enabled, bool immediate);
    void remElement(int index);
    bool istype(UIControl::eUIControlType type, int index);
    unsigned int getCurrentSelection() const;
    unsigned int getItemCount();
    void updateChildFocus(int index);
    unsigned char IsEnabled(int index);

private:
    std::vector<GenericPendingUpdate*> m_pendingUpdates;
    std::vector<int> m_elements;
    int m_currentSelection;
    int m_itemIndex;
    boost::unordered_map<unsigned int, UIControl::eUIControlType> m_elementTypes;
    boost::unordered_map<unsigned int, bool> m_elementEnabled;
};

#endif
