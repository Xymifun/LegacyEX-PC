#include "game/client/render/SilverfishModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/Mth.hpp"

#include <cstdlib>

const int SilverfishModel::BODY_PART_TEX_OFFS[BODY_PART_COUNT][2] = {{0, 0},  {0, 4},  {0, 9}, {0, 16},
                                                                     {0, 22}, {11, 0}, {13, 4}};

const int SilverfishModel::BODY_PART_SIZES[BODY_PART_COUNT][3] = {{3, 2, 2}, {4, 3, 2}, {6, 4, 3}, {3, 3, 3},
                                                                  {2, 2, 3}, {2, 1, 2}, {1, 1, 2}};

SilverfishModel::SilverfishModel(const ModelDefinition& modelDefinition)
{
    mBodyParts = arrayWithLength<ModelPart*>(BODY_PART_COUNT, true);
    mBodyLayers = arrayWithLength<ModelPart*>(BODY_LAYER_COUNT, true);

    if (modelDefinition.DoesOverrideModel())
    {
        mBodyParts.data[0] = new ModelPartSkin(this, L"bodyPart_0", modelDefinition, NULL, 0.0f, NULL);
        mBodyParts.data[1] = new ModelPartSkin(this, L"bodyPart_1", modelDefinition, NULL, 0.0f, NULL);
        mBodyParts.data[2] = new ModelPartSkin(this, L"bodyPart_2", modelDefinition, NULL, 0.0f, NULL);
        mBodyParts.data[3] = new ModelPartSkin(this, L"bodyPart_3", modelDefinition, NULL, 0.0f, NULL);
        mBodyParts.data[4] = new ModelPartSkin(this, L"bodyPart_4", modelDefinition, NULL, 0.0f, NULL);
        mBodyParts.data[5] = new ModelPartSkin(this, L"bodyPart_5", modelDefinition, NULL, 0.0f, NULL);
        mBodyParts.data[6] = new ModelPartSkin(this, L"bodyPart_6", modelDefinition, NULL, 0.0f, NULL);

        mBodyLayers.data[0] = new ModelPartSkin(this, L"bodyLayer_0", modelDefinition, NULL, 0.0f, NULL);
        mBodyLayers.data[1] = new ModelPartSkin(this, L"bodyLayer_1", modelDefinition, NULL, 0.0f, NULL);
        mBodyLayers.data[2] = new ModelPartSkin(this, L"bodyLayer_2", modelDefinition, NULL, 0.0f, NULL);

        compileSkinnedDrawlist();
    }
    else
    {
        float zPos = -3.5f;

        for (unsigned int i = 0; i < mBodyParts.length; ++i)
        {
            mBodyParts.data[i] = new ModelPartSkin(this, BODY_PART_TEX_OFFS[i][0], BODY_PART_TEX_OFFS[i][1], NULL);
            mBodyParts.data[i]->addBox(BODY_PART_SIZES[i][0] * -0.5f, 0.0f, BODY_PART_SIZES[i][2] * -0.5f,
                                       BODY_PART_SIZES[i][0], BODY_PART_SIZES[i][1], BODY_PART_SIZES[i][2]);
            mBodyParts.data[i]->setPos(0.0f, 24.0f - BODY_PART_SIZES[i][1], zPos);
            mBodyPartZ[i] = zPos;

            if (i < mBodyParts.length - 1)
            {
                zPos += (BODY_PART_SIZES[i][2] + BODY_PART_SIZES[i + 1][2]) * 0.5f;
            }
        }

        mBodyLayers.data[0] = new ModelPartSkin(this, 20, 0, NULL);
        mBodyLayers.data[0]->addBox(-5.0f, 0.0f, -1.5f, 10, 8, 3);
        mBodyLayers.data[0]->setPos(0.0f, 16.0f, mBodyPartZ[2]);

        mBodyLayers.data[1] = new ModelPartSkin(this, 20, 11, NULL);
        mBodyLayers.data[1]->addBox(-3.0f, 0.0f, -1.5f, 6, 4, 3);
        mBodyLayers.data[1]->setPos(0.0f, 20.0f, mBodyPartZ[4]);

        mBodyLayers.data[2] = new ModelPartSkin(this, 20, 18, NULL);
        mBodyLayers.data[2]->addBox(-3.0f, 0.0f, -1.5f, 6, 5, 2);
        mBodyLayers.data[2]->setPos(0.0f, 19.0f, mBodyPartZ[1]);

        compileSkinnedDrawlist();
    }
}

SilverfishModel::~SilverfishModel()
{
    for (unsigned int i = 0; i < mBodyParts.length; ++i)
    {
        delete mBodyParts.data[i];
    }

    for (unsigned int i = 0; i < mBodyLayers.length; ++i)
    {
        delete mBodyLayers.data[i];
    }
}

void SilverfishModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                             float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    for (unsigned int i = 0; i < mBodyParts.length; ++i)
    {
        mBodyParts.data[i]->render(scale, useDisplayList, false);
    }

    for (unsigned int i = 0; i < mBodyLayers.length; ++i)
    {
        mBodyLayers.data[i]->render(scale, useDisplayList, false);
    }

    callSkinnedDrawlist();
}

void SilverfishModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                                float headPitch, float scale, boost::shared_ptr<Entity> entity,
                                _SkinAdjustments skinAdjustments)
{
    for (unsigned int i = 0; i < mBodyParts.length; ++i)
    {
        mBodyParts.data[i]->mYRot = Mth::cos(ageInTicks * 0.9f + (float)i * 0.15f * 3.1415927f) * 3.1415927f * 0.05f
                                    * (float)(1 + abs((int)i - 2));
        mBodyParts.data[i]->mX = Mth::sin(ageInTicks * 0.9f + (float)i * 0.15f * 3.1415927f) * 3.1415927f * 0.2f
                                 * (float)abs((int)i - 2);
    }

    mBodyLayers.data[0]->mYRot = mBodyParts.data[2]->mYRot;

    mBodyLayers.data[1]->mYRot = mBodyParts.data[4]->mYRot;
    mBodyLayers.data[1]->mX = mBodyParts.data[4]->mX;

    mBodyLayers.data[2]->mYRot = mBodyParts.data[1]->mYRot;
    mBodyLayers.data[2]->mX = mBodyParts.data[1]->mX;
}
