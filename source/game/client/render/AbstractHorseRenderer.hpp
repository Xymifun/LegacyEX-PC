#ifndef GAME_CLIENT_RENDER_ABSTRACTHORSERENDERER_HPP
#define GAME_CLIENT_RENDER_ABSTRACTHORSERENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class EntityRenderDispatcher;
class LivingEntity;
class Model;

class AbstractHorseRenderer : public MobRenderer
{
public:
    AbstractHorseRenderer(EntityRenderDispatcher* dispatcher, Model* model, float scale);
    virtual ~AbstractHorseRenderer() {}

    virtual void scale(boost::shared_ptr<LivingEntity>& entity, float partialTicks);

protected:
    float mScale;
};

#endif
