#ifndef GAME_SERVER_DERIVEDSERVERLEVEL_HPP
#define GAME_SERVER_DERIVEDSERVERLEVEL_HPP

#include "game/server/ServerLevel.hpp"
#include "game/network/BorderChangeListener.hpp"

class WorldBorder;

class DerivedServerLevel : public ServerLevel
{
public:
    class DerivedLevelBorderChangeListener : public BorderChangeListener
    {
    public:
        explicit DerivedLevelBorderChangeListener(DerivedServerLevel* level);
        virtual ~DerivedLevelBorderChangeListener();

        virtual void onBorderSizeSet(WorldBorder* border, double size);
        virtual void onBorderSizeLerping(WorldBorder* border, double oldSize, double newSize, long long timeMs);
        virtual void onBorderCenterSet(WorldBorder* border, double centerX, double centerZ);
        virtual void onBorderSetWarningTime(WorldBorder* border, int warningTime);
        virtual void onBorderSetWarningBlocks(WorldBorder* border, int warningBlocks);
        virtual void onBorderSetDamagePerBlock(WorldBorder* border, double damagePerBlock);
        virtual void onBorderSetDamageSafeZOne(WorldBorder* border, double damageSafeZone);

    private:
        DerivedServerLevel* mLevel;
    };

    DerivedServerLevel(MinecraftServer* server, boost::shared_ptr<LevelStorage> storage, int dimension,
                       ServerLevel* level);
    virtual ~DerivedServerLevel();

    virtual void saveLevelData();
    virtual Level* postConstruct();
    void saveDimensionData();

private:
    ServerLevel* mLevel;
    DerivedLevelBorderChangeListener* mBorderListener;
};

#endif
