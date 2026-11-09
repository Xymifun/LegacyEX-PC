#include "game/client/render/PlayerModelPart.hpp"

const PlayerModelPart* PlayerModelPart::CAPE = new PlayerModelPart(0);
const PlayerModelPart* PlayerModelPart::JACKET = new PlayerModelPart(1);
const PlayerModelPart* PlayerModelPart::LEFT_SLEEVE = new PlayerModelPart(2);
const PlayerModelPart* PlayerModelPart::RIGHT_SLEEVE = new PlayerModelPart(3);
const PlayerModelPart* PlayerModelPart::LEFT_PANTS_LEG = new PlayerModelPart(4);
const PlayerModelPart* PlayerModelPart::RIGHT_PANTS_LEG = new PlayerModelPart(5);
const PlayerModelPart* PlayerModelPart::HAT = new PlayerModelPart(6);

PlayerModelPart::Internal PlayerModelPart::INTERNALS[7] = {PlayerModelPart::Internal(0, L"cape"),
                                                           PlayerModelPart::Internal(1, L"jacket"),
                                                           PlayerModelPart::Internal(2, L"left_sleeve"),
                                                           PlayerModelPart::Internal(3, L"right_sleeve"),
                                                           PlayerModelPart::Internal(4, L"left_pants_leg"),
                                                           PlayerModelPart::Internal(5, L"right_pants_leg"),
                                                           PlayerModelPart::Internal(6, L"hat")};

PlayerModelPart::PlayerModelPart(int part) : part(part)
{
}

int PlayerModelPart::getMask() const
{
    return INTERNALS[part].mask;
}

int PlayerModelPart::getBit() const
{
    return INTERNALS[part].bit;
}

std::wstring PlayerModelPart::getId() const
{
    return INTERNALS[part].id;
}

PlayerModelPart::Internal::Internal(int bit, const std::wstring& id) : bit(bit), mask(1 << bit), id(id)
{
}
