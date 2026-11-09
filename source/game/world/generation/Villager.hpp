#ifndef GAME_WORLD_GENERATION_VILLAGER_HPP
#define GAME_WORLD_GENERATION_VILLAGER_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include "game/core/MapDecoration.hpp"
#include "game/core/Merchant.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/Npc.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"

class CompoundTag;
class DataFixerUpper;
class DamageSource;
class DifficultyInstance;
class EntityTooltipDataHolder;
class Item;
class ItemEntity;
class Level;
class LightningBolt;
class LivingEntity;
class MerchantRecipe;
class MerchantRecipeList;
class MobGroupData;
class ParticleType;
class Player;
class Random;
class SimpleContainer;
class SoundEvent;
class Village;

template <class T> class EntityDataAccessor;

class Villager : public AgableMob, public Npc, public Merchant
{
public:
    class PriceRange
    {
    public:
        PriceRange();
        PriceRange(int min, int max);
        int getValue(Random* random);

    private:
        int mMin;
        int mMax;
    };

    class ItemListing
    {
    public:
        virtual void addMerchantRecipe(boost::shared_ptr<Merchant> merchant, MerchantRecipeList* recipes,
                                       Random* random) = 0;
    };

    class EmeraldForItems : public ItemListing
    {
    public:
        EmeraldForItems(Item* item, PriceRange price);
        virtual void addMerchantRecipe(boost::shared_ptr<Merchant> merchant, MerchantRecipeList* recipes,
                                       Random* random);

    private:
        Item* mItem;
        PriceRange mPrice;
    };

    class ItemForEmeralds : public ItemListing
    {
    public:
        ItemForEmeralds(Item* item, const PriceRange& price);
        ItemForEmeralds(const ItemInstance& item, const PriceRange& price);
        virtual void addMerchantRecipe(boost::shared_ptr<Merchant> merchant, MerchantRecipeList* recipes,
                                       Random* random);

    private:
        ItemInstance mItem;
        PriceRange mPrice;
    };

    class EnchantedItemForEmeralds : public ItemListing
    {
    public:
        EnchantedItemForEmeralds(Item* item, const PriceRange& price);
        virtual void addMerchantRecipe(boost::shared_ptr<Merchant> merchant, MerchantRecipeList* recipes,
                                       Random* random);

    private:
        ItemInstance mItem;
        PriceRange mPrice;
    };

    class ItemAndEmeraldToItem : public ItemListing
    {
    public:
        ItemAndEmeraldToItem(Item* firstItem, const PriceRange& firstPrice, Item* secondItem,
                             const PriceRange& secondPrice);
        virtual void addMerchantRecipe(boost::shared_ptr<Merchant> merchant, MerchantRecipeList* recipes,
                                       Random* random);

    private:
        ItemInstance mFirstItem;
        PriceRange mFirstPrice;
        ItemInstance mSecondItem;
        PriceRange mSecondPrice;
    };

    class EnchantBookForEmeralds : public ItemListing
    {
    public:
        EnchantBookForEmeralds();
        virtual void addMerchantRecipe(boost::shared_ptr<Merchant> merchant, MerchantRecipeList* recipes,
                                       Random* random);
    };

    class TreasureMapForEmeralds : public ItemListing
    {
    public:
        TreasureMapForEmeralds(const PriceRange& price, std::wstring name, const MapDecoration::Type* icon);
        virtual void addMerchantRecipe(boost::shared_ptr<Merchant> merchant, MerchantRecipeList* recipes,
                                       Random* random);

    private:
        PriceRange mPrice;
        std::wstring mName;
        const MapDecoration::Type* mIcon;
    };

    Villager(Level* level);
    Villager(Level* level, int profession);
    virtual ~Villager();

    static Entity* create(Level* level);
    static void staticCtor();
    static void addDataWalkers(DataFixerUpper* fixer);

    void _init(int profession);

    int getProfession();
    void setProfession(int profession);
    unsigned int getCareer();
    void setCareer(int career);
    unsigned int getTrading();
    void setTrading(int trading);
    bool isTrading();
    void increaseMerchantCareer();
    void setRewardPlayersInVillage();
    bool countFood(int multiplier);
    bool hasExcessFood();
    bool wantsMoreFood();
    bool hasEnoughFoodToBreed();
    bool isWillingToBreed(bool consumeFood);
    void setWillingToBreed(bool willing);
    bool isInLove();
    void setInLove(bool inLove);
    bool isChasing();
    void setChasing(bool chasing);
    bool wantsItem(Item* item);
    bool hasFarmSeeds();
    SimpleContainer* getInventory();
    void addParticlesAroundSelf(const ParticleType* particle);
    MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data, bool randomProfession,
                                   const std::wstring& dataString);

    virtual unsigned long long GetType();
    virtual void defineSynchedData();
    virtual bool hurt(const DamageSource* source, float damage);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& holder);
    virtual bool HideUseTooltips();
    virtual void handleEntityEvent(unsigned char event, int data);
    virtual void thunderHit(const LightningBolt* lightningBolt);
    virtual std::wstring getDisplayName();
    virtual float getEyeHeight();
    virtual bool setSlot(int slot, const not_null_ptr<ItemInstance>& item);
    virtual bool canBeLeashed(const boost::shared_ptr<Player>& player);
    virtual void registerAttributes();
    virtual void setLastHurtByMob(boost::shared_ptr<LivingEntity> attacker);
    virtual void die(const DamageSource* source);
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getDeathSound();
    virtual void registerGoals();
    virtual const SoundEvent* getAmbientSound();
    virtual ResourceLocation* getDefaultLootTable();
    virtual void pickUpItem(boost::shared_ptr<ItemEntity> itemEntity);
    virtual bool removeWhenFarAway();
    virtual void customServerAiStep();
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& dataString);
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    virtual void grewUp();
    virtual void loadPersonalAiGoals();

    virtual void setTradingPlayer(const boost::shared_ptr<Player>& player);
    virtual boost::shared_ptr<Player> getTradingPlayer();
    virtual void notifyTrade(MerchantRecipe* recipe);
    virtual void notifyTradeUpdated(const not_null_ptr<ItemInstance>& item);
    virtual MerchantRecipeList* getOffers(const boost::shared_ptr<Player>& player);
    virtual Level* getLevel();
    virtual BlockPos getPosition();
    virtual void overrideOffers(MerchantRecipeList* offers);
    virtual bool isAvailableToTrade();

private:
    static const EntityDataAccessor<int>* DATA_PROFESSION;
    static const EntityDataAccessor<int>* DATA_CAREER;
    static const EntityDataAccessor<int>* DATA_TRADING;
    static std::vector<std::vector<std::vector<arrayWithLength<ItemListing*> > > > sTrades;

    int mRandomTickDivider;
    bool mInLove;
    bool mChasing;
    boost::weak_ptr<Village> mVillage;
    boost::weak_ptr<Player> mTradingPlayer;
    MerchantRecipeList* mOffers;
    int mTimeUntilReset;
    bool mNeedsInitialization;
    bool mWillingToBreed;
    int mRiches;
    std::wstring mLastTradedPlayerName;
    int mCareerLevel;
    bool mRewardPlayersInVillage;
    bool mAdditionalTasksSet;
    SimpleContainer* mInventory;
};

#endif
