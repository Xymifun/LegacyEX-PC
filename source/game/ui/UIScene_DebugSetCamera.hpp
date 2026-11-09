#ifndef GAME_UI_UISCENE_DEBUGSETCAMERA_HPP
#define GAME_UI_UISCENE_DEBUGSETCAMERA_HPP

#include <string>

#include "game/core/CMinecraftApp.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_CheckBox.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_TextInput.hpp"

class UIScene_DebugSetCamera : public UIScene
{
public:
    UIScene_DebugSetCamera(int type, void* data, UILayer* layer);
    virtual ~UIScene_DebugSetCamera();

    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handlePress(int controlId, int childId);
    virtual void handleCheckboxToggled(int id, bool checked);
    virtual void handleDestroy();
    virtual int getSceneType();
    static int KeyboardCompleteCallback(void* data, bool accepted);

private:
    enum EControls
    {
        eControl_CamX,
        eControl_CamY,
        eControl_CamZ,
        eControl_YRot,
        eControl_Elevation,
        eControl_LockPlayer,
        eControl_Teleport
    };

    double* valueFor(int control);
    UIControl_TextInput* inputFor(int control);

    DebugSetCameraPosition m_position;
    int m_keyboardControl;
    bool m_keyboardActive;
    UIControl_TextInput m_textInputX;
    UIControl_TextInput m_textInputY;
    UIControl_TextInput m_textInputZ;
    UIControl_TextInput m_textInputYRot;
    UIControl_TextInput m_textInputElevation;
    UIControl_CheckBox m_checkboxLockPlayer;
    UIControl_Button m_buttonTeleport;
    UIControl_Label m_labelTitle;
    UIControl_Label m_labelCamX;
    UIControl_Label m_labelCamY;
    UIControl_Label m_labelCamZ;
    UIControl_Label m_labelYRotElev;
};

#endif
