#ifndef GAME_UI_FJ_LISTBUTTONCONTROLLER_HPP
#define GAME_UI_FJ_LISTBUTTONCONTROLLER_HPP

#include <string>
#include "game/ui/FJ_Button.hpp"

class fuiRenderNode;

class FJ_ListButtonController : public FJ_Button
{
public:
    explicit FJ_ListButtonController(fuiRenderNode* renderNode);
    virtual ~FJ_ListButtonController() {}
    float getOriginalHeight();
    virtual void Init(const std::wstring& name, int id);

private:
    float m_originalHeight;
};

#endif
