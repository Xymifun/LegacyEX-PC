#ifndef GAME_UI_MENUBUILDER_HPP
#define GAME_UI_MENUBUILDER_HPP

#include <string>
#include <vector>

#include "game/core/BannerPattern.hpp"
#include "game/core/DyeColor.hpp"
#include "game/item/BannerItem.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemInstanceClonerBuilder.hpp"
#include "game/item/Items.hpp"
#include "game/item/Potion.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/ui/UIScene_CreativeMenu.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/BannerBlockEntity.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/TypedBoxed.hpp"

class MenuBuilder
{
public:
    template <typename T>
    void ITEM_AUX(Block* block, const AbstractProperty<T>* property, T value);

    template <typename T>
    void ITEM_AUX(Item* item, T* type);

    void ITEM_BLOCKSTATE(const BlockState* state);

    void ADD(const not_null_ptr<ItemInstance>& item)
    {
        ItemInstance* copy = item->copy_not_shared();
        mItems->push_back(new ItemInstanceClonerBuilder(*copy));
        *copy = *ItemInstance::EMPTY;
        delete copy;
    }

    void BANNER(const DyeColor* color, const BannerPattern* pattern)
    {
        not_null_ptr<ItemInstance> banner(BannerItem::makeBanner(color, NULL));
        BannerBlockEntity::addTags(banner, color->getItemData(), NULL);
        if (pattern != NULL)
            BannerBlockEntity::fjAddPatternToItem(banner, pattern, DyeColor::BLACK);
        ADD(banner);
    }
    void FIREWORK(unsigned char type, int colour, int flight, bool flicker, bool trail, int fadeColour);
    void POTION(Item* item, Potion* potion)
    {
        class PotionItemBuilder : public ItemInstanceClonerBuilder
        {
        public:
            PotionItemBuilder(const ItemInstance& item, Potion* potion)
                : ItemInstanceClonerBuilder(item),
                  mPotion(potion)
            {
            }

            virtual not_null_ptr<ItemInstance> build() const
            {
                not_null_ptr<ItemInstance> copy = mItem.copy();
                return PotionUtils::setPotion(copy, mPotion);
            }

        private:
            Potion* mPotion;
        };

        mItems->push_back(new PotionItemBuilder(ItemInstance(item), potion));
    }

    void DEBUG_SPAWN_EGG(const std::wstring& entityId, const std::wstring& debugTag)
    {
        class SpawnEggItemBuilder : public ItemInstanceClonerBuilder
        {
        public:
            SpawnEggItemBuilder(const std::wstring& entityId, const std::wstring& debugTag)
                : ItemInstanceClonerBuilder(ItemInstance(Items::spawnEgg)),
                  mEntityId(entityId),
                  mDebugTag(debugTag)
            {
            }

            virtual not_null_ptr<ItemInstance> build() const
            {
                not_null_ptr<ItemInstance> egg = mItem.copy();
                SpawnEggItem::addSpawnEntity(egg, mEntityId);
                if (!mDebugTag.empty())
                    egg->getTag()->putString(L"DebugTag", mDebugTag);
                return egg;
            }

        private:
            ResourceLocation mEntityId;
            std::wstring mDebugTag;
        };

        mItems->push_back(new SpawnEggItemBuilder(entityId, debugTag));
    }

    std::vector<const UIScene_CreativeMenu::ItemBuilder*>* mItems;
};

template <typename T>
inline void MenuBuilder::ITEM_AUX(Block* block, const AbstractProperty<T>* property, T value)
{
    TypedBoxed<T> boxed(value);
    ITEM_BLOCKSTATE(block->defaultBlockState()->setBoxedValue(property, &boxed));
}

template <typename T>
inline void MenuBuilder::ITEM_AUX(Item* item, T* type)
{
    ItemInstance instance(item, 1, type->getData());
    mItems->push_back(new ItemInstanceClonerBuilder(instance));
}

#endif
