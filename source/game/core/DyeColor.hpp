#ifndef GAME_CORE_DYECOLOR_HPP
#define GAME_CORE_DYECOLOR_HPP

#include <string>
#include <vector>

#include "game/core/ColourTable.hpp"
#include "game/core/StringRepresentable.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DyeColor : public StringRepresentable
{
public:
    static const DyeColor* WHITE;
    static const DyeColor* ORANGE;
    static const DyeColor* MAGENTA;
    static const DyeColor* LIGHT_BLUE;
    static const DyeColor* YELLOW;
    static const DyeColor* LIME;
    static const DyeColor* PINK;
    static const DyeColor* GRAY;
    static const DyeColor* SILVER;
    static const DyeColor* CYAN;
    static const DyeColor* PURPLE;
    static const DyeColor* BLUE;
    static const DyeColor* BROWN;
    static const DyeColor* GREEN;
    static const DyeColor* RED;
    static const DyeColor* BLACK;

    static std::vector<const DyeColor*> COLORS;

    DyeColor(int blockData, int itemData, const std::wstring& serializedName, int name, int useName,
             int textureDiffuseColor, eMinecraftColour fontColorFormat, eMinecraftColour shulkerColor,
             eMinecraftColour bedColor);
    ~DyeColor();

    static void staticCtor();

    static const DyeColor* byBlockData(int blockData);
    static const DyeColor* byItemData(int itemData);

    int getBlockData() const;
    int getItemData() const;
    int getName() const;
    int getUseName() const;
    int getTextureDiffuseColor() const;
    arrayWithLength<const float> getTextureDiffuseColors() const;
    eMinecraftColour getFontColorFormat() const;
    eMinecraftColour getShulkerColor() const;
    eMinecraftColour getBedColor() const;
    std::wstring getTexture() const;

    virtual std::wstring toString() const;
    virtual std::wstring getSerializedName() const;

    static const std::wstring TEXTURE_NAMES[16];

private:
    friend class CraftingMenu;
    friend class InventoryMenu;
    friend class StainedGlassBlock;

    static const DyeColor* BY_BLOCK_DATA[16];
    static const DyeColor* BY_ITEM_DATA[16];

    int blockData;
    int itemData;
    std::wstring serializedName;
    int name;
    int useName;
    int textureDiffuseColor;
    arrayWithLength<const float> textureDiffuseColors;
    eMinecraftColour fontColorFormat;
    eMinecraftColour shulkerColor;
    eMinecraftColour bedColor;
};

#endif
