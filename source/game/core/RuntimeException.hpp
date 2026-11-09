#ifndef GAME_CORE_RUNTIMEEXCEPTION_HPP
#define GAME_CORE_RUNTIMEEXCEPTION_HPP

#include <exception>

class RuntimeException : public std::exception
{
public:
    ~RuntimeException() throw() {}
};

#endif
