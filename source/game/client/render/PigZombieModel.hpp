#ifndef GAME_CLIENT_RENDER_PIGZOMBIEMODEL_HPP
#define GAME_CLIENT_RENDER_PIGZOMBIEMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/HumanoidModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;

class PigZombieModel : public HumanoidModel
{
public:
    PigZombieModel(const ModelDefinition& definition = ModelDefinition());
    PigZombieModel(float scale, bool armourModel, const ModelDefinition& definition = ModelDefinition());


    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"pigzombie"; }

    void init(const ModelDefinition& definition, float scale, bool armourModel);
};

#endif
