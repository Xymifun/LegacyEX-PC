#include "game/client/render/ChestedHorseModel.hpp"

#include "game/client/render/ModelDefinition.hpp"

ChestedHorseModel::ChestedHorseModel(const ModelDefinition& modelDefinition)
    : AbstractHorseModel(false, modelDefinition)
{
}

void ChestedHorseModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                               float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    AbstractHorseModel::render(entity, limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale,
                               useDisplayList);
}
