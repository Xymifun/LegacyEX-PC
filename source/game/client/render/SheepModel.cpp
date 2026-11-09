#include "game/client/render/SheepModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Sheep.hpp"

SheepModel::SheepModel(const ModelDefinition& modelDefinition)
    : QuadrupedModel(12, 0.0f, modelDefinition),
      mHeadEatAngleScale(0.0f)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    if (modelDefinition.DoesOverrideModel())
    {
        mHasOverriddenParts = true;
        builder->setVertexAndPixelShader(eMinecraftEntityMaterial_Sheep);
        compileSkinnedDrawlist();
        builder->useDefaultShaders();
    }
    else
    {
        delete mHead;
        mHead = new ModelPartSkin(this, 0, 0, NULL);
        mHead->addBox(-3.0f, -4.0f, -6.0f, 6, 6, 8, 0.0f);
        mHead->setPos(0.0f, 6.0f, -8.0f);

        delete mBody;
        mBody = new ModelPartSkin(this, 28, 8, NULL);
        mBody->addBox(-4.0f, -10.0f, -7.0f, 8, 16, 6, 0.0f);
        mBody->setPos(0.0f, 5.0f, 2.0f);

        compileSkinnedDrawlist();
        builder->useDefaultShaders();
    }
}

void SheepModel::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                 float partialTicks)
{
    Model::prepareMobModel(entity, limbSwing, limbSwingAmount, partialTicks);

    boost::shared_ptr<Sheep> sheep = boost::dynamic_pointer_cast<Sheep>(entity);
    mHead->mY = 9.0f * sheep->getHeadEatPositionScale(partialTicks) + 6.0f;
    mHeadEatAngleScale = sheep->getHeadEatAngleScale(partialTicks);
}

void SheepModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    QuadrupedModel::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity,
                              _SkinAdjustments());
    mHead->mXRot = mHeadEatAngleScale;
}
