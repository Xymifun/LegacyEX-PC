#ifndef GAME_NETWORK_BORDERCHANGELISTENER_HPP
#define GAME_NETWORK_BORDERCHANGELISTENER_HPP

class WorldBorder;

class BorderChangeListener
{
public:
    virtual ~BorderChangeListener() {}

    virtual void onBorderSizeSet(WorldBorder* border, double size) = 0;
    virtual void onBorderSizeLerping(WorldBorder* border, double oldSize, double newSize, long long timeMs) = 0;
    virtual void onBorderCenterSet(WorldBorder* border, double centerX, double centerZ) = 0;
    virtual void onBorderSetWarningTime(WorldBorder* border, int warningTime) = 0;
    virtual void onBorderSetWarningBlocks(WorldBorder* border, int warningBlocks) = 0;
    virtual void onBorderSetDamagePerBlock(WorldBorder* border, double damagePerBlock) = 0;
    virtual void onBorderSetDamageSafeZOne(WorldBorder* border, double damageSafeZone) = 0;
};

#endif
