#include "game/client/render/EntityBlockRenderer.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/BlockEntityRenderDispatcher.hpp"
#include "game/client/render/SkullBlockRenderer.hpp"
#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/BannerBlockEntity.hpp"
#include "game/world/block/BedBlockEntity.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#include "game/world/block/ConduitBlockEntity.hpp"
#include "game/world/block/EnderChestBlockEntity.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"
#include "game/world/block/ShulkerBoxBlockEntity.hpp"
#include "game/world/block/SkullBlockEntity.hpp"

EntityBlockRenderer* EntityBlockRenderer::instance = NULL;

EntityBlockRenderer::EntityBlockRenderer()
{
    mChestBlockEntity = boost::shared_ptr<ChestBlockEntity>(new ChestBlockEntity());
    mTrappedChestBlockEntity = boost::shared_ptr<ChestBlockEntity>(new ChestBlockEntity(ChestBlock::TYPE_LEFT, false));
    mEnderChestBlockEntity = boost::shared_ptr<EnderChestBlockEntity>(new EnderChestBlockEntity());
    mBannerBlockEntity = boost::shared_ptr<BannerBlockEntity>(new BannerBlockEntity());
    mSkullBlockEntity = boost::shared_ptr<SkullBlockEntity>(new SkullBlockEntity());
    mShulkerBoxBlockEntity = boost::shared_ptr<ShulkerBoxBlockEntity>(new ShulkerBoxBlockEntity());
    mBedBlockEntity = boost::shared_ptr<BedBlockEntity>(new BedBlockEntity());
    mConduitBlockEntity = boost::shared_ptr<ConduitBlockEntity>(new ConduitBlockEntity());
}

void EntityBlockRenderer::renderByItem(not_null_ptr<ItemInstance> item, float partialTicks, bool bTranslucent,
                                       bool bUseDisplayList)
{
    Item* itemType = item->getItem();

    if (item->getItem() == Items::banner)
    {
        mBannerBlockEntity->setItemValues(item, false);
        BlockEntityRenderDispatcher::instance->render(mBannerBlockEntity, 0.0, 0.0, 0.0, 0.0F, true, partialTicks,
                                                      true);
    }
    else if (itemType == Items::bed)
    {
        mBedBlockEntity->setItemValues(item);
        BlockEntityRenderDispatcher::instance->render(mBedBlockEntity, 0.0, 0.0, 0.0, 0.0F, true, partialTicks, true);
    }
    else if (itemType == Items::skull)
    {
        if (SkullBlockRenderer::instance != NULL)
        {
            GlStateManager::pushMatrix();
            GlStateManager::disableCull();
            SkullBlockRenderer::instance->renderSkull(0.0F, 0.0F, 0.0F, Direction::UP, 180.0F, 0.0F,
                                                      item->getAuxValue(), -1, 0.0F, bUseDisplayList);
            GlStateManager::enableCull();
            GlStateManager::popMatrix();
        }
    }
    else if (itemType == Item::byBlock(Blocks::ender_chest))
    {
        BlockEntityRenderDispatcher::instance->render(mEnderChestBlockEntity, 0.0, 0.0, 0.0, 0.0F, bTranslucent,
                                                      partialTicks, bUseDisplayList);
    }
    else if (itemType == Item::byBlock(Blocks::trapped_chest))
    {
        BlockEntityRenderDispatcher::instance->render(mTrappedChestBlockEntity, 0.0, 0.0, 0.0, 0.0F, bTranslucent,
                                                      partialTicks, bUseDisplayList);
    }
    else if (itemType == Item::byBlock(Blocks::conduit))
    {
        BlockEntityRenderDispatcher::instance->render(mConduitBlockEntity, 0.0, 0.0, 0.0, 0.0F, bTranslucent,
                                                      partialTicks, bUseDisplayList);
    }
    else
    {
        if (dynamic_cast<ShulkerBoxBlock*>(Block::byItem(itemType)) != NULL)
        {
            *mShulkerBoxBlockEntity = ShulkerBoxBlockEntity(
                static_cast<ShulkerBoxBlock*>(Block::byItem(itemType))->getDyeColor());
            BlockEntityRenderDispatcher::instance->render(mShulkerBoxBlockEntity, 0.0, 0.0, 0.0, 0.0F, bTranslucent,
                                                          partialTicks, bUseDisplayList);
        }
        else
        {
            BlockEntityRenderDispatcher::instance->render(mChestBlockEntity, 0.0, 0.0, 0.0, 0.0F, bTranslucent,
                                                          partialTicks, bUseDisplayList);
        }
    }
}
