#include "game/core/StringUtil.hpp"

void findAndReplace(std::wstring& value, std::wstring search, std::wstring replacement)
{
    std::wstring::size_type position = 0;
    while ((position = value.find(search.c_str(), position, search.size())) != std::wstring::npos)
    {
        value.replace(position, search.size(), replacement, 0, std::wstring::npos);
        ++position;
    }
}

std::wstring escape(std::wstring text)
{
    std::wstring escaped = text;
    findAndReplace(escaped, L"&", L"&amp;");
    findAndReplace(escaped, L"<", L"&lt;");
    findAndReplace(escaped, L">", L"&gt;");
    return escaped;
}
