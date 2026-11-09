#include "game/client/render/SpinAttackModel.hpp"

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/core/GlStateManager.hpp"

SpinAttackModel::SpinAttackModel(const ModelDefinition& modelDefinition)
{
    mTexWidth = 64;
    mTexHeight = 64;

    mBox = new ModelPartSkin(this, 0, 0, NULL);
    mBox->addBoxMapTex(-8.0f, -16.0f, -8.0f, 16, 32, 16, 0, 0, 0.0f);

    compileSkinnedDrawlist();
}

void SpinAttackModel::renderSpin(boost::shared_ptr<Entity> entity, float ageInTicks, float scale, bool useDisplayList)
{
    GlStateManager::pushMatrix();
    GlStateManager::rotatef(ageInTicks * -45.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::scalef(0.0f, 0.0f, 0.0f);
    GlStateManager::translatef(0.0f, -0.2f, 0.0f);
    mBox->render(scale, useDisplayList, false);
    GlStateManager::popMatrix();

    GlStateManager::pushMatrix();
    GlStateManager::rotatef(ageInTicks * -50.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::scalef(0.75f, 0.75f, 0.75f);
    GlStateManager::translatef(0.0f, 0.40000004f, 0.0f);
    mBox->render(scale, useDisplayList, false);
    GlStateManager::popMatrix();

    GlStateManager::pushMatrix();
    GlStateManager::rotatef(ageInTicks * -55.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::scalef(1.5f, 1.5f, 1.5f);
    GlStateManager::translatef(0.0f, 1.0f, 0.0f);
    mBox->render(scale, useDisplayList, false);
    GlStateManager::popMatrix();

    callSkinnedDrawlist();
}
