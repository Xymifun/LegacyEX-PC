#ifndef GAME_CLIENT_RENDER_ABSTRACTHORSEMODEL_HPP
#define GAME_CLIENT_RENDER_ABSTRACTHORSEMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"

class Entity;
class LivingEntity;
class ModelPartSkin;

class AbstractHorseModel : public Model
{
public:
    AbstractHorseModel(bool horse = true, const ModelDefinition& modelDefinition = ModelDefinition());

    virtual ~AbstractHorseModel() {}

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float partialTicks);
    virtual std::wstring getName() { return L"horse.v2"; }

    float rotlerp(float from, float to, float delta);

    ModelPartSkin* mBody;
    ModelPartSkin* mHead;
    ModelPartSkin* mUMouth;
    ModelPartSkin* mNeck;
    ModelPartSkin* mMane;
    ModelPartSkin* mEar1;
    ModelPartSkin* mEar2;
    ModelPartSkin* mMuleEarL;
    ModelPartSkin* mMuleEarR;
    ModelPartSkin* mBag1;
    ModelPartSkin* mBag2;
    ModelPartSkin* mSaddle;
    ModelPartSkin* mSaddleMouthL;
    ModelPartSkin* mSaddleMouthR;
    ModelPartSkin* mSaddleMouthLine;
    ModelPartSkin* mSaddleMouthLineR;
    ModelPartSkin* mHeadSaddle;
    ModelPartSkin* mLeg1A;
    ModelPartSkin* mLeg2A;
    ModelPartSkin* mLeg3A;
    ModelPartSkin* mLeg4A;
    ModelPartSkin* mTailA;
    float mLeg3ADefaultY;
    float mLeg3ADefaultZ;
    float mBag2DefaultY;
    float mBag2DefaultZ;
};

#endif
