#ifndef GAME_WORLD_BLOCK_ROTATEDPILLARBLOCKIMPL_HPP
#define GAME_WORLD_BLOCK_ROTATEDPILLARBLOCKIMPL_HPP

#include <string>

#include "game/core/Direction.hpp"
#include "game/world/block/RotatedPillarBlock.hpp"

class BlockState;
class IconRegister;
class Material;
class MaterialColor;

class RotatedPillarBlockImpl : public RotatedPillarBlock
{
public:
    RotatedPillarBlockImpl(Material* material);
    RotatedPillarBlockImpl(Material* material, const MaterialColor* materialColor);
    virtual ~RotatedPillarBlockImpl() {}

    virtual void DerivedInit();
    virtual Direction::Axis* getAxis(const BlockState* state);
    RotatedPillarBlockImpl* setIconTopName(const std::wstring& name);
    virtual void registerIcons(IconRegister* iconRegister);

private:
    std::wstring mIconTopName;
};

#endif
