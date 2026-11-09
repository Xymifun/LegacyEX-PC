#ifndef GAME_CORE_GAMETYPE_HPP
#define GAME_CORE_GAMETYPE_HPP

#include <string>

class Abilities;

class GameType
{
public:
    static const GameType* NOT_SET;
    static const GameType* SURVIVAL;
    static const GameType* CREATIVE;
    static const GameType* ADVENTURE;
    static const GameType* SPECTATOR;
    static const GameType* LOBBY;
    GameType(int id, const std::wstring& name, const std::wstring& shortName, int label, int sliderLabel);
    static void staticCtor();
    int getId() const;
    std::wstring getName() const;
    int getLabel() const;
    int getSliderLabel() const;
    bool isBlockPlacingRestricted() const;
    void updatePlayerAbilities(Abilities* abilities) const;
    bool isCreative() const;
    bool isSurvival() const;
    static const GameType* byId(int id, const GameType* defaultType);
    static const GameType* byId(int id);
    static const GameType* byName(const std::wstring& name, const GameType* defaultType);
    static const GameType* byName(const std::wstring& name);

private:
    int id;
    std::wstring name;
    std::wstring shortName;
    int label;
    int sliderLabel;
};

#endif
