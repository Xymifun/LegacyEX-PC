#ifndef GAME_ITEM_MAPITEM_HPP
#define GAME_ITEM_MAPITEM_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/ComplexItem.hpp"
#include "game/item/not_null_ptr.hpp"

class IconRegister;
class ItemInstance;
class Level;
class MapItemSavedData;
class Player;
struct PlayerUID;
class Entity;
class Packet;
class TextureAtlasSprite;

class MapItem : public ComplexItem
{
public:
    MapItem();

    static not_null_ptr<ItemInstance> setupNewMap(PlayerUID uid, Level* level, int dimension, double x, double z,
                                                  unsigned char scale, bool trackingPosition, bool unlimitedTracking);
    static boost::shared_ptr<MapItemSavedData> getSavedDataIfExists(int mapId, Level* level);
    boost::shared_ptr<MapItemSavedData> getSavedData(int mapId, Level* level);
    boost::shared_ptr<MapItemSavedData> getSavedData(not_null_ptr<ItemInstance> itemInstance, Level* level);
    void update(Level* level, boost::shared_ptr<Entity> entity, boost::shared_ptr<MapItemSavedData> data);
    static void renderBiomePreviewMap(Level* level, const not_null_ptr<ItemInstance>& itemInstance);
    virtual void inventoryTick(not_null_ptr<ItemInstance> itemInstance, Level* level, boost::shared_ptr<Entity> entity,
                               int slot, bool selected);
    virtual boost::shared_ptr<Packet> getUpdatePacket(not_null_ptr<ItemInstance> itemInstance, Level* level,
                                                      boost::shared_ptr<Player> player);
    static void scaleMap(not_null_ptr<ItemInstance> itemInstance, Level* level, int scale);
    static void enableMapTracking(not_null_ptr<ItemInstance> itemInstance, Level* level);
    virtual void onCraftedBy(not_null_ptr<ItemInstance> itemInstance, Level* level, boost::shared_ptr<Player> player);
    static int getColor(const not_null_ptr<ItemInstance>& itemInstance);
    virtual bool hasMultipleSpriteLayers();
    virtual TextureAtlasSprite* getLayerIcon(int auxValue, int layer, not_null_ptr<ItemInstance> itemInstance);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    TextureAtlasSprite* mIcon;
};

#endif
