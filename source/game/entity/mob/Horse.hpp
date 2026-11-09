#ifndef GAME_ENTITY_MOB_HORSE_HPP
#define GAME_ENTITY_MOB_HORSE_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class CompoundTag;
class Container;
class DamageSource;
class DataFixerUpper;
class EntityDataAccessor_Base;
class EntityTooltipDataHolder;
class HorseArmorItem;
class SoundEvent;
class SoundType;

class Horse : public AbstractHorse
{
public:
    static const EntityDataAccessor<int>* DATA_VARIANT;
    static const EntityDataAccessor<not_null_ptr<ItemInstance> >* DATA_ARMOR;

    class HorseGroupData : public MobGroupData
    {
    public:
        HorseGroupData(int variant);
        virtual ~HorseGroupData();
        int mVariant;
    };

    Horse(Level* level);
    virtual ~Horse();
    static Entity* create(Level* level);
    HorseArmorItem* getArmorType();
    virtual void containerChanged(Container* container);
    static void addDataWalkers(DataFixerUpper* fixer);
    static void staticCtor();
    virtual void defineSynchedData();
    int getVariant();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    void clearLayeredTextureInfo();
    void setVariant(int variant);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    void rebuildLayeredTextureInfo();
    std::wstring getLayeredTextureHashName();
    arrayWithLength<int> getLayeredTextureLayers();
    bool isWearingDyedArmor();
    int getDyedArmorColor();
    int getDyedArmorLayer();
    void setArmorType(const not_null_ptr<ItemInstance>& item);
    virtual void updateEquipment();
    virtual void onSyncedDataUpdated(const EntityDataAccessor_Base* accessor);
    virtual void playGallopSound(const SoundType* sound);
    virtual void registerAttributes();
    virtual void tick();
    virtual const SoundEvent* getAmbientSound();
    virtual const SoundEvent* getDeathSound();
    virtual const SoundEvent* getHurtSound(const DamageSource* source);
    virtual const SoundEvent* getAngrySound();
    virtual ResourceLocation* getDefaultLootTable();
    virtual bool mobInteract(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);
    virtual bool HideUseTooltips();
    virtual bool canMate(const boost::shared_ptr<Animal>& other);
    virtual boost::shared_ptr<AgableMob> getBreedOffspring(boost::shared_ptr<AgableMob> partner);
    virtual bool wearsArmor();
    virtual bool isArmor(const not_null_ptr<ItemInstance>& item);
    virtual MobGroupData* finalizeMobSpawn(const DifficultyInstance& difficulty, MobGroupData* data,
                                           const std::wstring& name);
    virtual bool removeWhenFarAway();
    virtual unsigned long long GetType();
    bool hasTexture();

private:
    static const int VARIANT_TEXTURES[7];
    static const std::wstring VARIANT_HASHES[7];
    static const int MARKING_TEXTURES[5];
    static const std::wstring MARKING_HASHES[5];
    static HorseArmorItem* ARMOR_NONE;

    std::wstring mLayeredTextureHashName;
    arrayWithLength<int> mLayeredTextureLayers;
    bool mLayeredTextureInfoBuilt;
};

#endif
