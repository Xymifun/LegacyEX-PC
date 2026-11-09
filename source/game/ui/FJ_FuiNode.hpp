#ifndef GAME_UI_FJ_FUINODE_HPP
#define GAME_UI_FJ_FUINODE_HPP

#include <string>
#include <vector>

#include "game/ui/FJ_Event.hpp"
#include "game/ui/fuiRect.hpp"

class FJ_Event;
struct FJ_EventListener;
class FJ_FuiNodeStage;
class UIScene;
class fuiRenderNode;

class FJ_Base;
class FJ_BitmapIcon;
class FJ_Button;
class FJ_CheckBox;
class FJ_Cursor;
class FJ_Document;
class FJ_DragonHealth;
class FJ_DynamicHtmlText;
class FJ_ExperienceBar;
class FJ_HorseJumpBar;
class FJ_HtmlText;
class FJ_HtmlTextPanel;
class FJ_HtmlTextPanelLarge;
class FJ_HtmlText_WithPanel;
class FJ_Hud;
class FJ_IconSlot;
class FJ_Label;
class FJ_LabelOutline;
class FJ_Label_Blink;
class FJ_Label_Book;
class FJ_Label_HUD_Splash;
class FJ_Label_HUD_SplashBig;
class FJ_Label_HUD_SplashVita;
class FJ_LeaderboardList;
class FJ_List;
class FJ_List2D;
class FJ_ListButton;
class FJ_ListButtonAchievement;
class FJ_ListButtonController;
class FJ_List_ListButton;
class FJ_List_ListButtonAchievement;
class FJ_List_ListButtonPlayerList;
class FJ_List_ListButtonScoreBoard;
class FJ_List_MenuButton;
class FJ_List_MultiList;
class FJ_MenuButton;
class FJ_MobEffect;
class FJ_MobEffect_Tile;
class FJ_ProgressBar;
class FJ_ScrollArrow;
class FJ_SignInput;
class FJ_Slider;
class FJ_SlotList;
class FJ_TextInput;
class FJ_UpsellButton;

class FJ_FuiNode
{
public:
    fuiRenderNode* m_pRenderNode;
    int m_iTabIndex;
    std::vector<FJ_EventListener> m_eventListeners;
    UIScene* m_pCallbackScene;

    FJ_FuiNode(fuiRenderNode* pRenderNode);

    static FJ_FuiNode* Create(fuiRenderNode* pRenderNode);

    FJ_FuiNodeStage* getStage();

    virtual ~FJ_FuiNode();

    virtual void ASConstructor();

    float getX();
    float getY();
    float getWidth();
    float getHeight();

    void setScaleX(float fScaleX);
    void setScaleY(float fScaleY);
    float getScaleX();
    float getScaleY();

    const char* getName();

    fuiRect getRect(FJ_FuiNode* pNode);

    int getTabIndex();

    void setScrollRect(fuiRect& rect);

    int getNumChildren();
    FJ_FuiNode* getChildByName(const std::string& name);
    fuiRenderNode* getRenderNodeChildByName(const std::string& name);
    fuiRenderNode* getRenderNodeChildAt(int iIndex);
    fuiRenderNode* getRenderNodeParent();
    FJ_FuiNode* getChildAt(int iIndex);
    FJ_FuiNode* getParent();

    void removeChild(FJ_FuiNode* pNode, bool bDestroy);
    void removeChild(fuiRenderNode* pRenderNode, bool bDestroy);
    void removeChildAt(int iIndex, bool bDestroy);
    void addMask(FJ_FuiNode* pNode);
    void addChild(FJ_FuiNode* pNode);
    void addChildAt(FJ_FuiNode* pNode, int iIndex);
    void addChild(fuiRenderNode* pRenderNode);
    void addChildAt(fuiRenderNode* pRenderNode, int iIndex);
    int getChildIndex(FJ_FuiNode* pNode);

    FJ_FuiNode* asMovieClip();
    static FJ_FuiNode* asMovieClip(FJ_FuiNode* pNode);

