#include "game/client/render/BatRenderer.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/client/render/BatModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/Bat.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation BAT_LOCATION(TN_MOB_BAT);

BatRenderer::BatRenderer(EntityRenderDispatcher* dispatcher) : MobRenderer(dispatcher, new BatModel(), 0.25F)
{
}

const ResourceLocation* BatRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &BAT_LOCATION;
}

Model* BatRenderer::createModel(const ModelDefinition& definition)
{
    return new BatModel(definition);
}

void BatRenderer::setupPosition(boost::shared_ptr<LivingEntity>& entity, double x, double y, double z)
{
    LivingEntityRenderer::setupPosition(entity, x, y, z);
}

void BatRenderer::setupRotations(boost::shared_ptr<LivingEntity>& entity, float ageInTicks, float rotationYaw,
                                 float partialTicks)
{
    if (eTYPE_DERIVED_FROM(EntityType::BAT, entity->GetType()) && boost::static_pointer_cast<Bat>(entity)->isResting())
        GlStateManager::translatef(0.0F, -0.1F, 0.0F);
    else
        GlStateManager::translatef(0.0F, cos(ageInTicks * 0.3F) * 0.1F, 0.0F);

    LivingEntityRenderer::setupRotations(entity, ageInTicks, rotationYaw, partialTicks);
}

void BatRenderer::scale(boost::shared_ptr<LivingEntity>&, float)
{
    GlStateManager::scalef(0.35F, 0.35F, 0.35F);
}
