#ifndef GAME_CLIENT_RENDER_VEXMODEL_HPP
#define GAME_CLIENT_RENDER_VEXMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/HumanoidModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelPart;

class VexModel : public HumanoidModel
{
public:
    VexModel(const ModelDefinition& modelDefinition = ModelDefinition());
    VexModel(float scale, const ModelDefinition& modelDefinition = ModelDefinition());

    virtual ~VexModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"vex"; }

    void _init(float scale, const ModelDefinition& modelDefinition);
    unsigned int getModelVersion();

    ModelPart* mLeftWing;
    ModelPart* mRightWing;
};

#endif
