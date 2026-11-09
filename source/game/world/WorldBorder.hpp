#ifndef GAME_WORLD_WORLDBORDER_HPP
#define GAME_WORLD_WORLDBORDER_HPP

#include <vector>
#include <boost/shared_ptr.hpp>
#include "game/world/chunk/ChunkPos.hpp"

class AABB;
class BorderStatus;
class BlockPos;
class BorderChangeListener;
class ChunkPos;
class Entity;
class Level;

class WorldBorder
{
private:
    std::vector<BorderChangeListener*> m_listeners;
    double m_centerX;
    double m_centerZ;
    double m_size;
    double m_lerpTarget;
    long long m_lerpEnd;
    long long m_lerpStart;
    int m_absoluteMaxSize;
    double m_damagePerBlock;
    double m_damageSafeZone;
    int m_warningTime;
    int m_warningBlocks;

public:
    static BorderStatus* GROWING;
    static BorderStatus* SHRINKING;
    static BorderStatus* STATIONARY;

    explicit WorldBorder(Level* level);
    virtual ~WorldBorder();
    BorderStatus* getStatus();
    std::vector<BorderChangeListener*>& getListeners();
    void setSize(double size);
    double getSize();
    double getMinX();
    double getMaxX();
    double getMinZ();
    double getMaxZ();
    bool isWithinBounds(const BlockPos& pos);
    bool isWithinBounds(AABB* box);
    double getDistanceToBorder(double x, double z);
    double getDistanceToBorder(boost::shared_ptr<Entity> entity);
    virtual double getCenterX();
    virtual double getCenterZ();
    void setCenter(double x, double z);
    long long getLerpRemainingTime();
    double getLerpTarget();
    void lerpSizeBetween(double oldSize, double newSize, long long timeMs);
    void addListener(BorderChangeListener* listener);
    void setAbsoluteMaxSize(int size);
    int getAbsoluteMaxSize();
    double getDamageSafeZone();
    void setDamageSafeZone(double distance);
    double getDamagePerBlock();
    void setDamagePerBlock(double damage);
    int getWarningTime();
    void setWarningTime(int time);
    int getWarningBlocks();
    void setWarningBlocks(int blocks);
    bool isWithinBounds(ChunkPos* pos);
    bool isWithinBounds(double x, double z);
    void removeListener(BorderChangeListener* listener);
    long long getLerpSpeed();
};

#endif
