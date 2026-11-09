#ifndef GAME_UI_UICONTROL_SCOREBOARDBUTTONLIST_HPP
#define GAME_UI_UICONTROL_SCOREBOARDBUTTONLIST_HPP

#include <string>

#include "game/ui/UIControl_ButtonList.hpp"

class UIScene;
class fuiRenderNode;

class UIControl_ScoreboardButtonList : public UIControl_ButtonList
{
public:
    class PendingAddItem : public UIControl_List::PendingUpdate<UIControl_ScoreboardButtonList>
    {
    public:
        PendingAddItem(UIControl_ScoreboardButtonList* list, int type, std::wstring label, int playerIcon,
                       int voipIcon);
        virtual unsigned int OnTick();

    private:
        std::wstring m_label;
        int m_playerIcon;
        int m_voipIcon;
    };

    class PendingSetSlotTextureName : public UIControl_List::PendingUpdate<UIControl_ScoreboardButtonList>
    {
    public:
        PendingSetSlotTextureName(UIControl_ScoreboardButtonList* list, int item, int slot, std::wstring textureName)
            : UIControl_List::PendingUpdate<UIControl_ScoreboardButtonList>(list, item)
        {
            m_slot = slot;
            m_textureName = textureName;
        }
        virtual unsigned int OnTick();

    private:
        int m_slot;
        std::wstring m_textureName;
    };

    class PendingSetSlotsText : public UIControl_List::PendingUpdate<UIControl_ScoreboardButtonList>
    {
    public:
        PendingSetSlotsText(UIControl_ScoreboardButtonList* list, int item, std::wstring slot0, std::wstring slot1,
                            std::wstring slot2, std::wstring slot3, std::wstring slot4, std::wstring slot5,
                            std::wstring slot6);
        virtual ~PendingSetSlotsText();
        virtual unsigned int OnTick();

    private:
        std::wstring m_slot0;
        std::wstring m_slot1;
        std::wstring m_slot2;
        std::wstring m_slot3;
        std::wstring m_slot4;
        std::wstring m_slot5;
        std::wstring m_slot6;
    };

    virtual ~UIControl_ScoreboardButtonList() {}
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void addItem(const std::wstring& label, int playerIcon, int voipIcon, bool immediate);
    void SetSlotTextureName(int item, int slot, const std::wstring& textureName, bool immediate);
    void setSlotsText(int item, const std::wstring& slot0, const std::wstring& slot1, const std::wstring& slot2,
                      const std::wstring& slot3, const std::wstring& slot4, const std::wstring& slot5,
                      const std::wstring& slot6, bool immediate);
    void setPlayerIcon(int item, int icon);
    void setVOIPIcon(int item, int icon);
    void setNumColumns(int columns);
};

#endif
