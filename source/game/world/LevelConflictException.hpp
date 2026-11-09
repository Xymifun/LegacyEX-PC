#ifndef GAME_WORLD_LEVELCONFLICTEXCEPTION_HPP
#define GAME_WORLD_LEVELCONFLICTEXCEPTION_HPP

#include "game/core/RuntimeException.hpp"
#include <string>

class LevelConflictException : public RuntimeException
{
public:
    LevelConflictException(const std::wstring& message);
    ~LevelConflictException();
};

#endif
