#include "game/world/generation/VillagerGolemFlowerLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/Textures.hpp"
#include "game/client/render/VillagerGolemModel.hpp"
#include "game/client/render/VillagerGolemRenderer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/VillagerGolem.hpp"

VillagerGolemFlowerLayer::VillagerGolemFlowerLayer(VillagerGolemRenderer* renderer) : mRenderer(renderer)
{
}

VillagerGolemFlowerLayer::~VillagerGolemFlowerLayer()
{
}

void VillagerGolemFlowerLayer::render(boost::shared_ptr<LivingEntity>& entity, float, float, float, float, float, float,
                                      float, bool)
{
    boost::shared_ptr<VillagerGolem> golem = boost::dynamic_pointer_cast<VillagerGolem>(entity);

    if (golem->getOfferFlowerTick() == 0)
    {
        return;
    }

    GlStateManager::enableRescaleNormal();
    GlStateManager::pushMatrix();

    GlStateManager::rotatef(180.0f * static_cast<VillagerGolemModel*>(mRenderer->getModel())->mArm1->mXRot / 3.1415927f
                                + 5.0f,
                            1.0f, 0.0f, 0.0f);
    GlStateManager::translatef(-0.6875f, 1.25f, -0.9375f);
    GlStateManager::rotatef(90.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::scalef(0.8f, -0.8f, 0.8f);

    int lightColor = golem->getLightColor();
    int u = lightColor % 65536;
    int v = lightColor / 65536;
    GlStateManager::multiTexCoord2f(static_cast<float>(u), static_cast<float>(v));
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    mRenderer->bindTexture(&BLOCKS_LOCATION);

    const BlockState* flowerState = Blocks::red_flower->defaultBlockState();
    Minecraft::GetInstance()->getBlockRenderer()->renderSingleBlock(flowerState, 1.0f, 1.0f, true);

    GlStateManager::popMatrix();
    GlStateManager::disableRescaleNormal();
}

bool VillagerGolemFlowerLayer::colorsOnDamage()
{
    return false;
}
