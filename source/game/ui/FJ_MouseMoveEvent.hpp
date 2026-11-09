#ifndef GAME_UI_FJ_MOUSEMOVEEVENT_HPP
#define GAME_UI_FJ_MOUSEMOVEEVENT_HPP

#include "game/ui/FJ_Event.hpp"

class FJ_MouseMoveEvent : public FJ_Event
{
public:
    float m_fX;
    float m_fY;

    FJ_MouseMoveEvent(float fX, float fY, bool bBubbles, bool bCancelable);
};

#endif
