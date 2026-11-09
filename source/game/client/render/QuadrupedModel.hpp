#ifndef GAME_CLIENT_RENDER_QUADRUPEDMODEL_HPP
#define GAME_CLIENT_RENDER_QUADRUPEDMODEL_HPP

#include <boost/shared_ptr.hpp>

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;
class ModelDefinition;

class QuadrupedModel : public Model
{
public:
    QuadrupedModel(int legHeight, float scale, const ModelDefinition& modelDefinition);
    virtual ~QuadrupedModel() {}

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);

    void render(QuadrupedModel* model, float scale, bool useDisplayList);

    ModelPartSkin* mHead;
    ModelPartSkin* mBody;
    ModelPartSkin* mLeg0;
    ModelPartSkin* mLeg1;
    ModelPartSkin* mLeg2;
    ModelPartSkin* mLeg3;
    unsigned int mUnused;
    bool mModelOverridden;
};

#endif
