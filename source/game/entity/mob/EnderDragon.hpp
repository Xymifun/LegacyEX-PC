#ifndef GAME_ENTITY_MOB_ENDERDRAGON_HPP
#define GAME_ENTITY_MOB_ENDERDRAGON_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/BossMob.hpp"
#include "game/entity/mob/MultiEntityMob.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/entity/mob/Enemy.hpp"

class AABB;
class AreaEffectCloud;
class BinaryHeap;
class BlockPos;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class EndDragonFight;
class EnderCrystal;
class EntityTooltipDataHolder;
class Level;
struct Node;
class Path;
class MultiEntityMobPart;
class SoundEvent;
class Vec3;

class EnderDragon : public Mob, public BossMob, public MultiEntityMob, public Enemy
{
public:
    enum EEnderdragonAction
    {
        ACTION_HOLDING_PATTERN = 0,
        ACTION_STRAFE_PLAYER = 1,
        ACTION_LANDING_APPROACH = 2,
        ACTION_LANDING = 3,
        ACTION_TAKEOFF = 4,
        ACTION_SITTING_FLAMING = 5,
        ACTION_SITTING_SCANNING = 6,
        ACTION_SITTING_ATTACKING = 7
    };

    static const int POSITION_HISTORY_SIZE = 64;
    static const int PATH_NODE_COUNT = 24;
    static const int OUTER_RING_NODE_COUNT = 12;

    EnderDragon(Level* level);
    virtual ~EnderDragon();
    virtual std::wstring getDisplayName();
    virtual float getMaxHealth();
    virtual float getHealth();
    virtual bool hurt(boost::shared_ptr<MultiEntityMobPart> part, const DamageSource* source, float amount);
    virtual void reallyHurt(const DamageSource* source, float amount);
    virtual Level* getLevel();
    virtual void GetInteractTooltip(boost::shared_ptr<MultiEntityMobPart> part, const EntityTooltipDataHolder& holder);
    virtual int getDimension();
    unsigned int getSynchedAction();
    bool setSynchedAction(EEnderdragonAction action, bool force);
    unsigned int findClosestNode(double x, double y, double z);
    unsigned int findClosestNode();
    double getHeadYRotDiff(float partialTicks);
    Vec3* getHeadLookVector(float partialTicks);
    Path* reconstruct_path(Node* start, Node* end);
    Path* findPath(int start, int end, Node* target);
    void navigateToNextPathNode();
    void strafeAttackTarget();
    void findNewTarget();
    void AddParts();
    void onCrystalDestroyed(boost::shared_ptr<EnderCrystal> crystal, const BlockPos& pos, const DamageSource* source);
    void _init();
    static void addDataWalkers(DataFixerUpper* fixer);
    static void staticCtor();
    virtual void registerAttributes();
    virtual void defineSynchedData();
    bool canRide(boost::shared_ptr<Entity> entity);
    void getLatencyPos(arrayWithLength<double> positions, int index, float partialTicks);
    float getTilt(float partialTicks);
    double getHeadYOffset(float partialTicks);
    double rotWrap(double angle);
    void checkCrystals();
    void knockBack(std::vector<boost::shared_ptr<Entity> >* entities);
    void hurt(std::vector<boost::shared_ptr<Entity> >* entities);
    bool checkWalls(AABB* box);
    virtual void aiStep();
    virtual ResourceLocation* getDefaultLootTable();
    virtual bool hurt(const DamageSource* source, float amount);
    virtual void kill();
    void dropExperience(int experience);
    virtual void tickDeath();
    virtual void checkDespawn();
    virtual std::vector<boost::shared_ptr<Entity> >* getSubEntities();
    virtual bool isPickable();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual float getSoundVolume();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    double getHeadPartYOffset(int part, arrayWithLength<double> first, arrayWithLength<double> second);
    double getHeadPartYRotDiff(int part, arrayWithLength<double> first, arrayWithLength<double> second);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    virtual unsigned long long GetType();
    static Entity* create(Level* level);
    bool checkAttack();

    double mXTarget;
    double mYTarget;
    double mZTarget;
    double mPositions[POSITION_HISTORY_SIZE][3];
    int mPosPointer;
    std::vector<boost::shared_ptr<Entity> > mSubEntities;
    boost::shared_ptr<MultiEntityMobPart> mHead;
    boost::shared_ptr<MultiEntityMobPart> mNeck;
    boost::shared_ptr<MultiEntityMobPart> mBody;
    boost::shared_ptr<MultiEntityMobPart> mTail1;
    boost::shared_ptr<MultiEntityMobPart> mTail2;
    boost::shared_ptr<MultiEntityMobPart> mTail3;
    boost::shared_ptr<MultiEntityMobPart> mWing1;
    boost::shared_ptr<MultiEntityMobPart> mWing2;
    float mOldFlapTime;
    float mFlapTime;
    bool mNewTarget;
    bool mInWall;
    int mFireballCharge;
    float mUnusedFloat;
    bool mClockwise;
    int mActionTicks;
    int mSittingDamageReceived;
    int mFlameCount;
    int mGrowlTime;
    double mClientLerpReset;
    arrayWithLength<Node*>* mNodes;
    int mNodeAdjacency[PATH_NODE_COUNT];
    BinaryHeap* mOpenSet;
    Path* mPath;
    boost::shared_ptr<Entity> mAttackTarget;
    boost::shared_ptr<AreaEffectCloud> mFlameCloud;
    int mDeathTime;
    boost::shared_ptr<EnderCrystal> mNearestCrystal;
    EndDragonFight* mDragonFight;
};

#endif
