#ifndef GAME_CORE_ILLEGALARGUMENTEXCEPTION_HPP
#define GAME_CORE_ILLEGALARGUMENTEXCEPTION_HPP

#include <exception>
#include <string>

class IllegalArgumentException : public std::exception
{
public:
    IllegalArgumentException(const std::wstring& message);

private:
    std::wstring mMessage;
};

#endif
