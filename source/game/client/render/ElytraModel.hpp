#ifndef GAME_CLIENT_RENDER_ELYTRAMODEL_HPP
#define GAME_CLIENT_RENDER_ELYTRAMODEL_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelPart;

class ElytraModel : public Model
{
public:
    ElytraModel(const ModelDefinition& modelDefinition = ModelDefinition());


    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);

    void overrideAnim(float flyAmount, float netHeadYaw, float headPitch, float scale);

    bool mOverrideAnim;
    ModelPart* mRightWing;
    ModelPart* mLeftWing;
};

#endif
