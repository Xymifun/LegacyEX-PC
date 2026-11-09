#ifndef GAME_UI_UISCENE_MAPSELECTMENU_HPP
#define GAME_UI_UISCENE_MAPSELECTMENU_HPP

#include "game/minigame/EMiniGameId.hpp"
#include <stdint.h>
#include <string>
#include <vector>

#include "game/ui/UIScene.hpp"
#include "game/ui/IUIScene_DlcSelectMenu.hpp"
#include "game/ui/UIControl_BitmapIcon.hpp"
#include "game/ui/UIControl_CheckboxButtonList.hpp"
#include "game/ui/UIControl_HTMLLabel.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/world/LevelRules.hpp"

class LevelGenerationOptions;
class LaunchMoreOptionsMenuInitData;

class UIScene_MapSelectMenu : public UIScene, public IUIScene_DlcSelectMenu
{
public:
    UIScene_MapSelectMenu(int sceneType, void* data, UILayer* layer);
    virtual ~UIScene_MapSelectMenu();

    virtual unsigned int mapElementsAndNames();
    void _init();
    virtual std::wstring getMoviePath();
    virtual void updateTooltips();
    virtual void updateComponents();
    void UpdateMapInfo();
    virtual void tick();
    virtual void handleReload();
    int checkHasPurchased(int index, bool tryPurchase);
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleFocusChange(int focus, int childFocus);
    virtual void handlePress(int controlId, int childId);
    virtual int getSceneType();

private:
    std::vector<LevelRules::LevelRulesetGroup*> m_levelRuleGroups;
    LaunchMoreOptionsMenuInitData* m_initData;
    LevelGenerationOptions* m_generationOptions;
    unsigned char m_mapInfoPending;
    unsigned int m_mapSelection;
    EMiniGameId m_miniGameType;
    UIControl_CheckboxButtonList m_mapList;
    UIControl_Label m_mapListTitle;
    UIControl_Label m_mapTitle;
    UIControl_HTMLLabel m_mapDescription;
    UIControl_BitmapIcon m_mapIcon;
};

#endif
