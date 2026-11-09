#ifndef GAME_CLIENT_RENDER_LLAMAMODEL_HPP
#define GAME_CLIENT_RENDER_LLAMAMODEL_HPP

#include <string>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/QuadrupedModel.hpp"

class ModelPartSkin;

class LlamaModel : public QuadrupedModel
{
public:
    LlamaModel(float scale, const ModelDefinition& modelDefinition = ModelDefinition());
    virtual ~LlamaModel();

    virtual void render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList);
    virtual std::wstring getName() { return L"llama"; }

    ModelPartSkin* mChest1;
    ModelPartSkin* mChest2;
};

#endif
