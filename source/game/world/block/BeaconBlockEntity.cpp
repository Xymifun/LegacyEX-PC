#include "game/world/block/BeaconBlockEntity.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cstring>
#include <typeinfo>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/network/ClientboundBlockEntityDataPacket.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/util/AABB.hpp"
#include "game/ui/BeaconMenu.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/StainedGlassBlock.hpp"
#include "game/world/block/StainedGlassPaneBlock.hpp"

MobEffect* BeaconBlockEntity::sBeaconEffects[4][3];

namespace
{
MobEffect* sBeaconEffectOrder[6] = {0};
int sBeaconEffectCount = 0;
}

BeaconBlockEntity::BeaconBeamSection::BeaconBeamSection(arrayWithLength<float> color)
    : mRed(color.data[0]),
      mGreen(color.data[1]),
      mBlue(color.data[2]),
      mHeight(1)
{
}

float* BeaconBlockEntity::BeaconBeamSection::getColor()
{
    return &mRed;
}

void BeaconBlockEntity::BeaconBeamSection::setHeight(int height)
{
    mHeight = height;
}

void BeaconBlockEntity::BeaconBeamSection::increaseHeight()
{
    ++mHeight;
}

unsigned int BeaconBlockEntity::BeaconBeamSection::getHeight()
{
    return mHeight;
}

BeaconBlockEntity::BeaconBlockEntity()
{
    mNextClientSideUpdate = 0;
    mClientSideScale = 0.0F;
    mLevels = -1;
    mActive = false;
    mPrimaryPower = NULL;
    mSecondaryPower = NULL;
    mItem = ItemInstance::EMPTY;
    mName = boost::shared_ptr<CensoredString>(new CensoredString());
    mBeamSections = std::vector<BeaconBeamSection>();
    mBeamDirection = 0;
    mBeamLength = 0;
    mLastBeamBlockCount = -1;
    for (int i = 0; i < 16; ++i)
    {
        mBeamColorCounts[i] = 0;
    }
    mBeamRebuildTicks = 15;
    mTickCount = 0;
}

BeaconBlockEntity::~BeaconBlockEntity()
{
}

std::wstring BeaconBlockEntity::getName()
{
    if (hasCustomName())
    {
        return mName->get();
    }
    return std::wstring(L"");
}

std::wstring BeaconBlockEntity::getCustomName()
{
    if (hasCustomName())
    {
        return mName->get();
    }
    return std::wstring(L"");
}

bool BeaconBlockEntity::hasCustomName()
{
    if (!mName->checkParentalFilter())
    {
        return false;
    }
    return mName->raw().length() != 0;
}

unsigned int BeaconBlockEntity::getContainerSize()
{
    return 1;
}

bool BeaconBlockEntity::isEmpty()
{
    return mItem->isEmpty();
}

not_null_ptr<ItemInstance> BeaconBlockEntity::getItem(unsigned int slot)
{
    if (slot == 0)
    {
        return mItem;
    }
    return ItemInstance::EMPTY;
}

not_null_ptr<ItemInstance> BeaconBlockEntity::removeItem(unsigned int slot, int count)
{
    if (slot == 0 && !mItem->isEmpty())
    {
        if (count >= mItem->getCount())
        {
            not_null_ptr<ItemInstance> result = mItem;
            mItem = ItemInstance::EMPTY;
            return result;
        }
        mItem->split(count);
    }
    return ItemInstance::EMPTY;
}

not_null_ptr<ItemInstance> BeaconBlockEntity::removeItemNoUpdate(int slot)
{
    if (slot == 0)
    {
        not_null_ptr<ItemInstance> result = mItem;
        mItem = ItemInstance::EMPTY;
        return result;
    }
    return ItemInstance::EMPTY;
}

void BeaconBlockEntity::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    if (slot == 0)
    {
        mItem = item;
    }
}

int BeaconBlockEntity::getMaxStackSize()
{
    return 1;
}

void BeaconBlockEntity::setChanged()
{
    BlockEntity::setChanged();
}

bool BeaconBlockEntity::stillValid(boost::shared_ptr<Player> player)
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

void BeaconBlockEntity::startOpen(boost::shared_ptr<Player> player)
{
}

void BeaconBlockEntity::stopOpen(boost::shared_ptr<Player> player)
{
}

bool BeaconBlockEntity::canPlaceItem(int, not_null_ptr<ItemInstance> item)
{
    return item->getItem() == Items::emerald || item->getItem() == Items::diamond || item->getItem() == Items::goldIngot
           || item->getItem() == Items::ironIngot;
}

