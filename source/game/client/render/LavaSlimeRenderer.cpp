#include "game/client/render/LavaSlimeRenderer.hpp"

#include "game/client/render/LavaSlimeModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/MagmaCube.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation LAVA_SLIME_LOCATION(TN_MOB_LAVA);

LavaSlimeRenderer::LavaSlimeRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new LavaSlimeModel(), 0.25F)
{
}

void LavaSlimeRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                               float partialTick)
{
    if (mModel->hasOverriddenParts() && !entity->isOnFire())
    {
        int lightColor = Entity::GetLightColor(entity.get());
        int blockLight = lightColor % 65536;
        int skyLight = lightColor / 65536;
        GlStateManager::multiTexCoord2f((float)blockLight, (float)skyLight);
    }
    MobRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* LavaSlimeRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &LAVA_SLIME_LOCATION;
}

Model* LavaSlimeRenderer::createModel(const ModelDefinition& definition)
{
    return new LavaSlimeModel(definition);
}

void LavaSlimeRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTick)
{
    boost::shared_ptr<MagmaCube> magmaCube = boost::static_pointer_cast<MagmaCube>(entity);
    float size = (float)magmaCube->getSize();
    float squish = (magmaCube->mSquishO + (magmaCube->mSquish - magmaCube->mSquishO) * partialTick)
                   / (size * 0.5F + 1.0F);
    float inverseSquish = 1.0F / (squish + 1.0F);
    GlStateManager::scalef(inverseSquish * size, (1.0F / inverseSquish) * size, inverseSquish * size);
}
