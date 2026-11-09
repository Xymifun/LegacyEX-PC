#ifndef GAME_UI_FJ_ICONSLOT_HPP
#define GAME_UI_FJ_ICONSLOT_HPP

#include <string>

#include "game/ui/FJ_Base.hpp"

class FJ_BitmapIcon;
class FJ_FuiNode;
class fuiRenderNode;

class FJ_IconSlot : public FJ_Base
{
private:
    int m_iSlotId;
    FJ_FuiNode* m_pIconHolder;
    FJ_BitmapIcon* m_pBitmapIcon;

public:
    explicit FJ_IconSlot(fuiRenderNode* pRenderNode);

    static FJ_Base* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
    void ShowSlotHolder(bool bVisible);
    void ShowRedSlot(bool bRed);
    void SetParams(int iWidth, bool bUseSmallText);
    void Init(const std::wstring& label);
    virtual void SetLabel(const std::wstring& label);
    void SetSlotID(const std::string& slotName, int iSlotId);
    void SetTextureName(const std::wstring& textureName);
    int getSlotId() { return m_iSlotId; }
};

#endif