int BeaconBlockEntity::getContainerData(int id)
{
    switch (id)
    {
    case 0:
        return mLevels;
    case 1:
        return MobEffect::getId(mPrimaryPower);
    case 2:
        return MobEffect::getId(mSecondaryPower);
    default:
        return 0;
    }
}

MobEffect* BeaconBlockEntity::getValidEffectById(int id)
{
    MobEffect* effect = MobEffect::byId(id);
    for (int i = 0; i < sBeaconEffectCount; ++i)
    {
        if (sBeaconEffectOrder[i] == effect)
            return effect;
    }
    return NULL;
}

void BeaconBlockEntity::setContainerData(int id, int value)
{
    switch (id)
    {
    case 0:
        mLevels = value;
        break;
    case 1:
        mPrimaryPower = getValidEffectById(value);
        mLevel->playSound(boost::shared_ptr<Player>(), getBlockPos(), SoundEvents::BEACON_POWER_SELECT,
                          SoundSource::eSoundSource_Blocks, 1.0F, 1.0F, 1.0F);
        break;
    case 2:
        mSecondaryPower = getValidEffectById(value);
        break;
    }
}

int BeaconBlockEntity::getContainerDataCount()
{
    return 3;
}

void BeaconBlockEntity::clearContent()
{
    mItem = ItemInstance::EMPTY;
}

AbstractContainerMenu* BeaconBlockEntity::createMenu(boost::shared_ptr<Inventory> inventory,
                                                     boost::shared_ptr<Player> player)
{
    return new BeaconMenu(inventory, boost::dynamic_pointer_cast<BeaconBlockEntity>(shared_from_this()));
}

int BeaconBlockEntity::getTypeId(bool large)
{
    return ClientboundContainerOpenPacket::BEACON;
}

InteractionObject::NameData BeaconBlockEntity::GetNetworkNameData()
{
    return InteractionObject::NameData(mName->raw(), mName->getUUID(), mName->isUUIDLocal(), mName->isChatCensored());
}

arrayWithLength<int> BeaconBlockEntity::getSlotsForFace(const Direction* face)
{
    return arrayWithLength<int>();
}

bool BeaconBlockEntity::canPlaceItemThroughFace(int slot, not_null_ptr<ItemInstance> item, const Direction* face)
{
    return false;
}

bool BeaconBlockEntity::canTakeItemThroughFace(int slot, not_null_ptr<ItemInstance> item, const Direction* face)
{
    return false;
}

void BeaconBlockEntity::setCustomName(const std::wstring& name, const std::wstring& uuid, bool chatCensored,
                                      bool uuidLocal)
{
    mName->set(name, uuid, chatCensored, uuidLocal);
}

int BeaconBlockEntity::getBeamDirection()
{
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureCustomBeacons())
        return mBeamDirection;
    return eBEAM_DIRECTION_POSITIVE_Y;
}

void BeaconBlockEntity::updateShapeForGlide()
{
    mLevels = 0;
    mBeamSections.clear();
    mActive = true;
    if (getBeamDirection() == eBEAM_DIRECTION_NONE)
    {
        mActive = false;
        return;
    }
    float color[3] = {1.0F, 1.0F, 1.0F};
    arrayWithLength<float> colors(color, 3);
    mBeamSections.push_back(BeaconBeamSection(colors));
    BeaconBeamSection& section = mBeamSections[mBeamSections.size() - 1];
    if (getBeamDirection() == eBEAM_DIRECTION_NEGATIVE_Y)
    {
        section.setHeight(-mBeamLength);
    }
    else
    {
        section.setHeight(mBeamLength);
    }
}

bool BeaconBlockEntity::NeedsRebuild()
{
    short blockCount = 0;
    short colorCounts[16] = {0};
    int x = mPos.x;
    int z = mPos.z;
    for (int y = mPos.y + 1; y < mLevel->getHeight(); ++y)
    {
        BlockPos pos(x, y, z);
        const BlockState* state = mLevel->getBlockState(pos);
        if (state->getBlock() != Blocks::air && state->getBlock() != Blocks::water)
        {
            ++blockCount;
            if (state->getBlock() == Blocks::stained_glass)
            {
                const DyeColor* color = state->getValue(StainedGlassBlock::COLOR);
                int index = color->getBlockData();
                ++colorCounts[index];
            }
            else if (state->getBlock() == Blocks::stained_glass_pane)
            {
                const DyeColor* color = state->getValue(StainedGlassPaneBlock::COLOR);
                int index = color->getBlockData();
                ++colorCounts[index];
            }
        }
    }

    bool unchanged = true;
    for (int i = 0; i < 16; ++i)
    {
        if (colorCounts[i] != mBeamColorCounts[i])
        {
            unchanged = false;
            mBeamColorCounts[i] = colorCounts[i];
        }
    }

    if (blockCount == mLastBeamBlockCount)
    {
        return !unchanged;
    }
    mLastBeamBlockCount = blockCount;
    return true;
}

