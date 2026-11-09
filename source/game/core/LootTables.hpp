#ifndef GAME_CORE_LOOTTABLES_HPP
#define GAME_CORE_LOOTTABLES_HPP

#include <boost/unordered_map.hpp>

#include "game/util/ResourceLocation.hpp"

class File;
class LootTable;

class LootTables
{
public:
    LootTables();
    LootTables(File* lootTableFolder);

    void reload();

    LootTable* get(ResourceLocation* location);

private:
    boost::unordered_map<ResourceLocation*, LootTable*>* tables;
    File* lootTableFolder;
};

#endif
