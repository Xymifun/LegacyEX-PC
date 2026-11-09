#ifndef GAME_UI_UISCENE_DEBUGFILLAREA_HPP
#define GAME_UI_UISCENE_DEBUGFILLAREA_HPP

#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_TextInput.hpp"

class BlockState;

class UIScene_DebugFillArea : public UIScene
{
public:
    UIScene_DebugFillArea(int type, void* data, UILayer* layer);
    virtual ~UIScene_DebugFillArea();

    virtual unsigned int mapElementsAndNames();
    virtual void updateTooltips();
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
        eControl_FillTile,
        eControl_FillData,
        eControl_EdgeTile,
        eControl_EdgeData,
        eControl_Create,
        eControl_Undo
    };

    UIControl_TextInput* inputFor(int control);
    void pointLabel(int index);
    void setPoint(int index);
    void showStatus(const std::wstring& text);
    std::wstring areaText();
    const BlockState* readBlock(UIControl_TextInput& tile, UIControl_TextInput& data, bool allowEmpty,
                                std::wstring* error);

    static int s_points[2][3];
    static bool s_pointSet[2];
    static std::wstring s_texts[4];
    int m_keyboardControl;
    bool m_keyboardActive;
    UIControl_Label m_labelSetArea;
    UIControl_Button m_buttonPoint1;
    UIControl_Button m_buttonPoint2;
    UIControl_Label m_labelFillTile;
    UIControl_Label m_labelFillData;
    UIControl_Label m_labelEdgeTile;
    UIControl_Label m_labelEdgeData;
    UIControl_TextInput m_textFillTile;
    UIControl_TextInput m_textFillData;
    UIControl_TextInput m_textEdgeTile;
    UIControl_TextInput m_textEdgeData;
    UIControl_Button m_buttonCreate;
    UIControl_Button m_buttonUndo;
};

#endif
