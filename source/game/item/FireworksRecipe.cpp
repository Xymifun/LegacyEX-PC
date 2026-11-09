#include "game/item/FireworksRecipe.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/DyeItem.hpp"
#include "game/item/FireworksItem.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "platform/wiiu/TLSStorageCafe.hpp"

#include <vector>

static unsigned int g_fireworksRecipeTlsIndex = 0;
static FireworksRecipe::ThreadStorage* g_pDefaultThreadStorage = NULL;

FireworksRecipe::ThreadStorage::ThreadStorage()
{
    mResultItem = ItemInstance::EMPTY;
}

FireworksRecipe::FireworksRecipe() : Recipe()
{
}

FireworksRecipe::~FireworksRecipe()
{
}

void FireworksRecipe::updatePossibleRecipes(boost::shared_ptr<CraftingContainer> craftingContainer, bool* hasPaper,
                                            bool* hasGunpowder, bool* hasFireworkCharge)
{
    *hasPaper = false;
    *hasGunpowder = false;
    *hasFireworkCharge = false;

    unsigned int slot;
    int paper = 0;
    int gunpowder = 0;
    int dyes = 0;
    int charges = 0;
    int effects = 0;
    int shapes = 0;
    int extras;

    for (slot = 0; slot < craftingContainer->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);

        if (item->isEmpty())
        {
            continue;
        }

        if (item->getItem() == Items::gunpowder)
        {
            ++gunpowder;
        }
        else if (item->getItem() == Items::fireworkCharge)
        {
            ++charges;
        }
        else if (item->getItem() == Items::dye)
        {
            ++dyes;
        }
        else if (item->getItem() == Items::paper)
        {
            ++paper;
        }
        else if (item->getItem() == Items::glowstoneDust || item->getItem() == Items::diamond)
        {
            ++effects;
        }
        else if (item->getItem() == Items::fireCharge || item->getItem() == Items::feather
                 || item->getItem() == Items::goldNugget || item->getItem() == Items::skull)
        {
            ++shapes;
        }
        else
        {
            return;
        }
    }

    extras = effects + dyes + shapes;

    if (gunpowder > 3 || paper > 1)
    {
        return;
    }

    if (extras == 0)
    {
        *hasPaper = true;
    }

    if (gunpowder > 1)
    {
        return;
    }

    if (dyes >= 0 && paper == 0 && charges == 0 && shapes <= 1)
    {
        *hasGunpowder = true;
    }

    if (gunpowder != 0 || paper != 0 || charges > 1)
    {
        return;
    }

    if (dyes < 0)
    {
        return;
    }

    *hasFireworkCharge = true;
}

bool FireworksRecipe::isValidIngredient(not_null_ptr<ItemInstance> item, bool hasPaper, bool hasGunpowder,
                                        bool hasFireworkCharge)
{
    bool valid = false;

    if (item->getItem() == Items::gunpowder)
    {
        valid = hasPaper || hasGunpowder;
    }
    else if (item->getItem() == Items::fireworkCharge)
    {
        valid = hasPaper || hasFireworkCharge;
    }
    else if (item->getItem() == Items::dye)
    {
        valid = hasGunpowder || hasFireworkCharge;
    }
    else if (item->getItem() == Items::paper)
    {
        valid = hasPaper;
    }
    else if (item->getItem() == Items::glowstoneDust || item->getItem() == Items::diamond
             || item->getItem() == Items::fireCharge || item->getItem() == Items::feather
             || item->getItem() == Items::goldNugget || item->getItem() == Items::skull)
    {
        valid = hasGunpowder;
    }

    return valid;
}

void FireworksRecipe::setResultItem(not_null_ptr<ItemInstance> item)
{
    ThreadStorage* pStorage = static_cast<ThreadStorage*>(TlsGetValue(g_fireworksRecipeTlsIndex));

    pStorage->mResultItem = item;
}

