#include "game/util/ACTUuid.hpp"

bool ACTUuid::operator==(const ACTUuid& other) const
{
    return data[0] == other.data[0] && data[1] == other.data[1] && data[2] == other.data[2] && data[3] == other.data[3]
           && data[4] == other.data[4] && data[5] == other.data[5] && data[6] == other.data[6]
           && data[7] == other.data[7] && data[8] == other.data[8] && data[9] == other.data[9]
           && data[10] == other.data[10] && data[11] == other.data[11] && data[12] == other.data[12]
           && data[13] == other.data[13] && data[14] == other.data[14] && data[15] == other.data[15];
}
