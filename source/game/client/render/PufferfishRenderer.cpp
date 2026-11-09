#include "game/client/render/PufferfishRenderer.hpp"

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/FishModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/core/Pufferfish.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"

PufferfishRenderer::PufferfishRenderer(EntityRenderDispatcher* dispatcher)
    : FishRenderer(dispatcher, FISH_TYPE_PUFFERFISH, 0.2f)
{
    mModelMedium = new FishModel(ModelDefinition(), FISH_TYPE_PUFFERFISH, 1);
    mModelLarge = new FishModel(ModelDefinition(), FISH_TYPE_PUFFERFISH, 2);
    mOriginalModelMedium = mModelMedium;
    mOriginalModelLarge = mModelLarge;
    mPuffState = Pufferfish::PUFFED_STATE_SMALL;
}

void PufferfishRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                float partialTick)
{
    boost::shared_ptr<Pufferfish> pufferfish = boost::static_pointer_cast<Pufferfish>(entity);
    mPuffState = pufferfish->mPuffState;
    MobRenderer::render(entity, x, y, z, yaw, partialTick);
}

Model* PufferfishRenderer::getModel(boost::shared_ptr<LivingEntity>&)
{
    switch (mPuffState)
    {
    case Pufferfish::PUFFED_STATE_MEDIUM:
        return mModelMedium;
    case Pufferfish::PUFFED_STATE_LARGE:
        return mModelLarge;
    default:
        return mModel;
    }
}

void PufferfishRenderer::setupForTexturePack(TexturePack* pack)
{
    LivingEntityRenderer::setupForTexturePack(pack);

    if (mModelMedium != mOriginalModelMedium)
    {
        delete mModelMedium;
        mModelMedium = NULL;
    }
    bool hasMedium = pack->HasCustomModel(mOriginalModelMedium->getName());
    if (hasMedium)
    {
        mModelMedium = new FishModel(*pack->GetCustomModel(mOriginalModelMedium->getName()), FISH_TYPE_PUFFERFISH, 1);
    }
    else
    {
        mModelMedium = mOriginalModelMedium;
    }

    if (mModelLarge != mOriginalModelLarge)
    {
        delete mModelLarge;
        mModelLarge = NULL;
    }
    bool hasLarge = pack->HasCustomModel(mOriginalModelLarge->getName());
    if (hasLarge)
    {
        mModelLarge = new FishModel(*pack->GetCustomModel(mOriginalModelLarge->getName()), FISH_TYPE_PUFFERFISH, 2);
    }
    else
    {
        mModelLarge = mOriginalModelLarge;
    }
}

Model* PufferfishRenderer::createModel(const ModelDefinition& definition)
{
    return new FishModel(definition, FISH_TYPE_PUFFERFISH, 0);
}
