#include "game/core/BossEvent.hpp"

BossEvent::BossEvent(const std::wstring& id, int nameId, const std::wstring& name, BossBarColor color,
                     BossBarOverlay overlay)
{
    this->id = id;
    this->nameId = nameId;
    this->name = name;
    this->percent = 1.0f;
    this->color = color;
    this->overlay = overlay;
}

std::wstring BossEvent::getId()
{
    return id;
}

std::wstring BossEvent::getName()
{
    return name;
}

int BossEvent::getNameId()
{
    return nameId;
}

void BossEvent::setName(const std::wstring& name)
{
    this->name = name;
}

float BossEvent::getPercent()
{
    return percent;
}

void BossEvent::setPercent(float percent)
{
    this->percent = percent;
}

BossEvent::BossBarColor BossEvent::getColor()
{
    return color;
}

void BossEvent::setColor(BossBarColor color)
{
    this->color = color;
}

BossEvent::BossBarOverlay BossEvent::getOverlay()
{
    return overlay;
}

void BossEvent::setOverlay(BossBarOverlay overlay)
{
    this->overlay = overlay;
}

bool BossEvent::shouldDarkenScreen()
{
    return darkenScreen;
}

BossEvent* BossEvent::setDarkenScreen(bool darkenScreen)
{
    this->darkenScreen = darkenScreen;
    return this;
}

bool BossEvent::shouldPlayBossMusic()
{
    return playBossMusic;
}

BossEvent* BossEvent::setPlayBossMusic(bool playBossMusic)
{
    this->playBossMusic = playBossMusic;
    return this;
}

BossEvent* BossEvent::setCreateWorldFog(bool createWorldFog)
{
    this->createWorldFog = createWorldFog;
    return this;
}

bool BossEvent::shouldCreateWorldFog()
{
    return createWorldFog;
}
