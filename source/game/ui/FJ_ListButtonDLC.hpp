#ifndef GAME_UI_FJ_LISTBUTTONDLC_HPP
#define GAME_UI_FJ_LISTBUTTONDLC_HPP

#include <string>

#include "game/ui/FJ_Button.hpp"

class fuiRenderNode;

class FJ_ListButtonDLC : public FJ_Button
{
public:
    explicit FJ_ListButtonDLC(fuiRenderNode* renderNode);
    virtual ~FJ_ListButtonDLC() {}

    virtual void Init(const std::wstring& name, int id);
    void ShowTick(bool show);

private:
    FJ_FuiNode* m_tickNode;
};

#endif
