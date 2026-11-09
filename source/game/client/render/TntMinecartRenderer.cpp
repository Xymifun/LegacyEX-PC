#include "game/client/render/TntMinecartRenderer.hpp"

#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/MinecartTNT.hpp"
#include "game/core/Mth.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

TntMinecartRenderer::TntMinecartRenderer(EntityRenderDispatcher* dispatcher) : MinecartRenderer(dispatcher)
{
}

TntMinecartRenderer::~TntMinecartRenderer()
{
}

void TntMinecartRenderer::renderMinecartContents(boost::shared_ptr<Minecart> minecart, float partialTick,
                                                 const BlockState* state)
{
    boost::shared_ptr<MinecartTNT> tntMinecart = boost::static_pointer_cast<MinecartTNT>(minecart);
    int fuse = tntMinecart->getFuse();

    if (fuse >= 0)
    {
        float remainingFuse = ((float)fuse - partialTick) + 1.0F;
        if (remainingFuse < 10.0F)
        {
            float scale = 1.0F - remainingFuse / 10.0F;
            scale = Mth::clamp(scale, 0.0F, 1.0F);
            scale *= scale;
            scale *= scale;
            scale = scale * 0.3F + 1.0F;
            GlStateManager::scalef(scale, scale, scale);
        }
    }

    MinecartRenderer::renderMinecartContents(minecart, partialTick, state);

    if (fuse >= 0 && (fuse / 5 & 1) == 0)
    {
        BlockRenderDispatcher* blockRenderer = Minecraft::GetInstance()->getBlockRenderer();
        GlStateManager::disableTexture();
        GlStateManager::disableLighting();
        GlStateManager::enableBlend();
        GlStateManager::blendFunc(4, 6);
        GlStateManager::color4f(1.0F, 1.0F, 1.0F, (1.0F - (((float)fuse - partialTick) + 1.0F) / 100.0F) * 0.8F);
        GlStateManager::pushMatrix();
        blockRenderer->renderSingleBlock(Blocks::tnt->defaultBlockState(), 1.0F, 1.0F, true);
        GlStateManager::popMatrix();
        GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
        GlStateManager::disableBlend();
        GlStateManager::enableLighting();
        GlStateManager::enableTexture();
    }
}
