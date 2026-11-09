#ifndef GAME_UI_UISCENE_MAPVOTEMENU_HPP
#define GAME_UI_UISCENE_MAPVOTEMENU_HPP

#include <string>
#include <vector>

#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl_BitmapIcon.hpp"
#include "game/ui/UIControl_CheckboxButtonList.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/world/LevelRules.hpp"

class UIScene_MapVoteMenu : public UIScene
{
public:
    UIScene_MapVoteMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_MapVoteMenu();

    virtual unsigned int mapElementsAndNames();
    void InitForMap();
    void InitForVariant();
    void InitForTheme();
    void Init();
    virtual void updateTooltips();
    virtual void updateComponents();
    virtual void handleReload();
    void UpdateVotableInfo();
    virtual void tick();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handlePress(int controlId, int childId);
    virtual void handleFocusChange(int focus, int childFocus);
    virtual int getSceneType();

private:
    bool m_splitScreen;
    unsigned int m_votableType;
    std::vector<int> m_mapIds;
    int m_selectedVote;
    int m_previousSelection;
    bool m_updateVoteInfo;
    std::vector<LevelRules::LevelRulesetGroup*> m_levelRuleGroups;
    std::wstring m_voteTallyText;
    UIControl_CheckboxButtonList m_mapList;
    UIControl_Label m_mapListTitle;
    UIControl_Label m_mapTitle;
    UIControl_Label m_mapDescription;
    UIControl_BitmapIcon m_mapIcon;
};

#endif
