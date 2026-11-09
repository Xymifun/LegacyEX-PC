#ifndef GAME_CLIENT_RENDER_SHEEPFURMODEL_HPP
#define GAME_CLIENT_RENDER_SHEEPFURMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/QuadrupedModel.hpp"

class Entity;
class LivingEntity;

class SheepFurModel : public QuadrupedModel
{
public:
    SheepFurModel(const ModelDefinition& modelDefinition = ModelDefinition());

    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual void prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float partialTicks);
    virtual std::wstring getName() { return L"sheep"; }

    float mHeadEatAngleScale;
};

#endif
