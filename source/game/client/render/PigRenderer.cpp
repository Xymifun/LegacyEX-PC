#include "game/client/render/PigRenderer.hpp"

#include <algorithm>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/PigModel.hpp"
#include "game/core/Pig.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/generation/PigSaddleLayer.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcGoldenAge.hpp"
#include "platform/pc/PcSettings.hpp"
#endif

static ResourceLocation PIG_LOCATION(TN_MOB_PIG);

PigRenderer::PigRenderer(EntityRenderDispatcher* dispatcher) : MobRenderer(dispatcher, new PigModel(), 0.7F)
{
    mSaddleLayer = new PigSaddleLayer(this);
    addLayer(mSaddleLayer);
#ifdef MINECRAFT_PC
    mFlatFaceModel = new PigModel(ModelDefinition(), true);
#endif
}

#ifdef MINECRAFT_PC
Model* PigRenderer::getModel(boost::shared_ptr<LivingEntity>& entity)
{
    if (!mModel->hasOverriddenParts() && PcGoldenAgeOn(kPcGoldenAgeModels))
        return mFlatFaceModel;
    return MobRenderer::getModel(entity);
}
#endif

const ResourceLocation* PigRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    if (mModel->hasOverriddenParts() && boost::static_pointer_cast<Pig>(entity)->hasSaddle())
        return mSaddleLayer->getSaddleTextureLocation();
    return &PIG_LOCATION;
}

void PigRenderer::setupForTexturePack(TexturePack* pack)
{
    LivingEntityRenderer::setupForTexturePack(pack);
    if (mModel->hasOverriddenParts())
    {
        removeLayer(mSaddleLayer);
        return;
    }

    if (std::find(mLayers.begin(), mLayers.end(), mSaddleLayer) == mLayers.end())
        addLayer(mSaddleLayer);
}

Model* PigRenderer::createModel(const ModelDefinition& definition)
{
    return new PigModel(definition);
}
