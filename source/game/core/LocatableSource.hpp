#ifndef GAME_CORE_LOCATABLESOURCE_HPP
#define GAME_CORE_LOCATABLESOURCE_HPP

#include "game/core/Location.hpp"
#include "game/core/Source.hpp"

class LocatableSource : public Source, public Location
{
public:
    virtual ~LocatableSource() {}
};

#endif
