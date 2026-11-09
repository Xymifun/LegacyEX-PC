#ifndef GAME_UI_UISCENE_ACHIEVEMENTSMENU_HPP
#define GAME_UI_UISCENE_ACHIEVEMENTSMENU_HPP

#include <string>
#include <vector>

#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_AchievementsList.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIScene.hpp"

class UILayer;
class HtmlString;

struct AchievementInfo
{
    int id;
    const wchar_t* icon;
    int registrationId;
    int title;
    int description;
};

class UIScene_AchievementsMenu : public UIScene
{
public:
    enum
    {
        DISPLAY_COUNT = 93
    };
    static const int sDisplayList[DISPLAY_COUNT];
    UIScene_AchievementsMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_AchievementsMenu();
    static const AchievementInfo* GetInfo(int achievement);
    void registerTexture(int achievement);
    virtual void updateTooltips();
    void SetAchievementDescription(std::vector<HtmlString>* lines);
    void UpdateAchievementDetails();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual void updateComponents();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleGainFocus(bool hadFocus);
    virtual void handleFocusChange(int focus, int childFocus);
    virtual UIControl* GetMainPanel();
    virtual void handleReload();
    virtual void tick();

private:
    int m_achievementIndex;
    bool m_showDescription;
    bool m_updateDetails;
    unsigned char m_unknownCE[2];
    UIControl m_mainPanel;
    UIControl_AchievementsList m_achievementsList;
    UIControl_Label m_titleLabel;
    UIControl_Label m_achievementName;
};

#endif
