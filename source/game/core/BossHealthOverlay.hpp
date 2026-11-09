#ifndef GAME_CORE_BOSSHEALTHOVERLAY_HPP
#define GAME_CORE_BOSSHEALTHOVERLAY_HPP

#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

#include "game/core/GuiComponent.hpp"

class BossEvent;
class ClientboundBossEventPacket;
class LerpingBossEvent;
class Minecraft;

class BossHealthOverlay : public GuiComponent
{
    friend class UIScene_HUD;

public:
    BossHealthOverlay(Minecraft* minecraft);

    void update(boost::shared_ptr<ClientboundBossEventPacket> packet);
    void reset();
    bool shouldPlayMusic();
    bool shouldDarkenScreen();
    bool shouldCreateWorldFog();
    void render();
    void drawBar(int x, int y, BossEvent* bossEvent);
    bool hasBoss();

private:
    Minecraft* mMinecraft;
    boost::unordered_map<std::wstring, LerpingBossEvent*> mBossEvents;
};

#endif
