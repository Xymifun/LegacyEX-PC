#include "game/client/render/PhantomModel.hpp"

#include <cmath>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/block/MaterialTable.hpp"

#ifndef MCWIIU_EMISSIVE_MODEL_NO_MIPMAPS
#define MCWIIU_EMISSIVE_MODEL_NO_MIPMAPS 1
#endif

PhantomModel::PhantomModel(const ModelDefinition& modelDefinition)
    : mPhantomParts(NULL),
      mNumPhantomParts(0),
      mInvisibleParts()
{
    mTexHeight = 64;
    mTexWidth = 64;

    int numParts = 16;
    ModelPartSkin** parts = new ModelPartSkin*[numParts];

    for (ModelPartSkin** part = parts; part < parts + numParts; ++part)
    {
        *part = NULL;
    }

    mPhantomParts = parts;
    mNumPhantomParts = numParts;

    createModelParts(modelDefinition);
    createInvisibleModelParts(modelDefinition);
}

PhantomModel::~PhantomModel()
{
    delete[] mPhantomParts;
}

void PhantomModel::createModelParts(const ModelDefinition& modelDefinition)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    builder->setVertexAndPixelShader(eMinecraftEntityMaterial_Phantom);

    if (modelDefinition.DoesOverrideModel())
    {
        mHasOverriddenParts = true;
        mPhantomParts[0] = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, NULL);
        mPhantomParts[1] = new ModelPartSkin(this, L"wing0", modelDefinition, mPhantomParts[0], 0.0f, NULL);
        mPhantomParts[2] = new ModelPartSkin(this, L"wingtip0", modelDefinition, mPhantomParts[1], 0.0f, NULL);
        mPhantomParts[3] = new ModelPartSkin(this, L"wing1", modelDefinition, mPhantomParts[0], 0.0f, NULL);
        mPhantomParts[4] = new ModelPartSkin(this, L"wingtip1", modelDefinition, mPhantomParts[3], 0.0f, NULL);
        mPhantomParts[5] = new ModelPartSkin(this, L"head", modelDefinition, mPhantomParts[0], 0.0f, NULL);
        mPhantomParts[6] = new ModelPartSkin(this, L"tail", modelDefinition, mPhantomParts[0], 0.0f, NULL);
        mPhantomParts[7] = new ModelPartSkin(this, L"tailtip", modelDefinition, mPhantomParts[6], 0.0f, NULL);
    }
    else
    {
        mPhantomParts[0] = new ModelPartSkin(this, 0, 8, NULL);
        mPhantomParts[0]->addBox(-3.0f, -2.0f, -8.0f, 5, 3, 9);
        mPhantomParts[0]->mXRot = -0.1f;

        mPhantomParts[1] = new ModelPartSkin(this, 23, 12, NULL);
        mPhantomParts[1]->addBox(0.0f, 0.0f, 0.0f, 6, 2, 9);
        mPhantomParts[1]->setPos(2.0f, -2.0f, -8.0f);
        mPhantomParts[1]->mZRot = 0.1f;

        mPhantomParts[2] = new ModelPartSkin(this, 16, 24, NULL);
        mPhantomParts[2]->addBox(0.0f, 0.0f, 0.0f, 13, 1, 9);
        mPhantomParts[2]->setPos(6.0f, 0.0f, 0.0f);
        mPhantomParts[2]->mZRot = 0.1f;

        mPhantomParts[3] = new ModelPartSkin(this, 23, 12, NULL);
        mPhantomParts[3]->mirror();
        mPhantomParts[3]->addBox(-6.0f, 0.0f, 0.0f, 6, 2, 9);
        mPhantomParts[3]->setPos(-3.0f, -2.0f, -8.0f);
        mPhantomParts[3]->mZRot = -0.1f;

        mPhantomParts[4] = new ModelPartSkin(this, 16, 24, NULL);
        mPhantomParts[4]->mirror();
        mPhantomParts[4]->addBox(-13.0f, 0.0f, 0.0f, 13, 1, 9);
        mPhantomParts[4]->setPos(-6.0f, 0.0f, 0.0f);
        mPhantomParts[4]->mZRot = -0.1f;

        mPhantomParts[5] = new ModelPartSkin(this, 0, 0, NULL);
        mPhantomParts[5]->addBox(-4.0f, -2.0f, -5.0f, 7, 3, 5);
        mPhantomParts[5]->setPos(0.0f, 1.0f, -7.0f);
        mPhantomParts[5]->mXRot = 0.2f;

        mPhantomParts[6] = new ModelPartSkin(this, 3, 20, NULL);
        mPhantomParts[6]->addBox(-2.0f, 0.0f, 0.0f, 3, 2, 6);
        mPhantomParts[6]->setPos(0.0f, -2.0f, 1.0f);

        mPhantomParts[7] = new ModelPartSkin(this, 4, 29, NULL);
        mPhantomParts[7]->addBox(-1.0f, 0.0f, 0.0f, 1, 1, 6);
        mPhantomParts[7]->setPos(0.0f, 0.5f, 6.0f);

        mPhantomParts[0]->addChild(mPhantomParts[1]);
        mPhantomParts[0]->addChild(mPhantomParts[3]);
        mPhantomParts[0]->addChild(mPhantomParts[5]);
        mPhantomParts[0]->addChild(mPhantomParts[6]);
        mPhantomParts[1]->addChild(mPhantomParts[2]);
        mPhantomParts[3]->addChild(mPhantomParts[4]);
        mPhantomParts[6]->addChild(mPhantomParts[7]);
    }

    const bool mipmap = builder->setMipmapEnable(!MCWIIU_EMISSIVE_MODEL_NO_MIPMAPS);
    compileSkinnedDrawlist();
    builder->setMipmapEnable(mipmap);
    builder->useDefaultShaders();
}

