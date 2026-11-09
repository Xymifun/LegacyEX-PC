#include "game/core/Boxed.hpp"

Boxed::Boxed() : mValueClass(NULL)
{
}

Boxed::Boxed(const std::type_info* type) : mValueClass(type)
{
}

Boxed::~Boxed()
{
}

bool Boxed::isA(const std::type_info* type) const
{
    return *mValueClass == *type;
}

bool Boxed::isA(const std::type_info& type) const
{
    return *mValueClass == type;
}

bool Boxed::operator!=(const Boxed* value) const
{
    return !operator==(value);
}

bool Boxed::operator==(Boxed* value) const
{
    return operator==(static_cast<const Boxed*>(value));
}

bool Boxed::operator!=(Boxed* value) const
{
    return !operator==(static_cast<const Boxed*>(value));
}
