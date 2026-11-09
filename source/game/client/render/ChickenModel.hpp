#ifndef GAME_CLIENT_RENDER_CHICKENMODEL_HPP
#define GAME_CLIENT_RENDER_CHICKENMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelPart;

class ChickenModel : public Model
{
public:
    ChickenModel(const ModelDefinition& modelDefinition = ModelDefinition());


    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"chicken"; }

    ModelPart* mHead;
    ModelPart* mBody;
    ModelPart* mLeg0;
    ModelPart* mLeg1;
    ModelPart* mWing0;
    ModelPart* mWing1;
    ModelPart* mBeak;
    ModelPart* mComb;
};

#endif
