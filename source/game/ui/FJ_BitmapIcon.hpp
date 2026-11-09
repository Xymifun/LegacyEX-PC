#ifndef GAME_UI_FJ_BITMAPICON_HPP
#define GAME_UI_FJ_BITMAPICON_HPP

#include <string>

#include "game/ui/FJ_FuiNode.hpp"

class fuiRenderNode;

class FJ_BitmapIcon : public FJ_FuiNode
{
private:
    FJ_FuiNode* m_pBitmapNode;

public:
    FJ_BitmapIcon(fuiRenderNode* pRenderNode);

    static FJ_FuiNode* Create(fuiRenderNode* pRenderNode);
    virtual unsigned int getType();
    void SetTextureName(const std::wstring& textureName);
};

#endif
