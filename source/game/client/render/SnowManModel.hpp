#ifndef GAME_CLIENT_RENDER_SNOWMANMODEL_HPP
#define GAME_CLIENT_RENDER_SNOWMANMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelPart;

class SnowManModel : public Model
{
public:
    SnowManModel(const ModelDefinition& modelDefinition = ModelDefinition());


    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"snowgolem"; }

    ModelPart* mPiece1;
    ModelPart* mPiece2;
    ModelPart* mHead;
    ModelPart* mArm1;
    ModelPart* mArm2;
};

#endif
