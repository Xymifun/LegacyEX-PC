#ifndef GAME_CORE_IOEXCEPTION_HPP
#define GAME_CORE_IOEXCEPTION_HPP

#include <exception>
#include <string>

class IOException : public std::exception
{
public:
    IOException(const std::wstring& message);

private:
    std::wstring mMessage;
};

#endif
