#include "game/world/generation/MushroomCowMushroomLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/CowModel.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/MushroomCowRenderer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

static ResourceLocation MUSHROOM_LOCATION(TN_BLOCK_ATLAS);

MushroomCowMushroomLayer::MushroomCowMushroomLayer(MushroomCowRenderer* renderer) : mRenderer(renderer)
{
}

MushroomCowMushroomLayer::~MushroomCowMushroomLayer()
{
}

void MushroomCowMushroomLayer::render(boost::shared_ptr<LivingEntity>& entity, float, float, float, float, float, float,
                                      float, bool)
{
    if (entity->isBaby())
    {
        return;
    }

    if (entity->isInvisible())
    {
        return;
    }

    mRenderer->bindTexture(&MUSHROOM_LOCATION);
    GlStateManager::enableCull();
    GlStateManager::cullFace(0);
    GlStateManager::pushMatrix();
    GlStateManager::scalef(1.0F, -1.0F, 1.0F);
    GlStateManager::translatef(0.2F, 0.35F, 0.5F);
    GlStateManager::rotatef(42.0F, 0.0F, 1.0F, 0.0F);
    GlStateManager::pushMatrix();
    Minecraft::GetInstance()->getBlockRenderer()->renderSingleBlock(Blocks::red_mushroom->defaultBlockState(), 1.0F,
                                                                    1.0F, true);
    GlStateManager::popMatrix();
    GlStateManager::pushMatrix();
    GlStateManager::translatef(0.1F, 0.0F, -0.6F);
    GlStateManager::rotatef(42.0F, 0.0F, 1.0F, 0.0F);
    Minecraft::GetInstance()->getBlockRenderer()->renderSingleBlock(Blocks::red_mushroom->defaultBlockState(), 1.0F,
                                                                    1.0F, true);
    GlStateManager::popMatrix();
    GlStateManager::popMatrix();

    GlStateManager::pushMatrix();
    static_cast<CowModel*>(mRenderer->getModel())->mHead->translateTo(0.0625F);
    GlStateManager::scalef(1.0F, -1.0F, 1.0F);
    GlStateManager::translatef(0.0F, 0.7F, -0.2F);
    GlStateManager::rotatef(12.0F, 0.0F, 1.0F, 0.0F);
    Minecraft::GetInstance()->getBlockRenderer()->renderSingleBlock(Blocks::red_mushroom->defaultBlockState(), 1.0F,
                                                                    1.0F, true);
    GlStateManager::popMatrix();
    GlStateManager::cullFace(1);
    GlStateManager::disableCull();
}

bool MushroomCowMushroomLayer::colorsOnDamage()
{
    return true;
}
