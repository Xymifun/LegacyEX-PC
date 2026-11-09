#ifndef GAME_CLIENT_RENDER_ILLAGERMODEL_HPP
#define GAME_CLIENT_RENDER_ILLAGERMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/HumanoidArm.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelPart;

class IllagerModel : public Model
{
public:
    IllagerModel(float scale, float yOffset, int texWidth, int texHeight,
                 const ModelDefinition& modelDefinition = ModelDefinition());

    virtual ~IllagerModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"vindicator"; }
    virtual ModelPart* getArm(const HumanoidArm* arm);

    ModelPart* mHead;
    ModelPart* mBody;
    ModelPart* mArms;
    ModelPart* mLeg0;
    ModelPart* mLeg1;
    ModelPart* mNose;
    ModelPart* mRightArm;
    ModelPart* mLeftArm;
};

#endif
