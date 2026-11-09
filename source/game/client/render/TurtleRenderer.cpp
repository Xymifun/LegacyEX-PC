#include "game/client/render/TurtleRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/TurtleModel.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/core/Turtle.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation TURTLE_LOCATION(TN_MOB_SEA_TURTLE);

TurtleRenderer::TurtleRenderer(EntityRenderDispatcher* dispatcher)
    : MobRenderer(dispatcher, new TurtleModel(0.0f), 0.35f)
{
}

const ResourceLocation* TurtleRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return &TURTLE_LOCATION;
}

void TurtleRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                            float partialTick)
{
    boost::shared_ptr<Turtle> turtle = boost::static_pointer_cast<Turtle>(entity);
    if (turtle->isBaby())
        mShadowRadius *= 0.5f;

    MobRenderer::render(entity, x, y, z, yaw, partialTick);
}

Model* TurtleRenderer::createModel(const ModelDefinition& definition)
{
    return new TurtleModel(0.0f, definition);
}
