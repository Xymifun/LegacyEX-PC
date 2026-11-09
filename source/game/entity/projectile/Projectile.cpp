#include "game/entity/projectile/Projectile.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/GameMode.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/TntBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

Projectile::Projectile(Level* level, const boost::shared_ptr<LivingEntity>& owner) : mOwner(owner), mLevel(level)
{
}

bool Projectile::onHit(const HitResult* result)
{
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureProjectilesBreakBlocks() && !mLevel->mIsClientSide
        && result->mType == HitResult::BLOCK && mOwner && eTYPE_DERIVED_FROM(EntityType::PLAYER, mOwner->GetType()))
    {
        const BlockPos& pos = result->getBlockPos();
        GameMode* gameMode = boost::static_pointer_cast<Player>(mOwner)->GetGameMode();
        if (gameMode->MayDestroy(pos))
        {
            Block* block = mLevel->getBlock(pos);
            TntBlock* tntBlock = dynamic_cast<TntBlock*>(block);
            if (tntBlock != NULL)
            {
                const BlockState* state = mLevel->getBlockState(pos);
                tntBlock->destroy(mLevel, pos, state->setValue(TntBlock::EXPLODE, true), mOwner);
                mLevel->removeBlock(pos);
            }
            else
            {
                mLevel->destroyBlock(pos, false);
            }
            gameMode->RecordBlockDestroyed(pos, Block::getId(block));
            return true;
        }
    }
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureInvertedExplodingGrenades())
    {
        BlockPos pos = result->getBlockPos();
        mLevel->explode(boost::shared_ptr<Entity>(), pos.x, pos.y, pos.z, 2.0f, false, true, false, false, false, 0.0f);
    }
    return false;
}
