#ifndef GAME_CLIENT_RENDER_VILLAGERMODEL_HPP
#define GAME_CLIENT_RENDER_VILLAGERMODEL_HPP

#include <string>
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"

class Entity;
class ModelPart;

class VillagerModel : public Model
{
public:
    VillagerModel(float scale, float yOffset, int texWidth, int texHeight,
                  const ModelDefinition& modelDefinition = ModelDefinition());
    VillagerModel(float scale, const ModelDefinition& modelDefinition = ModelDefinition());
    virtual ~VillagerModel() {}
    void _init(float scale, float yOffset, int texWidth, int texHeight, const ModelDefinition& modelDefinition);
    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"villager"; }
    ModelPart* mHead;
    ModelPart* mBody;
    ModelPart* mArms;
    ModelPart* mLeg0;
    ModelPart* mLeg1;
    ModelPart* mNose;
    bool mHasCustomModel;
};

#endif
