#ifndef GAME_CLIENT_RENDER_HUMANOIDHEADMODEL_HPP
#define GAME_CLIENT_RENDER_HUMANOIDHEADMODEL_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/SkeletonHeadModel.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelDefinition;
class ModelPart;

class HumanoidHeadModel : public SkeletonHeadModel
{
public:
    HumanoidHeadModel(const ModelDefinition& modelDefinition);
    virtual ~HumanoidHeadModel();

    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);

    ModelPart* mHat;
};

#endif
