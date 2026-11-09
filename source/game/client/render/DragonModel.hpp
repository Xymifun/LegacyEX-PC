#ifndef GAME_CLIENT_RENDER_DRAGONMODEL_HPP
#define GAME_CLIENT_RENDER_DRAGONMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"


class DragonModel : public Model
{
public:
    DragonModel(const ModelDefinition& modelDefinition = ModelDefinition());


    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float partialTicks);
    virtual std::wstring getName() { return L"dragon"; }

    float rotWrap(double angle);

    ModelPart* mHead;
    ModelPart* mNeck[5];
    ModelPart* mTail[12];
    ModelPart* mJaw;
    ModelPart* mBody;
    ModelPart* mRearLeg;
    ModelPart* mFrontLeg;
    ModelPart* mRearLegTip;
    ModelPart* mFrontLegTip;
    ModelPart* mRearFoot;
    ModelPart* mFrontFoot;
    ModelPart* mWing;
    ModelPart* mWingTip;
    bool mHasLegs;
    ModelPart* mRearLeg1;
    ModelPart* mFrontLeg1;
    ModelPart* mRearLegTip1;
    ModelPart* mFrontLegTip1;
    ModelPart* mRearFoot1;
    ModelPart* mFrontFoot1;
    ModelPart* mWing1;
    ModelPart* mWingTip1;
    float mPartialTicks;
    float mWingYRot;
    float mWing1YRot;
    bool mHasNamedNeckParts;
};

#endif
