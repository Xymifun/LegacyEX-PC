#include "game/core/Joiner.hpp"

Joiner Joiner::on(const std::wstring& separator)
{
    return Joiner(separator);
}

Joiner Joiner::on(wchar_t separator)
{
    return Joiner(separator);
}

Joiner::Joiner(const std::wstring& separator) : mSeparator(separator)
{
}

Joiner::Joiner(wchar_t separator) : mSeparator(std::wstring() + separator)
{
}

void Joiner::appendTo(std::wstring& out, const std::vector<std::wstring>& parts) const
{
    for (std::vector<std::wstring>::const_iterator part = parts.begin(); part != parts.end(); ++part)
    {
        if (part + 1 != parts.end())
        {
            out += mSeparator;
        }

        out += *part;
    }
}
