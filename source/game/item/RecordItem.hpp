#ifndef GAME_ITEM_RECORDITEM_HPP
#define GAME_ITEM_RECORDITEM_HPP

#include <string>

#include "game/item/Item.hpp"

class SoundEvent;

class RecordItem : public Item
{
public:
    RecordItem(const std::wstring& name, int recordId, const SoundEvent* sound);
    virtual InteractionResult::EInteractionResult useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction* direction, float x, float y, float z,
                                                        bool flag);
    virtual void appendHoverText(not_null_ptr<ItemInstance> item, boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool advanced);
    std::wstring getDisplayName();
    virtual const Rarity* getRarity(not_null_ptr<ItemInstance> item);
    static RecordItem* getBySound(const SoundEvent* sound);
    const SoundEvent* getSound();
    virtual TextureAtlasSprite* getIcon(int auxValue);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    const SoundEvent* mSound;
    std::wstring mRecordName;
    int mRecordId;
};

#endif
