#include "game/core/AbstractStateMapper.hpp"

#include "game/world/block/Block.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/BlockStateDefinition.hpp"

boost::unordered_map<const BlockState*, ModelResourceLocation*>* AbstractStateMapper::mapStates(Block* block)
{
    arrayWithLength<BlockStateDefinition::BlockStateImpl*> states =
        *block->getBlockStateDefinition()->getPossibleBlockStates();

    for (unsigned int i = 0; i < states.length; i++)
    {
        mStateLocations[states.data[i]] = getLocation(states.data[i]);
    }

    return &mStateLocations;
}

std::wstring AbstractStateMapper::getValuesString(const boost::unordered_map<const Property*, Boxed*>* values)
{
    std::wstring valuesString(L"");

    for (boost::unordered_map<const Property*, Boxed*>::const_iterator it = values->begin(); it != values->end(); ++it)
    {
        if (!valuesString.empty())
        {
            valuesString.append(L",");
        }

        const Property* property = it->first;
        valuesString.append(property->getName());
        valuesString.append(L"=");
        valuesString.append(property->getName(it->second));
    }

    if (valuesString.empty())
    {
        valuesString.append(L"normal");
    }

    return valuesString;
}
