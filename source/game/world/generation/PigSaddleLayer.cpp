#include "game/world/generation/PigSaddleLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/PigModel.hpp"
#include "game/client/render/PigRenderer.hpp"
#include "game/core/Pig.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation PIG_SADDLE_LOCATION(TN_MOB_SADDLE);

PigSaddleLayer::PigSaddleLayer(PigRenderer* renderer)
{
    mRenderer = renderer;
    mModel = new PigModel(0.5f);
}

PigSaddleLayer::~PigSaddleLayer()
{
}

const ResourceLocation* PigSaddleLayer::getSaddleTextureLocation() const
{
    return &PIG_SADDLE_LOCATION;
}

void PigSaddleLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                            float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                            bool bUseDisplayList)
{
    boost::shared_ptr<Pig> pig = boost::dynamic_pointer_cast<Pig>(entity);

    if (!pig->hasSaddle())
    {
        return;
    }

    mRenderer->bindTexture(&PIG_SADDLE_LOCATION);
    mModel->copyPropertiesFrom(mRenderer->getModel());

    boost::shared_ptr<Entity> renderEntity = pig;
    mModel->render(renderEntity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, bUseDisplayList);
}

bool PigSaddleLayer::colorsOnDamage()
{
    return false;
}
