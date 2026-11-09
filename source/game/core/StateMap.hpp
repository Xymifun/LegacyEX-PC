#ifndef GAME_CORE_STATEMAP_HPP
#define GAME_CORE_STATEMAP_HPP

#include <string>
#include <vector>

#include "game/core/AbstractStateMapper.hpp"

class BlockState;
class Property;

class StateMap : public AbstractStateMapper
{
public:
    class Builder
    {
    public:
        Builder();
        Builder& nameFrom(const Property* property);
        Builder& withSuffix(const std::wstring& suffix);
        Builder& deleteProperties(int count, ...);
        StateMap* build();
    };

    StateMap(const Property* nameProperty, const std::wstring& suffix, std::vector<const Property*> deletedProperties);
    virtual ModelResourceLocation* getLocation(const BlockState* state);

private:
    const Property* mNameProperty;
    std::wstring mSuffix;
    std::vector<const Property*> mDeletedProperties;
};

#endif
