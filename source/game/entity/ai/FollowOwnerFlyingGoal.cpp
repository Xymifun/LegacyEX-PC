#include "game/entity/ai/FollowOwnerFlyingGoal.hpp"

#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

FollowOwnerFlyingGoal::FollowOwnerFlyingGoal(TamableAnimal* tamable, double speedModifier, float startDistance,
                                             float stopDistance)
    : FollowOwnerGoal(tamable, speedModifier, startDistance, stopDistance)
{
}

bool FollowOwnerFlyingGoal::isTeleportFriendlyBlock(int x, int z, int y, int dx, int dz)
{
    const BlockState* blockState = mLevel->getBlockState(BlockPos(x + dx, y - 1, z + dz));
    return (blockState->isTopSolidBlocking() || blockState->getMaterial() == Material::LEAVES)
           && mLevel->isEmptyBlock(BlockPos(x + dx, y, z + dz))
           && mLevel->isEmptyBlock(BlockPos(x + dx, y + 1, z + dz));
}
