#include "game/core/StringUtils.hpp"

#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>

std::vector<std::wstring>& stringSplit(const std::wstring& text, wchar_t separator, std::vector<std::wstring>& result)
{
    std::wstringstream stream(text);
    std::wstring field;
    while (std::getline(stream, field, separator))
    {
        result.push_back(field);
    }
    return result;
}

std::vector<std::wstring> stringSplit(const std::wstring& text, wchar_t separator)
{
    std::vector<std::wstring> result;
    return stringSplit(text, separator, result);
}

bool BothAreSpaces(wchar_t first, wchar_t second)
{
    return first == second && first == L' ';
}

std::wstring toLower(const std::wstring& text)
{
    std::wstring result(text);
    std::transform(result.begin(), result.end(), result.begin(), tolower);
    return result;
}

std::wstring trimString(const std::wstring& text)
{
    std::wstring result;
    int first = text.find_first_not_of(L" \t\n\r");
    int last = text.find_last_not_of(L" \t\n\r");
    if (first == std::wstring::npos)
    {
        first = 0;
    }
    if (last == std::wstring::npos)
    {
        last = -1;
    }
    if (first == 0 && last < 0)
    {
        result = L"";
    }
    else
    {
        result = text.substr(first, last - first + 1);
    }
    return result;
}

void stripWhitespaceForHtml(std::wstring& text, bool collapse)
{
    if (collapse)
    {
        text.erase(std::remove(text.begin(), text.end(), '\n'), text.end());
        text.erase(std::remove(text.begin(), text.end(), '\r'), text.end());
    }
    text.erase(std::remove(text.begin(), text.end(), '\t'), text.end());
    text.erase(std::unique(text.begin(), text.end(), BothAreSpaces), text.end());
    text = trimString(text);
}

std::wstring replaceAll(const std::wstring& text, const std::wstring& search, const std::wstring& replacement)
{
    std::wstring result(text);
    std::wstring::size_type position = 0;
    while ((position = result.find(search, position)) != std::wstring::npos)
    {
        result.replace(position, search.length(), replacement);
        ++position;
    }
    return result;
}

void makeReplacements(const boost::unordered_map<std::wstring, std::wstring>& replacements, std::wstring& text)
{
    boost::unordered_map<std::wstring, std::wstring>::const_iterator it = replacements.begin();
    while (it != replacements.end())
    {
        text = replaceAll(text, it->first, it->second);
        ++it;
    }
}

void toUtf8(const std::wstring& text, char** output, unsigned int& length)
{
}

bool equalsIgnoreCase(const std::wstring& first, const std::wstring& second)
{
    return toLower(first) == toLower(second);
}

std::wstring escapeXML(const std::wstring& text)
{
    std::wstring result = text;
    result = replaceAll(result, L"&", L"&amp;");
    result = replaceAll(result, L"<", L"&lt;");
    result = replaceAll(result, L">", L"&gt;");
    return result;
}
