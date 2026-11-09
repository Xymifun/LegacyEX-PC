#include "game/client/render/EndermiteRenderer.hpp"

#include "game/client/render/EndermiteModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation ENDERMITE_LOCATION(TN_MOB_ENDERMITE);

EndermiteRenderer::EndermiteRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new EndermiteModel(), 0.3F)
{
}

float EndermiteRenderer::getFlipDegrees(boost::shared_ptr<LivingEntity>&)
{
    return 180.0F;
}

const ResourceLocation* EndermiteRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &ENDERMITE_LOCATION;
}

Model* EndermiteRenderer::createModel(const ModelDefinition& definition)
{
    return new EndermiteModel(definition);
}
