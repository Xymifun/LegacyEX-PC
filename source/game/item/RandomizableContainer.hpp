#ifndef GAME_ITEM_RANDOMIZABLECONTAINER_HPP
#define GAME_ITEM_RANDOMIZABLECONTAINER_HPP

class ResourceLocation;

class RandomizableContainer
{
public:
    virtual ResourceLocation* getLootTable() = 0;
    virtual long long getLootTableSeed() = 0;
};

#endif
