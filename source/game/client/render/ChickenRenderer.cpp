#include "game/client/render/ChickenRenderer.hpp"

#include "game/client/render/ChickenModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/Chicken.hpp"
#include "game/core/Mth.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation CHICKEN_LOCATION(TN_MOB_CHICKEN);

ChickenRenderer::ChickenRenderer(EntityRenderDispatcher* dispatcher, Model* model, float shadowRadius)
    : MobRenderer(dispatcher, model, shadowRadius)
{
}

const ResourceLocation* ChickenRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &CHICKEN_LOCATION;
}

float ChickenRenderer::getBob(boost::shared_ptr<LivingEntity>& entity, float partialTicks)
{
    boost::shared_ptr<Chicken> chicken = boost::static_pointer_cast<Chicken>(entity);
    float flap = chicken->mOFlap + (chicken->mFlapTime - chicken->mOFlap) * partialTicks;
    float flapSpeed = chicken->mOFlapSpeed + (chicken->mFlapSpeed - chicken->mOFlapSpeed) * partialTicks;
    return (Mth::sin(flap) + 1.0F) * flapSpeed;
}

Model* ChickenRenderer::createModel(const ModelDefinition& definition)
{
    return new ChickenModel(definition);
}
