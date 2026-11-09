#ifndef GAME_CLIENT_RENDER_FISHMODEL_HPP
#define GAME_CLIENT_RENDER_FISHMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/FishRenderer.hpp"

class Entity;
class LivingEntity;
class ModelDefinition;
class ModelPartSkin;

class FishModel : public Model
{
public:
    FishModel(const ModelDefinition& modelDefinition, FishRenderer::FishType fishType, int variant);

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual void prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float partialTicks);
    virtual std::wstring getName();

private:
    ModelPartSkin* mPart0;
    ModelPartSkin* mPart1;
    ModelPartSkin* mPart2;
    ModelPartSkin* mPart3;
    ModelPartSkin* mPart4;
    ModelPartSkin* mPart5;
    ModelPartSkin* mPart6;
    ModelPartSkin* mPart7;
    ModelPartSkin* mPart8;
    ModelPartSkin* mPart9;
    ModelPartSkin* mPart10;
    ModelPartSkin* mPart11;
    ModelPartSkin* mPart12;
    ModelPartSkin* mPart13;
    ModelPartSkin* mPart14;
    ModelPartSkin* mPart15;
    ModelPartSkin* mPart16;
    ModelPartSkin* mPart17;
    ModelPartSkin* mPart18;
    ModelPartSkin* mPart19;
    ModelPartSkin* mPart20;
    FishRenderer::FishType mFishType;
    int mVariant;
    unsigned int : 32;
    float mAnimationScale;
    float mAnimationOffset;
};

#endif
