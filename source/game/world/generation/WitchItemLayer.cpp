#include "game/world/generation/WitchItemLayer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/ItemInHandRenderer.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/WitchModel.hpp"
#include "game/client/render/WitchRenderer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Witch.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemTransforms.hpp"
#include "game/item/Items.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/Block.hpp"

WitchItemLayer::WitchItemLayer(WitchRenderer* renderer) : mWitchRenderer(renderer)
{
}

WitchItemLayer::~WitchItemLayer()
{
}

void WitchItemLayer::render(boost::shared_ptr<LivingEntity>& entity, float, float, float, float, float, float, float,
                            bool)
{
    boost::shared_ptr<Witch> witch = boost::static_pointer_cast<Witch>(entity);
    not_null_ptr<ItemInstance> itemInstance = witch->getMainHandItem();

    if (itemInstance->isEmpty())
        return;

    GlStateManager::color3f(1.0f, 1.0f, 1.0f);
    GlStateManager::pushMatrix();

    if (static_cast<WitchModel*>(mWitchRenderer->getModel())->mYoung)
    {
        GlStateManager::translatef(0.0f, 0.625f, 0.0f);
        GlStateManager::rotatef(-20.0f, -1.0f, 0.0f, 0.0f);
        float scale = 0.5f;
        GlStateManager::scalef(scale, scale, scale);
    }

    static_cast<WitchModel*>(mWitchRenderer->getModel())->mNose->translateTo(0.0625f);
    GlStateManager::translatef(-0.0625f, 0.53125f, 0.21875f);

    Item* item = itemInstance->getItem();
    Minecraft* minecraft = Minecraft::GetInstance();

    if (Item::getId(item) < 256 && minecraft->getBlockRenderer()->canRenderGeneric(Block::byItem(item)))
    {
        GlStateManager::translatef(0.0f, 0.0625f, -0.25f);
        GlStateManager::rotatef(30.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(-5.0f, 0.0f, 1.0f, 0.0f);
        float scale = 0.375f;
        GlStateManager::scalef(scale, -scale, scale);
        GlStateManager::rotatef(-15.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(40.0f, 0.0f, 0.0f, 1.0f);
    }
    else if (item == Items::bow)
    {
        GlStateManager::translatef(0.0f, 0.125f, -0.125f);
        GlStateManager::rotatef(-45.0f, 0.0f, 1.0f, 0.0f);
        float scale = 0.625f;
        GlStateManager::scalef(scale, -scale, scale);
        GlStateManager::rotatef(-100.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(-20.0f, 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(-15.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(40.0f, 0.0f, 0.0f, 1.0f);
    }
    else if (item->isHandEquipped())
    {
        if (item->isMirroredArt())
        {
            GlStateManager::rotatef(180.0f, 0.0f, 0.0f, 1.0f);
            GlStateManager::translatef(0.0f, -0.0625f, 0.0f);
        }

        mWitchRenderer->translateWeaponItem();
        GlStateManager::translatef(0.0625f, -0.125f, 0.0f);
        float scale = 0.625f;
        GlStateManager::scalef(scale, -scale, scale);
        GlStateManager::rotatef(0.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(0.0f, 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(-15.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(40.0f, 0.0f, 0.0f, 1.0f);
    }
    else
    {
        GlStateManager::translatef(0.1875f, 0.1875f, 0.0f);
        float scale = 0.875f;
        GlStateManager::scalef(scale, scale, scale);
        GlStateManager::rotatef(-20.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::rotatef(-60.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(-30.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::rotatef(-15.0f, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(40.0f, 0.0f, 0.0f, 1.0f);
    }

    Minecraft::GetInstance()->getItemInHandRenderer()->renderItem(
        entity, itemInstance, ItemTransforms::THIRD_PERSON_RIGHT_HAND, false, 0, true);
    GlStateManager::popMatrix();
}

bool WitchItemLayer::colorsOnDamage()
{
    return false;
}
