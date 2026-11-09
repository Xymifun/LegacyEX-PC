#ifndef GAME_UI_UICONTROL_MULTILIST_HPP
#define GAME_UI_UICONTROL_MULTILIST_HPP

#include <stdint.h>
#include <string>
#include <vector>

#include "game/ui/UIControl_List.hpp"

class UIControl_MultiList : public UIControl_List
{
public:
    class PendingSetChecked : public UIControl_List::PendingUpdate<UIControl_MultiList>
    {
    public:
        PendingSetChecked(UIControl_MultiList* list, int item, bool checked)
            : UIControl_List::PendingUpdate<UIControl_MultiList>(list, item),
              m_checked(checked)
        {
        }
        virtual ~PendingSetChecked();
        virtual unsigned int OnTick();

    private:
        bool m_checked;
    };

    class PendingSetOpacity : public UIControl_List::PendingUpdate<UIControl_MultiList>
    {
    public:
        PendingSetOpacity(UIControl_MultiList* list, int item, float opacity)
            : UIControl_List::PendingUpdate<UIControl_MultiList>(list, item),
              m_opacity(opacity)
        {
        }
        virtual ~PendingSetOpacity();
        virtual unsigned int OnTick();

    private:
        float m_opacity;
    };

    class PendingSetSlider : public UIControl_List::PendingUpdate<UIControl_MultiList>
    {
    public:
        PendingSetSlider(UIControl_MultiList* list, int item, int value)
            : UIControl_List::PendingUpdate<UIControl_MultiList>(list, item),
              m_value(value)
        {
        }
        virtual ~PendingSetSlider();
        virtual unsigned int OnTick();

    private:
        int m_value;
    };

    class PendingSetToggleButtonOutline : public UIControl_List::PendingUpdate<UIControl_MultiList>
    {
    public:
        PendingSetToggleButtonOutline(UIControl_MultiList* list, int item, bool outline)
            : UIControl_List::PendingUpdate<UIControl_MultiList>(list, item),
              m_outline(outline)
        {
        }
        virtual ~PendingSetToggleButtonOutline();
        virtual unsigned int OnTick();

    private:
        bool m_outline;
    };

    UIControl_MultiList();
    virtual ~UIControl_MultiList() {}
    void AddNewButton(const std::wstring& label, int id, bool useEllipsis);
    void AddNewLabel(const std::wstring& label, int id);
    void AddNewCheckbox(const std::wstring& label, int id, bool checked);
    void AddNewControllerButton(const std::wstring& label, const std::wstring& secondaryLabel, bool html, int id);
    void AddNewSlider(const std::wstring& label, int id, int minValue, int maxValue, int value, bool repeat,
                      int repeatDelay, int repeatStep);
    void SetCheckboxValue(int id, bool checked, bool immediate);
    bool IsCheckbox(int id);
    bool GetCheckboxValue(int id);
    void AddNewTextInput(const std::wstring& label, int id, int charLimit);
    uint32_t GetSliderValue(int id);
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    virtual void setAllPossibleLabels(int firstLabel, unsigned int labelCount, wchar_t (*labels)[256]);
    void AddNewMenuButton(const std::wstring& label, int id);
    bool IsSlider(int id);
    void HighlightItem(unsigned int id, bool update);
    void SetSliderValue(int id, int value, bool immediate);
    void SetItemOpacity(int id, float opacity, bool immediate);
    void SetSecondaryLabel(int id, UIString label, bool html, bool autoSize, bool immediate);
    void ToggleButtonOutline(int id, bool outline, bool immediate);
    bool IsButton(int id);
    bool IsTextInput(int id);
};

#endif
