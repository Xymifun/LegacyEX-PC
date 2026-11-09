#include "game/world/block/SignBlockEntity.hpp"
#include "game/core/EntityType.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <cwchar>

#include "game/data/CompoundTag.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/CriticalSection.hpp"

class BlockState;
class ServerLevel;

namespace
{
OSFastMutex gSignUniqueIdMutex;
bool gSignUniqueIdMutexInitialized = true;
int gSignUniqueId = 0;
}

struct _STRING_VERIFY_RESPONSE
{
    unsigned short count;
    unsigned short reserved;
    int* results;
};

SignBlockEntity::SignBlockEntity()
{
    mMessages[0] = L"";
    mMessages[1] = L"";
    mMessages[2] = L"";
    mMessages[3] = L"";
    mEditable = true;
    mVerified = true;
    mCensored = false;
    mCreatedByRestrictedPlayer = false;
    mCreatedByHostPlayer = true;
    mSelectedLine = -1;
    mCreatedPlayer = L"";
    mTextCacheIndex = 0;
    mTextCacheEnabled = true;
    setUniqueId();
}

SignBlockEntity::~SignBlockEntity()
{
}

BlockEntity* SignBlockEntity::create()
{
    return new SignBlockEntity();
}

void SignBlockEntity::setUniqueId()
{
    if (gSignUniqueIdMutexInitialized)
    {
        InitializeCriticalSection(&gSignUniqueIdMutex);
        gSignUniqueIdMutexInitialized = false;
    }
    EnterCriticalSection(&gSignUniqueIdMutex);
    mUniqueId = gSignUniqueId++;
    if (gSignUniqueId == -1)
    {
        gSignUniqueId = 0;
    }
    LeaveCriticalSection(&gSignUniqueIdMutex);
}

void SignBlockEntity::SetMessage(int line, std::wstring& message)
{
    mMessages[line] = message;
    setUniqueId();
}

bool SignBlockEntity::IsVerified()
{
    return mVerified;
}

void SignBlockEntity::SetVerified(bool verified)
{
    mVerified = verified;
}

bool SignBlockEntity::IsCensored()
{
    return mCensored;
}

void SignBlockEntity::SetCensored(bool censored)
{
    mCensored = censored;
}

void SignBlockEntity::setCreatedByRestrictedPlayer(bool restricted)
{
    mCreatedByRestrictedPlayer = restricted;
}

bool SignBlockEntity::wasCreatedByRestrictedPlayer() const
{
    return mCreatedByRestrictedPlayer;
}

bool SignBlockEntity::wasCreatedByHostPlayer()
{
    return mCreatedByHostPlayer;
}

void SignBlockEntity::setCreatedByHostPlayer(bool host)
{
    mCreatedByHostPlayer = host;
}

void SignBlockEntity::tick()
{
    BlockEntity::tick();
}

CompoundTag* SignBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putString(L"Text1", mMessages[0]);
    tag->putString(L"Text2", mMessages[1]);
    tag->putString(L"Text3", mMessages[2]);
    tag->putString(L"Text4", mMessages[3]);
    tag->putBoolean(L"Verified", mVerified);
    tag->putBoolean(L"Censored", mCensored);
    tag->putBoolean(L"CreatedByRestrictedPlayer", mCreatedByRestrictedPlayer);
    tag->putInt(L"CreatedByHost", mCreatedByHostPlayer);
    tag->putString(L"uuid", mCreatedPlayer);
    return tag;
}

void SignBlockEntity::setLevelForLoading(Level* level)
{
    setLevel(level);
}

