#ifndef GAME_ITEM_FIREWORKCHARGEITEM_HPP
#define GAME_ITEM_FIREWORKCHARGEITEM_HPP

#include <string>
#include <vector>

#include "game/item/Item.hpp"

class CompoundTag;
class IconRegister;
class Tag;
class TextureAtlasSprite;
class FireworkChargeItem : public Item
{
public:
    FireworkChargeItem();
    static Tag* getExplosionTagField(not_null_ptr<ItemInstance> item, const std::wstring& name);
    static void appendHoverText(CompoundTag* tag, std::vector<HtmlString>* lines);
    virtual void appendHoverText(not_null_ptr<ItemInstance> item, boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool advanced);
    virtual TextureAtlasSprite* getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> item);
    virtual bool hasMultipleSpriteLayers();
    virtual void registerIcons(IconRegister* iconRegister);

private:
    TextureAtlasSprite* mOverlayIcon;
};

#endif
