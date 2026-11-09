#include "game/client/render/GuardianModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/Vec3.hpp"

#include "game/core/Guardian.hpp"
#include "game/core/Minecraft.hpp"

GuardianModel::GuardianModel(const ModelDefinition& modelDefinition)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    if (modelDefinition.DoesOverrideModel())
    {
        mHead = new ModelPartSkin(this, L"head", modelDefinition, NULL, 0.0f, NULL);
        mEye = new ModelPartSkin(this, L"eye", modelDefinition, NULL, 0.0f, NULL);

        mSpikeParts[0] = new ModelPartSkin(this, L"spikepart0", modelDefinition, NULL, 0.0f, NULL);
        mSpikeParts[1] = new ModelPartSkin(this, L"spikepart1", modelDefinition, NULL, 0.0f, NULL);
        mSpikeParts[2] = new ModelPartSkin(this, L"spikepart2", modelDefinition, NULL, 0.0f, NULL);
        mSpikeParts[3] = new ModelPartSkin(this, L"spikepart3", modelDefinition, NULL, 0.0f, NULL);
        mSpikeParts[4] = new ModelPartSkin(this, L"spikepart4", modelDefinition, NULL, 0.0f, NULL);
        mSpikeParts[5] = new ModelPartSkin(this, L"spikepart5", modelDefinition, NULL, 0.0f, NULL);
        mSpikeParts[6] = new ModelPartSkin(this, L"spikepart6", modelDefinition, NULL, 0.0f, NULL);
        mSpikeParts[7] = new ModelPartSkin(this, L"spikepart7", modelDefinition, NULL, 0.0f, NULL);
        mSpikeParts[8] = new ModelPartSkin(this, L"spikepart8", modelDefinition, NULL, 0.0f, NULL);
        mSpikeParts[9] = new ModelPartSkin(this, L"spikepart9", modelDefinition, NULL, 0.0f, NULL);
        mSpikeParts[10] = new ModelPartSkin(this, L"spikepart10", modelDefinition, NULL, 0.0f, NULL);
        mSpikeParts[11] = new ModelPartSkin(this, L"spikepart11", modelDefinition, NULL, 0.0f, NULL);

        mTailParts[0] = new ModelPartSkin(this, L"tailpart0", modelDefinition, NULL, 0.0f, NULL);
        mTailParts[1] = new ModelPartSkin(this, L"tailpart1", modelDefinition, NULL, 0.0f, NULL);
        mTailParts[2] = new ModelPartSkin(this, L"tailpart2", modelDefinition, NULL, 0.0f, NULL);

        mHead->addChild(mEye);
        mHead->addChild(mTailParts[0]);
        mTailParts[0]->addChild(mTailParts[1]);
        mTailParts[1]->addChild(mTailParts[2]);

        for (int i = 0; i < 12; ++i)
        {
            mHead->addChild(mSpikeParts[i]);
        }

        builder->setVertexAndPixelShader(eMinecraftEntityMaterial_Guardian);
        compileSkinnedDrawlist();
        builder->useDefaultShaders();
    }
    else
    {
        mTexWidth = 64;
        mTexHeight = 64;

        mHead = new ModelPartSkin(this);
        mHead->texOffs(0, 0)->addBox(-6.0f, 10.0f, -8.0f, 12, 12, 16);
        mHead->texOffs(0, 28)->addBox(-8.0f, 10.0f, -6.0f, 2, 12, 12);
        mHead->texOffs(0, 28)->addBox(6.0f, 10.0f, -6.0f, 2, 12, 12, true);
        mHead->texOffs(16, 40)->addBox(-6.0f, 8.0f, -6.0f, 12, 2, 12);
        mHead->texOffs(16, 40)->addBox(-6.0f, 22.0f, -6.0f, 12, 2, 12);

        for (int i = 0; i < 12; ++i)
        {
            mSpikeParts[i] = new ModelPartSkin(this, 0, 0, NULL);
            mSpikeParts[i]->addBox(-1.0f, -4.5f, -1.0f, 2, 9, 2);
            mHead->addChild(mSpikeParts[i]);
        }

        mEye = new ModelPartSkin(this, 8, 0, NULL);
        mEye->addBox(-1.0f, 15.0f, 0.0f, 2, 2, 1);
        mHead->addChild(mEye);

        mTailParts[0] = new ModelPartSkin(this, 40, 0, NULL);
        mTailParts[0]->addBox(-2.0f, 14.0f, 7.0f, 4, 4, 8);

        mTailParts[1] = new ModelPartSkin(this, 0, 54, NULL);
        mTailParts[1]->addBox(0.0f, 14.0f, 0.0f, 3, 3, 7);

        mTailParts[2] = new ModelPartSkin(this);
        mTailParts[2]->texOffs(41, 32)->addBox(0.0f, 14.0f, 0.0f, 2, 2, 6);
        mTailParts[2]->texOffs(25, 19)->addBox(1.0f, 10.5f, 3.0f, 1, 9, 9);

        mHead->addChild(mTailParts[0]);
        mTailParts[0]->addChild(mTailParts[1]);
        mTailParts[1]->addChild(mTailParts[2]);

        compileSkinnedDrawlist();
        builder->useDefaultShaders();
    }
}

