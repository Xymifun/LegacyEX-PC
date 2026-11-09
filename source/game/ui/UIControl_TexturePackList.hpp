#ifndef GAME_UI_UICONTROL_TEXTUREPACKLIST_HPP
#define GAME_UI_UICONTROL_TEXTUREPACKLIST_HPP

#include <string>
#include <vector>

#include "game/ui/UIControl_Base.hpp"

class UIControl_TexturePackList : public UIControl_Base
{
public:
    struct PackDetails
    {
        int id;
        std::wstring name;
    };

    UIControl_TexturePackList();
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    virtual void ReInit();
    virtual void setEnabled(bool enabled);
    void init(const std::wstring& label, int id);
    void addPack(int id, const std::wstring& name, bool alreadyAdded);
    void selectSlot(int slot);
    void clearSlots();
    void SetTouchFocus(int x, int y, bool focus);
    unsigned int CanTouchTrigger(int x, int y);
    int GetRealHeight();

private:
    std::vector<PackDetails> m_packDetails;
};

#endif
