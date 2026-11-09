#include "game/world/generation/SnowManHeadLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/ItemInHandRenderer.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/SnowManRenderer.hpp"
#include "game/client/render/SnowManModel.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/SnowMan.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/Blocks.hpp"

SnowManHeadLayer::SnowManHeadLayer(SnowManRenderer* renderer) : mRenderer(renderer)
{
}

SnowManHeadLayer::~SnowManHeadLayer()
{
}

void SnowManHeadLayer::render(boost::shared_ptr<LivingEntity>& entity, float limbSwing, float limbSwingAmount,
                              float partialTicks, float ageInTicks, float netHeadYaw, float headPitch, float scale,
                              bool bUseDisplayList)
{
    boost::shared_ptr<SnowMan> snowMan = boost::dynamic_pointer_cast<SnowMan>(entity);

    bool invisible = snowMan->isInvisible();
    if (invisible || !snowMan->hasPumpkin())
    {
        return;
    }

    GlStateManager::pushMatrix();
    static_cast<SnowManModel*>(mRenderer->getModel())->mHead->translateTo(0.0625f);
    float headScale = 0.625f;
    GlStateManager::translatef(0.0f, -0.34375f, 0.0f);
    GlStateManager::rotatef(180.0f, 0.0f, 1.0f, 0.0f);
    GlStateManager::scalef(headScale, -headScale, -headScale);

    Minecraft::GetInstance()->getItemInHandRenderer()->renderItem(
        snowMan, not_null_ptr<ItemInstance>(new ItemInstance(Blocks::carved_pumpkin, 1)), ItemTransforms::NONE, false,
        0, true);

    GlStateManager::popMatrix();
}

bool SnowManHeadLayer::colorsOnDamage()
{
    return true;
}
