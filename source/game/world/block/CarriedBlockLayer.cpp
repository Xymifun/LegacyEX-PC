#include "game/world/block/CarriedBlockLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/EndermanRenderer.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/EnderMan.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/BlockState.hpp"

CarriedBlockLayer::CarriedBlockLayer(EndermanRenderer* renderer) : mEndermanRenderer(renderer)
{
}

CarriedBlockLayer::~CarriedBlockLayer()
{
}

void CarriedBlockLayer::render(boost::shared_ptr<LivingEntity>& entity, float, float, float partialTicks, float, float,
                               float, float, bool)
{
    boost::shared_ptr<EnderMan> enderman = boost::dynamic_pointer_cast<EnderMan>(entity);
    const BlockState* blockState = enderman->getCarryingBlock();

    if (blockState == NULL)
    {
        return;
    }

    BlockRenderDispatcher* blockRenderer = Minecraft::GetInstance()->getBlockRenderer();
    GlStateManager::enableRescaleNormal();
    GlStateManager::pushMatrix();
    GlStateManager::translatef(0.0F, 0.6875F, -0.75F);
    GlStateManager::rotatef(20.0F, 1.0F, 0.0F, 0.0F);
    GlStateManager::rotatef(45.0F, 0.0F, 1.0F, 0.0F);
    GlStateManager::translatef(0.25F, 0.1875F, 0.25F);
    GlStateManager::scalef(-0.5F, -0.5F, 0.5F);

    int lightColor = enderman->getLightColor();
    GlStateManager::multiTexCoord2f((float)(lightColor % 65536), (float)(lightColor / 65536));
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    mEndermanRenderer->bindTexture(&BLOCKS_LOCATION);
    GlStateManager::translatef(0.5F, 0.5F, -0.5F);
    blockRenderer->renderSingleBlock(blockState, 1.0F, 1.0F, true);

    GlStateManager::popMatrix();
    GlStateManager::disableRescaleNormal();
}

bool CarriedBlockLayer::colorsOnDamage()
{
    return false;
}
