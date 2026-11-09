#include "game/client/render/SquidRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/SquidModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Squid.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation SQUID_LOCATION(TN_MOB_SQUID);

SquidRenderer::SquidRenderer(EntityRenderDispatcher* dispatcher) : MobRenderer(dispatcher, new SquidModel(), 0.7F)
{
}

void SquidRenderer::setupRotations(boost::shared_ptr<LivingEntity>& entity, float, float rotationYaw,
                                   float partialTicks)
{
    boost::shared_ptr<Squid> squid = boost::static_pointer_cast<Squid>(entity);
    float bodyXRot = squid->mXBodyRotO + (squid->mXBodyRot - squid->mXBodyRotO) * partialTicks;
    float bodyZRot = squid->mZBodyRotO + (squid->mZBodyRot - squid->mZBodyRotO) * partialTicks;

    GlStateManager::translatef(0.0F, 0.5F, 0.0F);
    GlStateManager::rotatef(180.0F - rotationYaw, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef(bodyXRot, 1.0F, 0.0F, 0.0F);
    GlStateManager::rotatef(bodyZRot, 0.0F, 1.0F, 0.0F);
    GlStateManager::translatef(0.0F, -1.2F, 0.0F);
}

float SquidRenderer::getBob(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    boost::shared_ptr<Squid> squid = boost::static_pointer_cast<Squid>(entity);
    return (squid->mTentacleAngle - squid->mOldTentacleAngle) * partialTicks + squid->mOldTentacleAngle;
}

const ResourceLocation* SquidRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &SQUID_LOCATION;
}

Model* SquidRenderer::createModel(const ModelDefinition& definition)
{
    return new SquidModel(definition);
}
