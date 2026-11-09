#ifndef GAME_CORE_ARMORSTAND_HPP
#define GAME_CORE_ARMORSTAND_HPP

#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/Predicate.hpp"

class BlockPos;
class CompoundTag;
class DamageSource;
class DataFixerUpper;
class Direction;
class EntityTooltipDataHolder;
class EquipmentSlot;
class Level;
class LightningBolt;
class Player;
class HumanoidArm;

class ArmorStand : public LivingEntity
{
public:
    enum EInteraction
    {
        eInteraction_None,
        eInteraction_Place,
        eInteraction_Take,
        eInteraction_Swap,
        eInteraction_ChangePose
    };

    class Pose
    {
    public:
        Pose(const Vec3* head, const Vec3* body, const Vec3* rightArm, const Vec3* leftArm, const Vec3* rightLeg,
             const Vec3* leftLeg, const Vec3* rightItem);
        Pose();

        Vec3* head;
        Vec3* body;
        Vec3* rightArm;
        Vec3* leftArm;
        Vec3* rightLeg;
        Vec3* leftLeg;
        Vec3* rightItem;
    };

    class RidableMinecartEntitySelector : public Predicate<boost::shared_ptr<Entity> >
    {
    public:
        virtual bool apply(boost::shared_ptr<Entity> entity) const;
        virtual ~RidableMinecartEntitySelector();
    };

    ArmorStand(Level* level);
    ArmorStand(Level* level, double x, double y, double z);
    virtual ~ArmorStand();

    static void staticCtor();
    static const EntityDataAccessor<unsigned char>* DATA_FLAGS;
    static const EntityDataAccessor<int>* DATA_POSE;
    static const RidableMinecartEntitySelector* IS_RIDEABLE_MINECART;
    static void addDataWalkers(DataFixerUpper* fixer);
    static Entity* create(Level* level);
    void _init();

    unsigned char setBit(unsigned char value, int bit, bool enabled);
    void setShowArms(bool showArms);
    NonNullList<not_null_ptr<ItemInstance> > getHandSlots();
    NonNullList<not_null_ptr<ItemInstance> > getArmorSlots();
    not_null_ptr<ItemInstance> getItemSlot(const EquipmentSlot* slot);
    virtual void setItemSlot(const EquipmentSlot* slot, not_null_ptr<ItemInstance> item);
    bool setSlot(int slot, const not_null_ptr<ItemInstance>& item);
    virtual bool isEffectiveAi();
    bool isSmall();
    bool isNoBasePlate();
    bool isMarker();
    int getPoseIndex();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    void setSmall(bool small);
    void setNoBasePlate(bool noBasePlate);
    void setMarker(bool marker);
    void setPoseIndex(int pose);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual bool isPushable(boost::shared_ptr<Entity> entity);
    const EquipmentSlot* getClickedSlot(const Vec3* hit);
    bool isDisabled(const EquipmentSlot* slot);
    void swapItem(const boost::shared_ptr<Player>& player, const EquipmentSlot* slot,
                  const not_null_ptr<ItemInstance>& item, InteractionHand::EInteractionHand hand);
    bool isShowArms();
    InteractionResult::EInteractionResult interactAt(const boost::shared_ptr<Player>& player, Vec3* hit,
                                                     InteractionHand::EInteractionHand hand);
    EInteraction test_interactAt(boost::shared_ptr<Player> player, Vec3* hit, not_null_ptr<ItemInstance> item,
                                 InteractionHand::EInteractionHand hand);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual unsigned int GetHitTooltip(const EntityTooltipDataHolder& tooltipData);
    void playBrokenSound();
    void brokenByAnything();
    void causeDamage(float damage);
    void showBreakingParticles();
    void brokenByPlayer();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual void handleEntityEvent(unsigned char event, int data);
    virtual bool shouldRenderAtSqrDistance(double distanceSqr);
    virtual void setInvisible(bool invisible);
    virtual bool isBaby();
    virtual void kill();
    bool ignoreExplosion();
    virtual int getPistonPushReaction();
    virtual bool isAffectedByPotions();
    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    virtual bool attackable();
    Vec3* getHeadPose();
    Vec3* getBodyPose();
    Vec3* getLeftArmPose();
    Vec3* getRightArmPose();
    Vec3* getLeftLegPose();
    Vec3* getRightLegPose();
    Vec3* getRightItemPose();
    virtual bool isPickable();
    virtual const HumanoidArm* getMainArm();
    virtual float tickHeadTurn(float bodyRot, float headTurnDelta);
    virtual double getRidingHeight();
    virtual float getEyeHeight();
    virtual void travel(float xxa, float yya, float zza);
    virtual void setYBodyRot(float bodyRot);
    virtual void setYHeadRot(float headRot);
    bool updatePoseForSignal(const BlockPos& pos, const Direction* direction);
    void updateBoundingBox(bool marker);
    virtual void tick();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getFallDamageSound(int distance);
    virtual void setSize(float width, float height);
    virtual void defineSynchedData();
    virtual void doPush(boost::shared_ptr<Entity> entity);
    virtual void pushEntities();
    virtual void updateInvisibilityStatus();
    virtual bool generatesBubbles();
    virtual void setAirSupply(int airSupply);
    virtual unsigned long long GetType();
    virtual void thunderHit(const LightningBolt* lightningBolt);

    long long mLastHit;

private:
    NonNullList<not_null_ptr<ItemInstance> > mHandItems;
    NonNullList<not_null_ptr<ItemInstance> > mArmorItems;
    bool mInvisible;
    int mDisabledSlots;
    bool mWasMarker;
    int mLastSignal;
};

#endif
