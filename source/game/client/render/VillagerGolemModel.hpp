#ifndef GAME_CLIENT_RENDER_VILLAGERGOLEMMODEL_HPP
#define GAME_CLIENT_RENDER_VILLAGERGOLEMMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class LivingEntity;
class ModelPart;

class VillagerGolemModel : public Model
{
public:
    VillagerGolemModel(float scale = 0.0F, float yOffset = -7.0F,
                       const ModelDefinition& modelDefinition = ModelDefinition());


    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual void prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float partialTicks);
    virtual std::wstring getName() { return L"irongolem"; }

    float triangleWave(float time, float period);

    ModelPart* mHead;
    ModelPart* mBody;
    ModelPart* mArm1;
    ModelPart* mArm0;
    ModelPart* mLeg0;
    ModelPart* mLeg1;
};

#endif
