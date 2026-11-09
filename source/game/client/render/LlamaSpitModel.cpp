#include "game/client/render/LlamaSpitModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"

LlamaSpitModel::LlamaSpitModel(const ModelDefinition& modelDefinition)
{
    _init(0.0f, modelDefinition);
}

LlamaSpitModel::LlamaSpitModel(float delta, const ModelDefinition& modelDefinition)
{
    _init(delta, modelDefinition);
}

void LlamaSpitModel::_init(float delta, const ModelDefinition& modelDefinition)
{
    mSpit = new ModelPart(this);

    mSpit->texOffs(0, 0)->addBox(-4.0f, 0.0f, 0.0f, 2, 2, 2, delta);
    mSpit->texOffs(0, 0)->addBox(0.0f, -4.0f, 0.0f, 2, 2, 2, delta);
    mSpit->texOffs(0, 0)->addBox(0.0f, 0.0f, -4.0f, 2, 2, 2, delta);
    mSpit->texOffs(0, 0)->addBox(0.0f, 0.0f, 0.0f, 2, 2, 2, delta);
    mSpit->texOffs(0, 0)->addBox(2.0f, 0.0f, 0.0f, 2, 2, 2, delta);
    mSpit->texOffs(0, 0)->addBox(0.0f, 2.0f, 0.0f, 2, 2, 2, delta);
    mSpit->texOffs(0, 0)->addBox(0.0f, 0.0f, 2.0f, 2, 2, 2, delta);

    mSpit->setPos(0.0f, 0.0f, 0.0f);
    mSpit->compile(0.0625f);
}

void LlamaSpitModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                            float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    mSpit->render(scale, useDisplayList, false);
}
