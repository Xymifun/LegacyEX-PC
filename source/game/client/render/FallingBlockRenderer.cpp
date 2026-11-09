#include "game/client/render/FallingBlockRenderer.hpp"

#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/BlockRenderer.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/FallingBlockEntity.hpp"

FallingBlockRenderer::FallingBlockRenderer(EntityRenderDispatcher* dispatcher) : EntityRenderer(dispatcher)
{
    mShadowRadius = 0.5F;
}

FallingBlockRenderer::~FallingBlockRenderer()
{
}

void FallingBlockRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                  float partialTick)
{
    boost::shared_ptr<FallingBlockEntity> fallingBlock = boost::static_pointer_cast<FallingBlockEntity>(entity);
    if (fallingBlock->getBlock() == NULL)
        return;

    bindTexture(&BLOCKS_LOCATION);

    const BlockState* blockState = fallingBlock->getBlock();
    Block* block = blockState->getBlock();
    BlockPos blockPos(fallingBlock);
    Level* level = fallingBlock->getLevel();
    if (blockState == level->getBlockState(blockPos)
        || block->defaultBlockState()->getRenderShape() == BLOCK_RENDER_INVISIBLE)
        return;

    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y, (float)z);
    GlStateManager::disableLighting();
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    BlockRenderer* blockRenderer = Minecraft::GetInstance()->getBlockRenderer()->getBlockRenderer();

    float offset = 0.5F;
    if (block == Blocks::anvil && block->defaultBlockState()->getRenderShape() == BLOCK_RENDER_ANVIL)
    {
        blockRenderer->mLevelSource = level;
        builder->begin();
        builder->offset((float)-(int)fallingBlock->mX - offset, (float)-(int)fallingBlock->mY,
                        (float)-(int)fallingBlock->mZ - offset);
        blockRenderer->tesselateAnvilInWorld(
            blockState, BlockPos((int)fallingBlock->mX, (int)fallingBlock->mY, (int)fallingBlock->mZ));
        builder->offset(0.0F, 0.0F, 0.0F);
        builder->end();
    }
    else if (block != NULL)
    {
        blockRenderer->mLevelSource = level;
        builder->begin();
        builder->offset((float)-(int)fallingBlock->mX - offset, (float)-(int)fallingBlock->mY,
                        (float)-(int)fallingBlock->mZ - offset);
        blockRenderer->tesselateInWorldNoCulling(
            blockState, BlockPos((int)fallingBlock->mX, (int)fallingBlock->mY, (int)fallingBlock->mZ), NULL,
            boost::shared_ptr<BlockEntity>());
        builder->offset(0.0F, 0.0F, 0.0F);
        builder->end();
    }

    GlStateManager::enableLighting();
    GlStateManager::popMatrix();
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* FallingBlockRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &BLOCKS_LOCATION;
}
