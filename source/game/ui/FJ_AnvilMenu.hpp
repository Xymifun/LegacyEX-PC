#ifndef GAME_UI_FJ_ANVILMENU_HPP
#define GAME_UI_FJ_ANVILMENU_HPP

#include "game/ui/FJ_AbstractContainerDocument.hpp"

class fuiRenderNode;

class FJ_AnvilMenu : public FJ_AbstractContainerDocument
{
public:
    explicit FJ_AnvilMenu(fuiRenderNode* renderNode);
    void SetCostLabel(const std::wstring& label, bool safe);
    void ShowRedCross(bool show);
    static FJ_AnvilMenu* Create(fuiRenderNode* renderNode);
    virtual unsigned int getType();
    virtual void SetSafeZone(int top, int bottom, int left, int right);
};

#endif
