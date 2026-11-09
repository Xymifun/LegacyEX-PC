#ifndef GAME_CLIENT_RENDER_DOLPHINMODEL_HPP
#define GAME_CLIENT_RENDER_DOLPHINMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelPart;

class DolphinModel : public Model
{
public:
    DolphinModel(const ModelDefinition& modelDefinition = ModelDefinition());

    virtual ~DolphinModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"dolphin"; }

    ModelPart* mHead;
    ModelPart* mBody;
    ModelPart* mTail;
    ModelPart* mTailFin;
    ModelPart* mBackFin;
    ModelPart* mLeftFin;
    ModelPart* mRightFin;
    ModelPart* mNose;
};

#endif
