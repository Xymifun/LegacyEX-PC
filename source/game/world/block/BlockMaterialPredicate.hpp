#ifndef GAME_WORLD_BLOCK_BLOCKMATERIALPREDICATE_HPP
#define GAME_WORLD_BLOCK_BLOCKMATERIALPREDICATE_HPP

#include "game/world/block/Predicate.hpp"

class BlockState;
class Material;

class BlockMaterialPredicate : public Predicate<const BlockState*>
{
public:
    static Predicate<const BlockState*>* forMaterial(Material* material);

    BlockMaterialPredicate(Material* material);

    virtual bool apply(const BlockState* state) const;
    virtual ~BlockMaterialPredicate();

private:
    Material* mMaterial;
};

#endif
