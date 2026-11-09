#ifndef GAME_CLIENT_RENDER_RABBITMODEL_HPP
#define GAME_CLIENT_RENDER_RABBITMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class LivingEntity;
class ModelPartSkin;

class RabbitModel : public Model
{
public:
    RabbitModel(const ModelDefinition& modelDefinition = ModelDefinition());

    virtual ~RabbitModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual void prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float partialTicks);
    virtual std::wstring getName() { return L"rabbit"; }

    void render(RabbitModel* model, float scale, bool useDisplayList);
    void setRotation(ModelPartSkin* part, float x, float y, float z);

    ModelPartSkin* mRearFootLeft;
    ModelPartSkin* mRearFootRight;
    ModelPartSkin* mHaunchLeft;
    ModelPartSkin* mHaunchRight;
    ModelPartSkin* mBody;
    ModelPartSkin* mFrontLegLeft;
    ModelPartSkin* mFrontLegRight;
    ModelPartSkin* mHead;
    ModelPartSkin* mEarRight;
    ModelPartSkin* mEarLeft;
    ModelPartSkin* mTail;
    ModelPartSkin* mNose;
    float mJumpRotation;
    float mOldJumpRotation;
};

#endif
