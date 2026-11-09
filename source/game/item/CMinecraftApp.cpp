#include <string>

static char s_wideStringBuffer[1024];

char* wstringtochararray(const std::wstring& value)
{
    char* buffer = s_wideStringBuffer;
    std::wstring::size_type length = value.size();
    for (std::wstring::size_type index = 0; index < length; ++index)
        buffer[index] = static_cast<char>(value[index]);
    buffer[length] = 0;
    return buffer;
}
