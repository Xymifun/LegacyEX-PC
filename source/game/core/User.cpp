#include "game/core/User.hpp"

#include "game/world/block/Blocks.hpp"

std::vector<Block*> User::creativeBlocks;

void User::staticCtor()
{
    creativeBlocks.push_back(Blocks::stone);
    creativeBlocks.push_back(Blocks::cobblestone);
    creativeBlocks.push_back(Blocks::brick_block);
    creativeBlocks.push_back(Blocks::dirt);
    creativeBlocks.push_back(Blocks::planks);
    creativeBlocks.push_back(Blocks::log);
    creativeBlocks.push_back(Blocks::log2);
    creativeBlocks.push_back(Blocks::leaves);
    creativeBlocks.push_back(Blocks::leaves2);
    creativeBlocks.push_back(Blocks::torch);
    creativeBlocks.push_back(Blocks::stone_slab);
    creativeBlocks.push_back(Blocks::glass_pane);
    creativeBlocks.push_back(Blocks::mossy_cobblestone);
    creativeBlocks.push_back(Blocks::sapling);
    creativeBlocks.push_back(Blocks::yellow_flower);
    creativeBlocks.push_back(Blocks::red_flower);
    creativeBlocks.push_back(Blocks::brown_mushroom);
    creativeBlocks.push_back(Blocks::red_mushroom);
    creativeBlocks.push_back(Blocks::sand);
    creativeBlocks.push_back(Blocks::gravel);
    creativeBlocks.push_back(Blocks::sponge);
    creativeBlocks.push_back(Blocks::wool);
    creativeBlocks.push_back(Blocks::coal_ore);
    creativeBlocks.push_back(Blocks::iron_ore);
    creativeBlocks.push_back(Blocks::gold_ore);
    creativeBlocks.push_back(Blocks::iron_block);
    creativeBlocks.push_back(Blocks::gold_block);
    creativeBlocks.push_back(Blocks::bookshelf);
    creativeBlocks.push_back(Blocks::tnt);
    creativeBlocks.push_back(Blocks::obsidian);
}

User::User(const std::wstring& name, const std::wstring& sessionId)
{
    this->name = name;
    this->sessionId = sessionId;
}
