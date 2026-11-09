#ifndef GAME_WORLD_GENERATION_GLIDERINGGENERATOR_HPP
#define GAME_WORLD_GENERATION_GLIDERINGGENERATOR_HPP

#include <vector>
#include <string>

#include "game/world/block/BlockPos.hpp"

class AABB;
class IconRegister;
class TextureAtlasSprite;

class GlideRingGenerator
{
public:
    enum eGlideRingSize
    {
        eGlideRingSize_Small = 0,
        eGlideRingSize_Medium = 1,
        eGlideRingSize_Large = 2,
        eGlideRingSize_Count = 3
    };

    enum eGlideRingSection
    {
        eGlideRingSection_Right = 0,
        eGlideRingSection_Left = 1,
        eGlideRingSection_Bottom = 2,
        eGlideRingSection_Top = 3,
        eGlideRingSection_LeftCorner = 4,
        eGlideRingSection_RightCorner = 5,
        eGlideRingSection_BottomCorner = 6,
        eGlideRingSection_TopCorner = 7,
        eGlideRingSection_Count = 8
    };

    enum eGlideRingOrientation
    {
        eGlideRingOrientation_XY = 0,
        eGlideRingOrientation_YZ = 1,
        eGlideRingOrientation_XZ = 2
    };

    static void BuildRing(std::vector<BlockPos>& positions, const AABB* bounds, eGlideRingSize size);

    static void registerIcons(IconRegister* iconRegister);
    static TextureAtlasSprite* getIcon(eGlideRingSize size);

private:
    static BlockPos GetNextPos(const BlockPos& centre, eGlideRingOrientation orientation, int firstMod, int secondMod);

    static int GetFirstMod(eGlideRingSize size, eGlideRingSection section, int index);
    static int GetSecondMod(eGlideRingSize size, eGlideRingSection section, int index);

    static void GetSectionPositions(std::vector<BlockPos>& positions, const BlockPos& centre, eGlideRingSize size,
                                    eGlideRingSection section, eGlideRingOrientation orientation);
};

#endif
