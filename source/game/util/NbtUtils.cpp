#include "game/util/NbtUtils.hpp"

#include <typeinfo>
#include <vector>

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/Tag.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/SandStoneBlock.hpp"
#include "game/world/block/StoneBlock.hpp"
#include "game/world/block/StoneBrickBlock.hpp"

bool NbtUtils::compareNbt(Tag* first, Tag* second, bool compareLists)
{
    if (first == second)
        return true;
    if (first == NULL)
        return true;
    if (second == NULL)
        return false;
    if (typeid(first) != typeid(second))
        return false;
    if (dynamic_cast<CompoundTag*>(first) != NULL)
    {
        CompoundTag* firstCompound = dynamic_cast<CompoundTag*>(first);
        CompoundTag* secondCompound = dynamic_cast<CompoundTag*>(second);
        std::vector<std::wstring>* keys = firstCompound->getAllKeys();
        std::vector<std::wstring>::const_iterator it;
        for (it = keys->begin(); it != keys->end(); ++it)
        {
            std::wstring key = *it;
            Tag* firstChild = firstCompound->get(key);
            Tag* secondChild = secondCompound->get(key);
            if (!compareNbt(firstChild, secondChild, compareLists))
                return false;
        }
        delete keys;
        return true;
    }
    if (dynamic_cast<ListTag*>(first) != NULL && compareLists)
    {
        ListTag* firstList = dynamic_cast<ListTag*>(first);
        ListTag* secondList = dynamic_cast<ListTag*>(second);
        if (firstList->size() == 0)
            return secondList->size() == 0;
        for (int i = 0; i < firstList->size(); ++i)
        {
            Tag* element = firstList->get(i);
            bool found = false;
            for (int j = 0; j < secondList->size(); ++j)
            {
                if (compareNbt(element, secondList->get(j), compareLists))
                {
                    found = true;
                    break;
                }
            }
            if (!found)
                return false;
        }
        return true;
    }
    return first->equals(second);
}

BlockPos NbtUtils::readBlockPos(CompoundTag* tag)
{
    return BlockPos(tag->getInt(L"X"), tag->getInt(L"Y"), tag->getInt(L"Z"));
}

CompoundTag* NbtUtils::writeBlockPos(const BlockPos& pos)
{
    CompoundTag* tag = new CompoundTag();
    tag->putInt(L"X", pos.x);
    tag->putInt(L"Y", pos.y);
    tag->putInt(L"Z", pos.z);
    return tag;
}

void NbtUtils::mapToBlock(std::wstring& name, int& data)
{
    if (name == L"minecraft:stone_bricks")
    {
        name = L"minecraft:stonebrick";
        data = 0;
        return;
    }
    if (name == L"minecraft:bricks")
    {
        name = L"minecraft:brick_block";
        data = 0;
        return;
    }
    if (name == L"minecraft:chiseled_stone_bricks")
    {
        name = L"minecraft:stonebrick";
        data |= StoneBrickBlock::Variant::CHISELED->getData();
        return;
    }
    if (name == L"minecraft:cracked_stone_bricks")
    {
        name = L"minecraft:stonebrick";
        data |= StoneBrickBlock::Variant::CRACKED->getData();
        return;
    }
    if (name == L"minecraft:mossy_stone_bricks")
    {
        name = L"minecraft:stonebrick";
        data |= StoneBrickBlock::Variant::MOSSY->getData();
        return;
    }
    if (name == L"minecraft:polished_granite")
    {
        name = L"minecraft:stone";
        data |= StoneBlock::Variant::SMOOTH_GRANITE->getData();
        return;
    }
    if (name == L"minecraft:diorite")
    {
        name = L"minecraft:stone";
        data |= StoneBlock::Variant::DIORITE->getData();
        return;
    }
    if (name == L"minecraft:polished_diorite")
    {
        name = L"minecraft:stone";
        data |= StoneBlock::Variant::SMOOTH_DIORITE->getData();
        return;
    }
    if (name == L"minecraft:dark_oak_planks")
    {
        name = L"minecraft:planks";
        data |= PlanksBlock::Variant::DARK_OAK->getData();
        return;
    }
    if (name == L"minecraft:spruce_planks")
    {
        name = L"minecraft:planks";
        data |= PlanksBlock::Variant::SPRUCE->getData();
        return;
    }
    if (name == L"minecraft:cut_sandstone")
    {
        name = L"minecraft:sandstone";
        data |= SandStoneBlock::SMOOTH->getData();
        return;
    }
    if (name == L"minecraft:light_blue_terracotta")
    {
        name = L"minecraft:stained_hardened_clay";
        data |= 3;
        return;
    }
    if (name == L"minecraft:magma_block")
    {
        name = L"minecraft:magma";
        return;
    }
    if (name == L"minecraft:stripped_oak_log")
    {
        name = L"minecraft:stripped_log_oak";
        return;
    }
    if (name == L"minecraft:stripped_spruce_log")
    {
        name = L"minecraft:stripped_log_spruce";
        return;
    }
    if (name == L"minecraft:stripped_birch_log")
    {
        name = L"minecraft:stripped_log_birch";
        return;
    }
    if (name == L"minecraft:stripped_jungle_log")
    {
        name = L"minecraft:stripped_log_jungle";
        return;
    }
    if (name == L"minecraft:stripped_acacia_log")
    {
        name = L"minecraft:stripped_log_acacia";
        return;
    }
    if (name == L"minecraft:stripped_dark_oak_log")
    {
        name = L"minecraft:stripped_log_dark_oak";
        return;
    }
    if (name == L"minecraft:chiseled_sandstone")
    {
        name = L"minecraft:sandstone";
        data |= SandStoneBlock::CHISELED->getData();
    }
}

const BlockState* NbtUtils::readBlockState(CompoundTag* tag)
{
    if (!tag->contains(L"Name", Tag::TAG_STRING))
        return Blocks::air->defaultBlockState();
    std::wstring name = tag->getString(L"Name");
    int data = 0;
    mapToBlock(name, data);
    Block* block = Block::byString(name);
    const BlockState* state = block->defaultBlockState();
    if (tag->contains(L"Properties", Tag::TAG_COMPOUND))
    {
        CompoundTag* properties = tag->getCompound(L"Properties");
        BlockStateDefinition* definition = block->getBlockStateDefinition();
        std::vector<std::wstring>* keys = properties->getAllKeys();
        std::vector<const Property*> blockProperties = definition->getProperties();
        for (std::vector<std::wstring>::iterator it = keys->begin(); it != keys->end(); ++it)
        {
            std::wstring key = *it;
            for (std::vector<const Property*>::iterator property = blockProperties.begin();
                 property != blockProperties.end(); ++property)
            {
                const Property* blockProperty = *property;
                if (key == blockProperty->getName())
                {
                    state = block->setPropertyValueFromString(state, blockProperty, properties->getString(key));
                }
            }
        }
        delete keys;
    }
    return block->getBlockState(block->convertBlockStateToLegacyData(state) | data);
}
