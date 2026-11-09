#ifndef GAME_WORLD_BLOCK_HALFTRANSPARENTBLOCK_HPP
#define GAME_WORLD_BLOCK_HALFTRANSPARENTBLOCK_HPP

#include "game/world/block/Block.hpp"

#include <string>

class BlockPos;
class BlockState;
class Direction;
class IconRegister;
class LevelSource;
class Material;
class MaterialColor;

class HalfTransparentBlock : public Block
{
public:
    HalfTransparentBlock(const std::wstring& iconName, Material* material, bool ignoreSimilarity);
    HalfTransparentBlock(const std::wstring& iconName, Material* material, bool ignoreSimilarity,
                         const MaterialColor* materialColor);

    virtual ~HalfTransparentBlock() {}
    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                  const Direction* direction);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual void registerIcons(IconRegister* iconRegister);

protected:
    bool mIgnoreSimilarity;
    std::wstring mTextureName;
};

#endif
