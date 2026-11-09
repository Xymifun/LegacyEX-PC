#ifndef GAME_CLIENT_RENDER_LAVASLIMEMODEL_HPP
#define GAME_CLIENT_RENDER_LAVASLIMEMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"

class Entity;
class LivingEntity;
class ModelPart;

class LavaSlimeModel : public Model
{
public:
    LavaSlimeModel(const ModelDefinition& modelDefinition = ModelDefinition());

    virtual ~LavaSlimeModel() {}

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float partialTicks);
    virtual std::wstring getName() { return L"lavaslime"; }

    int getModelVersion();

    ModelPart* mBodyCubes[8];
    ModelPart* mInsideCube;
};

#endif
