#ifndef GAME_UI_FJ_LISTBUTTONLAYERICON_HPP
#define GAME_UI_FJ_LISTBUTTONLAYERICON_HPP

#include <string>

#include "game/ui/FJ_Button.hpp"

class fuiRenderNode;

class FJ_ListButtonLayerIcon : public FJ_Button
{
public:
    explicit FJ_ListButtonLayerIcon(fuiRenderNode* renderNode);
    virtual ~FJ_ListButtonLayerIcon() {}

    void Init(const std::wstring& name, int id);
    void SetSlotID(int slotId, bool visible);
    void SetTextureName(const std::wstring& textureName, const std::wstring& layerName);

private:
    FJ_FuiNode* m_textureNode;
};

#endif
