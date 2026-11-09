#ifndef GAME_CLIENT_RENDER_SQUIDMODEL_HPP
#define GAME_CLIENT_RENDER_SQUIDMODEL_HPP

#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"

class ModelPartSkin;

class SquidModel : public Model
{
public:
    explicit SquidModel(const ModelDefinition& definition = ModelDefinition());
    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual std::wstring getName() { return L"squid"; }

    ModelPartSkin* mBody;
    ModelPartSkin* mTentacles[8];
};

#endif