GuardianModel::~GuardianModel()
{
    return;
}

void GuardianModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                           float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    Model::setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    mHead->render(scale, true, false);

    callSkinnedDrawlist();
}

void GuardianModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                              float headPitch, float scale, boost::shared_ptr<Entity> entity,
                              _SkinAdjustments skinAdjustments)
{
    boost::shared_ptr<Guardian> guardian = boost::dynamic_pointer_cast<Guardian>(entity);

    float partialTicks = ageInTicks - (float)guardian->mTickCount;

    mHead->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
    mHead->mXRot = headPitch * Mth::DEG_TO_RAD;

    float spikeXRot[12] = {1.75f, 0.25f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.5f, 1.25f, 0.75f, 0.0f, 0.0f};
    float spikeYRot[12] = {0.0f, 0.0f, 0.0f, 0.0f, 0.25f, 1.75f, 1.25f, 0.75f, 0.0f, 0.0f, 0.0f, 0.0f};
    float spikeZRot[12] = {0.0f, 0.0f, 0.25f, 1.75f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.75f, 1.25f};
    float spikeX[12] = {0.0f, 0.0f, 8.0f, -8.0f, -8.0f, 8.0f, 8.0f, -8.0f, 0.0f, 0.0f, 8.0f, -8.0f};
    float spikeY[12] = {-8.0f, -8.0f, -8.0f, -8.0f, 0.0f, 0.0f, 0.0f, 0.0f, 8.0f, 8.0f, 8.0f, 8.0f};
    float spikeZ[12] = {8.0f, -8.0f, 0.0f, 0.0f, -8.0f, -8.0f, 8.0f, 8.0f, 8.0f, -8.0f, 0.0f, 0.0f};

    float spikeRetraction = (1.0f - guardian->getSpikesAnimation(partialTicks)) * 0.55f;

    for (int i = 0; i < 12; ++i)
    {
        mSpikeParts[i]->mXRot = 3.1415927f * spikeXRot[i];
        mSpikeParts[i]->mYRot = 3.1415927f * spikeYRot[i];
        mSpikeParts[i]->mZRot = 3.1415927f * spikeZRot[i];
        mSpikeParts[i]->mX = spikeX[i] * (Mth::cos(ageInTicks * 1.5f + (float)i) * 0.01f + 1.0f - spikeRetraction);
        mSpikeParts[i]->mY = spikeY[i] * (Mth::cos(ageInTicks * 1.5f + (float)i) * 0.01f + 1.0f - spikeRetraction)
                             + 16.0f;
        mSpikeParts[i]->mZ = spikeZ[i] * (Mth::cos(ageInTicks * 1.5f + (float)i) * 0.01f + 1.0f - spikeRetraction);
    }

    mEye->mZ = -8.25f;

    boost::shared_ptr<Entity> target = Minecraft::GetInstance()->getCameraEntity();

    if (guardian->hasActiveAttackTarget())
    {
        target = guardian->getActiveAttackTarget();
    }

    if (!target)
    {
        mEye->mShow = true;
    }
    else
    {
        Vec3* targetEyePosition = target->getEyePosition(0.0f);
        Vec3* eyePosition = entity->getEyePosition(0.0f);

        if (targetEyePosition->y - eyePosition->y > 0.0)
        {
            mEye->mY = 0.0f;
        }
        else
        {
            mEye->mY = 1.0f;
        }

        Vec3* viewVector = entity->getViewVector(0.0f);
        Vec3* look = Vec3::newTemp(viewVector->x, 0.0, viewVector->z);
        Vec3* toTarget = Vec3::newTemp(eyePosition->x - targetEyePosition->x, 0.0,
                                       eyePosition->z - targetEyePosition->z)
                             ->normalize()
                             ->yRot(1.5707964f);

        double alignment = look->dot(toTarget);
        float eyeRotation = Mth::sqrt((float)Mth::abs(alignment));

        mEye->mShow = true;
        mEye->mX = (eyeRotation + eyeRotation) * (float)((alignment > 0.0) - (alignment < 0.0));
    }

    float tailAnimation = guardian->getTailAnimation(partialTicks);

    mTailParts[0]->mYRot = Mth::sin(tailAnimation) * 3.1415927f * 0.05f;

    mTailParts[1]->mYRot = Mth::sin(tailAnimation) * 3.1415927f * 0.1f;
    mTailParts[1]->mX = -1.5f;
    mTailParts[1]->mY = 0.5f;
    mTailParts[1]->mZ = 14.0f;

    mTailParts[2]->mYRot = Mth::sin(tailAnimation) * 3.1415927f * 0.15f;
    mTailParts[2]->mX = 0.5f;
    mTailParts[2]->mY = 0.5f;
    mTailParts[2]->mZ = 6.0f;
}
