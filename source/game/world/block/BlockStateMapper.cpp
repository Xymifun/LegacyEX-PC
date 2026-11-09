#include "game/world/block/BlockStateMapper.hpp"

#include "game/world/block/DefaultedMappedRegistry.hpp"

#include "game/core/DefaultStateMapper.hpp"
#include "game/core/StateMapper.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/Block.hpp"

#include <cstdarg>

void BlockStateMapper::add(Block* block, StateMapper* stateMapper)
{
    stateMappers[block] = stateMapper;
}

void BlockStateMapper::ignore(Block* block, ...)
{
    va_list args;
    va_start(args, block);
    while (block != NULL)
    {
        ignoredBlocks.insert(block);
        block = va_arg(args, Block*);
    }
    va_end(args);
}

boost::unordered_map<const BlockState*, ModelResourceLocation*>* BlockStateMapper::getMapping()
{
    boost::unordered_map<const BlockState*, ModelResourceLocation*>* mapping =
        new boost::unordered_map<const BlockState*, ModelResourceLocation*>();

    boost::unordered_map<Block*, ResourceLocation>::const_iterator it;

    for (it = Block::blockRegistry->mInverseRegistry.begin(); it != Block::blockRegistry->mInverseRegistry.end(); ++it)
    {
        Block* block = it->first;

        if (ignoredBlocks.find(block) != ignoredBlocks.end())
        {
            continue;
        }

        bool temporary = false;
        StateMapper* stateMapper;

        if (stateMappers.find(block) != stateMappers.end())
        {
            stateMapper = stateMappers[block];
        }
        else
        {
            temporary = true;
            stateMapper = new DefaultStateMapper();
        }

        boost::unordered_map<const BlockState*, ModelResourceLocation*>* states = stateMapper->mapStates(block);
        mapping->insert(states->begin(), states->end());

        if (temporary)
        {
            delete stateMapper;
        }
    }

    return mapping;
}