void PhantomModel::createInvisibleModelParts(const ModelDefinition& modelDefinition)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    builder->setVertexAndPixelShader(eMinecraftEntityMaterial_PhantomInvisible);

    if (modelDefinition.DoesOverrideModel())
    {
        mHasOverriddenParts = true;
        mPhantomParts[8] = new ModelPartSkin(this, L"body", modelDefinition, NULL, 0.0f, &mInvisibleParts);
        mPhantomParts[9] = new ModelPartSkin(this, L"wing0", modelDefinition, mPhantomParts[8], 0.0f, &mInvisibleParts);
        mPhantomParts[10] = new ModelPartSkin(this, L"wingtip0", modelDefinition, mPhantomParts[9], 0.0f,
                                              &mInvisibleParts);
        mPhantomParts[11] = new ModelPartSkin(this, L"wing1", modelDefinition, mPhantomParts[8], 0.0f,
                                              &mInvisibleParts);
        mPhantomParts[12] = new ModelPartSkin(this, L"wingtip1", modelDefinition, mPhantomParts[11], 0.0f,
                                              &mInvisibleParts);
        mPhantomParts[13] = new ModelPartSkin(this, L"head", modelDefinition, mPhantomParts[8], 0.0f, &mInvisibleParts);
        mPhantomParts[14] = new ModelPartSkin(this, L"tail", modelDefinition, mPhantomParts[8], 0.0f, &mInvisibleParts);
        mPhantomParts[15] = new ModelPartSkin(this, L"tailtip", modelDefinition, mPhantomParts[14], 0.0f,
                                              &mInvisibleParts);
    }
    else
    {
        mPhantomParts[8] = new ModelPartSkin(this, 0, 8, &mInvisibleParts);
        mPhantomParts[8]->addBox(-3.0f, -2.0f, -8.0f, 5, 3, 9);
        mPhantomParts[8]->mXRot = -0.1f;
        mPhantomParts[9] = new ModelPartSkin(this, 23, 12, &mInvisibleParts);
        mPhantomParts[9]->addBox(0.0f, 0.0f, 0.0f, 6, 2, 9);
        mPhantomParts[9]->setPos(2.0f, -2.0f, -8.0f);
        mPhantomParts[9]->mZRot = 0.1f;
        mPhantomParts[10] = new ModelPartSkin(this, 16, 24, &mInvisibleParts);
        mPhantomParts[10]->addBox(0.0f, 0.0f, 0.0f, 13, 1, 9);
        mPhantomParts[10]->setPos(6.0f, 0.0f, 0.0f);
        mPhantomParts[10]->mZRot = 0.1f;
        mPhantomParts[11] = new ModelPartSkin(this, 23, 12, &mInvisibleParts);
        mPhantomParts[11]->mirror();
        mPhantomParts[11]->addBox(-6.0f, 0.0f, 0.0f, 6, 2, 9);
        mPhantomParts[11]->setPos(-3.0f, -2.0f, -8.0f);
        mPhantomParts[11]->mZRot = -0.1f;
        mPhantomParts[12] = new ModelPartSkin(this, 16, 24, &mInvisibleParts);
        mPhantomParts[12]->mirror();
        mPhantomParts[12]->addBox(-13.0f, 0.0f, 0.0f, 13, 1, 9);
        mPhantomParts[12]->setPos(-6.0f, 0.0f, 0.0f);
        mPhantomParts[12]->mZRot = -0.1f;
        mPhantomParts[13] = new ModelPartSkin(this, 0, 0, &mInvisibleParts);
        mPhantomParts[13]->addBox(-4.0f, -2.0f, -5.0f, 7, 3, 5);
        mPhantomParts[13]->setPos(0.0f, 1.0f, -7.0f);
        mPhantomParts[13]->mXRot = 0.2f;
        mPhantomParts[14] = new ModelPartSkin(this, 3, 20, &mInvisibleParts);
        mPhantomParts[14]->addBox(-2.0f, 0.0f, 0.0f, 3, 2, 6);
        mPhantomParts[14]->setPos(0.0f, -2.0f, 1.0f);
        mPhantomParts[15] = new ModelPartSkin(this, 4, 29, &mInvisibleParts);
        mPhantomParts[15]->addBox(-1.0f, 0.0f, 0.0f, 1, 1, 6);
        mPhantomParts[15]->setPos(0.0f, 0.5f, 6.0f);

        mPhantomParts[8]->addChild(mPhantomParts[9]);
        mPhantomParts[8]->addChild(mPhantomParts[11]);
        mPhantomParts[8]->addChild(mPhantomParts[13]);
        mPhantomParts[8]->addChild(mPhantomParts[14]);
        mPhantomParts[9]->addChild(mPhantomParts[10]);
        mPhantomParts[11]->addChild(mPhantomParts[12]);
        mPhantomParts[14]->addChild(mPhantomParts[15]);
    }

    const bool mipmap = builder->setMipmapEnable(!MCWIIU_EMISSIVE_MODEL_NO_MIPMAPS);
    mInvisibleParts.compile(0.0625f);
    builder->setMipmapEnable(mipmap);
    builder->useDefaultShaders();
}

void PhantomModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                          float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    if (entity->isInvisible())
    {
        mPhantomParts[8]->render(scale, useDisplayList, false);
        mInvisibleParts.render();
    }
    else
    {
        mPhantomParts[0]->render(scale, useDisplayList, false);
        callSkinnedDrawlist();
    }
}

void PhantomModel::setupAnim(float limbSwing, float limbSwingAmount, float ageInTicks, float netHeadYaw,
                             float headPitch, float scale, boost::shared_ptr<Entity> entity,
                             _SkinAdjustments skinAdjustments)
{
    float angle = (entity->getId() * 3.0f + ageInTicks) * 0.13f;
    float amplitude = 16.0f;

    mPhantomParts[1]->mZRot = (amplitude + Mth::cos(angle) * amplitude) * Mth::DEG_TO_RAD;
    mPhantomParts[2]->mZRot = (amplitude + Mth::cos(angle) * amplitude) * Mth::DEG_TO_RAD;
    mPhantomParts[3]->mZRot = -mPhantomParts[1]->mZRot;
    mPhantomParts[4]->mZRot = -mPhantomParts[2]->mZRot;
    mPhantomParts[6]->mXRot = -(5.0f + Mth::cos(angle * 2.0f) * 5.0f) * Mth::DEG_TO_RAD;
    mPhantomParts[7]->mXRot = -(5.0f + Mth::cos(angle * 2.0f) * 5.0f) * Mth::DEG_TO_RAD;
}
