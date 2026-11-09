#include "game/client/render/SilverfishRenderer.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/SilverfishModel.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation SILVERFISH_LOCATION(TN_MOB_SILVERFISH);

SilverfishRenderer::SilverfishRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new SilverfishModel(), 0.3F)
{
}

float SilverfishRenderer::getFlipDegrees(boost::shared_ptr<LivingEntity>&)
{
    return 180.0F;
}

const ResourceLocation* SilverfishRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &SILVERFISH_LOCATION;
}

Model* SilverfishRenderer::createModel(const ModelDefinition& definition)
{
    return new SilverfishModel(definition);
}
