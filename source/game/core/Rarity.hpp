#ifndef GAME_CORE_RARITY_HPP
#define GAME_CORE_RARITY_HPP

#include <string>

#include "game/core/ColourTable.hpp"

class Rarity
{
public:
    static const Rarity* COMMON;
    static const Rarity* UNCOMMON;
    static const Rarity* RARE;
    static const Rarity* EPIC;

    Rarity(eMinecraftColour colour, const std::wstring& name, int weight);

    unsigned int getWeight() const;

    eMinecraftColour colour;
    std::wstring name;

private:
    int weight;
};

#endif
