#ifndef GAME_UI_FJ_EVENT_HPP
#define GAME_UI_FJ_EVENT_HPP

class FJ_FuiNode;

enum eFJEventType
{
    eFJEventType_KeyUp,
    eFJEventType_KeyDown,
    eFJEventType_FocusIn,
    eFJEventType_FocusOut,
    eFJEventType_Press,
    eFJEventType_FocusChangeInit,
    eFJEventType_FocusChangeUser,
    eFJEventType_Init,
    eFJEventType_UpdateLabel,
    eFJEventType_AnimationStep,
    eFJEventType_CheckboxToggled,
    eFJEventType_CheckboxInit,
    eFJEventType_InitHud,
    eFJEventType_ShowAnimatedLogoText,
    eFJEventType_EnterFrame,
    eFJEventType_MouseMove,
    eFJEventType_SlideComplete,
    eFJEventType_AnimationEnd,
    eFJEventType_ShowTimerAnimation
};

class FJ_Event
{
public:
    FJ_FuiNode* m_pTarget;
    eFJEventType m_eType;
    bool m_bBubbles;
    bool m_bCancelable;

    FJ_Event(eFJEventType eType, bool bBubbles, bool bCancelable);
};

#endif
