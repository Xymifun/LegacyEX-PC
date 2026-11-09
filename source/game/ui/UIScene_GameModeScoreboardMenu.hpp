#ifndef GAME_UI_UISCENE_GAMEMODESCOREBOARDMENU_HPP
#define GAME_UI_UISCENE_GAMEMODESCOREBOARDMENU_HPP

#include <stdint.h>
#include <string>
#include <boost/shared_ptr.hpp>
#include "game/world/generation/IUIScene_PlayerList.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "game/ui/UIScene.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_Progress.hpp"
#include "game/ui/UIControl_ScoreboardButtonList.hpp"
#include "game/core/IScoreboardLayout.hpp"
#include "game/core/ClientScoreboard.hpp"

class INetworkPlayer;
class MultiplayerLocalPlayer;
class StatsUID;
class UILayer;
struct fuiRect;

class UIScene_GameModeScoreboardMenu : public IUIScene_PlayerList, public UIScene
{
public:
    UIScene_GameModeScoreboardMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_GameModeScoreboardMenu();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual void updateTooltips();
    virtual void handleReload();
    virtual void tick();
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual UIControl* GetMainPanel();
    virtual std::wstring getMoviePath();
    virtual void handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled);
    virtual void handleGainFocus(bool hadFocus);
    virtual void handlePress(int controlId, int childId);
    virtual void handleDestroy();
    virtual void handleFocusChange(int focus, int childFocus);
    virtual void BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info);
    virtual PlayerInfo* CreatePlayerInfo();
    virtual int GetPlayerCount();
    virtual PlayerInfo* GetPlayer(int index);
    virtual bool RemovePlayer(unsigned char smallId);
    virtual void AddPlayer(unsigned char smallId);
    virtual UIControl_ButtonList* GetPlayerList();
    virtual void UpdatePlayer(int index, PlayerInfo* oldInfo, PlayerInfo* newInfo);
    virtual int GetPad();
    boost::shared_ptr<MultiplayerLocalPlayer> GetPlayer();
    ClientScoreboard* GetScoreboard();
    void registerTexture(int index);
    void UpdateStats(const StatsUID& uid);
    void SetStatIconTextureName(int index, const std::wstring& name);
    void SetTitleIconTextureName(int index, const std::wstring& name);
    void InitialiseScoreboard(INetworkPlayer* player);
    void SetHasPlayerBeenKilled(bool killed);
    bool CanKickPlayer(INetworkPlayer* player);
    void setScoreboardType(int type);
    void initNumColumns(int columns);
    void CheckReloadMovie();
    int getScoreboardType();
    void SetTitleSlotText(int index, const std::wstring& text);
    void ShowTitleSlot(int index, bool show);

private:
    EScoreboardType m_scoreboardType;
    bool m_statsPending;
    StatsUID m_selectedUid;
    unsigned char m_localPlayerCanModerate;
    unsigned char m_canKickPlayers;
    unsigned char m_canVoteToKick;
    IScoreboardLayout* m_scoreboardLayout;
    unsigned char m_initialisePending;
    unsigned char m_registeredTextures[119];
    int m_columnCount;
    UIControl m_awardPanel;
    UIControl m_mainPanel;
    UIControl_Progress m_killBar;
    UIControl_Label m_playerTitleLabel;
    UIControl_Label m_stat1Label;
    UIControl_Label m_stat2Label;
    UIControl_Label m_stat3Label;
    UIControl_Label m_stat4Label;
    UIControl_Label m_stat5Label;
    UIControl_Label m_killedByLabel;
    UIControl_Label m_gamertagLabel;
    UIControl_ScoreboardButtonList m_playerList;
};

#endif