void BeaconBlockEntity::updateShape()
{
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureCustomBeacons())
    {
        updateShapeForGlide();
        return;
    }

    int oldLevels = mLevels;
    int x = mPos.x;
    int y = mPos.y;
    int z = mPos.z;

    mLevels = 0;
    for (int layer = 1; layer <= 4; mLevels = layer++)
    {
        int layerY = y - layer;
        if (layerY < 0)
            break;
        bool complete = true;
        for (int layerX = x - layer; layerX <= x + layer && complete; ++layerX)
        {
            for (int layerZ = z - layer; layerZ <= z + layer; ++layerZ)
            {
                BlockPos pos(layerX, layerY, layerZ);
                Block* block = mLevel->getBlockState(pos)->getBlock();
                if (block != Blocks::emerald_block && block != Blocks::gold_block && block != Blocks::diamond_block
                    && block != Blocks::iron_block)
                {
                    complete = false;
                    break;
                }
            }
        }
        if (!complete)
            break;
    }
    mActive = mLevels != 0;

    if (mActive)
    {
        if (NeedsRebuild() || --mBeamRebuildTicks == 0)
        {
            mBeamSections.clear();
            mBeamRebuildTicks = 15;

            float color[3] = {1.0F, 1.0F, 1.0F};
            arrayWithLength<float> colors(color, 3);
            arrayWithLength<const float> white = DyeColor::WHITE->getTextureDiffuseColors();
            memcpy(color, white.data, sizeof(color));
            mBeamSections.push_back(BeaconBeamSection(colors));
            BeaconBeamSection* section = &mBeamSections[mBeamSections.size() - 1];

            bool first = true;
            for (int beamY = y + 1; beamY < mLevel->getHeight(); ++beamY)
            {
                BlockPos beamPos(x, beamY, z);
                const BlockState* state = mLevel->getBlockState(beamPos);
                int lightBlock = state->getLightBlock();
                int extraLightBlock = mLevel->getExtraBlockState(beamPos)->getLightBlock();
                if (lightBlock < extraLightBlock)
                    lightBlock = extraLightBlock;

                if (state->getBlock() == Blocks::stained_glass)
                {
                    arrayWithLength<const float> glassColor =
                        state->getValue(StainedGlassBlock::COLOR)->getTextureDiffuseColors();
                    memcpy(color, glassColor.data, sizeof(color));
                }
                else if (state->getBlock() == Blocks::stained_glass_pane)
                {
                    arrayWithLength<const float> paneColor =
                        state->getValue(StainedGlassPaneBlock::COLOR)->getTextureDiffuseColors();
                    memcpy(color, paneColor.data, sizeof(color));
                }
                else
                {
                    if (lightBlock < 15 || state->getBlock() == Blocks::bedrock)
                    {
                        section->increaseHeight();
                        continue;
                    }
                    mActive = false;
                    mBeamSections.clear();
                    break;
                }

                if (!first)
                {
                    color[0] = (section->getColor()[0] + color[0]) * 0.5F;
                    color[1] = (section->getColor()[1] + color[1]) * 0.5F;
                    color[2] = (section->getColor()[2] + color[2]) * 0.5F;
                }
                float* sectionColor = section->getColor();
                if (color[0] == sectionColor[0] && color[1] == sectionColor[1] && color[2] == sectionColor[2])
                {
                    section->increaseHeight();
                }
                else
                {
                    mBeamSections.push_back(BeaconBeamSection(colors));
                    section = &mBeamSections[mBeamSections.size() - 1];
                }
                first = false;
            }
        }
    }

    if (!mActive)
    {
        mBeamSections.clear();
        mLastBeamBlockCount = -1;
        for (int i = 0; i < 16; ++i)
            mBeamColorCounts[i] = 0;
        mBeamRebuildTicks = 15;
    }

    if (!mLevel->mIsClientSide && mLevels == 4 && oldLevels < 4)
    {
        AABB* area = AABB::newTemp(x, y, z, x, y - 4, z)->grow(10.0, 5.0, 10.0);
        std::vector<boost::shared_ptr<Entity> >* players = mLevel->getEntitiesOfClass(typeid(Player), area);
        for (std::vector<boost::shared_ptr<Entity> >::iterator it = players->begin(); it != players->end(); ++it)
        {
            boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(*it);
            player->awardStat(GenericStats::fullBeacon(), GenericStats::param_fullBeacon());
        }
        delete players;
    }
}

