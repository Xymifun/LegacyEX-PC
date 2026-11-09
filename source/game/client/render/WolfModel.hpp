#ifndef GAME_CLIENT_RENDER_WOLFMODEL_HPP
#define GAME_CLIENT_RENDER_WOLFMODEL_HPP
#include <string>
#include <boost/shared_ptr.hpp>
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
class Entity;
class LivingEntity;
class ModelPart;
class WolfModel : public Model
{
public:
    WolfModel(const ModelDefinition& modelDefinition = ModelDefinition());
    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float partialTicks);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"wolf"; }
    ModelPart* mHead;
    ModelPart* mBody;
    ModelPart* mLeg1;
    ModelPart* mLeg2;
    ModelPart* mLeg3;
    ModelPart* mLeg4;
    ModelPart* mTail;
    ModelPart* mUpperBody;
};
#endif
