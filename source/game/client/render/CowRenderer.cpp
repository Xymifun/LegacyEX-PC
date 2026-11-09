#include "game/client/render/CowRenderer.hpp"

#include "game/client/render/CowModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation COW_LOCATION(TN_MOB_COW);

CowRenderer::CowRenderer(EntityRenderDispatcher* dispatcher, Model* model, float shadowRadius)
    : MobRenderer(dispatcher, model, shadowRadius)
{
}

const ResourceLocation* CowRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return &COW_LOCATION;
}

Model* CowRenderer::createModel(const ModelDefinition& definition)
{
    return new CowModel(definition);
}
