#ifndef GAME_CORE_LERPINGBOSSEVENT_HPP
#define GAME_CORE_LERPINGBOSSEVENT_HPP

#include <boost/shared_ptr.hpp>

#include "game/core/BossEvent.hpp"
#include "game/network/ClientboundBossEventPacket.hpp"

class LerpingBossEvent : public BossEvent
{
public:
    LerpingBossEvent(boost::shared_ptr<ClientboundBossEventPacket> packet);

    virtual float getPercent();
    virtual void setPercent(float percent);

    void update(boost::shared_ptr<ClientboundBossEventPacket> packet);

private:
    float targetPercent;
    long long setTime;
};

#endif
