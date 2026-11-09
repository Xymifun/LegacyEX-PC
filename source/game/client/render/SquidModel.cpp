#include "game/client/render/SquidModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/Mth.hpp"

SquidModel::SquidModel(const ModelDefinition& definition) : Model()
{
    if (definition.DoesOverrideModel())
    {
        const float scale = 0.0f;
        mBody = new ModelPartSkin(this, L"body", definition, 0, scale, NULL);
        mBody->setYOffset(-16.0f);
        mTentacles[0] = new ModelPartSkin(this, L"tentacle1", definition, 0, scale, NULL);
        mTentacles[1] = new ModelPartSkin(this, L"tentacle2", definition, 0, scale, NULL);
        mTentacles[2] = new ModelPartSkin(this, L"tentacle3", definition, 0, scale, NULL);
        mTentacles[3] = new ModelPartSkin(this, L"tentacle4", definition, 0, scale, NULL);
        mTentacles[4] = new ModelPartSkin(this, L"tentacle5", definition, 0, scale, NULL);
        mTentacles[5] = new ModelPartSkin(this, L"tentacle6", definition, 0, scale, NULL);
        mTentacles[6] = new ModelPartSkin(this, L"tentacle7", definition, 0, scale, NULL);
        mTentacles[7] = new ModelPartSkin(this, L"tentacle8", definition, 0, scale, NULL);
        for (int i = 0; i < 8; i++)
        {
            const double turn = (float)i * 3.1415927f;
            mTentacles[i]->setYOffset(-16.0f);
            mTentacles[i]->mYRot = (float)(turn * -2.0 * 0.125 + 1.5707963705062866);
        }
        compileSkinnedDrawlist();
    }
    else
    {
        int yOffset = -16;
        mBody = new ModelPartSkin(this, 0, 0, NULL);
        mBody->addBox(-6.0f, -8.0f, -6.0f, 12, 16, 12);
        mBody->mY += 24 + yOffset;

        for (int i = 0; i < 8; i++)
        {
            mTentacles[i] = new ModelPartSkin(this, 48, 0, NULL);
            double angle = (float)i * 3.1415927f * 2.0 / 8.0;
            const float offsetX = Mth::cos(angle) * 5.0f;
            const float offsetZ = Mth::sin(angle) * 5.0f;
            mTentacles[i]->addBox(-1.0f, 0.0f, -1.0f, 2, 18, 2);
            mTentacles[i]->mX = offsetX;
            mTentacles[i]->mZ = offsetZ;
            mTentacles[i]->mY = 31 + yOffset;
            angle = (float)i * 3.1415927f * -2.0 / 8.0 + 1.5707963705062866;
            mTentacles[i]->mYRot = (float)angle;
        }
        compileSkinnedDrawlist();
    }
}

void SquidModel::setupAnim(float, float, float ageInTicks, float, float, float, boost::shared_ptr<Entity>,
                           _SkinAdjustments)
{
    int i;
    for (i = 0; i < 8; ++i)
        mTentacles[i]->mXRot = ageInTicks;
}

void SquidModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                        float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());
    mBody->render(scale, useDisplayList, false);
    for (int i = 0; i < 8; i++)
        mTentacles[i]->render(scale, useDisplayList, false);
    callSkinnedDrawlist();
}
