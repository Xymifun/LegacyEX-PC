#ifndef GAME_UI_UICOMPONENT_TUTORIALPOPUP_HPP
#define GAME_UI_UICOMPONENT_TUTORIALPOPUP_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/item/ItemInstance.hpp"

class UILayer;
class _TutorialPopupInfo;
class Tutorial;
class ConsoleUIController;

class UIComponent_TutorialPopup : public UIScene
{
public:
    enum EIcons
    {
        EIcons_None = 0
    };

    UIComponent_TutorialPopup(int sceneType, void*, UILayer* layer);
    virtual ~UIComponent_TutorialPopup();

    void SetVisible(bool visible);
    void setupIconHolder(EIcons icon);
    std::wstring _SetIcon(not_null_ptr<ItemInstance> item, int itemId, int auxValue, bool showIcon,
                          const wchar_t* image);
    std::wstring _SetImage(std::wstring& image);
    void ParseDescription(int player, std::wstring& description);
    void _SetDescription(UIScene* scene, const std::wstring& title, const std::wstring& description, bool showTitle,
                         bool showExitScreenshot, int displayTime);
    void SetTutorialDescription(_TutorialPopupInfo* info);
    void RemoveInteractSceneReference(UIScene* scene);
    void UpdateTutorialPopup();
    void UpdateInteractScenePosition(bool visible);
    bool IsVisible();

    virtual unsigned int mapElementsAndNames();
    virtual std::wstring getMoviePath();
    virtual void handleReload();
    virtual void render(int width, int height, C4JRender::eViewportType viewport);
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual void handleTimerComplete(int timerId);
    virtual int getSceneType();
    virtual bool stealsFocus();
    virtual bool hasFocus(int controller);
    virtual bool hidesLowerScenes();

private:
    friend class ConsoleUIController;

    UIScene* m_interactScene;
    UIScene* m_previousInteractScene;
    bool m_interactScenePosition;
    bool m_showTitleTimer;
    bool m_showExitScreenshot;
    std::wstring m_description;
    Tutorial* m_tutorial;
    not_null_ptr<ItemInstance> m_icon;
    bool m_showIcon;
    bool m_containerMenuDisplayed;
    bool m_splitScreen;
    std::wstring m_title;
    EIcons m_iconHolder;
    UIControl_Label m_descriptionLabel;
    UIControl_Label m_titleLabel;
    UIControl m_iconHolderControl;
    UIControl m_exitScreenshotControl;
};

#endif
