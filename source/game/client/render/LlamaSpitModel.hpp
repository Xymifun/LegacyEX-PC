#ifndef GAME_CLIENT_RENDER_LLAMASPITMODEL_HPP
#define GAME_CLIENT_RENDER_LLAMASPITMODEL_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"


class LlamaSpitModel : public Model
{
public:
    LlamaSpitModel(const ModelDefinition& modelDefinition = ModelDefinition());
    LlamaSpitModel(float delta, const ModelDefinition& modelDefinition = ModelDefinition());


    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);

    void _init(float delta, const ModelDefinition& modelDefinition);

    ModelPart* mSpit;
};

#endif
