#ifndef GAME_CLIENT_RENDER_PIGZOMBIERENDERER_HPP
#define GAME_CLIENT_RENDER_PIGZOMBIERENDERER_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/HumanoidMobRenderer.hpp"

class Entity;
class EntityRenderDispatcher;
class Model;
class ModelDefinition;
class ResourceLocation;

class PigZombieRenderer : public HumanoidMobRenderer
{
public:
    PigZombieRenderer(EntityRenderDispatcher* dispatcher);

    virtual const ResourceLocation* getTextureLocation(boost::shared_ptr<Entity>& entity);
    virtual std::wstring getModelName();
    virtual Model* createModel(const ModelDefinition& definition);
};

#endif
