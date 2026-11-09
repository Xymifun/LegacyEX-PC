#include "game/client/render/EndermiteModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/Mth.hpp"

const int EndermiteModel::BODY_PART_SIZES[BODY_PART_COUNT][3] = {{4, 3, 2}, {6, 4, 5}, {3, 3, 1}, {1, 2, 1}};

const int EndermiteModel::BODY_PART_TEX_OFFS[BODY_PART_COUNT][2] = {{0, 0}, {0, 5}, {0, 14}, {0, 18}};

EndermiteModel::EndermiteModel(const ModelDefinition& modelDefinition)
{
    if (modelDefinition.DoesOverrideModel())
    {
        mBodyParts[0] = new ModelPartSkin(this, L"bodyPart_0", modelDefinition, NULL, 0.0f, NULL);
        mBodyParts[1] = new ModelPartSkin(this, L"bodyPart_1", modelDefinition, NULL, 0.0f, NULL);
        mBodyParts[2] = new ModelPartSkin(this, L"bodyPart_2", modelDefinition, NULL, 0.0f, NULL);
        mBodyParts[3] = new ModelPartSkin(this, L"bodyPart_3", modelDefinition, NULL, 0.0f, NULL);

        compileSkinnedDrawlist();
    }
    else
    {
        float zPos = -3.5f;

        for (int i = 0; i < BODY_PART_COUNT; ++i)
        {
            mBodyParts[i] = new ModelPartSkin(this, BODY_PART_TEX_OFFS[i][0], BODY_PART_TEX_OFFS[i][1], NULL);
            mBodyParts[i]->addBox(BODY_PART_SIZES[i][0] * -0.5f, 0.0f, BODY_PART_SIZES[i][2] * -0.5f,
                                  BODY_PART_SIZES[i][0], BODY_PART_SIZES[i][1], BODY_PART_SIZES[i][2]);
            mBodyParts[i]->setPos(0.0f, 24.0f - BODY_PART_SIZES[i][1], zPos);

            if (i < BODY_PART_COUNT - 1)
            {
                zPos += (BODY_PART_SIZES[i][2] + BODY_PART_SIZES[i + 1][2]) * 0.5f;
            }
        }

        compileSkinnedDrawlist();
    }
}

EndermiteModel::~EndermiteModel()
{
}

void EndermiteModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                            float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    Model::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    for (int i = 0; i < BODY_PART_COUNT; ++i)
    {
        mBodyParts[i]->render(scale, true, false);
    }

    callSkinnedDrawlist();
}

void EndermiteModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                               float headPitch, float scale, boost::shared_ptr<Entity> entity,
                               _SkinAdjustments skinAdjustments)
{
    for (int i = 0; i < BODY_PART_COUNT; ++i)
    {
        mBodyParts[i]->mYRot = Mth::cos(ageInTicks * 0.9f + (float)i * 0.15f * 3.1415927f) * 3.1415927f * 0.01f
                               * (float)(1 + Mth::abs(i - 2));
        mBodyParts[i]->mX = Mth::sin(ageInTicks * 0.9f + (float)i * 0.15f * 3.1415927f) * 3.1415927f * 0.1f
                            * (float)Mth::abs(i - 2);
    }
}
