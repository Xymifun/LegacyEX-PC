#ifndef GAME_ITEM_BANNERITEM_HPP
#define GAME_ITEM_BANNERITEM_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BlockItem.hpp"

class BannerPattern;
class BlockPos;
class CompoundTag;
class Direction;
class DyeColor;
class Level;
class ListTag;
class Player;
class ItemInstance;

class BannerItem : public BlockItem
{
public:
    BannerItem();
    virtual ~BannerItem();

    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float hitX, float hitY, float hitZ,
                                                        bool simulate);
    virtual int getColor(not_null_ptr<ItemInstance> itemInstance, int layer);
    virtual void appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool advanced);
    virtual std::wstring getName(not_null_ptr<ItemInstance> itemInstance);

    static const DyeColor* getColor(const not_null_ptr<ItemInstance>& itemInstance);
    static void appendHoverTextFromBannerBlockEntityTag(not_null_ptr<ItemInstance> itemInstance,
                                                        std::vector<HtmlString>* lines);
    static ItemInstance* makeBanner(const DyeColor* color, ListTag* patterns);
    static int getFirstPatternName(not_null_ptr<ItemInstance> itemInstance);
};

#endif
