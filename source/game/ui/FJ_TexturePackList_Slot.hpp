#ifndef GAME_UI_FJ_TEXTUREPACKLIST_SLOT_HPP
#define GAME_UI_FJ_TEXTUREPACKLIST_SLOT_HPP

#include <string>

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;

class FJ_TexturePackList_Slot : public FJ_FuiNode
{
private:
    FJ_FuiNode* m_pSlotOutline;
    int m_iSlotId;

public:
    FJ_TexturePackList_Slot(fuiRenderNode* pRenderNode);

    void SetSelected(bool bSelected);
    void SetSlotID(int iSlotId);
    void SetTextureName(const std::wstring& textureName);
    static FJ_TexturePackList_Slot* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
    int getSlotId();
};

#endif
