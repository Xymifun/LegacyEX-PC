#ifndef GAME_UI_UISCENE_QUADRANTSIGNIN_HPP
#define GAME_UI_UISCENE_QUADRANTSIGNIN_HPP

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_BitmapIcon.hpp"
#include "game/ui/UIControl_Label.hpp"

class UILayer;

class UIScene_QuadrantSignin : public UIScene
{
public:
    enum EControllerStatus
    {
        eControllerStatus_Disconnected = 0,
        eControllerStatus_Connected = 1,
        eControllerStatus_SignedIn = 2
    };

    struct SignInInfo
    {
        int (*callback)(void* context, bool signedIn, int userIndex);
        void* context;
        bool onlineGame;
        bool returnToHome;
    };

    UIScene_QuadrantSignin(int playerId, void* data, UILayer* layer);
    virtual ~UIScene_QuadrantSignin();
    virtual unsigned int mapElementsAndNames();
    void setControllerState(int controller, EControllerStatus status);
    void _initQuadrants();
    virtual void updateTooltips();
    virtual bool hasFocus(int sceneType);
    virtual bool hidesLowerScenes();
    static int AvatarReturned(void* context, unsigned char* data, unsigned int size);
    void updateState();
    virtual void tick();
    virtual std::wstring getMoviePath();
    static int SignInReturned(void* context, bool result, int error, int userIndex);
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleReload();
    virtual int getSceneType();

private:
    bool m_signInPending;
    SignInInfo m_signInInfo;
    EControllerStatus m_controllerStates[4];
    bool m_avatarPending[4];
    UIControl m_controllerControls[4];
    UIControl_Label m_pressLabels[4];
    UIControl_Label m_gamerTagLabels[4];
    UIControl_Label m_accountTypeLabels[4];
    UIControl_Label m_playerNumberLabels[4];
    UIControl_Label m_connectControllerLabels[4];
    UIControl_BitmapIcon m_playerPictures[4];
};

#endif
