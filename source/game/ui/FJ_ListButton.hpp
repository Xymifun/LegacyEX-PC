#ifndef GAME_UI_FJ_LISTBUTTON_HPP
#define GAME_UI_FJ_LISTBUTTON_HPP

#include <string>
#include "game/ui/FJ_Button.hpp"

class fuiRenderNode;

class FJ_ListButton : public FJ_Button
{
    friend class fuiRenderNodeTimeline;
    friend class FJ_List_MultiList;
    friend class FJ_List_ListButton;

public:
    explicit FJ_ListButton(fuiRenderNode* renderNode);
    virtual ~FJ_ListButton() {}

    virtual void Init(const std::wstring& name, int id);
    void UseEllipsis(bool useEllipsis);

private:
    bool m_listButtonFlag;
    int m_labelAlignment;
};

#endif
