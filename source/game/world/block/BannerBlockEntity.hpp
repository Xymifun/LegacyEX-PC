#ifndef GAME_WORLD_BLOCK_BANNERBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_BANNERBLOCKENTITY_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/world/block/BlockEntity.hpp"
#include "game/core/Nameable.hpp"
#include "game/core/CensoredString.hpp"
#include "game/item/not_null_ptr.hpp"

class BannerPattern;
class CompoundTag;
class DyeColor;
class ItemInstance;
class ListTag;

class BannerBlockEntity : public BlockEntity, public Nameable
{
public:
    BannerBlockEntity();
    virtual ~BannerBlockEntity();
    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual bool hasCustomName();
    virtual std::wstring getDisplayName();
    static const DyeColor* getColor(const not_null_ptr<ItemInstance>& item);
    void setCustomName(const std::wstring& name, const std::wstring& uuid, bool chatCensored, bool uuidLocal);
    void setItemValues(not_null_ptr<ItemInstance> item, bool useBannerItemColor);
    static void addTags(ItemInstance* item, int color, ListTag* patterns);
    static void addTags(not_null_ptr<ItemInstance> item, int color, ListTag* patterns);
    not_null_ptr<ItemInstance> getItem();
    virtual CompoundTag* save(CompoundTag* tag);
    static void addTags(CompoundTag* tag, int color, ListTag* patterns);
    virtual void load(CompoundTag* tag);
    virtual boost::shared_ptr<Packet> getUpdatePacket();
    virtual CompoundTag* getUpdateTag();
    static const DyeColor* getBaseColor(const not_null_ptr<ItemInstance>& item);
    static int getPatternCount(const not_null_ptr<ItemInstance>& item);
    void createPatternList();
    std::vector<const BannerPattern*>* getPatterns();
    std::vector<const DyeColor*>* getColors();
    std::wstring getTextureHashName();
    static void fjAddPatternToItem(const not_null_ptr<ItemInstance>& item, const BannerPattern* pattern,
                                   const DyeColor* color);
    static void removeLastPattern(not_null_ptr<ItemInstance> item);
    virtual boost::shared_ptr<BlockEntity> clone();
    virtual unsigned long long GetType();
    static BlockEntity* create();
    const DyeColor* getBaseColor();
    static bool fjCanAddPatternsToItem(const not_null_ptr<ItemInstance>& item);

    static const std::wstring TAG_BASE;
    static const std::wstring TAG_PATTERNS;
    static const std::wstring TAG_PATTERN;
    static const std::wstring TAG_COLOR;

private:
    std::wstring mName;
    const DyeColor* mBaseColor;
    ListTag* mPatterns;
    bool mPatternDataSet;
    std::vector<const BannerPattern*>* mPatternList;
    std::vector<const DyeColor*>* mColorList;
    std::wstring mTextureHashName;
    boost::shared_ptr<CensoredString> mCustomName;
};

#endif
