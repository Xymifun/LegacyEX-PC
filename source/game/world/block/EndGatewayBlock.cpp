#include "game/world/block/EndGatewayBlock.hpp"
#include "game/world/block/Blocks.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/client/render/BlockEntityRenderDispatcher.hpp"
#include "game/entity/mob/TheEndGatewayEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"

AbstractProperty<int>* EndGatewayBlock::PORTAL_NUMBER = static_cast<AbstractProperty<int>*>(
    IntegerProperty::create(L"fj_portal_number", 0, 15));

EndGatewayBlock::EndGatewayBlock(Material* material) : BaseEntityBlock(material)
{
    DerivedInit();
    setLightEmission(1.0f);
}

boost::shared_ptr<BlockEntity> EndGatewayBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<BlockEntity> entity = boost::shared_ptr<TheEndGatewayEntity>(new TheEndGatewayEntity());
    entity->setLevel(level);
    return entity;
}

bool EndGatewayBlock::shouldRenderFace(const BlockState*, LevelSource* levelSource, const BlockPos& pos,
                                       const Direction* direction)
{
    const BlockState* state = levelSource->getBlockState(pos.relative(direction));
    const Block* block = state->getBlock();
    return !state->isSolidRender() && block != Blocks::end_gateway;
}

const AABB* EndGatewayBlock::getClipAABB(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL_AABB;
}

bool EndGatewayBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool EndGatewayBlock::isCubeShaped(const BlockState*)
{
    return false;
}

int EndGatewayBlock::getResourceCount(Random*)
{
    return 0;
}

void EndGatewayBlock::animateTick(const BlockState*, Level* level, const BlockPos& pos, Random* random)
{
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    bool isGateway = eTYPE_DERIVED_FROM(BlockEntityType::THE_END_GATEWAY_ENTITY, blockEntity->GetType());
    if (!isGateway)
        return;

    int amount = boost::static_pointer_cast<TheEndGatewayEntity>(blockEntity)->getParticleAmount();
    for (int i = 0; i < amount; ++i)
    {
        double x = pos.x + random->nextFloat();
        double y = pos.y + random->nextFloat();
        double z = pos.z + random->nextFloat();
        double velocityX = (random->nextFloat() - 0.5) * 0.5;
        double velocityY = (random->nextFloat() - 0.5) * 0.5;
        double velocityZ = (random->nextFloat() - 0.5) * 0.5;
        int sign = random->nextInt(2) * 2 - 1;
        if (random->nextBoolean())
        {
            z = static_cast<double>(pos.z) + 0.5 + 0.25 * sign;
            velocityZ = random->nextFloat() * 2.0f * sign;
        }
        else
        {
            x = static_cast<double>(pos.x) + 0.5 + 0.25 * sign;
            velocityX = random->nextFloat() * 2.0f * sign;
        }
        level->addParticle(PARTICLE_PORTAL, x, y, z, velocityX, velocityY, velocityZ, arrayWithLength<int>());
    }
}

not_null_ptr<ItemInstance> EndGatewayBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return ItemInstance::EMPTY;
}

const MaterialColor* EndGatewayBlock::getMapColor(const BlockState*, LevelSource*, const BlockPos&)
{
    return MaterialColor::COLOR_BLACK;
}

EBlockFaceShape EndGatewayBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

bool EndGatewayBlock::isWaterBlocking() const
{
    return true;
}

bool EndGatewayBlock::waterSpreadCausesSpawn()
{
    return false;
}

bool EndGatewayBlock::canContainLiquid(const BlockState*) const
{
    return false;
}

BlockStateDefinition* EndGatewayBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {PORTAL_NUMBER};
    return new BlockStateDefinition(this, properties, 1);
}

const BlockState* EndGatewayBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(PORTAL_NUMBER, data);
}

int EndGatewayBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(PORTAL_NUMBER);
}
