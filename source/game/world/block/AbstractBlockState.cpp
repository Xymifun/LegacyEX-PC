#include "game/world/block/AbstractBlockState.hpp"

#include "game/core/Boxed.hpp"
#include "game/core/Joiner.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/DefaultedMappedRegistry.hpp"
#include "game/world/block/Property.hpp"

static Joiner COMMA_JOINER = Joiner::on(L',');

std::wstring AbstractBlockState::PROPERTY_ENTRY_TO_STRING_FUNCTION(
    boost::unordered_map<const Property*, Boxed*>::const_iterator* entry)
{
    if (entry == 0)
    {
        return L"<NULL>";
    }

    const Property* property = (*entry)->first;

    return property->getName() + L"=" + property->getName((*entry)->second);
}

Boxed* AbstractBlockState::findNextInCollection(const Property* property, const std::vector<Boxed*>& values,
                                                Boxed* value)
{
    std::vector<Boxed*>::const_iterator iterator = values.begin();

    while (iterator != values.end())
    {
        Boxed* current = *iterator;
        ++iterator;

        if (current == value)
        {
            if (iterator != values.end())
            {
                return *iterator;
            }

            return *values.begin();
        }
    }

    return *values.begin();
}

const BlockState* AbstractBlockState::cycle(const Property* property) const
{
    return setBoxedValue(property,
                         findNextInCollection(property, property->getPossibleValues(), getBoxedValue(property)));
}

std::wstring AbstractBlockState::toString() const
{
    std::wstring result = L"";

    result += Block::blockRegistry->getKey(getBlock()).toString();

    std::vector<std::wstring> propertyStrings;

    const boost::unordered_map<const Property*, Boxed*>* values = getValues();

    for (boost::unordered_map<const Property*, Boxed*>::const_iterator iterator = values->begin();
         iterator != values->end(); ++iterator)
    {
        propertyStrings.push_back(PROPERTY_ENTRY_TO_STRING_FUNCTION(&iterator));
    }

    if (!values->empty())
    {
        result += L"[";
        COMMA_JOINER.appendTo(result, propertyStrings);
        result += L"]";
    }

    delete values;

    return result;
}
