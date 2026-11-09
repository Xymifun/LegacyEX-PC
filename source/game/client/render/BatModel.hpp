#ifndef GAME_CLIENT_RENDER_BATMODEL_HPP
#define GAME_CLIENT_RENDER_BATMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelPart;

class BatModel : public Model
{
public:
    BatModel(const ModelDefinition& modelDefinition = ModelDefinition());


    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"bat"; }

    ModelPart* mHead;
    ModelPart* mRightEar;
    ModelPart* mLeftEar;
    ModelPart* mBody;
    ModelPart* mRightWing;
    ModelPart* mLeftWing;
    ModelPart* mRightWingTip;
    ModelPart* mLeftWingTip;
};

#endif
