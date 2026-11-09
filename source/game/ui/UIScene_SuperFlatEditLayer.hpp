#ifndef GAME_UI_UISCENE_SUPERFLATEDITLAYER_HPP
#define GAME_UI_UISCENE_SUPERFLATEDITLAYER_HPP

#include "game/ui/UIVec2D.hpp"
#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/UILayer.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/world/generation/FlatLayerInfo.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Button.hpp"
#include "game/ui/UIControl_Cursor.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_Slider.hpp"
#include "game/ui/UIControl_SlotList.hpp"
#include "game/ui/UIControl_Touch.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/ui/SuperflatMenu.hpp"

class ItemInstance;
class UILayer;
struct fuiRect;
class UIScene_SuperFlatEditLayer : public UIScene
{
public:
    enum ESnap
    {
        ESnap_None = 0,
        ESnap_Up = 1,
        ESnap_Down = 2,
        ESnap_Left = 3,
        ESnap_Right = 4
    };

    enum EExit
    {
        EExit_Save = 0,
        EExit_Cancel = 1
    };

    UIScene_SuperFlatEditLayer(int sceneType, void* data, UILayer* layer);
    virtual ~UIScene_SuperFlatEditLayer();
    virtual unsigned int mapElementsAndNames();
    void updateLayerButtons();
    void updateScrollCurrentPage(int page);
    int getSlotId(int column, int row);
    int getSlotId(float x, float y);
    void componentExtents(UIControl& control, float& left, float& right, float& top, float& bottom);
    bool isOverComponent(UIControl& control, float x, float y);
    int getComponentByLocation(float x, float y);
    int getComponentByLocation(const _UIVec2D& point);
    int getSlotId(const _UIVec2D& point);
    void SetPointerText(not_null_ptr<ItemInstance> item);
    unsigned int switchFocus(UIControl* control);
    void setNewPointerPos(float x, float y);
    void setNewPointerPos(const _UIVec2D& point);
    void _init();
    virtual void updateTooltips();
    virtual void updateComponents();
    ESnap getSnap(float x, float y) const;
    void dmouse(float& moveX, float& moveY, float& directionX, float& directionY);
    void componentCentre(UIControl& control, float& x, float& y);
    void slotCentre(int slot, float& x, float& y);
    bool onRow(int slot, int row);
    static int modulo(int value, int divisor);
    int getNextSlot(int slot, ESnap snap);
    void doSnap(const _UIVec2D& point, ESnap snap);
    void setNewPointerPos(UIControl* control);
    void onMouseTick();
    virtual void tick();
    virtual void handleReload();
    void exit(EExit result);
    ESnap getSnap(int action) const;
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handlePress(int controlId, int childId);
    virtual void handleSliderMove(int id, int value);
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual void handleTouchInput(unsigned int playerId, int touchX, int touchY, int touchBox, bool pressed,
                                  bool repeat, bool released);
    virtual UIControl* GetMainPanel();
    virtual void SetFocusToElement(int focus);
    virtual std::wstring getMoviePath() { return L"SuperFlatEditLayerMenu"; }
    virtual int getSceneType() { return EUIScene_SuperFlatEditLayer; }

private:
    ESnap m_pendingSnap;
    _UIVec2D m_moveStartPosition;
    int m_moveTicks;
    int m_maxLayerHeight;
    bool m_newLayer;
    int m_layerIndex;
    FlatLayerInfo m_layer;
    FlatGeneratorInfo m_generator;
    SuperflatMenu::CustomSuperflatBlocks* m_customBlocks;
    _UIVec2D m_pointerOffset;
    float m_listLeft;
    float m_listRight;
    float m_listTop;
    float m_listBottom;
    _UIVec2D m_pointerPosition;
    int m_scrollPage;
    UIControl* m_focusedControl;
    UIControl_Cursor m_cursor;
    UIControl_Label m_title;
    UIControl_Slider m_heightSlider;
    UIControl_Button m_confirmButton;
    UIControl_Label m_heightLabel;
    UIControl_Label m_typeLabel;
    UIControl_SlotList m_slotList;
    UIControl_Touch m_sliderTouch;
    UIControl_Touch m_listTouch;
    UIControl m_mainPanel;
};

#endif
