#include "game/world/generation/GlideRingGenerator.hpp"

#include <string>

#include "game/core/IconRegister.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"

static const int GLIDE_RING_CORNER_COUNTS[GlideRingGenerator::eGlideRingSize_Count] = {0, 1, 3};
static const int GLIDE_RING_EDGE_RADIUS[GlideRingGenerator::eGlideRingSize_Count] = {2, 3, 4};
static const int GLIDE_RING_CORNER_RADIUS[GlideRingGenerator::eGlideRingSize_Count] = {0, 2, 3};
static const int GLIDE_RING_EDGE_OFFSETS[3] = {-1, 0, 1};
static const int GLIDE_RING_CORNER_OFFSETS[3] = {-2, -3, 2};

static TextureAtlasSprite* GLIDE_RING_ICONS[GlideRingGenerator::eGlideRingSize_Count];

static const std::wstring GLIDE_RING_ICON_NAMES[GlideRingGenerator::eGlideRingSize_Count] = {
    std::wstring(L"diamondRing"), std::wstring(L"goldRing"), std::wstring(L"emeraldRing")};

BlockPos GlideRingGenerator::GetNextPos(const BlockPos& centre, eGlideRingOrientation orientation, int firstMod,
                                        int secondMod)
{
    switch (orientation)
    {
    case eGlideRingOrientation_XY:
        return BlockPos(centre.x + firstMod, centre.y + secondMod, centre.z);
    case eGlideRingOrientation_YZ:
        return BlockPos(centre.x, centre.y + firstMod, centre.z + secondMod);
    case eGlideRingOrientation_XZ:
        return BlockPos(centre.x + firstMod, centre.y, centre.z + secondMod);
    default:
        return BlockPos();
    }
}

int GlideRingGenerator::GetFirstMod(eGlideRingSize size, eGlideRingSection section, int index)
{
    switch (section)
    {
    case eGlideRingSection_Right:
        return GLIDE_RING_EDGE_RADIUS[size];
    case eGlideRingSection_Left:
        return -GLIDE_RING_EDGE_RADIUS[size];
    case eGlideRingSection_Bottom:
    case eGlideRingSection_Top:
        return GLIDE_RING_EDGE_OFFSETS[index];
    case eGlideRingSection_LeftCorner:
        return -GLIDE_RING_CORNER_RADIUS[size];
    case eGlideRingSection_RightCorner:
        return GLIDE_RING_CORNER_RADIUS[size];
    case eGlideRingSection_BottomCorner:
        return GLIDE_RING_CORNER_OFFSETS[index];
    case eGlideRingSection_TopCorner:
        return -GLIDE_RING_CORNER_OFFSETS[index];
    default:
        return 0;
    }
}

int GlideRingGenerator::GetSecondMod(eGlideRingSize size, eGlideRingSection section, int index)
{
    switch (section)
    {
    case eGlideRingSection_Right:
    case eGlideRingSection_Left:
        return GLIDE_RING_EDGE_OFFSETS[index];
    case eGlideRingSection_Bottom:
        return -GLIDE_RING_EDGE_RADIUS[size];
    case eGlideRingSection_Top:
        return GLIDE_RING_EDGE_RADIUS[size];
    case eGlideRingSection_LeftCorner:
        return -GLIDE_RING_CORNER_OFFSETS[index];
    case eGlideRingSection_RightCorner:
        return GLIDE_RING_CORNER_OFFSETS[index];
    case eGlideRingSection_BottomCorner:
        return -GLIDE_RING_CORNER_RADIUS[size];
    case eGlideRingSection_TopCorner:
        return GLIDE_RING_CORNER_RADIUS[size];
    default:
        return 0;
    }
}

void GlideRingGenerator::GetSectionPositions(std::vector<BlockPos>& positions, const BlockPos& centre,
                                             eGlideRingSize size, eGlideRingSection section,
                                             eGlideRingOrientation orientation)
{
    int count = 3;
    if (section >= eGlideRingSection_LeftCorner && section <= eGlideRingSection_TopCorner)
    {
        count = GLIDE_RING_CORNER_COUNTS[size];
    }

    for (int index = 0; index < count; index++)
    {
        int firstMod = GetFirstMod(size, section, index);
        int secondMod = GetSecondMod(size, section, index);
        positions.push_back(GetNextPos(centre, orientation, firstMod, secondMod));
    }
}

void GlideRingGenerator::BuildRing(std::vector<BlockPos>& positions, const AABB* bounds, eGlideRingSize size)
{
    BlockPos centre(bounds->getCenter()->x, bounds->getCenter()->y, bounds->getCenter()->z);

    double sizeX = bounds->mMaxX - bounds->mMinX;
    double sizeY = bounds->mMaxY - bounds->mMinY;
    double sizeZ = bounds->mMaxZ - bounds->mMinZ;

    eGlideRingOrientation orientation;
    if (sizeY > sizeX && sizeZ > sizeX)
    {
        orientation = eGlideRingOrientation_YZ;
    }
    else if (sizeX > sizeY && sizeZ > sizeY)
    {
        orientation = eGlideRingOrientation_XZ;
    }
    else if (sizeX > sizeZ && sizeY > sizeZ)
    {
        orientation = eGlideRingOrientation_XY;
    }
    else
    {
        return;
    }

    for (int section = 0; section < eGlideRingSection_Count; section++)
    {
        GetSectionPositions(positions, centre, size, (eGlideRingSection)section, orientation);
    }
}

void GlideRingGenerator::registerIcons(IconRegister* iconRegister)
{
    for (int size = 0; size < eGlideRingSize_Count; size++)
    {
        GLIDE_RING_ICONS[size] = iconRegister->registerIcon(GLIDE_RING_ICON_NAMES[size]);
    }
}

TextureAtlasSprite* GlideRingGenerator::getIcon(eGlideRingSize size)
{
    return GLIDE_RING_ICONS[size];
}
