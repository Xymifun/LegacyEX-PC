#ifndef GAME_ITEM_COMPASSITEM_HPP
#define GAME_ITEM_COMPASSITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/ItemPropertyFunction.hpp"

class Entity;
class IconRegister;
class ItemFrame;
class Level;
class LivingEntity;
class TextureAtlasSprite;

class CompassItem : public Item
{
public:
    class CompassItem_ItemPropertyFunction : public ItemPropertyFunction
    {
    public:
        virtual float call(not_null_ptr<ItemInstance> itemInstance, Level* level,
                           boost::shared_ptr<LivingEntity> entity);

    private:
        double getFrameRotation(boost::shared_ptr<ItemFrame> frame);
        double getSpawnToAngle(Level* level, boost::shared_ptr<Entity> entity);
        double wobble(Level* level, double value);

        double mRotation;
        double mWobble;
        long long mLastUpdate;
    };

    CompassItem();
    virtual TextureAtlasSprite* getIcon(int auxValue);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    TextureAtlasSprite* mIcons[4];
};

#endif