void FireworksRecipe::CreateNewThreadStorage()
{
    ThreadStorage* pStorage = new ThreadStorage();

    if (g_pDefaultThreadStorage == NULL)
    {
        g_fireworksRecipeTlsIndex = TlsAlloc();
        g_pDefaultThreadStorage = pStorage;
    }

    TlsSetValue(g_fireworksRecipeTlsIndex, pStorage);
}

void FireworksRecipe::UseDefaultThreadStorage()
{
    TlsSetValue(g_fireworksRecipeTlsIndex, g_pDefaultThreadStorage);
}

void FireworksRecipe::ReleaseThreadStorage()
{
    ThreadStorage* pStorage = static_cast<ThreadStorage*>(TlsGetValue(g_fireworksRecipeTlsIndex));

    if (pStorage != g_pDefaultThreadStorage)
    {
        delete pStorage;
    }
}

bool FireworksRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level)
{
    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    unsigned int slot;
    int gunpowder = 0;
    int charges = 0;
    int dyes = 0;
    int paper = 0;
    int effects = 0;
    int shapes = 0;
    int extras;

    for (slot = 0; slot < craftingContainer->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);

        if (item->isEmpty())
        {
            continue;
        }

        if (item->getItem() == Items::gunpowder)
        {
            ++gunpowder;
        }
        else if (item->getItem() == Items::fireworkCharge)
        {
            ++charges;
        }
        else if (item->getItem() == Items::dye)
        {
            ++dyes;
        }
        else if (item->getItem() == Items::paper)
        {
            ++paper;
        }
        else if (item->getItem() == Items::glowstoneDust || item->getItem() == Items::diamond)
        {
            ++effects;
        }
        else if (item->getItem() == Items::fireCharge || item->getItem() == Items::feather
                 || item->getItem() == Items::goldNugget || item->getItem() == Items::skull)
        {
            ++shapes;
        }
        else
        {
            setResultItem(result);
            return false;
        }
    }

    extras = effects + dyes + shapes;

    if (gunpowder > 3 || paper > 1)
    {
        setResultItem(result);
        return false;
    }

    if (gunpowder >= 1 && paper == 1 && extras == 0)
    {
        result = not_null_ptr<ItemInstance>(new ItemInstance(Items::fireworks, 3));
        CompoundTag* fireworks = new CompoundTag();

        if (charges > 0)
        {
            ListTag* explosions = new ListTag();

            for (slot = 0; slot < craftingContainer->getContainerSize(); ++slot)
            {
                not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);

                if (item->getItem() != Items::fireworkCharge)
                {
                    continue;
                }

                if (item->hasTag() && item->getTag()->contains(FireworksItem::TAG_EXPLOSION, Tag::TAG_COMPOUND))
                {
                    explosions->add(item->getTag()->getCompound(FireworksItem::TAG_EXPLOSION)->copy());
                }
            }

            fireworks->put(FireworksItem::TAG_EXPLOSIONS, explosions);
        }

        fireworks->putByte(FireworksItem::TAG_FLIGHT, static_cast<unsigned char>(gunpowder));

        CompoundTag* tag = new CompoundTag();
        tag->put(FireworksItem::TAG_FIREWORKS, fireworks);
        result->setTag(tag);
        setResultItem(result);
        return true;
    }

    if (gunpowder == 1 && paper == 0 && charges == 0 && dyes >= 1 && shapes <= 1)
    {
        result = not_null_ptr<ItemInstance>(new ItemInstance(Items::fireworkCharge));
        CompoundTag* tag = new CompoundTag();
        CompoundTag* explosion = new CompoundTag();
        std::vector<int> colors;
        unsigned char shape = 0;

        for (slot = 0; slot < craftingContainer->getContainerSize(); ++slot)
        {
            not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);

            if (item->isEmpty())
            {
                continue;
            }

            if (item->getItem() == Items::dye)
            {
                colors.push_back(DyeItem::DYE_COLORS[item->getAuxValue() & 15]);
            }
            else if (item->getItem() == Items::glowstoneDust)
            {
                explosion->putBoolean(FireworksItem::TAG_E_FLICKER, true);
            }
            else if (item->getItem() == Items::diamond)
            {
                explosion->putBoolean(FireworksItem::TAG_E_TRAIL, true);
            }
            else if (item->getItem() == Items::fireCharge)
            {
                shape = 1;
            }
            else if (item->getItem() == Items::feather)
            {
                shape = 4;
            }
            else if (item->getItem() == Items::goldNugget)
            {
                shape = 2;
            }
            else if (item->getItem() == Items::skull)
            {
                shape = 3;
            }
        }

        arrayWithLength<int> explosionColors(colors.size(), true);

        for (unsigned int i = 0; i < explosionColors.length; ++i)
        {
            explosionColors[i] = colors.at(i);
        }

        explosion->putIntArray(FireworksItem::TAG_E_COLORS, explosionColors);
        explosion->putByte(FireworksItem::TAG_E_TYPE, shape);
        tag->put(FireworksItem::TAG_EXPLOSION, explosion);
        result->setTag(tag);
        setResultItem(result);
        return true;
    }

    if (gunpowder == 0 && paper == 0 && charges == 1 && dyes >= 1 && dyes == extras)
    {
        std::vector<int> colors;

        for (slot = 0; slot < craftingContainer->getContainerSize(); ++slot)
        {
            not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);

            if (item->isEmpty())
            {
                continue;
            }

            if (item->getItem() == Items::dye)
            {
                colors.push_back(DyeItem::DYE_COLORS[item->getAuxValue() & 15]);
            }
            else if (item->getItem() == Items::fireworkCharge)
            {
                result = item->copy();
                result->setCount(1);
            }
        }

        arrayWithLength<int> fadeColors(colors.size(), true);

        for (unsigned int i = 0; i < fadeColors.length; ++i)
        {
            fadeColors[i] = colors.at(i);
        }

        if (!result->isEmpty() && result->hasTag())
        {
            CompoundTag* explosion = result->getTag()->getCompound(FireworksItem::TAG_EXPLOSION);

            if (explosion == NULL)
            {
                delete fadeColors.data;
                setResultItem(result);
                return false;
            }

            explosion->putIntArray(FireworksItem::TAG_E_FADECOLORS, fadeColors);
        }
        else
        {
            delete fadeColors.data;
            setResultItem(result);
            return false;
        }

        setResultItem(result);
        return true;
    }

    setResultItem(result);
    return false;
}

