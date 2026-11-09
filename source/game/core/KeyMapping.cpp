#include "game/core/KeyMapping.hpp"

KeyMapping::KeyMapping(const std::wstring& name, int key)
{
    mName.assign(name, 0, 0xffffffff);
    mKey = key;
}
