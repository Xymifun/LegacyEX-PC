#include "game/client/render/UndeadHorseRenderer.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/AbstractHorseModel.hpp"
#include "game/client/render/EntityRenderer.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation ZOMBIE_HORSE_LOCATION(TN_MOB_HORSE_HORSE_ZOMBIE);
static ResourceLocation SKELETON_HORSE_LOCATION(TN_MOB_HORSE_HORSE_SKELETON);

UndeadHorseRenderer::UndeadHorseRenderer(EntityRenderDispatcher* dispatcher)
    : AbstractHorseRenderer(dispatcher, new AbstractHorseModel(), 1.0F)
{
}

const ResourceLocation* UndeadHorseRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    switch (entity->GetType())
    {
    case EntityType::ZOMBIE_HORSE:
        return &ZOMBIE_HORSE_LOCATION;
    case EntityType::SKELETON_HORSE:
        return &SKELETON_HORSE_LOCATION;
    default:
        return EntityRenderer::getTextureLocation(entity);
    }
}

Model* UndeadHorseRenderer::createModel(const ModelDefinition& definition)
{
    return new AbstractHorseModel(true, definition);
}
