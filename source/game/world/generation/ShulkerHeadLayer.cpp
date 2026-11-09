#include "game/world/generation/ShulkerHeadLayer.hpp"
#include "game/core/Mth.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ModelPart.hpp"
#include "game/client/render/ShulkerModel.hpp"
#include "game/client/render/ShulkerRenderer.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Shulker.hpp"
#include "game/util/ResourceLocation.hpp"

ResourceLocation ShulkerHeadLayer::SHULKER_LOCATION(TN_MOB_SHULKER_ENDERGOLEM);

ShulkerHeadLayer::ShulkerHeadLayer(ShulkerRenderer* renderer) : mRenderer(renderer)
{
}

ShulkerHeadLayer::~ShulkerHeadLayer()
{
}

void ShulkerHeadLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                              float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                              bool bUseDisplayList)
{
    boost::shared_ptr<Shulker> shulker = boost::static_pointer_cast<Shulker>(entity);
    GlStateManager::pushMatrix();
    const Direction* direction = shulker->getAttachFace();
    int facing = direction->get3DDataValue();
    switch (facing)
    {
    default:
        break;
    case 5:
        GlStateManager::rotatef(90.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::rotatef(90.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::translatef(1.0f, -1.0f, 0.0f);
        GlStateManager::rotatef(180.0f, 0.0f, 1.0f, 0.0f);
        break;
    case 4:
        GlStateManager::rotatef(-90.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::rotatef(90.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::translatef(-1.0f, -1.0f, 0.0f);
        GlStateManager::rotatef(180.0f, 0.0f, 1.0f, 0.0f);
        break;
    case 2:
        GlStateManager::rotatef(90.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::translatef(0.0f, -1.0f, -1.0f);
        break;
    case 3:
        GlStateManager::rotatef(180.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::rotatef(90.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::translatef(0.0f, -1.0f, 1.0f);
        break;
    case 1:
        GlStateManager::rotatef(180.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::translatef(0.0f, -2.0f, 0.0f);
        break;
    }

    ShulkerModel* model = static_cast<ShulkerModel*>(mRenderer->getModel());
    ModelPart* head = model->mHead;
    head->mYRot = netHeadYaw * Mth::DEG_TO_RAD;
    head->mXRot = headPitch * Mth::DEG_TO_RAD;
    mRenderer->bindTexture(&SHULKER_LOCATION);
    head->render(scale, bUseDisplayList, false);

    GlStateManager::popMatrix();
}

bool ShulkerHeadLayer::colorsOnDamage()
{
    return false;
}
