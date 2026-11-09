#include "game/client/render/BlazeRenderer.hpp"

#include "game/client/render/BlazeModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation BLAZE_LOCATION(TN_MOB_FIRE);

BlazeRenderer::BlazeRenderer(EntityRenderDispatcher* dispatcher) : MobRenderer(dispatcher, new BlazeModel(), 0.5F)
{
}

const ResourceLocation* BlazeRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &BLAZE_LOCATION;
}

void BlazeRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                           float partialTick)
{
    if (mModel->hasOverriddenParts() && !entity->isOnFire())
    {
        int lightColor = Entity::GetLightColor(entity.get());
        int u = lightColor % 65536;
        int v = lightColor / 65536;
        GlStateManager::multiTexCoord2f((float)u, (float)v);
    }
    MobRenderer::render(entity, x, y, z, yaw, partialTick);
}

Model* BlazeRenderer::createModel(const ModelDefinition& definition)
{
    return new BlazeModel(definition);
}
