#ifndef GAME_CLIENT_RENDER_MOBRENDERER_HPP
#define GAME_CLIENT_RENDER_MOBRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/LivingEntityRenderer.hpp"

class Entity;
class Mob;
class Culler;

class MobRenderer : public LivingEntityRenderer
{
public:
    MobRenderer(EntityRenderDispatcher* dispatcher, Model* model, float shadowRadius);
    virtual ~MobRenderer() {}

    virtual bool shouldShowName(boost::shared_ptr<Entity>& entity);
    virtual bool shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double x, double y, double z);
    virtual void render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick);
    void setLightColor(boost::shared_ptr<Entity>& entity);
    double lerp(double first, double second, double fraction);
    virtual void renderLeash(boost::shared_ptr<Mob>& mob, double x, double y, double z, float yaw, float partialTick);
};

#endif
