#ifndef GAME_CORE_BANNERPATTERN_HPP
#define GAME_CORE_BANNERPATTERN_HPP

#include <string>
#include <vector>

#include "game/world/biome/arrayWithLength.hpp"

class ItemInstance;

class BannerPattern
{
public:
    BannerPattern(unsigned int id, const std::wstring& hashname, int* displayName, wchar_t craftingMenuTab);
    BannerPattern(unsigned int id, const std::wstring& hashname, const std::wstring& pattern0,
                  const std::wstring& pattern1, const std::wstring& pattern2, int* displayName,
                  wchar_t craftingMenuTab);
    BannerPattern(unsigned int id, const std::wstring& hashname, ItemInstance* patternItem, int* displayName,
                  wchar_t craftingMenuTab);

    int getDisplayName(int index) const;
    static BannerPattern* byHash(const std::wstring& hashname);
    std::wstring getHashname() const;
    void _init(unsigned int id, const std::wstring& hashname, int* displayName, wchar_t craftingMenuTab);
    static void staticCtor();
    arrayWithLength<std::wstring> getPatterns() const;
    bool hasPattern() const;
    bool hasPatternItem() const;
    ItemInstance* getPatternItem() const;
    unsigned int getAtlasIndex() const;
    unsigned short getCraftingMenuTab() const;

    static std::vector<const BannerPattern*> PATTERNS;
    static const BannerPattern* BASE;
    static BannerPattern* SQUARE_BOTTOM_LEFT;
    static BannerPattern* SQUARE_BOTTOM_RIGHT;
    static BannerPattern* SQUARE_TOP_LEFT;
    static BannerPattern* SQUARE_TOP_RIGHT;
    static BannerPattern* STRIPE_BOTTOM;
    static BannerPattern* STRIPE_TOP;
    static BannerPattern* STRIPE_LEFT;
    static BannerPattern* STRIPE_RIGHT;
    static BannerPattern* STRIPE_CENTER;
    static BannerPattern* STRIPE_MIDDLE;
    static BannerPattern* STRIPE_DOWNRIGHT;
    static BannerPattern* STRIPE_DOWNLEFT;
    static BannerPattern* STRIPE_SMALL;
    static BannerPattern* CROSS;
    static BannerPattern* STRAIGHT_CROSS;
    static BannerPattern* TRIANGLE_BOTTOM;
    static BannerPattern* TRIANGLE_TOP;
    static BannerPattern* TRIANGLES_BOTTOM;
    static BannerPattern* TRIANGLES_TOP;
    static BannerPattern* DIAGONAL_LEFT;
    static BannerPattern* DIAGONAL_RIGHT;
    static BannerPattern* DIAGONAL_LEFT_MIRROR;
    static BannerPattern* DIAGONAL_RIGHT_MIRROR;
    static BannerPattern* CIRCLE_MIDDLE;
    static BannerPattern* RHOMBUS_MIDDLE;
    static BannerPattern* HALF_VERTICAL;
    static BannerPattern* HALF_HORIZONTAL;
    static BannerPattern* HALF_VERTICAL_MIRROR;
    static BannerPattern* HALF_HORIZONTAL_MIRROR;
    static BannerPattern* BORDER;
    static BannerPattern* CURLY_BORDER;
    static BannerPattern* CREEPER;
    static BannerPattern* GRADIENT;
    static BannerPattern* GRADIENT_UP;
    static BannerPattern* BRICKS;
    static BannerPattern* SKULL;
    static BannerPattern* FLOWER;
    static BannerPattern* MOJANG;

private:
    std::wstring hashname;
    std::wstring patterns[3];
    ItemInstance* patternItem;
    unsigned int atlasIndex;
    unsigned short craftingMenuTab;
    int* displayName;
};

#endif
