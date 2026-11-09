#ifndef GAME_UI_UICONTROL_HPP
#define GAME_UI_UICONTROL_HPP

#include <string>

class UIScene;
class fuiRenderNode;

class UIControl
{
    friend class ConsoleUIController;
    friend class UIScene;
    friend class UIScene_FullscreenProgress;

public:
    enum eUIControlType
    {
        eUIControlType_Unknown = 0,
        eUIControlType_Button = 1,
        eUIControlType_CheckBox = 2,
        eUIControlType_Cursor = 3,
        eUIControlType_DynamicLabel = 4,
        eUIControlType_EnchantmentBook = 5,
        eUIControlType_EnchantmentButton = 6,
        eUIControlType_HTMLLabel = 7,
        eUIControlType_Label = 8,
        eUIControlType_LeaderboardList = 9,
        eUIControlType_MinecraftPlayer = 10,
        eUIControlType_MinecraftHorse = 11,
        eUIControlType_PlayerSkinPreview = 12,
        eUIControlType_Progress = 13,
        eUIControlType_SaveList = 14,
        eUIControlType_Slider = 15,
        eUIControlType_SlotList = 16,
        eUIControlType_TextInput = 17,
        eUIControlType_TexturePackList = 18,
        eUIControlType_BitmapIcon = 19,
        eUIControlType_Touch = 20,
        eUIControlType_ButtonList = 22,
        eUIControlType_CheckboxButtonList = 23,
        eUIControlType_DLCButtonList = 24,
        eUIControlType_LayerIconButtonList = 25,
        eUIControlType_LeftIconButtonList = 26,
        eUIControlType_LobbyButtonList = 27,
        eUIControlType_MultiList = 28,
        eUIControlType_PlayerButtonList = 29,
        eUIControlType_ScoreboardButtonList = 30,
        eUIControlType_AchievementsList = 31
    };

    UIControl();
    virtual ~UIControl() {}
    virtual bool setupControl(UIScene* scene, fuiRenderNode* node, const std::string& name);
    virtual void UpdateControl();
    virtual void tick();
    virtual void ReInit();
    virtual void setFocus(bool focus);
    virtual bool hasFocus();

    void setVisible(bool visible);
    float getWidth();
    float getHeight();
    float getXPos();
    float getYPos();
    bool getVisible();
    void removeUINodeRef();
    fuiRenderNode* getFuiRenderNode();
    void setWidth(float width);
    void setOpacity(float opacity);
    void setId(int id) { m_controlId = id; }
    int getId() { return m_controlId; }
    UIControl& operator=(const UIControl& other);

protected:
    eUIControlType m_controlType;
    int m_controlId;
    bool m_removed;
    std::string m_name;
    fuiRenderNode* m_fuiRenderNode;
    UIScene* m_scene;
    float m_x;
    float m_y;
    float m_width;
    float m_height;
    float m_opacity;
    bool m_visible;
};

#endif
