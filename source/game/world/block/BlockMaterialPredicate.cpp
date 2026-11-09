#include "game/world/block/BlockMaterialPredicate.hpp"

#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

BlockMaterialPredicate::BlockMaterialPredicate(Material* material) : mMaterial(material)
{
}

Predicate<const BlockState*>* BlockMaterialPredicate::forMaterial(Material* material)
{
    return new BlockMaterialPredicate(material);
}

bool BlockMaterialPredicate::apply(const BlockState* state) const
{
    return state != NULL && state->getMaterial() == mMaterial;
}

BlockMaterialPredicate::~BlockMaterialPredicate()
{
}