void SignBlockEntity::load(CompoundTag* tag)
{
    mEditable = false;
    BlockEntity::load(tag);
    for (int line = 0; line < 4; ++line)
    {
        wchar_t key[256];
        swprintf(key, 256, L"Text%d", line + 1);
        mMessages[line] = tag->getString(key);
        if (mMessages[line].size() > 15)
        {
            mMessages[line] = mMessages[line].substr(0, 15);
        }
    }
    if (tag->contains(L"Verified"))
    {
        mVerified = tag->getBoolean(L"Verified");
    }
    if (tag->contains(L"Censored"))
    {
        mCensored = tag->getBoolean(L"Censored");
    }
    if (tag->contains(L"CreatedByRestrictedPlayer"))
    {
        mCreatedByRestrictedPlayer = tag->getBoolean(L"CreatedByRestrictedPlayer");
    }
    if (tag->contains(L"CreatedByHost"))
    {
        mCreatedByHostPlayer = tag->getBoolean(L"CreatedByHost");
    }
    if (tag->contains(L"uuid"))
    {
        mCreatedPlayer = tag->getString(L"uuid");
    }
    setChanged();
}

boost::shared_ptr<Packet> SignBlockEntity::getUpdatePacket()
{
    return boost::shared_ptr<ClientboundBlockEntityDataPacket>(
        new ClientboundBlockEntityDataPacket(mPos, 9, mLevel->mDimension->getType()->getId(), getUpdateTag()));
}

CompoundTag* SignBlockEntity::getUpdateTag()
{
    return save(new CompoundTag());
}

bool SignBlockEntity::onlyOpCanSetNbt()
{
    return true;
}

void SignBlockEntity::setCreatedPlayer(const std::wstring& player)
{
    mCreatedPlayer = player;
}

std::wstring SignBlockEntity::getCreatedPlayerUUID()
{
    return mCreatedPlayer;
}

unsigned int SignBlockEntity::StringVerifyCallback(void* context, _STRING_VERIFY_RESPONSE* response)
{
    boost::shared_ptr<SignBlockEntity> sign = static_cast<boost::weak_ptr<SignBlockEntity>*>(context)->lock();
    if (sign)
    {
        sign->mVerified = true;
        sign->mCensored = false;
        for (int i = 0; i < response->count; ++i)
        {
            if (response->results[i] != 0)
            {
                sign->mCensored = true;
            }
        }
        if (sign->mLevel != NULL && !sign->mLevel->mIsClientSide)
        {
            Level* level = sign->mLevel;
            if (level->hasChunkAt(sign->getBlockPos(), false))
            {
                BlockState* state = const_cast<BlockState*>(level->getBlockState(sign->getBlockPos()));
                static_cast<ServerLevel*>(level)->queueSendBlockUpdate(sign->getBlockPos().x, sign->getBlockPos().y,
                                                                       sign->getBlockPos().z, state, state);
            }
        }
        sign->setUniqueId();
    }
    delete static_cast<boost::weak_ptr<SignBlockEntity>*>(context);
    return 0;
}

void SignBlockEntity::setChanged()
{
    mVerified = true;
    setUniqueId();
}

boost::shared_ptr<BlockEntity> SignBlockEntity::clone()
{
    boost::shared_ptr<SignBlockEntity> result = boost::shared_ptr<SignBlockEntity>(new SignBlockEntity());
    BlockEntity::clone(result);
    result->mMessages[0] = mMessages[0];
    result->mMessages[1] = mMessages[1];
    result->mMessages[2] = mMessages[2];
    result->mMessages[3] = mMessages[3];
    result->mVerified = mVerified;
    result->mCensored = mCensored;
    return result;
}

bool SignBlockEntity::executeClickCommands(boost::shared_ptr<Player> player)
{
    return true;
}

int SignBlockEntity::getUniqueId()
{
    return mUniqueId;
}

void SignBlockEntity::setTextCacheIndex(int index)
{
    mTextCacheIndex = index;
}

int SignBlockEntity::getTextCacheIndex()
{
    return mTextCacheIndex;
}

void SignBlockEntity::disableTextCache()
{
    mTextCacheEnabled = false;
}

bool SignBlockEntity::getTextCacheEnabled()
{
    return mTextCacheEnabled;
}

unsigned long long SignBlockEntity::GetType()
{
    return BlockEntityType::SIGN;
}
