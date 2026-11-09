#include "game/world/generation/RandomPatchesLayerTheme.hpp"
#include "game/world/block/BlockId.hpp"

#include "game/core/SimplexNoise.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockTransaction.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/StoneSlabBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

const BlockState* sanitiseBottomLayerBlocks(const BlockState* pState)
{
    Block* block = pState->getBlock();
    int blockId = Block::getId(block);

    if (blockId == BlockId::STONE_SLAB)
    {
        StoneSlabBlock* slab = static_cast<StoneSlabBlock*>(block);
        StoneSlabBlock::Variant* variant = pState->getValue(
            static_cast<const AbstractProperty<StoneSlabBlock::Variant*>*>(slab->getTypeProperty()));

        switch (variant->getData())
        {
        case 0:
            return Blocks::stone->defaultBlockState();
        case 1:
            return Blocks::sandstone->defaultBlockState();
        case 2:
            return Blocks::planks->defaultBlockState();
        case 3:
            return Blocks::cobblestone->defaultBlockState();
        case 4:
            return Blocks::brick_block->defaultBlockState();
        case 5:
            return Blocks::stonebrick->defaultBlockState();
        case 6:
            return Blocks::nether_brick->defaultBlockState();
        case 7:
            return Blocks::quartz_block->defaultBlockState();
        default:
            return Blocks::stone->defaultBlockState();
        }
    }

    if (blockId == BlockId::SANDSTONE_STAIRS)
    {
        return Blocks::sandstone->defaultBlockState();
    }

    return pState;
}

RandomPatchesLayerTheme::RandomPatchesLayerTheme(unsigned int uSeed, double dNoiseScale)
    : m_uSeed(uSeed),
      m_dNoiseScale(dNoiseScale)
{
}

int RandomPatchesLayerTheme::ABOVE_SURFACE(int iDistance)
{
    return iDistance;
}

int RandomPatchesLayerTheme::BELOW_SURFACE(int iDistance)
{
    return -iDistance;
}

bool RandomPatchesLayerTheme::IsSolidEnough(const BlockState* pState)
{
    return pState->isSolid();
}

int RandomPatchesLayerTheme::HighestSolidBlock(BlockTransaction* pTransaction, int iX, int iZ)
{
    for (int iY = pTransaction->mSize.y - 1; iY >= 0; --iY)
    {
        Vec3i vPos(iX, iY, iZ);
        const BlockState* pState =
            Block::byId(pTransaction->getBlock(vPos))->getBlockState(pTransaction->getData(vPos));
        if (IsSolidEnough(pState))
        {
            return iY;
        }
    }

    return -1;
}

void RandomPatchesLayerTheme::Apply(Random& random, BlockTransaction* pTransaction, int& iMinY, int& iMaxY) const
{
    SimplexNoise noise(&random);

    for (int iX = 0; iX < pTransaction->mSize.x; ++iX)
    {
        for (int iZ = 0; iZ < pTransaction->mSize.z; ++iZ)
        {
            int iSurfaceY = HighestSolidBlock(pTransaction, iX, iZ);
            if (iSurfaceY == -1)
            {
                continue;
            }

            float fNoise = (float)noise.getValue((double)iX / m_dNoiseScale, (double)iZ / m_dNoiseScale);

            for (int iY = 0; iY < pTransaction->mSize.y; ++iY)
            {
                Vec3i vPos(iX, iY, iZ);
                Block* pBlock = Block::byId(pTransaction->getBlock(vPos));
                const BlockState* pExistingState = pBlock->getBlockState(pTransaction->getData(vPos));
                bool bIsAir = pBlock == Blocks::air;

                const BlockState* pState = BlockAt(random, fNoise, iY - iSurfaceY, bIsAir);
                if (pState != NULL)
                {
                    if (iY == pTransaction->getMin().y)
                    {
                        pTransaction->setBlock(vPos, sanitiseBottomLayerBlocks(pState));
                    }
                    else
                    {
                        pTransaction->setBlock(vPos, pState);
                    }
                }
            }
        }
    }
}
