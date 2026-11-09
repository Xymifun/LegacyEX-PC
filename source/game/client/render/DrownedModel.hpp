#ifndef GAME_CLIENT_RENDER_DROWNEDMODEL_HPP
#define GAME_CLIENT_RENDER_DROWNEDMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ZombieModel.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class LivingEntity;
class ModelPart;

class DrownedModel : public ZombieModel
{
public:
    DrownedModel(const ModelDefinition& definition = ModelDefinition());
    DrownedModel(float scale, const ModelDefinition& definition = ModelDefinition());


    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual void prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float partialTicks);
    virtual std::wstring getName() { return L"zombie.drowned"; }
    virtual void setAllVisible(bool visible);

    void init(float scale, const ModelDefinition& definition);

    ModelPart* mLeftSleeve;
    ModelPart* mRightSleeve;
    ModelPart* mLeftPants;
    ModelPart* mRightPants;
    ModelPart* mJacket;
};

#endif
