#ifndef GAME_UI_FJ_KEYBOARDEVENT_HPP
#define GAME_UI_FJ_KEYBOARDEVENT_HPP

#include "game/ui/FJ_Event.hpp"

class FJ_KeyboardEvent : public FJ_Event
{
public:
    int m_iKeyCode;

    FJ_KeyboardEvent(bool bKeyUp, int iKeyCode, bool bBubbles, bool bCancelable);
};

#endif
