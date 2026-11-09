#include "game/client/render/GhastModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"

GhastModel::GhastModel(const ModelDefinition& modelDefinition)
{
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();

    if (modelDefinition.DoesOverrideModel())
    {
        mHasOverriddenParts = true;

        builder->setVertexAndPixelShader(eMinecraftEntityMaterial_Ghast);

        mBody = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, NULL);

        mTentacles[0] = new ModelPartSkin(this, L"tentacles_0", modelDefinition, NULL, 0.0f, NULL);
        mTentacles[1] = new ModelPartSkin(this, L"tentacles_1", modelDefinition, NULL, 0.0f, NULL);
        mTentacles[2] = new ModelPartSkin(this, L"tentacles_2", modelDefinition, NULL, 0.0f, NULL);
        mTentacles[3] = new ModelPartSkin(this, L"tentacles_3", modelDefinition, NULL, 0.0f, NULL);
        mTentacles[4] = new ModelPartSkin(this, L"tentacles_4", modelDefinition, NULL, 0.0f, NULL);
        mTentacles[5] = new ModelPartSkin(this, L"tentacles_5", modelDefinition, NULL, 0.0f, NULL);
        mTentacles[6] = new ModelPartSkin(this, L"tentacles_6", modelDefinition, NULL, 0.0f, NULL);
        mTentacles[7] = new ModelPartSkin(this, L"tentacles_7", modelDefinition, NULL, 0.0f, NULL);
        mTentacles[8] = new ModelPartSkin(this, L"tentacles_8", modelDefinition, NULL, 0.0f, NULL);

        compileSkinnedDrawlist();

        builder->useDefaultShaders();
    }
    else
    {
        mBody = new ModelPartSkin(this, 0, 0, NULL);
        mBody->addBox(-8.0f, -8.0f, -8.0f, 16, 16, 16);
        mBody->setPos(0.0f, 8.0f, 0.0f);

        Random* random = new Random(1660);

        for (int i = 0; i < 9; ++i)
        {
            mTentacles[i] = new ModelPartSkin(this, 0, 0, NULL);

            float x = (((float)(i % 3) - (float)(i / 3 % 2) * 0.5f + 0.25f) / 2.0f * 2.0f - 1.0f) * 5.0f;
            float z = ((float)(i / 3) / 2.0f * 2.0f - 1.0f) * 5.0f;
            int height = random->nextInt(7) + 8;

            mTentacles[i]->addBox(-1.0f, 0.0f, -1.0f, 2, height, 2);
            mTentacles[i]->setPos(x, 15.0f, z);
        }

        compileSkinnedDrawlist();

        builder->useDefaultShaders();
    }
}

void GhastModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    GlStateManager::pushMatrix();
    GlStateManager::translatef(0.0f, 0.6f, 0.0f);

    mBody->render(scale, useDisplayList, false);

    for (int i = 0; i < 9; ++i)
    {
        mTentacles[i]->render(scale, useDisplayList, false);
    }

    GlStateManager::popMatrix();

    callSkinnedDrawlist();
}

void GhastModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw, float headPitch,
                           float scale, boost::shared_ptr<Entity> entity, _SkinAdjustments skinAdjustments)
{
    for (int i = 0; i < 9; ++i)
    {
        mTentacles[i]->mXRot = 0.2f * Mth::sin(ageInTicks * 0.3f + (float)i) + 0.4f;
    }
}
