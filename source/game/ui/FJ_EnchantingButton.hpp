#ifndef GAME_UI_FJ_ENCHANTINGBUTTON_HPP
#define GAME_UI_FJ_ENCHANTINGBUTTON_HPP

#include "game/ui/FJ_Base.hpp"

class FJ_Event;
class FJ_FuiNode;
class fuiRenderNode;

class FJ_EnchantingButton : public FJ_Base
{
public:
    explicit FJ_EnchantingButton(fuiRenderNode* pRenderNode);
    virtual ~FJ_EnchantingButton() {}

    void HandleUpdateLabel(FJ_Event* pEvent);
    static void _HandleUpdateLabel(FJ_FuiNode* pNode, FJ_Event* pEvent);
    void SetupIggySlot();
    void SetEnchantmentLevel(int level);
    void ChangeState(int state);

private:
    int m_state;
    int m_enchantmentLevel;
};

#endif
