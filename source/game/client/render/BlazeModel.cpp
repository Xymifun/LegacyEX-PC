#include "game/client/render/BlazeModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Mth.hpp"

BlazeModel::BlazeModel(const ModelDefinition& modelDefinition)
{
    mUpperBodyParts = arrayWithLength<ModelPartSkin*>(12, true);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    if (modelDefinition.DoesOverrideModel())
    {
        mHasOverriddenParts = true;

        mHead = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, NULL);

        mUpperBodyParts.data[0] = new ModelPartSkin(this, L"upperBodyParts0", modelDefinition, NULL, 0.0f, NULL);
        mUpperBodyParts.data[1] = new ModelPartSkin(this, L"upperBodyParts1", modelDefinition, NULL, 0.0f, NULL);
        mUpperBodyParts.data[2] = new ModelPartSkin(this, L"upperBodyParts2", modelDefinition, NULL, 0.0f, NULL);
        mUpperBodyParts.data[3] = new ModelPartSkin(this, L"upperBodyParts3", modelDefinition, NULL, 0.0f, NULL);
        mUpperBodyParts.data[4] = new ModelPartSkin(this, L"upperBodyParts4", modelDefinition, NULL, 0.0f, NULL);
        mUpperBodyParts.data[5] = new ModelPartSkin(this, L"upperBodyParts5", modelDefinition, NULL, 0.0f, NULL);
        mUpperBodyParts.data[6] = new ModelPartSkin(this, L"upperBodyParts6", modelDefinition, NULL, 0.0f, NULL);
        mUpperBodyParts.data[7] = new ModelPartSkin(this, L"upperBodyParts7", modelDefinition, NULL, 0.0f, NULL);
        mUpperBodyParts.data[8] = new ModelPartSkin(this, L"upperBodyParts8", modelDefinition, NULL, 0.0f, NULL);
        mUpperBodyParts.data[9] = new ModelPartSkin(this, L"upperBodyParts9", modelDefinition, NULL, 0.0f, NULL);
        mUpperBodyParts.data[10] = new ModelPartSkin(this, L"upperBodyParts10", modelDefinition, NULL, 0.0f, NULL);
        mUpperBodyParts.data[11] = new ModelPartSkin(this, L"upperBodyParts11", modelDefinition, NULL, 0.0f, NULL);

        builder->setVertexAndPixelShader(eMinecraftEntityMaterial_BlazeHead);

        compileSkinnedDrawlist();

        builder->useDefaultShaders();
    }
    else
    {
        for (unsigned int i = 0; i < mUpperBodyParts.length; ++i)
        {
            mUpperBodyParts.data[i] = new ModelPartSkin(this, 0, 16, NULL);
            mUpperBodyParts.data[i]->addBox(0.0f, 0.0f, 0.0f, 2, 8, 2);
        }

        mHead = new ModelPartSkin(this, 0, 0, NULL);
        mHead->addBox(-4.0f, -4.0f, -4.0f, 8, 8, 8);

        compileSkinnedDrawlist();

        builder->useDefaultShaders();
    }
}

void BlazeModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    mHead->render(scale, useDisplayList, false);

    for (unsigned int i = 0; i < mUpperBodyParts.length; ++i)
    {
        mUpperBodyParts.data[i]->render(scale, useDisplayList, false);
    }

    callSkinnedDrawlist();
}

void BlazeModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    float angle = ageInTicks * 3.1415927f * -0.1f;

    for (int i = 0; i < 4; ++i)
    {
        mUpperBodyParts.data[i]->mY = -2.0f + Mth::cos(((float)(i * 2) + ageInTicks) * 0.25f);
        mUpperBodyParts.data[i]->mX = Mth::cos(angle) * 9.0f;
        mUpperBodyParts.data[i]->mZ = Mth::sin(angle) * 9.0f;
        angle += 1.5707964f;
    }

    angle = 0.7853982f + ageInTicks * 3.1415927f * 0.03f;

    for (int i = 4; i < 8; ++i)
    {
        mUpperBodyParts.data[i]->mY = 2.0f + Mth::cos(((float)(i * 2) + ageInTicks) * 0.25f);
        mUpperBodyParts.data[i]->mX = Mth::cos(angle) * 7.0f;
        mUpperBodyParts.data[i]->mZ = Mth::sin(angle) * 7.0f;
        angle += 1.5707964f;
    }

    angle = 0.47123894f + ageInTicks * 3.1415927f * -0.05f;

    for (int i = 8; i < 12; ++i)
    {
        mUpperBodyParts.data[i]->mY = 11.0f + Mth::cos(((float)i * 1.5f + ageInTicks) * 0.5f);
        mUpperBodyParts.data[i]->mX = Mth::cos(angle) * 5.0f;
        mUpperBodyParts.data[i]->mZ = Mth::sin(angle) * 5.0f;
        angle += 1.5707964f;
    }

    mHead->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
    mHead->mXRot = headPitch * Mth::DEG_TO_RAD;
}
