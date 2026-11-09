#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/BlockRenderer.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockColors.hpp"
#include "game/world/block/BlockState.hpp"

BlockRenderDispatcher::BlockRenderDispatcher(BlockColors* blockColors) : mBlockRenderer(new BlockRenderer(blockColors))
{
}

void BlockRenderDispatcher::renderSingleBlock(const BlockState* state, float x, float y, bool color)
{
    int renderShape = state->getRenderShape();
    if (renderShape != BLOCK_RENDER_INVISIBLE
        && (BlockRenderer::canRender(renderShape) || renderShape == BLOCK_RENDER_CROSS))
    {
        mBlockRenderer->renderBlock(state, x, y, color, true);
    }
}

bool BlockRenderDispatcher::canRenderGeneric(Block* block)
{
    if (block == NULL)
    {
        return false;
    }
    return block->defaultBlockState()->getRenderShape() == BLOCK_RENDER_ENTITY;
}

void BlockRenderDispatcher::setColor(bool color)
{
    mBlockRenderer->mUseColor = color;
}

BlockRenderer* BlockRenderDispatcher::getBlockRenderer()
{
    return mBlockRenderer;
}

void BlockRenderDispatcher::prepare(Level* level)
{
    mBlockRenderer->prepare(level);
}
