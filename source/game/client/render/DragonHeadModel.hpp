#ifndef GAME_CLIENT_RENDER_DRAGONHEADMODEL_HPP
#define GAME_CLIENT_RENDER_DRAGONHEADMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"


class DragonHeadModel : public Model
{
public:
    DragonHeadModel(float scale, const ModelDefinition& modelDefinition = ModelDefinition());


    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual std::wstring getName() { return L"dragon_head"; }

    bool hasJawPart();

    ModelPart* mHead;
    ModelPart* mJaw;
    bool mUsingDefaultModel;
};

#endif
