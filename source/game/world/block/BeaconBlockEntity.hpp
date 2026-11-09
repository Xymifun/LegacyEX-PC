#ifndef GAME_WORLD_BLOCK_BEACONBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_BEACONBLOCKENTITY_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/core/CensoredString.hpp"
#include "game/core/Direction.hpp"
#include "game/core/InteractionObject.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/Container.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/WorldlyContainer.hpp"
#include "game/world/block/LockableContainerBlockEntity.hpp"

class AbstractContainerMenu;
class Inventory;
class Level;
class MobEffect;

class BeaconBlockEntity : public LockableContainerBlockEntity, public WorldlyContainer
{
public:
    enum eBEAM_DIRECTION
    {
        eBEAM_DIRECTION_NONE = 0,
        eBEAM_DIRECTION_POSITIVE_Y = 1,
        eBEAM_DIRECTION_NEGATIVE_Y = 2,
        eBEAM_DIRECTION_POSITIVE_X = 3,
        eBEAM_DIRECTION_NEGATIVE_X = 4,
        eBEAM_DIRECTION_POSITIVE_Z = 5,
        eBEAM_DIRECTION_NEGATIVE_Z = 6
    };

    class BeaconBeamSection
    {
    public:
        BeaconBeamSection(arrayWithLength<float> color);
        float* getColor();
        void setHeight(int height);
        void increaseHeight();
        unsigned int getHeight();

        float mRed;
        float mGreen;
        float mBlue;
        int mHeight;
    };

    BeaconBlockEntity();
    virtual ~BeaconBlockEntity();
    virtual unsigned long long GetType();

    virtual unsigned int getContainerSize();
    virtual bool isEmpty();
    virtual not_null_ptr<ItemInstance> getItem(unsigned int slot);
    virtual not_null_ptr<ItemInstance> removeItem(unsigned int slot, int count);
    virtual not_null_ptr<ItemInstance> removeItemNoUpdate(int slot);
    virtual void setItem(unsigned int slot, not_null_ptr<ItemInstance> item);
    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual bool hasCustomName();
    virtual int getMaxStackSize();
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual void startOpen(boost::shared_ptr<Player> player);
    virtual void stopOpen(boost::shared_ptr<Player> player);
    virtual bool canPlaceItem(int slot, not_null_ptr<ItemInstance> item);
    virtual int getTypeId(bool large);
    virtual AbstractContainerMenu* createMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Player> player);
    virtual int getContainerData(int id);
    virtual void setContainerData(int id, int value);
    virtual int getContainerDataCount();
    virtual void clearContent();
    virtual arrayWithLength<int> getSlotsForFace(const Direction* face);
    virtual bool canPlaceItemThroughFace(int slot, not_null_ptr<ItemInstance> item, const Direction* face);
    virtual bool canTakeItemThroughFace(int slot, not_null_ptr<ItemInstance> item, const Direction* face);
    virtual InteractionObject::NameData GetNetworkNameData();

    virtual void setChanged();
    MobEffect* getValidEffectById(int id);
    void setCustomName(const std::wstring& name, const std::wstring& uuid, bool chatCensored, bool uuidLocal);
    int getBeamDirection();
    void updateShapeForGlide();
    bool NeedsRebuild();
    void updateShape();
    void applyEffects();
    void checkValidity();
    virtual boost::shared_ptr<BlockEntity> clone();
    static MobEffect* sBeaconEffects[4][3];
    static void AddEffect(int level, int slot, MobEffect* effect);
    static void staticCtor();
    virtual void tick();
    std::vector<BeaconBeamSection>* getBeamSections();
    float getAndUpdateClientSideScale();
    virtual boost::shared_ptr<Packet> getUpdatePacket();
    virtual CompoundTag* getUpdateTag();
    virtual double getViewDistance();
    virtual void load(CompoundTag* tag);
    virtual CompoundTag* save(CompoundTag* tag);
    virtual bool triggerEvent(int id, int param);
    void setBeamDirection(eBEAM_DIRECTION direction);
    void setBeamLength(int length);
    virtual std::wstring getDisplayName();
    static BlockEntity* create();

    MobEffect* getPrimaryPower();
    MobEffect* getSecondaryPower();
    int getLevels();
    void setLevels(int levels);
    void setPrimaryPower(MobEffect* effect);
    void setSecondaryPower(MobEffect* effect);

private:
    std::vector<BeaconBeamSection> mBeamSections;
    short mLastBeamBlockCount;
    short mBeamColorCounts[16];
    unsigned short mBeamRebuildTicks;
    long long mNextClientSideUpdate;
    float mClientSideScale;
    bool mActive;
    int mLevels;
    MobEffect* mPrimaryPower;
    MobEffect* mSecondaryPower;
    not_null_ptr<ItemInstance> mItem;
    boost::shared_ptr<CensoredString> mName;
    unsigned int mBeamDirection;
    int mBeamLength;
    int mTickCount;
};

#endif