void BeaconBlockEntity::applyEffects()
{
    if (mActive && mLevels > 0 && !mLevel->mIsClientSide)
    {
        if (mPrimaryPower != NULL)
        {
            double range = mLevels * 10 + 10;
            int amplifier = 0;
            if (mLevels >= 4 && mPrimaryPower == mSecondaryPower)
                amplifier = 1;
            int duration = (9 + mLevels * 2) * 20;
            int x = mPos.x;
            int y = mPos.y;
            int z = mPos.z;
            AABB* area = AABB::newTemp(x, y, z, x + 1, y + 1, z + 1)
                             ->grow(range, range, range)
                             ->expand(0.0, mLevel->getMaxBuildHeight(), 0.0);
            std::vector<boost::shared_ptr<Entity> >* players = mLevel->getEntitiesOfClass(typeid(Player), area);
            for (std::vector<boost::shared_ptr<Entity> >::iterator it = players->begin(); it != players->end(); ++it)
            {
                boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(*it);
                player->addEffect(new MobEffectInstance(mPrimaryPower, duration, amplifier, true, true),
                                  boost::shared_ptr<Entity>());
            }
            if (mLevels >= 4 && mPrimaryPower != mSecondaryPower && mSecondaryPower != NULL)
            {
                for (std::vector<boost::shared_ptr<Entity> >::iterator it = players->begin(); it != players->end();
                     ++it)
                {
                    boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(*it);
                    player->addEffect(new MobEffectInstance(mSecondaryPower, duration, 0, true, true),
                                      boost::shared_ptr<Entity>());
                }
            }
            delete players;
        }
        else if (mLevels >= 4 && mSecondaryPower != NULL)
        {
            double range = mLevels * 10 + 10;
            int duration = (9 + mLevels * 2) * 20;
            int x = mPos.x;
            int y = mPos.y;
            int z = mPos.z;
            AABB* area = AABB::newTemp(x, y, z, x + 1, y + 1, z + 1)
                             ->grow(range, range, range)
                             ->expand(0.0, mLevel->getMaxBuildHeight(), 0.0);
            std::vector<boost::shared_ptr<Entity> >* players = mLevel->getEntitiesOfClass(typeid(Player), area);
            for (std::vector<boost::shared_ptr<Entity> >::iterator it = players->begin(); it != players->end(); ++it)
            {
                boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(*it);
                player->addEffect(new MobEffectInstance(mSecondaryPower, duration, 0, true, true),
                                  boost::shared_ptr<Entity>());
            }
            delete players;
        }
    }
}

void BeaconBlockEntity::checkValidity()
{
    if (mLevel == NULL)
        return;
    updateShape();
    applyEffects();
}

boost::shared_ptr<BlockEntity> BeaconBlockEntity::clone()
{
    boost::shared_ptr<BeaconBlockEntity> copy(new BeaconBlockEntity());
    BlockEntity::clone(copy);
    copy->mPrimaryPower = mPrimaryPower;
    copy->mSecondaryPower = mSecondaryPower;
    copy->mLevels = mLevels;
    copy->mName->set(mName->raw(), std::wstring(mName->getUUID()), mName->isChatCensored(), mName->isUUIDLocal());
    return copy;
}

void BeaconBlockEntity::AddEffect(int level, int slot, MobEffect* effect)
{
    sBeaconEffects[level][slot] = effect;
    sBeaconEffectOrder[sBeaconEffectCount] = effect;
    ++sBeaconEffectCount;
}

void BeaconBlockEntity::staticCtor()
{
    for (int level = 0; level < 4; ++level)
    {
        for (int slot = 0; slot < 3; ++slot)
            sBeaconEffects[level][slot] = NULL;
    }
    AddEffect(0, 0, MobEffects::MOVEMENT_SPEED);
    AddEffect(0, 1, MobEffects::DIG_SPEED);
    AddEffect(1, 0, MobEffects::DAMAGE_RESISTANCE);
    AddEffect(1, 1, MobEffects::JUMP);
    AddEffect(2, 0, MobEffects::DAMAGE_BOOST);
    AddEffect(3, 0, MobEffects::REGENERATION);
}

