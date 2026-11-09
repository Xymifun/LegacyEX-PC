#ifndef GAME_CLIENT_RENDER_ENTITYBLOCKRENDERER_HPP
#define GAME_CLIENT_RENDER_ENTITYBLOCKRENDERER_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"

class BannerBlockEntity;
class BedBlockEntity;
class ChestBlockEntity;
class ConduitBlockEntity;
class EnderChestBlockEntity;
class ItemInstance;
class ShulkerBoxBlockEntity;
class SkullBlockEntity;

class EntityBlockRenderer
{
public:
    EntityBlockRenderer();

    void renderByItem(not_null_ptr<ItemInstance> item, float partialTicks, bool bTranslucent, bool bUseDisplayList);

    static EntityBlockRenderer* instance;

private:
    boost::shared_ptr<ChestBlockEntity> mChestBlockEntity;
    boost::shared_ptr<ChestBlockEntity> mTrappedChestBlockEntity;
    boost::shared_ptr<EnderChestBlockEntity> mEnderChestBlockEntity;
    boost::shared_ptr<BannerBlockEntity> mBannerBlockEntity;
    boost::shared_ptr<ShulkerBoxBlockEntity> mShulkerBoxBlockEntity;
    boost::shared_ptr<SkullBlockEntity> mSkullBlockEntity;
    boost::shared_ptr<BedBlockEntity> mBedBlockEntity;
    boost::shared_ptr<ConduitBlockEntity> mConduitBlockEntity;
};

#endif