not_null_ptr<ItemInstance> FireworksRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    ThreadStorage* pStorage = static_cast<ThreadStorage*>(TlsGetValue(g_fireworksRecipeTlsIndex));

    return pStorage->mResultItem->copy();
}

int FireworksRecipe::size()
{
    return 10;
}

ItemInstance* FireworksRecipe::getResultItem()
{
    ThreadStorage* pStorage = static_cast<ThreadStorage*>(TlsGetValue(g_fireworksRecipeTlsIndex));

    return pStorage->mResultItem.get();
}

ItemInstance* FireworksRecipe::getIngredientItem(int column, int row)
{
    return ItemInstance::EMPTY.get();
}

NonNullList<not_null_ptr<ItemInstance> > FireworksRecipe::getRemainingItems(
    boost::shared_ptr<CraftingContainer> craftingContainer)
{
    unsigned int slot;
    NonNullList<not_null_ptr<ItemInstance> > remaining = NonNullList<not_null_ptr<ItemInstance> >::withSize(
        craftingContainer->getContainerSize(), ItemInstance::EMPTY);

    for (slot = 0; slot < remaining.size(); ++slot)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);

        if (item->getItem()->hasCraftingRemainingItem())
        {
            remaining.set(slot,
                          not_null_ptr<ItemInstance>(new ItemInstance(item->getItem()->getCraftingRemainingItem())));
        }
    }

    return remaining;
}

boost::shared_ptr<CraftingContainer> FireworksRecipe::generateExpectedCraftSlots()
{
    return boost::shared_ptr<CraftingContainer>();
}

Recipe::_eGroupType FireworksRecipe::getGroup()
{
    return Recipe::eGroupType_None;
}

bool FireworksRecipe::recipeRequires(int itemId)
{
    return false;
}

void FireworksRecipe::recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired)
{
}
