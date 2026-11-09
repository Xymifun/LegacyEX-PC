#include "game/item/ItemColors.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/FireworkChargeItem.hpp"
#include "game/item/FireworksItem.hpp"
#include "game/data/IntArrayTag.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockColors.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/BlockItem.hpp"
#include "game/core/GrassColor.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/MapItem.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/entity/mob/EntityIO.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcGoldenAge.hpp"
#include "platform/pc/PcSettings.hpp"
#endif

static const int FIREWORK_DEFAULT_COLOR = 0x8a8a8a;

ItemColors::ItemColors()
{
    mItemColorIds = new IdMapper<ItemColor*>();
}

void ItemColors::registerColor(ItemColor* color, arrayWithLength<Item*> items)
{
    for (unsigned int i = 0; i < items.length; i++)
    {
        int id = Item::getId(items.data[i]);
        mItemColorIds->addMapping(color, id);
    }
}

void ItemColors::registerColor(ItemColor* color, arrayWithLength<Block*> blocks)
{
    for (unsigned int i = 0; i < blocks.length; i++)
    {
        Item* item = Item::byBlock(blocks.data[i]);
        mItemColorIds->addMapping(color, Item::getId(item));
    }
}

ItemColors* ItemColors::createDefault(BlockColors* blockColors)
{
    class Armor_ItemColor : public ItemColor
    {
    public:
        int getColor(const not_null_ptr<ItemInstance>& item, int layer)
        {
#ifdef MINECRAFT_PC
            if (PcGoldenAgeOn(kPcGoldenAgeItems))
                return -1;
#endif
            return layer > 0 ? -1 : item->getItem()->getColor(item, 0);
        }
    };
    class Bed_ItemColor : public ItemColor
    {
    public:
        int getColor(const not_null_ptr<ItemInstance>& item, int layer)
        {
            const DyeColor* dye = DyeColor::byBlockData(item->getAuxValue());
            if (layer == 0)
            {
                ColourTable* table = Minecraft::GetInstance()->getColourTable();
                eMinecraftColour colour = dye->getBedColor();
                return static_cast<int>(table->getColour(colour));
            }
            return -1;
        }
    };
    class DoublePlant_ItemColor : public ItemColor
    {
    public:
        int getColor(const not_null_ptr<ItemInstance>& item, int)
        {
            DoublePlantBlock::Variant* variant = DoublePlantBlock::Variant::byData(item->getAuxValue());
            return (variant == gDoublePlantGrass || variant == gDoublePlantFern)
                       ? static_cast<int>(GrassColor::get(0.5, 1.0))
                       : -1;
        }
    };
    class FilledMap_ItemColor : public ItemColor
    {
    public:
        int getColor(const not_null_ptr<ItemInstance>& item, int layer)
        {
            return layer == 0 ? -1 : MapItem::getColor(item);
        }
    };
    class FireworkCharge_ItemColor : public ItemColor
    {
    public:
        int getColor(const not_null_ptr<ItemInstance>& item, int layer)
        {
            if (layer != 1)
                return -1;
            Tag* tag = FireworkChargeItem::getExplosionTagField(item, FireworksItem::TAG_E_COLORS);
            if (tag == NULL || dynamic_cast<IntArrayTag*>(tag) == NULL)
                return FIREWORK_DEFAULT_COLOR;
            arrayWithLength<int> values = static_cast<IntArrayTag*>(tag)->getAsIntArray();
            if (values.length == 1)
                return values.data[0];
            unsigned int red = 0;
            unsigned int green = 0;
            unsigned int blue = 0;
            for (unsigned int i = 0; i < values.length; ++i)
            {
                unsigned int color = static_cast<unsigned int>(values.data[i]);
                red += (color >> 16) & 0xff;
                green += (color >> 8) & 0xff;
                blue += color & 0xff;
            }
            red /= values.length;
            green /= values.length;
            blue /= values.length;
            return static_cast<int>((red << 16) | (green << 8) | blue);
        }
    };
    class Grass_ItemColor : public ItemColor
    {
        BlockColors* colors;

    public:
        explicit Grass_ItemColor(BlockColors* value) : colors(value) {}
        int getColor(const not_null_ptr<ItemInstance>& item, int layer)
        {
            BlockItem* blockItem = static_cast<BlockItem*>(item->getItem());
            const BlockState* state = blockItem->getBlock()->getBlockState(item->getAuxValue());
            return colors->getColor(state, NULL, NULL, layer);
        }
    };
    class LeatherHorseArmor_ItemColor : public ItemColor
    {
    public:
        int getColor(const not_null_ptr<ItemInstance>& item, int layer)
        {
            return item->getItem()->getColor(item, layer);
        }
    };
    class Potion_ItemColor : public ItemColor
    {
    public:
        int getColor(const not_null_ptr<ItemInstance>& item, int layer)
        {
            if (layer > 0)
                return -1;
            std::vector<MobEffectInstance*>* effects = PotionUtils::getMobEffects(item);
            int color = PotionUtils::getColor(effects);
            delete effects;
            return color;
        }
    };
    class SpawnEgg_ItemColor : public ItemColor
    {
    public:
        int getColor(const not_null_ptr<ItemInstance>& item, int layer)
        {
            EntityIO::SpawnInfo::iterator found = EntityIO::sSpawnInfo.find(SpawnEggItem::getNamedIdFrom(item));
            if (found != EntityIO::sSpawnInfo.end())
            {
                EntityIO::SpawnableMobInfo* info = found->second;
                if (layer == 0)
                {
                    if (info->colour != 0)
                        return static_cast<int>(Minecraft::GetInstance()->getColourTable()->getColour(info->colour));
                }
                else if (info->secondaryColour != 0)
                {
                    return static_cast<int>(
                        Minecraft::GetInstance()->getColourTable()->getColour(info->secondaryColour));
                }
            }
            return -1;
        }
    };
    class TallGrass_ItemColor : public ItemColor
    {
    public:
        int getColor(const not_null_ptr<ItemInstance>&, int) { return (int)GrassColor::get(0.5, 1.0); }
    };
    class TippedArrow_ItemColor : public ItemColor
    {
    public:
        int getColor(const not_null_ptr<ItemInstance>& item, int layer)
        {
            if (layer == 0)
            {
                std::vector<MobEffectInstance*>* effects = PotionUtils::getMobEffects(item);
                int color = PotionUtils::getColor(effects);
                delete effects;
                return color;
            }
            return -1;
        }
    };

    ItemColors* colors = new ItemColors();
    Item* armor[] = {Items::leatherHelmet, Items::leatherChestplate, Items::leatherLeggings, Items::leatherBoots};
    colors->registerColor(new Armor_ItemColor(), arrayWithLength<Item*>(armor, 4));
    Block* doublePlant[] = {Blocks::double_plant};
    colors->registerColor(new DoublePlant_ItemColor(), arrayWithLength<Block*>(doublePlant, 1));
    Item* firework[] = {Items::fireworkCharge};
    colors->registerColor(new FireworkCharge_ItemColor(), arrayWithLength<Item*>(firework, 1));
    Item* potions[] = {Items::potion, Items::splashPotion, Items::lingeringPotion};
    colors->registerColor(new Potion_ItemColor(), arrayWithLength<Item*>(potions, 3));
    Item* spawnEgg[] = {Items::spawnEgg};
    colors->registerColor(new SpawnEgg_ItemColor(), arrayWithLength<Item*>(spawnEgg, 1));
    Block* foliage[] = {Blocks::grass,  Blocks::tallgrass, Blocks::vine,
                        Blocks::leaves, Blocks::leaves2,   Blocks::waterlily};
    colors->registerColor(new Grass_ItemColor(blockColors), arrayWithLength<Block*>(foliage, 6));
    Item* tippedArrow[] = {Items::tippedArrow};
    colors->registerColor(new TippedArrow_ItemColor(), arrayWithLength<Item*>(tippedArrow, 1));
    Item* filledMap[] = {Items::filledMap};
    colors->registerColor(new FilledMap_ItemColor(), arrayWithLength<Item*>(filledMap, 1));
    Item* bed[] = {Items::bed};
    colors->registerColor(new Bed_ItemColor(), arrayWithLength<Item*>(bed, 1));
    Item* horseArmor[] = {Items::leatherHorseArmor};
    colors->registerColor(new LeatherHorseArmor_ItemColor(), arrayWithLength<Item*>(horseArmor, 1));
    Block* tallGrass[] = {Blocks::tallgrass};
    colors->registerColor(new TallGrass_ItemColor(), arrayWithLength<Block*>(tallGrass, 1));
    return colors;
}

int ItemColors::getColor(not_null_ptr<ItemInstance> item, int layer)
{
    ItemColor* color = mItemColorIds->byId(Item::itemRegistry.getId(item->getItem()));
    return color == NULL ? -1 : color->getColor(item, layer);
}
