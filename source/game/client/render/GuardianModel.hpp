#ifndef GAME_CLIENT_RENDER_GUARDIANMODEL_HPP
#define GAME_CLIENT_RENDER_GUARDIANMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelPart;

class GuardianModel : public Model
{
public:
    GuardianModel(const ModelDefinition& modelDefinition = ModelDefinition());

    virtual ~GuardianModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"guardian"; }

    ModelPart* mHead;
    ModelPart* mEye;
    ModelPart* mSpikeParts[12];
    ModelPart* mTailParts[3];
};

#endif
