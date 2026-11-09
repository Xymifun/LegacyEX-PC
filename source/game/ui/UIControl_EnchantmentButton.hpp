#ifndef GAME_UI_UICONTROL_ENCHANTMENTBUTTON_HPP
#define GAME_UI_UICONTROL_ENCHANTMENTBUTTON_HPP

#include <string>
#include <vector>

#include "game/ui/UIControl_Button.hpp"
#include "game/util/Random.hpp"

struct fuiRect;

class UIControl_EnchantmentButton : public UIControl_Button
{
public:
    class EnchantmentNames : public Random
    {
    public:
        EnchantmentNames();
        std::wstring getRandomName();

    private:
        std::vector<std::wstring> m_names;
    };

    UIControl_EnchantmentButton();
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    void updateState();
    virtual void tick();
    void init(int index);
    virtual void ReInit();
    void render(fuiRect* rect);
    void SetEnchantmentLevel(int level);
    bool IsInactive();
    virtual void setFocus(bool focus);

private:
    int m_state;
    int m_level;
    int m_index;
    std::wstring m_name;
    bool m_focus;
    int m_colour;
    int m_focusColour;
    int m_inactiveColour;
};

#endif
