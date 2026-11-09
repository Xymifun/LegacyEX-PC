#include "platform/wiiu/WiiU_Platform.hpp"

#include "game/core/L10N.hpp"

wchar_t* GetRichPresenceStringFromId(int id)
{
    return const_cast<wchar_t*>(L10N::GetString(id));
}
