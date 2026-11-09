#include "game/world/block/EnderChestBlockEntity.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"

EnderChestBlockEntity::EnderChestBlockEntity() : BlockEntity()
{
    mOpen = 0.0F;
    mOpenOld = 0.0F;
    mOpenCount = 0;
    mTickCount = 0;
    mName = boost::shared_ptr<CensoredString>(new CensoredString());
}

BlockEntity* EnderChestBlockEntity::create()
{
    return new EnderChestBlockEntity();
}

void EnderChestBlockEntity::setCustomName(const std::wstring& name, const std::wstring& uuid, bool chatCensored,
                                          bool uuidLocal)
{
    mName->set(name, uuid, chatCensored, uuidLocal);
}

void EnderChestBlockEntity::tick()
{
    if (++mTickCount % 20 * 4 == 0)
        mLevel->blockEvent(mPos, Blocks::ender_chest, 1, mOpenCount);

    mOpenOld = mOpen;
    const int x = mPos.x;
    const int y = mPos.y;
    const int z = mPos.z;
    if (mOpenCount > 0 && mOpen == 0.0f)
    {
        BlockPos soundPos(x + 0.5, y + 0.5, z + 0.5);
        mLevel->playSound(boost::shared_ptr<Player>(), soundPos, SoundEvents::ENDERCHEST_OPEN,
                          SoundSource::eSoundSource_Blocks, 0.5f, mLevel->mRandom->nextFloat() * 0.1f + 0.9f, 16.0f);
    }
    if ((mOpenCount == 0 && mOpen > 0.0f) || (mOpenCount > 0 && mOpen < 1.0f))
    {
        const float oldOpen = mOpen;
        if (mOpenCount > 0)
            mOpen += 0.1f;
        else
            mOpen -= 0.1f;
        if (mOpen > 1.0f)
            mOpen = 1.0f;
        if (mOpen < 0.5f && oldOpen >= 0.5f)
        {
            mLevel->playSound(boost::shared_ptr<Player>(), x + 0.5, y + 0.5, z + 0.5, SoundEvents::ENDERCHEST_CLOSE,
                              SoundSource::eSoundSource_Blocks, 0.5f, mLevel->mRandom->nextFloat() * 0.1f + 0.9f,
                              16.0f);
        }
        if (mOpen < 0.0f)
            mOpen = 0.0f;
    }
}

bool EnderChestBlockEntity::triggerEvent(int id, int param)
{
    if (id == 1)
    {
        mOpenCount = param;
        return true;
    }
    return BlockEntity::triggerEvent(id, param);
}

void EnderChestBlockEntity::setRemoved()
{
    BlockEntity::setRemoved();
}

void EnderChestBlockEntity::startOpen()
{
    ++mOpenCount;
    mLevel->blockEvent(mPos, Blocks::ender_chest, 1, mOpenCount);
}

void EnderChestBlockEntity::stopOpen()
{
    --mOpenCount;
    mLevel->blockEvent(mPos, Blocks::ender_chest, 1, mOpenCount);
}

bool EnderChestBlockEntity::stillValid(boost::shared_ptr<Player> player)
{
    if (mLevel->getBlockEntity(mPos) != shared_from_this())
    {
        return false;
    }
    if (player->distanceAsIfHitscanned(mPos, *player->mBoundingBox) > SharedConstants::CONTAINER_INTERACT_DISTANCE_SQR)
    {
        return false;
    }
    return true;
}

InteractionObject::NameData EnderChestBlockEntity::GetNetworkNameData()
{
    return InteractionObject::NameData(mName->raw(), mName->getUUID(), mName->isUUIDLocal(), mName->isChatCensored());
}

boost::shared_ptr<BlockEntity> EnderChestBlockEntity::clone()
{
    boost::shared_ptr<EnderChestBlockEntity> result(new EnderChestBlockEntity());
    BlockEntity::clone(result);
    return result;
}

unsigned long long EnderChestBlockEntity::GetType()
{
    return BlockEntityType::ENDER_CHEST;
}
