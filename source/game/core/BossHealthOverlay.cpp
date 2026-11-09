#include "game/core/BossHealthOverlay.hpp"


#include "game/core/BossEvent.hpp"
#include "game/core/LerpingBossEvent.hpp"
#include "game/network/ClientboundBossEventPacket.hpp"

BossHealthOverlay::BossHealthOverlay(Minecraft* minecraft) : GuiComponent()
{
    mMinecraft = minecraft;
}

void BossHealthOverlay::update(boost::shared_ptr<ClientboundBossEventPacket> packet)
{
    if (packet->getOperation() == ClientboundBossEventPacket::ADD)
    {
        mBossEvents[packet->getId()] = new LerpingBossEvent(packet);
    }
    else if (packet->getOperation() == ClientboundBossEventPacket::REMOVE)
    {
        boost::unordered_map<std::wstring, LerpingBossEvent*>::const_iterator iterator = mBossEvents.find(
            packet->getId());
        if (iterator != mBossEvents.end())
        {
            mBossEvents.erase(iterator);
        }
    }
    else
    {
        if (mBossEvents.find(packet->getId()) != mBossEvents.end())
        {
            mBossEvents[packet->getId()]->update(packet);
        }
    }
}

void BossHealthOverlay::reset()
{
    mBossEvents.clear();
}

bool BossHealthOverlay::shouldPlayMusic()
{
    for (boost::unordered_map<std::wstring, LerpingBossEvent*>::iterator it = mBossEvents.begin();
         it != mBossEvents.end(); ++it)
    {
        LerpingBossEvent* event = it->second;
        if (event->shouldPlayBossMusic())
        {
            return true;
        }
    }
    return false;
}

bool BossHealthOverlay::shouldDarkenScreen()
{
    for (boost::unordered_map<std::wstring, LerpingBossEvent*>::iterator it = mBossEvents.begin();
         it != mBossEvents.end(); ++it)
    {
        LerpingBossEvent* event = it->second;
        if (event->shouldDarkenScreen())
        {
            return true;
        }
    }
    return false;
}

bool BossHealthOverlay::shouldCreateWorldFog()
{
    for (boost::unordered_map<std::wstring, LerpingBossEvent*>::iterator it = mBossEvents.begin();
         it != mBossEvents.end(); ++it)
    {
        LerpingBossEvent* event = it->second;
        if (event->shouldCreateWorldFog())
        {
            return true;
        }
    }
    return false;
}
