#include "game/client/render/LavaSlimeModel.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/MagmaCube.hpp"
#include "game/entity/mob/LivingEntity.hpp"

LavaSlimeModel::LavaSlimeModel(const ModelDefinition& modelDefinition)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    if (modelDefinition.DoesOverrideModel())
    {
        mHasOverriddenParts = true;
        builder->setVertexAndPixelShader(eMinecraftEntityMaterial_MagmaCube);

        mBodyCubes[0] = new ModelPartSkin(this, L"bodyCube_0", modelDefinition, NULL, 0.0f, NULL);
        mBodyCubes[1] = new ModelPartSkin(this, L"bodyCube_1", modelDefinition, NULL, 0.0f, NULL);
        mBodyCubes[2] = new ModelPartSkin(this, L"bodyCube_2", modelDefinition, NULL, 0.0f, NULL);
        mBodyCubes[3] = new ModelPartSkin(this, L"bodyCube_3", modelDefinition, NULL, 0.0f, NULL);
        mBodyCubes[4] = new ModelPartSkin(this, L"bodyCube_4", modelDefinition, NULL, 0.0f, NULL);
        mBodyCubes[5] = new ModelPartSkin(this, L"bodyCube_5", modelDefinition, NULL, 0.0f, NULL);
        mBodyCubes[6] = new ModelPartSkin(this, L"bodyCube_6", modelDefinition, NULL, 0.0f, NULL);
        mBodyCubes[7] = new ModelPartSkin(this, L"bodyCube_7", modelDefinition, NULL, 0.0f, NULL);

        mInsideCube = new ModelPartSkin(this, L"insideCube", modelDefinition, NULL, 0.0f, NULL);

        compileSkinnedDrawlist();
        builder->useDefaultShaders();
    }
    else
    {
        for (int i = 0; i < 8; ++i)
        {
            int xTexOffs = 0;
            int yTexOffs = i;

            if (i == 2)
            {
                xTexOffs = 24;
                yTexOffs = 10;
            }
            else if (i == 3)
            {
                xTexOffs = 24;
                yTexOffs = 19;
            }

            mBodyCubes[i] = new ModelPartSkin(this, xTexOffs, yTexOffs, NULL);
            mBodyCubes[i]->addBox(-4.0f, (float)i + 16.0f, -4.0f, 8, 1, 8);
        }

        mInsideCube = new ModelPartSkin(this, 0, 16, NULL);
        mInsideCube->addBox(-2.0f, 18.0f, -2.0f, 4, 4, 4);

        compileSkinnedDrawlist();
        builder->useDefaultShaders();
    }
}

void LavaSlimeModel::render(boost::shared_ptr<Entity>& entity, float limbSwing, float limbSwingAmount, float ageInTicks,
                            float netHeadYaw, float headPitch, float scale, bool useDisplayList)
{
    setupAnim(limbSwing, limbSwingAmount, ageInTicks, netHeadYaw, headPitch, scale, entity, _SkinAdjustments());

    mInsideCube->render(scale, useDisplayList, false);

    for (int i = 0; i < 8; ++i)
    {
        mBodyCubes[i]->render(scale, useDisplayList, false);
    }

    callSkinnedDrawlist();
}

void LavaSlimeModel::prepareMobModel(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                                     float partialTicks)
{
    boost::shared_ptr<MagmaCube> magmaCube = boost::static_pointer_cast<MagmaCube>(entity);

    float squish = magmaCube->mSquishO + (magmaCube->mSquish - magmaCube->mSquishO) * partialTicks;

    if (squish < 0.0f)
    {
        squish = 0.0f;
    }

    for (int i = 0; i < 8; ++i)
    {
        mBodyCubes[i]->mY = (float)(i - 4) * squish * 1.7f;
    }
}
