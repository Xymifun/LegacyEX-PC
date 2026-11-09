#include "game/entity/mob/EntityPaintingItemFrameDirectionFix.hpp"

#include <string>

class Vec3i;
#define GAME_UTIL_VEC3I_HPP
#include "game/core/Direction.hpp"
#undef GAME_UTIL_VEC3I_HPP
#include "game/data/CompoundTag.hpp"

EntityPaintingItemFrameDirectionFix::~EntityPaintingItemFrameDirectionFix()
{
}

int EntityPaintingItemFrameDirectionFix::getVersion()
{
    return 111;
}

CompoundTag* EntityPaintingItemFrameDirectionFix::fix(CompoundTag* tag)
{
    bool painting = CheckID(tag, L"Painting");
    bool itemFrame = CheckID(tag, L"ItemFrame");

    if ((painting || itemFrame) && !tag->contains(L"Facing", Tag::TAG_ANY_NUMERIC))
    {
        const Direction* direction;

        if (tag->contains(L"Direction", Tag::TAG_ANY_NUMERIC))
        {
            direction = Direction::from2DDataValue(tag->getByte(L"Direction"));

            tag->putInt(L"TileX", tag->getInt(L"TileX") + direction->getStepX());
            tag->putInt(L"TileY", tag->getInt(L"TileY") + direction->getStepY());
            tag->putInt(L"TileZ", tag->getInt(L"TileZ") + direction->getStepZ());

            tag->remove(L"Direction");

            if (itemFrame && tag->contains(L"ItemRotation", Tag::TAG_ANY_NUMERIC))
            {
                tag->putByte(L"ItemRotation", tag->getByte(L"ItemRotation") * 2);
            }
        }
        else
        {
            direction = Direction::from2DDataValue(tag->getByte(L"Dir"));

            tag->remove(L"Dir");
        }

        tag->putByte(L"Facing", direction->get2DDataValue());
    }

    return tag;
}
