#ifndef GAME_SERVER_SERVERBOSSEVENT_HPP
#define GAME_SERVER_SERVERBOSSEVENT_HPP

#include <map>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/BossEvent.hpp"
#include "game/network/ClientboundBossEventPacket.hpp"

class ServerPlayer;

class ServerBossEvent : public BossEvent
{
public:
    ServerBossEvent(int nameId, const std::wstring& name, BossBarColor color, BossBarOverlay overlay);

    void broadcast(ClientboundBossEventPacket::Operation operation);

    virtual void setName(const std::wstring& name);
    virtual void setPercent(float percent);
    virtual void setColor(BossBarColor color);
    virtual void setOverlay(BossBarOverlay overlay);
    virtual BossEvent* setDarkenScreen(bool darkenScreen);
    virtual BossEvent* setPlayBossMusic(bool playBossMusic);
    virtual BossEvent* setCreateWorldFog(bool createWorldFog);

    void addPlayer(boost::shared_ptr<ServerPlayer> player, int party);
    void removePlayer(boost::shared_ptr<ServerPlayer> player);
    void removeAllPlayers();
    void setVisible(bool visible);
    std::vector<boost::shared_ptr<ServerPlayer> >* getPlayers();
    std::map<boost::shared_ptr<ServerPlayer>, int>& getPartyPlayers();
    bool isVisible();

private:
    std::map<boost::shared_ptr<ServerPlayer>, int> players;
    bool visible;
};

#endif
