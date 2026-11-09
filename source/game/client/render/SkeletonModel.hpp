#ifndef GAME_CLIENT_RENDER_SKELETONMODEL_HPP
#define GAME_CLIENT_RENDER_SKELETONMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/HumanoidModel.hpp"
#include "game/client/render/ModelDefinition.hpp"

class Entity;
class HumanoidArm;
class LivingEntity;

class SkeletonModel : public HumanoidModel
{
public:
    enum SkeletonType
    {
        eSkeletonType_Skeleton = 1,
        eSkeletonType_Stray = 2,
        eSkeletonType_WitherSkeleton = 3
    };

    SkeletonModel(const ModelDefinition& definition = ModelDefinition());
    SkeletonModel(float scale, bool armourModel, const ModelDefinition& definition = ModelDefinition());
    SkeletonModel(SkeletonType skeletonType, const ModelDefinition& definition = ModelDefinition());


    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual void prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float partialTicks);
    virtual std::wstring getName() { return L"skeleton"; }
    virtual void translateToHandItem(float scale, const HumanoidArm* arm);

    void _init(float scale, bool armourModel, SkeletonType skeletonType, const ModelDefinition& definition);
};

#endif
