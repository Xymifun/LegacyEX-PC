#ifndef GAME_CORE_BOSSEVENT_HPP
#define GAME_CORE_BOSSEVENT_HPP

#include <string>

class BossEvent
{
public:
    enum BossBarColor
    {
        PINK,
        BLUE,
        RED,
        GREEN,
        YELLOW,
        PURPLE,
        WHITE
    };

    enum BossBarOverlay
    {
        PROGRESS,
        NOTCHED_6,
        NOTCHED_10,
        NOTCHED_12,
        NOTCHED_20
    };

    BossEvent(const std::wstring& id, int nameId, const std::wstring& name, BossBarColor color, BossBarOverlay overlay);
    virtual ~BossEvent() {}

    std::wstring getId();

    std::wstring getName();
    int getNameId();
    virtual void setName(const std::wstring& name);

    virtual float getPercent();
    virtual void setPercent(float percent);

    BossBarColor getColor();
    virtual void setColor(BossBarColor color);

    BossBarOverlay getOverlay();
    virtual void setOverlay(BossBarOverlay overlay);

    bool shouldDarkenScreen();
    virtual BossEvent* setDarkenScreen(bool darkenScreen);

    bool shouldPlayBossMusic();
    virtual BossEvent* setPlayBossMusic(bool playBossMusic);

    virtual BossEvent* setCreateWorldFog(bool createWorldFog);
    bool shouldCreateWorldFog();

protected:
    std::wstring id;
    std::wstring name;
    int nameId;
    float percent;
    BossBarColor color;
    BossBarOverlay overlay;
    bool darkenScreen;
    bool playBossMusic;
    bool createWorldFog;
};

#endif
