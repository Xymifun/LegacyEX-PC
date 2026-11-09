#ifndef GAME_CLIENT_RENDER_ZOMBIEMODEL_HPP
#define GAME_CLIENT_RENDER_ZOMBIEMODEL_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/client/render/C4JRender.hpp"
#include "game/client/render/HumanoidModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "vendor/_SkinAdjustments.hpp"

class Entity;

class ZombieModel : public HumanoidModel
{
public:
    ZombieModel(const ModelDefinition& definition = ModelDefinition());
    ZombieModel(float scale, bool armourModel, const ModelDefinition& definition = ModelDefinition());
    ZombieModel(float scale, float yOffset, int texWidth, int texHeight,
                const ModelDefinition& definition = ModelDefinition());
    ZombieModel(C4JRender::eVertexType vertexType, C4JRender::ePixelShaderType pixelShaderType,
                const ModelDefinition& definition, float scale, float yOffset, int texWidth, int texHeight,
                bool armourModel, bool createParts, bool skinned);

    virtual ~ZombieModel() {}

    virtual void setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments);
    virtual std::wstring getName() { return L"zombie"; }
};

#endif
