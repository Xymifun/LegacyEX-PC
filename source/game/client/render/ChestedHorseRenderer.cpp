#include "game/client/render/ChestedHorseRenderer.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/ChestedHorseModel.hpp"
#include "game/client/render/EntityRenderer.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation DONKEY_LOCATION(TN_MOB_HORSE_DONKEY);
static ResourceLocation MULE_LOCATION(TN_MOB_HORSE_MULE);

ChestedHorseRenderer::ChestedHorseRenderer(EntityRenderDispatcher* dispatcher, float shadowRadius)
    : AbstractHorseRenderer(dispatcher, new ChestedHorseModel(), shadowRadius)
{
}

const ResourceLocation* ChestedHorseRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    switch (entity->GetType())
    {
    case EntityType::DONKEY:
        return &DONKEY_LOCATION;
    case EntityType::MULE:
        return &MULE_LOCATION;
    default:
        return EntityRenderer::getTextureLocation(entity);
    }
}

Model* ChestedHorseRenderer::createModel(const ModelDefinition& definition)
{
    return new ChestedHorseModel(definition);
}
