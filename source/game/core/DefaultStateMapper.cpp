#include "game/core/DefaultStateMapper.hpp"

#include "game/client/render/ModelResourceLocation.hpp"
#include "game/world/block/DefaultedMappedRegistry.hpp"
#include "game/world/block/BlockState.hpp"

#include "game/world/block/Block.hpp"

ModelResourceLocation* DefaultStateMapper::getLocation(const BlockState* state)
{
    return ModelResourceLocation::Make(Block::blockRegistry->getKey(state->getBlock()),
                                       getValuesString(state->getValues()));
}
