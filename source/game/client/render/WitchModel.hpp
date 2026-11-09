#ifndef GAME_CLIENT_RENDER_WITCHMODEL_HPP
#define GAME_CLIENT_RENDER_WITCHMODEL_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/VillagerModel.hpp"

class Entity;
class ModelPart;

class WitchModel : public VillagerModel
{
public:
    WitchModel(float scale, const ModelDefinition& modelDefinition = ModelDefinition());
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"villager.witch"; }
    int getModelVersion();

    bool mHoldingItem;
    ModelPart* mWitchNose;
    ModelPart* mHat;
    ModelPart* mHat2;
    ModelPart* mHat3;
    ModelPart* mHat4;
};

#endif
