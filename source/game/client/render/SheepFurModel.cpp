#include "game/client/render/SheepFurModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Sheep.hpp"

SheepFurModel::SheepFurModel(const ModelDefinition& modelDefinition)
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
        delete mBody;
        delete mLeg0;
        delete mLeg1;
        delete mLeg2;
        delete mLeg3;

        mHead = new ModelPartSkin(this, 0, 0, NULL);
        mHead->addBox(-3.0f, -4.0f, -4.0f, 6, 6, 6, 0.6f);
        mHead->setPos(0.0f, 6.0f, -8.0f);

        mBody = new ModelPartSkin(this, 28, 8, NULL);
        mBody->addBox(-4.0f, -10.0f, -7.0f, 8, 16, 6, 1.75f);
        mBody->setPos(0.0f, 5.0f, 2.0f);

        mLeg0 = new ModelPartSkin(this, 0, 16, NULL);
        mLeg0->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, 0.5f);
        mLeg0->setPos(-3.0f, 12.0f, 7.0f);

        mLeg1 = new ModelPartSkin(this, 0, 16, NULL);
        mLeg1->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, 0.5f);
        mLeg1->setPos(3.0f, 12.0f, 7.0f);

        mLeg2 = new ModelPartSkin(this, 0, 16, NULL);
        mLeg2->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, 0.5f);
        mLeg2->setPos(-3.0f, 12.0f, -5.0f);

        mLeg3 = new ModelPartSkin(this, 0, 16, NULL);
        mLeg3->addBox(-2.0f, 0.0f, -2.0f, 4, 6, 4, 0.5f);
        mLeg3->setPos(3.0f, 12.0f, -5.0f);

        compileSkinnedDrawlist();
        builder->useDefaultShaders();
    }
}

void SheepFurModel::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                    float partialTicks)
{
    Model::prepareMobModel(entity, limbSwing, limbSwingAmount, partialTicks);

    boost::shared_ptr<Sheep> sheep = boost::dynamic_pointer_cast<Sheep>(entity);
    mHead->mY = 9.0f * sheep->getHeadEatPositionScale(partialTicks) + 6.0f;
    mHeadEatAngleScale = sheep->getHeadEatAngleScale(partialTicks);
}

void SheepFurModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                              float headPitch, float scale, boost::shared_ptr<Entity> entity,
                              _SkinAdjustments skinAdjustments)
{
    QuadrupedModel::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity,
                              _SkinAdjustments());
    mHead->mXRot = mHeadEatAngleScale;
}
