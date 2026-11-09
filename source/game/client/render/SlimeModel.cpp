#include "game/client/render/SlimeModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/GlStateManager.hpp"

SlimeModel::SlimeModel(int size, const ModelDefinition& modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mBody = new ModelPartSkin(this, L"cube", modelDefinition, NULL, 0.0f, NULL);
        mEye0 = new ModelPartSkin(this, L"eye0", modelDefinition, NULL, 0.0f, NULL);
        mEye1 = new ModelPartSkin(this, L"eye1", modelDefinition, NULL, 0.0f, NULL);
        mMouth = new ModelPartSkin(this, L"mouth", modelDefinition, NULL, 0.0f, NULL);

        compileSkinnedDrawlist();
    }
    else
    {
        mBody = new ModelPartSkin(this, 0, size, NULL);

        if (size > 0)
        {
            mBody->addBox(-3.0f, 17.0f, -3.0f, 6, 6, 6);

            mEye0 = new ModelPartSkin(this, 32, 0, NULL);
            mEye0->addBox(-3.25f, 18.0f, -3.5f, 2, 2, 2);

            mEye1 = new ModelPartSkin(this, 32, 4, NULL);
            mEye1->addBox(1.25f, 18.0f, -3.5f, 2, 2, 2);

            mMouth = new ModelPartSkin(this, 32, 8, NULL);
            mMouth->addBox(0.0f, 21.0f, -3.5f, 1, 1, 1);
        }
        else
        {
            mBody->addBox(-4.0f, 16.0f, -4.0f, 8, 8, 8);
            mEye0 = NULL;
            mEye1 = NULL;
            mMouth = NULL;
        }

        compileSkinnedDrawlist();
    }
}

void SlimeModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    GlStateManager::translatef(0.0f, 0.001f, 0.0f);
    mBody->render(scale, useDisplayList, false);

    if (mEye0 != NULL)
    {
        mEye0->render(scale, useDisplayList, false);
        mEye1->render(scale, useDisplayList, false);
        mMouth->render(scale, useDisplayList, false);
    }

    callSkinnedDrawlist();
}
