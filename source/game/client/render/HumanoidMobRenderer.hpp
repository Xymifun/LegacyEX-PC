#ifndef GAME_CLIENT_RENDER_HUMANOIDMOBRENDERER_HPP
#define GAME_CLIENT_RENDER_HUMANOIDMOBRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/MobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class HumanoidModel;
class ResourceLocation;

class HumanoidMobRenderer : public MobRenderer
{
public:
    HumanoidMobRenderer(EntityRenderDispatcher* dispatcher, HumanoidModel* model, float shadowRadius);
    virtual ~HumanoidMobRenderer() {}

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual void translateWeaponItem();
};

#endif