    FJ_Label* asFJ_Label();
    static FJ_Label* asFJ_Label(FJ_FuiNode* pNode);
    FJ_Base* asFJ_Base();
    bool isFJ_Base();
    FJ_Button* asFJ_Button();
    bool isFJ_Button();
    FJ_MenuButton* asFJ_MenuButton();
    bool isFJ_MenuButton();
    bool isFJ_Label();
    FJ_LabelOutline* asFJ_LabelOutline();
    bool isFJ_LabelOutline();
    FJ_Label_HUD_Splash* asFJ_Label_HUD_Splash();
    bool isFJ_Label_HUD_Splash();
    FJ_Label_HUD_SplashBig* asFJ_Label_HUD_SplashBig();
    bool isFJ_Label_HUD_SplashBig();
    FJ_Label_HUD_SplashVita* asFJ_Label_HUD_SplashVita();
    bool isFJ_Label_HUD_SplashVita();
    FJ_Label_Blink* asFJ_Label_Blink();
    bool isFJ_Label_Blink();
    FJ_List* asFJ_List();
    bool isFJ_List();
    FJ_List2D* asFJ_List2D();
    bool isFJ_List2D();
    FJ_List_MenuButton* asFJ_List_MenuButton();
    bool isFJ_List_MenuButton();
    FJ_ListButton* asFJ_ListButton();
    bool isFJ_ListButton();
    FJ_UpsellButton* asFJ_UpsellButton();
    bool isFJ_UpsellButton();
    FJ_ListButtonController* asFJ_ListButtonController();
    bool isFJ_ListButtonController();
    FJ_List_ListButton* asFJ_List_ListButton();
    bool isFJ_List_ListButton();
    FJ_List_ListButtonAchievement* asFJ_List_ListButtonAchievement();
    bool isFJ_List_ListButtonAchievement();
    FJ_List_ListButtonPlayerList* asFJ_List_ListButtonPlayerList();
    bool isFJ_List_ListButtonPlayerList();
    FJ_List_ListButtonScoreBoard* asFJ_List_ListButtonScoreBoard();
    bool isFJ_List_ListButtonScoreBoard();
    FJ_HtmlText* asFJ_HtmlText();
    bool isFJ_HtmlText();
    FJ_DynamicHtmlText* asFJ_DynamicHtmlText();
    bool isFJ_DynamicHtmlText();
    FJ_HtmlText_WithPanel* asFJ_HtmlText_WithPanel();
    bool isFJ_HtmlText_WithPanel();
    FJ_Slider* asFJ_Slider();
    bool isFJ_Slider();
    FJ_CheckBox* asFJ_CheckBox();
    bool isFJ_CheckBox();
    FJ_TextInput* asFJ_TextInput();
    bool isFJ_TextInput();
    FJ_DragonHealth* asFJ_DragonHealth();
    bool isFJ_DragonHealth();
    FJ_HorseJumpBar* asFJ_HorseJumpBar();
    bool isFJ_HorseJumpBar();
    FJ_ExperienceBar* asFJ_ExperienceBar();
    bool isFJ_ExperienceBar();
    FJ_Cursor* asFJ_Cursor();
    bool isFJ_Cursor();
    FJ_SlotList* asFJ_SlotList();
    bool isFJ_SlotList();
    FJ_ScrollArrow* asFJ_ScrollArrow();
    bool isFJ_ScrollArrow();
    FJ_MobEffect* asFJ_MobEffect();
    bool isFJ_MobEffect();
    FJ_MobEffect_Tile* asFJ_MobEffect_Tile();
    bool isFJ_MobEffect_Tile();
    FJ_ProgressBar* asFJ_ProgressBar();
    bool isFJ_ProgressBar();
    FJ_BitmapIcon* asFJ_BitmapIcon();
    bool isFJ_BitmapIcon();
    FJ_IconSlot* asFJ_IconSlot();
    bool isFJ_IconSlot();
    FJ_Label_Book* asFJ_Label_Book();
    bool isFJ_Label_Book();
    FJ_List_MultiList* asFJ_List_MultiList();
    bool isFJ_List_MultiList();
    FJ_Document* asFJ_Document();
    bool isFJ_Document();
    FJ_HtmlTextPanel* asFJ_HtmlTextPanel();
    bool isFJ_HtmlTextPanel();
    FJ_HtmlTextPanelLarge* asFJ_HtmlTextPanelLarge();
    bool isFJ_HtmlTextPanelLarge();
    FJ_ListButtonAchievement* asFJ_ListButtonAchievement();
    bool isFJ_ListButtonAchievement();
    FJ_SignInput* asFJ_SignInput();
    bool isFJ_SignInput();
    FJ_LeaderboardList* asFJ_LeaderboardList();
    bool isFJ_LeaderboardList();
    FJ_Hud* asFJ_Hud();
    bool isFJ_Hud();

    void gotoAndPlay(int iFrame);
    void gotoAndPlay(const std::string& sLabel);
    void gotoAndStop(int iFrame);
    void gotoAndStop(const std::string& sLabel);
    void stop();
    int getFrameByLabel(const std::string& sLabel);
    std::string currentLabel();
    unsigned int currentFrame();

    virtual void setX(float fX);
    void decX(float fAmount);
    void incX(float fAmount);

    virtual void setY(float fY);
    void decY(float fAmount);
    void incY(float fAmount);

    virtual void setHeight(float fHeight);
    void decHeight(float fAmount);
    void incHeight(float fAmount);

    void setWidth(float fWidth);
    void decWidth(float fAmount);
    void incWidth(float fAmount);

    void setTabIndex(int iTabIndex);

    virtual void setVisible(bool bVisible);
    bool isVisible();

    float getAlpha();
    void setAlpha(float fAlpha);
    void decAlpha(float fAmount);
    void incAlpha(float fAmount);

    void eventListen(FJ_Event* pEvent, bool bCapture);
    void dispatchEvent(FJ_Event* pEvent);

    void addEventListener(eFJEventType eType, FJ_FuiNode* pNode, void (*pfnHandler)(FJ_FuiNode*, FJ_Event*),
                          bool bCapture);
    void removeEventListener(eFJEventType eType, FJ_FuiNode* pNode, void (*pfnHandler)(FJ_FuiNode*, FJ_Event*));

    FJ_FuiNode* getRoot();

    virtual unsigned int getType();

    void setCallbackScene(UIScene* pScene);
};

#endif
