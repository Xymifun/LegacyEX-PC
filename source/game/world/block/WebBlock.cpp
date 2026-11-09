#include "game/world/block/WebBlock.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Material.hpp"

WebBlock::WebBlock() : Block(Material::WEB)
{
    DerivedInit();
}

void WebBlock::entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                            const boost::shared_ptr<Entity>& entity)
{
    entity->makeStuckInWeb();

    if (entity->GetType() == EntityType::FIREWORKS_ROCKET_ENTITY
        && Minecraft::GetInstance()->GetMiniGame()->HasFeatureDirectionalFireworks())
    {
        entity->mCollision = true;
    }
}

bool WebBlock::isSolidRender(const BlockState* state) const
{
    return false;
}

const AABB* WebBlock::getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    return NULL_AABB;
}

bool WebBlock::isCubeShaped(const BlockState* state)
{
    return false;
}

Item* WebBlock::getResource(const BlockState* state, Random* random, int lootBonus)
{
    return Items::string;
}

bool WebBlock::isSilkTouchable()
{
    return true;
}

void WebBlock::playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                             const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                             not_null_ptr<ItemInstance> tool)
{
    if (!level->mIsClientSide && tool->getItem() == Items::shears)
    {
        popResource(level, pos, not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(this), 1)));
        return;
    }

    Block::playerDestroy(level, player, pos, state, blockEntity, tool);
}

EBlockFaceShape WebBlock::getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                            const Direction* direction)
{
    return BLOCK_FACE_UNDEFINED;
}

int WebBlock::getRenderLayer()
{
    return 0;
}

EBlockRenderShape WebBlock::getRenderShape(const BlockState* state)
{
    return BLOCK_RENDER_CROSS;
}

bool WebBlock::waterSpreadCausesSpawn()
{
    return true;
}
