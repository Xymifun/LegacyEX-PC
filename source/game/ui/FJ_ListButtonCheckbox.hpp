#ifndef GAME_UI_FJ_LISTBUTTONCHECKBOX_HPP
#define GAME_UI_FJ_LISTBUTTONCHECKBOX_HPP

#include <stdint.h>
#include <string>

#include "game/ui/FJ_Button.hpp"

class FJ_FuiNode;
class fuiRenderNode;

class FJ_ListButtonCheckbox : public FJ_Button
{
public:
    explicit FJ_ListButtonCheckbox(fuiRenderNode* renderNode);
    virtual ~FJ_ListButtonCheckbox() {}

    void SetCheckBox(bool checked);
    void HandleTriggerCheckBox(int id);
    static void _HandleTriggerCheckBox(FJ_FuiNode* node, int id);
    void UpdateCheckBoxScales();
    static void _UpdateCheckBoxScales(FJ_Button* button);
    virtual void Init(const std::wstring& name, int id);
    void SetSlotID(int slotId, bool visible);
    void SetTextureName(const std::wstring& textureName);
    virtual void EnableButton(bool enabled);
    uint8_t GetCheckBox();

private:
    bool m_checkBox;
};

#endif
