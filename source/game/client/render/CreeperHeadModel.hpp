#ifndef GAME_CLIENT_RENDER_CREEPERHEADMODEL_HPP
#define GAME_CLIENT_RENDER_CREEPERHEADMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"


class CreeperHeadModel : public Model
{
public:
    CreeperHeadModel(float delta, const ModelDefinition& modelDefinition = ModelDefinition());

    virtual ~CreeperHeadModel() {}

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual std::wstring getName() { return L"creeper_head"; }

    ModelPart* mHead;
};

#endif
