#include "game/client/render/EnderCrystalModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/GlStateManager.hpp"

EnderCrystalModel::EnderCrystalModel(float, bool renderBase, const ModelDefinition&) : Model()
{
    mGlass = new ModelPartSkin(this, L"glass");
    mGlass->texOffs(0, 0)->addBox(-4.0f, -4.0f, -4.0f, 8, 8, 8);

    mCube = new ModelPartSkin(this, L"cube");
    mCube->texOffs(32, 0)->addBox(-4.0f, -4.0f, -4.0f, 8, 8, 8);

    if (renderBase)
    {
        mBase = new ModelPartSkin(this, L"base");
        mBase->texOffs(0, 16)->addBox(-6.0f, 0.0f, -6.0f, 12, 4, 12);
    }
    else
    {
        mBase = NULL;
    }

    compileSkinnedDrawlist();
}

void EnderCrystalModel::render(boost::shared_ptr<Entity>&, float, float limbSwingAmount, float ageInTicks, float, float,
                               float scale, bool useDisplayList)
{
    GlStateManager::pushMatrix();
    GlStateManager::scalef(2.0f, 2.0f, 2.0f);
    GlStateManager::translatef(0.0f, -0.5f, 0.0f);

    if (mBase != NULL)
    {
        mBase->render(scale, useDisplayList, false);
    }

    GlStateManager::rotatef(limbSwingAmount, 0.0f, 1.0f, 0.0f);
    GlStateManager::translatef(0.0f, ageInTicks + 0.8f, 0.0f);
    GlStateManager::rotatef(60.0f, 0.7071f, 0.0f, 0.7071f);
    mGlass->render(scale, useDisplayList, false);

    GlStateManager::scalef(0.875f, 0.875f, 0.875f);
    GlStateManager::rotatef(60.0f, 0.7071f, 0.0f, 0.7071f);
    GlStateManager::rotatef(limbSwingAmount, 0.0f, 1.0f, 0.0f);
    mGlass->render(scale, useDisplayList, false);

    GlStateManager::scalef(0.875f, 0.875f, 0.875f);
    GlStateManager::rotatef(60.0f, 0.7071f, 0.0f, 0.7071f);
    GlStateManager::rotatef(limbSwingAmount, 0.0f, 1.0f, 0.0f);
    mCube->render(scale, useDisplayList, false);

    GlStateManager::popMatrix();
    callSkinnedDrawlist();
}
