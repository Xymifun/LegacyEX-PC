#include "game/world/block/EndPortalBlock.hpp"
#include "game/core/Direction.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/TheEndPortalBlockEntity.hpp"
#include "game/core/IconRegister.hpp"

namespace
{
const _eTerrainFeatureType STRONGHOLD_END_PORTAL_FEATURE = static_cast<_eTerrainFeatureType>(eTerrainFeatureType_0 + 6);
}

static const AABB* gEndPortalShape = AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 0.75, 1.0);

EndPortalBlock::EndPortalBlock(Material* material) : BaseEntityBlock(material)
{
    DerivedInit();
    setLightEmission(1.0f);
}

boost::shared_ptr<BlockEntity> EndPortalBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<BlockEntity> entity = boost::shared_ptr<TheEndPortalBlockEntity>(new TheEndPortalBlockEntity());
    entity->setLevel(level);
    return entity;
}

const AABB* EndPortalBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return gEndPortalShape;
}

bool EndPortalBlock::shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                      const Direction* direction)
{
    if (direction == Direction::DOWN)
        return Block::shouldRenderFace(state, levelSource, pos, direction);
    return false;
}

void EndPortalBlock::addCollisionAABBs(const BlockState*, Level*, const BlockPos&, const AABB*, std::vector<AABB*>*,
                                       boost::shared_ptr<Entity>, bool)
{
}

bool EndPortalBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool EndPortalBlock::isCubeShaped(const BlockState*)
{
    return false;
}

int EndPortalBlock::getResourceCount(Random*)
{
    return 0;
}

void EndPortalBlock::entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                                  const boost::shared_ptr<Entity>& entity)
{
    if (entity->GetType() == EntityType::EXPERIENCE_ORB)
        return;

    if (Minecraft::InMiniGame(eMiniGameId_Any, true) && Minecraft::GetInstance()->GetMiniGame()->ArePortalsDisabled())
        return;

    if (entity->isPassenger() || entity->isVehicle() || !entity->canChangeDimensions())
        return;

    AABB* portalBox;
    if (eTYPE_DERIVED_FROM(EntityType::ARROW, entity->GetType()))
    {
        portalBox = AABB::newTemp(pos.x, pos.y, pos.z, pos.x + 1.0, pos.y + 1.0, pos.z + 1.0);
    }
    else
    {
        portalBox = state->getShape(level, pos)->move(pos);
    }

    if (level->mIsClientSide)
        return;

    if (!entity->mBoundingBox->intersects(portalBox->mMinX, portalBox->mMinY, portalBox->mMinZ, portalBox->mMaxX,
                                          portalBox->mMaxY, portalBox->mMaxZ))
        return;

    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
    {
        int x = 0;
        int z = 0;
        if (level->mDimension == NULL && !level->getLevelData()->getHasStrongholdEndPortal()
            && gApp.GetTerrainFeaturePosition(STRONGHOLD_END_PORTAL_FEATURE, &x, &z))
        {
            level->getLevelData()->setXStrongholdEndPortal(x);
            level->getLevelData()->setZStrongholdEndPortal(z);
            level->getLevelData()->setHasStrongholdEndPortal();
        }
    }

    if (!entity->mIgnorePortals)
        entity->changeDimension(1);
}

void EndPortalBlock::animateTick(const BlockState*, Level* level, const BlockPos& pos, Random* random)
{
    double x = (float)pos.x + random->nextFloat();
    double y = (float)pos.y + 0.8f;
    double z = (float)pos.z + random->nextFloat();
    level->addParticle(PARTICLE_END_PORTAL, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
}

not_null_ptr<ItemInstance> EndPortalBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return ItemInstance::EMPTY;
}

const MaterialColor* EndPortalBlock::getMapColor(const BlockState*, LevelSource*, const BlockPos&)
{
    return MaterialColor::COLOR_BLACK;
}

EBlockFaceShape EndPortalBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

bool EndPortalBlock::isWaterBlocking() const
{
    return true;
}

bool EndPortalBlock::waterSpreadCausesSpawn()
{
    return false;
}

bool EndPortalBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

void EndPortalBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"portal");
}

EBlockRenderShape EndPortalBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_INVISIBLE;
}
