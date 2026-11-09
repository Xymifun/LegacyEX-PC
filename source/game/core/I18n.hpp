#ifndef GAME_CORE_I18N_HPP
#define GAME_CORE_I18N_HPP

#include <cstdarg>
#include <string>

class I18n
{
public:
    static std::wstring get(const std::wstring& key, ...);
    static std::wstring get(const std::wstring& key, va_list args);
};

#endif
