#include "game/core/IdsStringBuilder.hpp"
#include "game/core/L10N.hpp"

IdsStringBuilder::IdsStringBuilder(int id) : id(id)
{
}

IdsStringBuilder::~IdsStringBuilder()
{
}

std::wstring IdsStringBuilder::getString()
{
    return std::wstring(L10N::GetString(id));
}
