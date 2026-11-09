#ifndef GAME_CLIENT_RENDER_OCELOTMODEL_HPP
#define GAME_CLIENT_RENDER_OCELOTMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class LivingEntity;
class ModelDefinition;
class ModelPart;

class OcelotModel : public Model
{
public:
    OcelotModel();
    OcelotModel(const ModelDefinition& modelDefinition);

    void LoadOcelotModelDefinition(const ModelDefinition& modelDefinition);
    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    void render(OcelotModel* model, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual void prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float partialTicks);
    virtual std::wstring getName() { return L"ocelot"; }

    ModelPart* mBackLegL;
    ModelPart* mBackLegR;
    ModelPart* mFrontLegL;
    ModelPart* mFrontLegR;
    ModelPart* mTail1;
    ModelPart* mTail2;
    ModelPart* mHead;
    ModelPart* mBody;
    int mPose;
};

#endif
