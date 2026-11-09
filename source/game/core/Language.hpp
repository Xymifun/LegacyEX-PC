#ifndef GAME_CORE_LANGUAGE_HPP
#define GAME_CORE_LANGUAGE_HPP

#include <cstdarg>
#include <string>

class Language
{
public:
    Language();

    static Language* getInstance();

    std::wstring getElement(const std::wstring& key, ...);
    std::wstring getElement(const std::wstring& key, va_list args);
    std::wstring getElementName(const std::wstring& key);
    std::wstring getElementDescription(const std::wstring& key);
};

#endif
