#ifndef GAME_CLIENT_RENDER_PARROTMODEL_HPP
#define GAME_CLIENT_RENDER_PARROTMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class LivingEntity;
class ModelPart;

class ParrotModel : public Model
{
public:
    ParrotModel(const ModelDefinition& modelDefinition = ModelDefinition());

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual void prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float partialTicks);
    virtual std::wstring getName() { return L"parrot"; }

    ModelPart* mBody;
    ModelPart* mTail;
    ModelPart* mWing0;
    ModelPart* mWing1;
    ModelPart* mHead;
    ModelPart* mHead2;
    ModelPart* mBeak1;
    ModelPart* mBeak2;
    ModelPart* mFeather;
    ModelPart* mLeg0;
    ModelPart* mLeg1;
    int mAnimationState;
    float mFeatherDefaultXRot;
    float mBodyDefaultXRot;
    float mWing0DefaultXRot;
    float mWing0DefaultYRot;
    float mWing1DefaultXRot;
    float mWing1DefaultYRot;
    float mLeg0DefaultY;
    float mLeg1DefaultY;
    float mLeg0DefaultXRot;
    float mLeg0DefaultZRot;
    float mLeg1DefaultXRot;
    float mLeg1DefaultZRot;
    float mHeadDefaultZRot;
    float mHeadDefaultX;
    float mHeadDefaultY;
    float mBodyDefaultX;
    float mBodyDefaultY;
    float mTailDefaultX;
    float mTailDefaultY;
    float mWing0DefaultX;
    float mWing0DefaultY;
    float mWing1DefaultX;
    float mWing1DefaultY;
};

#endif
