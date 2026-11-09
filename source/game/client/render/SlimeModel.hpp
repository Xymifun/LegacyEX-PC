#ifndef GAME_CLIENT_RENDER_SLIMEMODEL_HPP
#define GAME_CLIENT_RENDER_SLIMEMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"

class Entity;
class ModelPart;

class SlimeModel : public Model
{
public:
    SlimeModel(int size, const ModelDefinition& modelDefinition = ModelDefinition());


    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual std::wstring getName() { return L"slime"; }

    ModelPart* mBody;
    ModelPart* mEye0;
    ModelPart* mEye1;
    ModelPart* mMouth;
};

#endif
