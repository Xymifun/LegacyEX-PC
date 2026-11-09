#ifndef GAME_CLIENT_RENDER_PLAYERMODELPART_HPP
#define GAME_CLIENT_RENDER_PLAYERMODELPART_HPP

#include <string>

class PlayerModelPart
{
public:
    static const PlayerModelPart* CAPE;
    static const PlayerModelPart* JACKET;
    static const PlayerModelPart* LEFT_SLEEVE;
    static const PlayerModelPart* RIGHT_SLEEVE;
    static const PlayerModelPart* LEFT_PANTS_LEG;
    static const PlayerModelPart* RIGHT_PANTS_LEG;
    static const PlayerModelPart* HAT;

    PlayerModelPart(int part);

    int getMask() const;
    int getBit() const;
    std::wstring getId() const;

private:
    class Internal
    {
    public:
        Internal(int bit, const std::wstring& id);

        int bit;
        int mask;
        std::wstring id;
    };

    static Internal INTERNALS[7];

    int part;
};

#endif
