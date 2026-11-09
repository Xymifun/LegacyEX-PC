#ifndef GAME_CORE_USER_HPP
#define GAME_CORE_USER_HPP

#include <string>
#include <vector>

class Block;

class User
{
public:
    User(const std::wstring& name, const std::wstring& sessionId);

    static void staticCtor();

    static std::vector<Block*> creativeBlocks;

    std::wstring name;
    std::wstring sessionId;
    std::wstring playerId;
};

#endif
