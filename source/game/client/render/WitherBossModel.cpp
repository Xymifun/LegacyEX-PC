#include "game/client/render/WitherBossModel.hpp"
#include "game/core/Mth.hpp"

#include <cmath>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/entity/mob/Entity.hpp"

WitherBossModel::WitherBossModel(float scale, const ModelDefinition& modelDefinition)
{
    mUpperBodyParts = arrayWithLength<ModelPartSkin*>(3, true);
    mHeads = arrayWithLength<ModelPartSkin*>(3, true);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    if (modelDefinition.DoesOverrideModel())
    {
        mUpperBodyParts.data[0] = new ModelPartSkin(this, L"upperBodyPart1", modelDefinition, NULL, scale, NULL);
        mUpperBodyParts.data[1] = new ModelPartSkin(this, L"upperBodyPart2", modelDefinition, NULL, scale, NULL);
        mUpperBodyParts.data[2] = new ModelPartSkin(this, L"upperBodyPart3", modelDefinition, NULL, scale, NULL);
        mHeads.data[0] = new ModelPartSkin(this, L"head1", modelDefinition, NULL, scale, NULL);
        mHeads.data[1] = new ModelPartSkin(this, L"head2", modelDefinition, NULL, scale, NULL);
        mHeads.data[2] = new ModelPartSkin(this, L"head3", modelDefinition, NULL, scale, NULL);

        builder->setVertexAndPixelShader(eMinecraftEntityMaterial_WitherBoss);
        compileSkinnedDrawlist();
        builder->useDefaultShaders();
    }
    else
    {
        mTexWidth = 64;
        mTexHeight = 64;

        ModelPartSkin* part = new ModelPartSkin(this, 0, 16, NULL);
        mUpperBodyParts.data[0] = part;
        part->addBox(-10.0f, 3.9f, -0.5f, 20, 3, 3, scale);

        part = new ModelPartSkin(this);
        mUpperBodyParts.data[1] = part;
        part->setTexSize(mTexWidth, mTexHeight);
        mUpperBodyParts.data[1]->setPos(-2.0f, 6.9f, -0.5f);
        mUpperBodyParts.data[1]->texOffs(0, 22)->addBox(0.0f, 0.0f, 0.0f, 3, 10, 3, scale);
        mUpperBodyParts.data[1]->texOffs(24, 22)->addBox(-4.0f, 1.5f, 0.5f, 11, 2, 2, scale);
        mUpperBodyParts.data[1]->texOffs(24, 22)->addBox(-4.0f, 4.0f, 0.5f, 11, 2, 2, scale);
        mUpperBodyParts.data[1]->texOffs(24, 22)->addBox(-4.0f, 6.5f, 0.5f, 11, 2, 2, scale);

        part = new ModelPartSkin(this, 12, 22, NULL);
        mUpperBodyParts.data[2] = part;
        part->addBox(0.0f, 0.0f, 0.0f, 3, 6, 3, scale);

        part = new ModelPartSkin(this, 0, 0, NULL);
        mHeads.data[0] = part;
        part->addBox(-4.0f, -4.0f, -4.0f, 8, 8, 8, scale);

        part = new ModelPartSkin(this, 32, 0, NULL);
        mHeads.data[1] = part;
        part->addBox(-4.0f, -4.0f, -4.0f, 6, 6, 6, scale);
        mHeads.data[1]->mX = -8.0f;
        mHeads.data[1]->mY = 4.0f;

        part = new ModelPartSkin(this, 32, 0, NULL);
        mHeads.data[2] = part;
        part->addBox(-4.0f, -4.0f, -4.0f, 6, 6, 6, scale);
        mHeads.data[2]->mX = 10.0f;
        mHeads.data[2]->mY = 4.0f;

        compileSkinnedDrawlist();
        builder->useDefaultShaders();
    }
}

void WitherBossModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount,
                             float ageInTicks, float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    for (unsigned int i = 0; i < mHeads.length; ++i)
    {
        mHeads.data[i]->render(scale, useDisplayList, false);
    }

    for (unsigned int i = 0; i < mUpperBodyParts.length; ++i)
    {
        mUpperBodyParts.data[i]->render(scale, useDisplayList, false);
    }

    callSkinnedDrawlist();
}

void WitherBossModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                                float headPitch, float scale, boost::shared_ptr<Entity> entity,
                                _SkinAdjustments skinAdjustments)
{
    float angle = ageInTicks * 0.1f;
    float swing = cosf(angle);
    float headRotation = (swing * 0.05f + 0.065f) * 3.1415927f + headPitch * Mth::DEG_TO_RAD;

    mUpperBodyParts.data[1]->mXRot = headRotation;
    float cosRotation = cosf(headRotation);
    float sinRotation = sinf(mUpperBodyParts.data[1]->mXRot);
    mUpperBodyParts.data[2]->setPos(-2.0f, cosRotation * 10.0f + 6.9f, sinRotation * 10.0f + -0.5f);
    mUpperBodyParts.data[2]->mXRot = (swing * 0.1f + 0.265f) * 3.1415927f;
    mHeads.data[0]->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
    mHeads.data[0]->mXRot = headPitch * Mth::DEG_TO_RAD;
}

void WitherBossModel::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                      float partialTicks)
{
    boost::shared_ptr<WitherBoss> wither = boost::dynamic_pointer_cast<WitherBoss>(entity);
    mHeads.data[1]->mYRot = (wither->getHeadYRot(0) - entity->mYBodyRot) * Mth::DEG_TO_RAD;
    mHeads.data[1]->mXRot = wither->getHeadXRot(0) * Mth::DEG_TO_RAD;
    mHeads.data[2]->mYRot = (wither->getHeadYRot(1) - entity->mYBodyRot) * Mth::DEG_TO_RAD;
    mHeads.data[2]->mXRot = wither->getHeadXRot(1) * Mth::DEG_TO_RAD;
}
