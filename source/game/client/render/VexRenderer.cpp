#include "game/client/render/VexRenderer.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/VexModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Vex.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation VEX_LOCATION(TN_MOB_ILLAGER_VEX);
static ResourceLocation VEX_CHARGING_LOCATION(TN_MOB_ILLAGER_VEX_CHARGING);

VexRenderer::VexRenderer(EntityRenderDispatcher* dispatcher) : HumanoidMobRenderer(dispatcher, new VexModel(), 0.3F)
{
    mModelVersion = static_cast<VexModel*>(mModel)->getModelVersion();
}

const ResourceLocation* VexRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    if (Minecraft::InMiniGame(eMiniGameId_Any, true) && eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
    {
        return &VEX_LOCATION;
    }

    boost::shared_ptr<Vex> vex = boost::static_pointer_cast<Vex>(entity);
    if (vex->isCharging())
        return &VEX_CHARGING_LOCATION;

    return &VEX_LOCATION;
}

void VexRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick)
{
    VexModel* model = static_cast<VexModel*>(mModel);
    unsigned int modelVersion = model->getModelVersion();
    if (modelVersion != mModelVersion)
    {
        delete model;
        mModel = new VexModel();
        mModelVersion = modelVersion;
    }

    MobRenderer::render(entity, x, y, z, yaw, partialTick);
}

Model* VexRenderer::createModel(const ModelDefinition& definition)
{
    return new VexModel(definition);
}

void VexRenderer::scale(boost::shared_ptr<LivingEntity>&, float)
{
    GlStateManager::scalef(0.4F, 0.4F, 0.4F);
}
