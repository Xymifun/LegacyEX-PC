#include "game/core/Language.hpp"

static Language* sLanguage = 0;

Language* Language::getInstance()
{
    return sLanguage;
}

std::wstring Language::getElement(const std::wstring& key, ...)
{
    return L"";
}

std::wstring Language::getElement(const std::wstring& key, va_list args)
{
    return L"";
}
