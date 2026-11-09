#include "game/client/render/FishRenderer.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/FishModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation FISH_TEXTURES[3] = {ResourceLocation(TN_MOB_FISH_COD), ResourceLocation(TN_MOB_FISH_PUFFERFISH),
                                            ResourceLocation(TN_MOB_FISH_SALMON)};

FishRenderer::FishRenderer(EntityRenderDispatcher* dispatcher, FishType fishType, float shadowRadius)
    : MobRenderer(dispatcher, new FishModel(ModelDefinition(), fishType, 0), shadowRadius),
      mFishType(fishType)
{
}

Model* FishRenderer::getModel(boost::shared_ptr<LivingEntity>& entity)
{
    const bool isPufferfish = eTYPE_DERIVED_FROM(EntityType::PUFFERFISH, entity->GetType());
    if (isPufferfish)
        return getModel(entity);
    return mModel;
}

const ResourceLocation* FishRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    if (eTYPE_DERIVED_FROM(EntityType::FISH, entity->GetType()))
        return &FISH_TEXTURES[FISH_TYPE_COD];
    else if (eTYPE_DERIVED_FROM(EntityType::PUFFERFISH, entity->GetType()))
        return &FISH_TEXTURES[FISH_TYPE_PUFFERFISH];
    else if (eTYPE_DERIVED_FROM(EntityType::SALMON, entity->GetType()))
        return &FISH_TEXTURES[FISH_TYPE_SALMON];
    else
        return &FISH_TEXTURES[FISH_TYPE_COD];
}

Model* FishRenderer::createModel(const ModelDefinition& definition)
{
    return new FishModel(definition, mFishType, 0);
}
