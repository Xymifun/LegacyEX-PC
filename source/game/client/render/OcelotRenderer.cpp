#include "game/client/render/OcelotRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/OcelotModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Ocelot.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation sBlackOcelotTexture(TN_MOB_CAT_BLACK);
static ResourceLocation sOcelotTexture(TN_MOB_OZELOT);
static ResourceLocation sRedOcelotTexture(TN_MOB_CAT_RED);
static ResourceLocation sSiameseOcelotTexture(TN_MOB_CAT_SIAMESE);

OcelotRenderer::OcelotRenderer(EntityRenderDispatcher* dispatcher) : MobRenderer(dispatcher, new OcelotModel(), 0.4F)
{
}

const ResourceLocation* OcelotRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<Ocelot> ocelot = boost::static_pointer_cast<Ocelot>(entity);
    switch (ocelot->getCatType())
    {
    case 0:
    default:
        return &sOcelotTexture;
    case 1:
        return &sBlackOcelotTexture;
    case 2:
        return &sRedOcelotTexture;
    case 3:
        return &sSiameseOcelotTexture;
    }
}

void OcelotRenderer::scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    boost::shared_ptr<Ocelot> ocelot = boost::static_pointer_cast<Ocelot>(entity);
    LivingEntityRenderer::scale(entity, partialTicks);
    if (ocelot->isTame())
        GlStateManager::scalef(0.8F, 0.8F, 0.8F);
}

Model* OcelotRenderer::createModel(const ModelDefinition& definition)
{
    return new OcelotModel(definition);
}
