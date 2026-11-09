#include "game/core/LerpingBossEvent.hpp"

#include "game/core/Mth.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/ClientboundBossEventPacket.hpp"

LerpingBossEvent::LerpingBossEvent(boost::shared_ptr<ClientboundBossEventPacket> packet)
    : BossEvent(packet->getId(), packet->getNameId(), packet->getName(), packet->getColor(), packet->getOverlay())
{
    this->targetPercent = packet->getPercent();
    this->percent = packet->getPercent();
    this->setTime = Minecraft::currentTimeMillis();
    setDarkenScreen(packet->shouldDarkenScreen());
    setPlayBossMusic(packet->shouldPlayMusic());
    setCreateWorldFog(packet->shouldCreateWorldFog());
}

float LerpingBossEvent::getPercent()
{
    long long elapsed = Minecraft::currentTimeMillis() - this->setTime;
    float t = Mth::clamp((float)elapsed / 100.0f, 0.0f, 1.0f);
    return this->percent + (this->targetPercent - this->percent) * t;
}

void LerpingBossEvent::setPercent(float percent)
{
    this->percent = getPercent();
    this->targetPercent = percent;
    this->setTime = Minecraft::currentTimeMillis();
}

void LerpingBossEvent::update(boost::shared_ptr<ClientboundBossEventPacket> packet)
{
    switch (packet->getOperation())
    {
    case ClientboundBossEventPacket::UPDATE_NAME:
        setName(packet->getName());
        break;

    case ClientboundBossEventPacket::UPDATE_PROGRESS:
        setPercent(packet->getPercent());
        break;

    case ClientboundBossEventPacket::UPDATE_STYLE:
        setColor(packet->getColor());
        setOverlay(packet->getOverlay());
        break;

    case ClientboundBossEventPacket::UPDATE_PROPERTIES:
        setDarkenScreen(packet->shouldDarkenScreen());
        setPlayBossMusic(packet->shouldPlayMusic());
        break;

    default:
        break;
    }
}
