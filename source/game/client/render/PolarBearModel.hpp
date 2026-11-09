#ifndef GAME_CLIENT_RENDER_POLARBEARMODEL_HPP
#define GAME_CLIENT_RENDER_POLARBEARMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/QuadrupedModel.hpp"

class Entity;

class PolarBearModel : public QuadrupedModel
{
public:
    PolarBearModel(const ModelDefinition& modelDefinition = ModelDefinition());

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"polarbear"; }
};

#endif
