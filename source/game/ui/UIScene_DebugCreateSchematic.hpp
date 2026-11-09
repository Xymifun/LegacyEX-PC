#ifndef GAME_UI_UISCENE_DEBUGCREATESCHEMATIC_HPP
#define GAME_UI_UISCENE_DEBUGCREATESCHEMATIC_HPP

#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_TextInput.hpp"

class UIScene_DebugCreateSchematic : public UIScene
{
public:
    UIScene_DebugCreateSchematic(int type, void* data, UILayer* layer);
    virtual ~UIScene_DebugCreateSchematic();

    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
    virtual void tick();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handlePress(int controlId, int childId);
    virtual void handleDestroy();
    virtual int getSceneType();
    static int KeyboardCompleteCallback(void* data, bool accepted);

private:
    enum EControls
    {
        eControl_Point1,
        eControl_Point2,
        eControl_Copy,
        eControl_Rot0,
        eControl_Rot90,
        eControl_Rot180,
        eControl_Rot270,
        eControl_MirNone,
        eControl_MirX,
        eControl_MirY,
        eControl_MirZ,
        eControl_Paste,
        eControl_Undo,
        eControl_Name,
        eControl_Save,
        eControl_Load
    };

    void targetBlock(int* x, int* y, int* z, bool* lookedAt);
    void pointLabel(int index, bool live);
    std::wstring areaText();
    std::wstring rotationText();
    std::wstring mirrorText();

    static int s_points[2][3];
    static bool s_pointSet[2];
    static int s_rotation;
    static int s_mirror;
    static std::wstring s_name;
    unsigned int m_statusRevision;
    bool m_keyboardActive;

    UIControl_Label m_labelSetArea;
    UIControl_Button m_buttonPoint1;
    UIControl_Button m_buttonPoint2;
    UIControl_Button m_buttonCopy;
    UIControl_Label m_labelRotation;
    UIControl_Button m_buttonRot[4];
    UIControl_Label m_labelMirror;
    UIControl_Button m_buttonMirror[4];
    UIControl_Label m_labelPlace;
    UIControl_Button m_buttonPaste;
    UIControl_Button m_buttonUndo;
    UIControl_Label m_labelFile;
    UIControl_TextInput m_textName;
    UIControl_Button m_buttonSave;
    UIControl_Button m_buttonLoad;
};

#endif