void BeaconBlockEntity::tick()
{
    bool wasActive = mActive;
    if (!mLevel->mIsClientSide && mLevels < 0)
        checkValidity();
    else if (mLevel->getGameTime() % 80 == 0)
        checkValidity();

    ++mTickCount;
    if (!mLevel->mIsClientSide)
        return;

    BlockPos pos = getBlockPos();
    long long seed = pos.hashCode();
    Random random(seed);
    if ((mTickCount + random.nextInt(9001)) % 80 == 0 && mActive)
    {
        mLevel->playLocalSound(getBlockPos().x, getBlockPos().y, getBlockPos().z, SoundEvents::BEACON_AMBIENT,
                               SoundSource::eSoundSource_Blocks, 0.5F, 1.0F, false, 16.0F);
    }

    if (wasActive != mActive)
    {
        const SoundEvent* sound = mActive ? SoundEvents::BEACON_ACTIVATE : SoundEvents::BEACON_DEACTIVATE;
        mLevel->playLocalSound(getBlockPos().x, getBlockPos().y, getBlockPos().z, sound,
                               SoundSource::eSoundSource_Blocks, 1.0F, 1.0F, false, 16.0F);
    }
}

std::vector<BeaconBlockEntity::BeaconBeamSection>* BeaconBlockEntity::getBeamSections()
{
    return &mBeamSections;
}

float BeaconBlockEntity::getAndUpdateClientSideScale()
{
    if (!mActive)
    {
        return 0.0F;
    }
    int elapsed = (int)(mLevel->getGameTime() - mNextClientSideUpdate);
    mNextClientSideUpdate = mLevel->getGameTime();
    if (elapsed > 1)
    {
        mClientSideScale -= (float)elapsed / 40.0F;
        if (mClientSideScale < 0.0F)
        {
            mClientSideScale = 0.0F;
        }
    }
    mClientSideScale += 0.025F;
    if (mClientSideScale > 1.0F)
    {
        mClientSideScale = 1.0F;
    }
    return mClientSideScale;
}

boost::shared_ptr<Packet> BeaconBlockEntity::getUpdatePacket()
{
    return boost::shared_ptr<ClientboundBlockEntityDataPacket>(
        new ClientboundBlockEntityDataPacket(mPos, 3, mLevel->mDimension->getType()->getId(), getUpdateTag()));
}

CompoundTag* BeaconBlockEntity::getUpdateTag()
{
    return save(new CompoundTag());
}

double BeaconBlockEntity::getViewDistance()
{
    return 65536.0;
}

void BeaconBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mName->loadFromTag(tag);
    mPrimaryPower = getValidEffectById(tag->getInt(L"Primary"));
    mSecondaryPower = getValidEffectById(tag->getInt(L"Secondary"));
    mLevels = tag->getInt(L"Levels");
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureCustomBeacons())
    {
        mBeamDirection = tag->getInt(L"BeamDirection");
        mBeamLength = tag->getInt(L"BeamLength");
        updateShapeForGlide();
    }
}

CompoundTag* BeaconBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    mName->saveToTag(tag);
    tag->putInt(L"Primary", MobEffect::getId(mPrimaryPower));
    tag->putInt(L"Secondary", MobEffect::getId(mSecondaryPower));
    tag->putInt(L"Levels", mLevels);
    if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureCustomBeacons())
    {
        tag->putInt(L"BeamDirection", mBeamDirection);
        tag->putInt(L"BeamLength", mBeamLength);
    }
    return tag;
}

bool BeaconBlockEntity::triggerEvent(int id, int param)
{
    if (id == 1)
    {
        checkValidity();
        return true;
    }
    return BlockEntity::triggerEvent(id, param);
}

void BeaconBlockEntity::setBeamDirection(eBEAM_DIRECTION direction)
{
    mBeamDirection = direction;
}

void BeaconBlockEntity::setBeamLength(int length)
{
    mBeamLength = length;
}

std::wstring BeaconBlockEntity::getDisplayName()
{
    return LockableContainerBlockEntity::getDisplayName();
}

BlockEntity* BeaconBlockEntity::create()
{
    return new BeaconBlockEntity();
}

unsigned long long BeaconBlockEntity::GetType()
{
    return BlockEntityType::BEACON;
}
