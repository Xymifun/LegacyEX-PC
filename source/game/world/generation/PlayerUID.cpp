#include "game/world/generation/PlayerUID.hpp"

#include <cstring>
#include <cwchar>

#include <boost/functional/hash.hpp>

#include <nn/ffl/FFLCreateID.h>

#include "game/core/StringConversion.hpp"

PlayerUID::PlayerUID(int)
{
    std::memset(this, 0, sizeof(PlayerUID));
}

PlayerUID::PlayerUID(ACTUuid uuid, bool onlineAccount, bool drcController)
{
    this->uuid = uuid;
    flags = (((onlineAccount & 1u) | 2u) << 2 | (drcController & 1u)) << FlagShift | (flags & FlagPayloadMask);
}

PlayerUID::PlayerUID(FFLCreateID createId, unsigned int principalId, bool hasPrincipalId, bool drcController)
{
    std::memcpy(mii.createId, &createId, sizeof(mii.createId));
    mii.principalId = hasPrincipalId ? principalId : 0;
    flags = ((hasPrincipalId & 1u) << 1 | (drcController & 1u)) << FlagShift | (flags & FlagPayloadMask);
}

PlayerUID::PlayerUID(std::wstring uidString)
{
    if (uidString.length() > 0x1c)
    {
        for (int i = 0; i < 16; ++i)
            uuid.data[i] = (unsigned char)_fromHEXString<int>(uidString.substr(i * 2, 2));
        flags = (flags & FlagUuidMask) | FlagUuid;
    }
    else if (uidString.length() > 0x14)
    {
        for (int i = 0; i < 10; ++i)
            mii.createId[i] = (unsigned char)_fromHEXString<int>(uidString.substr(i * 2, 2));
        for (int i = 0; i < 4; ++i)
        {
            mii.principalId = 0;
            mii.principalIdBytes[i] = (unsigned char)_fromHEXString<int>(uidString.substr(20 + i * 2, 2));
        }
        flags = (flags & FlagMiiMask) | FlagMiiPrincipal;
    }
    else
    {
        for (int i = 0; i < 10; ++i)
            mii.createId[i] = (unsigned char)_fromHEXString<int>(uidString.substr(i * 2, 2));
        mii.principalId = 0;
        flags &= FlagMiiMask;
    }
}

std::wstring PlayerUID::UuidString() const
{
    wchar_t text[40];
    swprintf(text, 40, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", uuid.data[0], uuid.data[1],
             uuid.data[2], uuid.data[3], uuid.data[4], uuid.data[5], uuid.data[6], uuid.data[7], uuid.data[8],
             uuid.data[9], uuid.data[10], uuid.data[11], uuid.data[12], uuid.data[13], uuid.data[14], uuid.data[15]);
    return std::wstring(text);
}

std::wstring PlayerUID::toString() const
{
    wchar_t text[40];
    if ((flags & FlagUuid) != 0)
        return UuidString();
    if ((flags & FlagMiiPrincipal) != 0)
    {
        swprintf(text, 40, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", mii.createId[0],
                 mii.createId[1], mii.createId[2], mii.createId[3], mii.createId[4], mii.createId[5], mii.createId[6],
                 mii.createId[7], mii.createId[8], mii.createId[9], mii.principalIdBytes[0], mii.principalIdBytes[1],
                 mii.principalIdBytes[2], mii.principalIdBytes[3]);
    }
    else
    {
        swprintf(text, 40, L"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", mii.createId[0], mii.createId[1],
                 mii.createId[2], mii.createId[3], mii.createId[4], mii.createId[5], mii.createId[6], mii.createId[7],
                 mii.createId[8], mii.createId[9]);
    }
    return std::wstring(text);
}

bool PlayerUID::operator==(const PlayerUID& other) const
{
    unsigned int uuidType = (flags & FlagUuid) >> 31;
    if (uuidType != ((other.flags & FlagUuid) >> 31))
        return false;
    if (uuidType != 0)
        return uuid == other.uuid;
    return mii.principalId == other.mii.principalId && std::memcmp(this, &other, 10) == 0;
}

bool PlayerUID::operator!=(const PlayerUID& other) const
{
    return !(*this == other);
}

unsigned int hash_value(const PlayerUID& uid)
{
    std::size_t seed = 0;
    if ((uid.flags & PlayerUID::FlagUuid) == 0)
    {
        for (int i = 0; i < 10; ++i)
        {
            boost::hash_combine(seed, uid.mii.createId[i]);
        }
        boost::hash_combine(seed, uid.mii.principalId);
    }
    else
    {
        for (int i = 0; i < 16; ++i)
        {
            boost::hash_combine(seed, uid.uuid.data[i]);
        }
    }
    return seed;
}
