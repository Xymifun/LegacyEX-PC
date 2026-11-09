#ifndef GAME_CORE_RIDERPOSITIONOVERRIDES_HPP
#define GAME_CORE_RIDERPOSITIONOVERRIDES_HPP

#include <vector>

#include <boost/shared_ptr.hpp>

class DataInputStream;
class Entity;

struct PositionOverrideStruct
{
    bool isTamed;
    bool isSaddled;
    double x;
    double y;
    double z;
};

class RiderPositionOverrides
{
public:
    static void ClearOverrides();
    static void ReadOverrides(DataInputStream* stream);
    static bool OverridePosition(const boost::shared_ptr<Entity>& vehicle, const boost::shared_ptr<Entity>& passenger);
};

#endif
