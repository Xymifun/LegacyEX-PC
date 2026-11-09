#ifndef GAME_ITEM_BUCKETITEM_HPP
#define GAME_ITEM_BUCKETITEM_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/InteractionHand.hpp"
#include "game/core/InteractionResultHolder.hpp"
#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"

class Block;
class BlockPos;
class Direction;
class Entity;
class HitResult;
class ItemInstance;
class ItemTooltipDataHolder;
class Level;
class LivingEntity;
class Player;

class BucketItem : public Item
{
public:
    enum BucketFillType
    {
        NONE = -1,
        EMPTY = 0,
        MILK = 1,
        FISH = 2,
        SALMON = 3,
        TROPICAL = 4,
        PUFFER = 5,
        WATER = 8,
        LAVA = 10
    };

    explicit BucketItem(BucketFillType fillType);

    static int getSchoolName(int a, int b, int c, int d);
    bool isFishBucket();
    bool supportsEntityType(const unsigned long long& type, int& variant) const;
    void addBucketEntitySaveData(boost::shared_ptr<Entity> entity, not_null_ptr<ItemInstance> itemInstance);
    not_null_ptr<ItemInstance> createResultItem(not_null_ptr<ItemInstance> itemInstance,
                                                boost::shared_ptr<Player> player, Item* item,
                                                boost::shared_ptr<Entity> entity);
    not_null_ptr<ItemInstance> collectFish(boost::shared_ptr<Player> player, boost::shared_ptr<Entity> entity,
                                           boost::shared_ptr<ItemInstance> itemInstance);
    InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos,
                                                InteractionHand::EInteractionHand hand, const Direction* direction,
                                                float hitX, float hitY, float hitZ, bool simulate);
    HitResult* ScanForHitResultEntities(boost::shared_ptr<Player> player, Level* level, const HitResult* result);
    HitResult* GetPlayerHitResult(boost::shared_ptr<Player> player, Level* level, bool useLiquids);
    bool TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand);
    bool isFishBucketNameCensored(const std::wstring& name, const std::wstring& owner, bool restricted, bool host);
    bool readBucketEntitySaveData(boost::shared_ptr<Entity> entity, not_null_ptr<ItemInstance> itemInstance);
    bool emptyBucket(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos,
                     not_null_ptr<ItemInstance> itemInstance);
    InteractionResultHolder use(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand);
    not_null_ptr<ItemInstance> finishUsingItem(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                               boost::shared_ptr<LivingEntity> entity);
    int getUseDuration(not_null_ptr<ItemInstance> itemInstance);
    int getUseAnimation(not_null_ptr<const ItemInstance> itemInstance) const;
    unsigned int GetUseTooltip(const ItemTooltipDataHolder& tooltipData);
    std::wstring getName(not_null_ptr<ItemInstance> itemInstance);

private:
    Block* mBlock;
    BucketFillType mFillType;
};

#endif
