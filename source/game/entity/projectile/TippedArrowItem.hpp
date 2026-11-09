#ifndef GAME_ENTITY_PROJECTILE_TIPPEDARROWITEM_HPP
#define GAME_ENTITY_PROJECTILE_TIPPEDARROWITEM_HPP

#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/entity/projectile/ArrowItem.hpp"
#include "game/item/not_null_ptr.hpp"

class Arrow;
class HtmlString;
class IconRegister;
class ItemInstance;
class Level;
class LivingEntity;
class Player;
class TextureAtlasSprite;

class TippedArrowItem : public ArrowItem
{
public:
    virtual not_null_ptr<ItemInstance> getDefaultInstance();
    virtual boost::shared_ptr<Arrow> createArrow(Level* level, not_null_ptr<ItemInstance> itemInstance,
                                                 boost::shared_ptr<LivingEntity> entity);
    virtual void appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool advanced);
    virtual std::wstring getName(not_null_ptr<ItemInstance> itemInstance);
    virtual int getUseDescriptionId(not_null_ptr<ItemInstance> itemInstance);
    virtual bool hasMultipleSpriteLayers();
    virtual TextureAtlasSprite* getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> itemInstance);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    TextureAtlasSprite* mArrowHeadIcon;
};

#endif
