#ifndef GAME_UI_FJ_LISTBUTTONICONLEFT_HPP
#define GAME_UI_FJ_LISTBUTTONICONLEFT_HPP

#include <string>

#include "game/ui/FJ_Button.hpp"

class FJ_FuiNode;
class fuiRenderNode;

class FJ_ListButtonIconLeft : public FJ_Button
{
public:
    explicit FJ_ListButtonIconLeft(fuiRenderNode* renderNode);
    virtual ~FJ_ListButtonIconLeft() {}

    void Init(const std::wstring& name, int id);
    void SetSlotID(int slotId, bool hasSlot);
    void SetTextureName(const std::wstring& textureName, const std::wstring& textureName1);

private:
    FJ_FuiNode* m_pIconPlaceholder;
    FJ_FuiNode* m_pIconPlaceholder1;
};

#endif
