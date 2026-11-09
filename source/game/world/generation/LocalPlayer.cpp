#include "game/core/UnityClient6.hpp"
#include "game/item/ItemId.hpp"
#include "game/world/block/BlockId.hpp"
#include "game/core/EntityType.hpp"
#include "game/world/generation/LocalPlayer.hpp"

#include <algorithm>
#include <cfloat>

#include "game/core/GameSettings.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/PlayerRideableJumping.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/client/audio/ConsoleSoundEngine.hpp"
#include "game/client/audio/RidingMinecartSoundInstance.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ClientboundSoundPacket.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/network/ServerboundContainerClosePacket.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/Direction.hpp"
#include "game/network/ServerboundPlayerActionPacket.hpp"
#include "game/network/ServerboundChatPacket.hpp"
#include "game/network/ServerboundMovePlayerPacket.hpp"
#include "game/network/ServerboundPlayerAbilitiesPacket.hpp"
#include "game/network/ServerboundSwingPacket.hpp"
#include "game/network/ServerboundUseItemPacket.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/GameType.hpp"
#include "game/core/User.hpp"
#include "game/core/MOJANG_DATA.hpp"
#include "game/core/Input.hpp"
#include "platform/wiiu/CInput.hpp"
#include "game/core/FoodConstants.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/core/StatsCounter.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/CommonStats.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/core/Achievement.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Gui.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/ui/Tooltips.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ElytraItem.hpp"
#include "game/item/Items.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/item/SpawnEggItem.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/core/CTelemetryManager.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/DamageSource.hpp"
#include "game/core/HitResult.hpp"
#include "game/client/render/ParticleEngine.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/ItemInHandRenderer.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/item/PowerupItems.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/block/BeaconBlockEntity.hpp"
#include "game/world/block/BrewingStandBlockEntity.hpp"
#include "game/world/block/DispenserBlockEntity.hpp"
#include "game/world/block/DropperBlockEntity.hpp"
#include "game/world/block/FlowerPotBlockEntity.hpp"
#include "game/world/block/FurnaceBlockEntity.hpp"
#include "game/world/block/ShulkerBoxBlockEntity.hpp"
#include "game/core/MinecartChest.hpp"
#include "game/core/MinecartHopper.hpp"
#include "game/core/Minecart.hpp"
#include "game/core/Boat.hpp"
#include "game/core/ArmorStand.hpp"
#include "game/core/EnderCrystal.hpp"
#include "game/core/Painting.hpp"
#include "game/entity/mob/LeashFenceKnotEntity.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/ServerboundPlayerCommandPacket.hpp"
#include "game/network/ServerboundPlayerInputPacket.hpp"
#include "game/util/Math.hpp"
#include "game/network/ServerboundMoveVehiclePacket.hpp"
#include "game/network/ServerboundClientCommandPacket.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/ui/AnvilMenu.hpp"
#include "game/ui/BrewingStandMenu.hpp"
#include "game/ui/EnchantmentMenu.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/util/phys__Vec2.hpp"

#include <cmath>
#include <cstring>
#include <algorithm>
#include <iostream>
#ifdef MINECRAFT_PC
#include "platform/pc/PcModViews.hpp"
#include "platform/pc/PcNativeMods.hpp"
#include "game/ui/BrewingStandMenu.hpp"
#include "game/ui/ChestMenu.hpp"
#include "game/ui/CraftingMenu.hpp"
#include "game/ui/FurnaceMenu.hpp"
#include "game/ui/HopperMenu.hpp"
#include "game/ui/ShulkerBoxMenu.hpp"
#include "game/ui/TrapMenu.hpp"

static AbstractContainerMenu* hiddenMenu = NULL;
#endif

LocalPlayer::LocalPlayer(Minecraft* minecraft, Level* level, User* user, int dimension)
    : AbstractClientPlayer(level, user->name)
{
    std::memset(mAwardedAchievements, 0, sizeof(mAwardedAchievements));
    mFlyTogglePending = false;
    mRightClickDelay = 0;
    mHealthInitialized = false;
    mFallFlyingAlignmentO = 0.0f;
    mViewYRot = 0.0f;
    mViewXRot = 0.0f;
    mFlyMotionY = 0.0f;
    mWasSprinting = false;
    mAutoJumpEnabled = true;
    mIdleTicks = 0;
    mMinecraft = minecraft;
    mFallFlyYaw = 0.0f;
    mFlyMotionX = 0.0f;
    mPortalEffectIntensity = 0.0f;
    mLastSentYRot = 0.0f;
    mWasSwimming = false;
    mPermissionLevel = 0;
    mPortalEffectIntensityO = 0.0f;
    mUsedItemHand = InteractionHand::eInteractionHand_MainHand;
    mFallFlyYawOld = 0.0f;
    mLastSentY = 0.0;
    mPreviousOnGround = false;
    mFlyMotionZ = 0.0f;
    mViewYRotO = 0.0f;
    mHandsBusy = false;
    mThirdPersonViewTimer = 0;
    mLastSentXRot = 0.0f;
    mWasSneaking = false;
    mPositionSendCounter = 0;
    mSprinting = 0;
    mSprintDelay = 0;
    mAutoJumpTime = 0;
    mViewXRotO = 0.0f;
    mSprintPending = false;
    mLastSentZ = 0.0;
    mWasDamagingNearbyMobs = false;
    mFallFlyingAlignment = 0.0f;
    mLastSentX = 0.0;
    mItemUsePending = false;
    mRideJumpTicks = 0;
    mRideJumpScale = 0.0f;
    mDimension = dimension;
    if (user != NULL)
    {
        if (!user->name.empty())
        {
            std::wstring skinUrl = std::wstring(L"http://s3.amazonaws.com/MinecraftSkins/").append(user->name);
            mSkinLocation = skinUrl + L".png";
        }
        setName(user->name);
        MOJANG_DATA* data = gApp.GetMojangDataForXuid(mOnlineXuid);
        if (data != NULL)
            mSkinLocation = data->skinPath;
    }

    mFlyButtonsDown[1] = 0;
    mMissTime = 0;
    mFlyButtons[0] = 0;
    mButtonsDown = 0;
    mLastSingleFlyButtons[0] = 0;
    mViewport = 0;
    mInput = NULL;
    mUserIndex = -1;
    mHasOpenedInventory = true;
    mLastAttackTick = 0;
    mFlyButtonsDown[0] = 0;
    mLastSingleFlyButtons[1] = 0;
    mDRCTouching = false;
    mPresenceIdleSent = false;
    mWaitingForScoreboard = false;
    mFlyButtons[1] = 0;
    mReserved0x93c = false;
    mThirdPersonView = 0;
    mReserved0x178 = false;
    mLastUseTick = 0;
    SetSessionTimerStart();
    for (int i = 0; i < 8; ++i)
        mDamageIndicatorSegments[i] = false;
    mUnderwaterTicks[1] = 0;
    mDamageIndicatorAllDirections = false;
    mDamageNearbyMobs = false;
    mWaterVisionClarityO = 0.0f;
    mDamageTaken = 0.0f;
    mWaterVisionClarityTarget = 0.0f;
    mInWallThirdPersonView = 0;
    mPercentageTraversed = 0.0;
    mCreativeMouseRepeatDelay = 0.0f;
    mWaterVisionClarity = 0.0f;
    mUnderwaterTicks[0] = 0;
    mCreativeRepeatState = 0;
}

LocalPlayer::~LocalPlayer()
{
    delete mInput;
}

bool LocalPlayer::hasPermission(EGameCommand command)
{
    return mLevel->getLevelData()->getAllowCommands();
}

BlockPos LocalPlayer::getCommandSenderBlockPosition()
{
    return BlockPos(mX + 0.5, mY + 0.5, mZ + 0.5);
}

bool LocalPlayer::hurt(const DamageSource* source, float damage)
{
    return false;
}

void LocalPlayer::tick()
{
    if (isSpectator() && IsSpectatingOtherPlayer())
    {
        boost::shared_ptr<Entity> camera = GetCamera();
        moveTo(camera->mX, camera->mY, camera->mZ, mYRot, mXRot);
    }

    if (!mLevel->hasChunkAt(BlockPos(mX, 0.0, mZ)))
        return;

    double oldY = mY;
    double oldZ = mZ;
    if (mRightClickDelay > 0)
        --mRightClickDelay;
    double oldX = mX;
    Player::tick();

    Vec3* newPos = Vec3::newTemp(mX, mY, mZ);
    if (canMoveOrBeMovedFromTo(oldX, oldY, oldZ, mX, mY, mZ, newPos))
    {
        if (isPassenger())
        {
            mPacketListener->send(
                boost::shared_ptr<Packet>(new ServerboundMovePlayerPacket::Rot(mYRot, mXRot, mOnGround, false)));
            mPacketListener->send(boost::shared_ptr<Packet>(
                new ServerboundPlayerInputPacket(mXxa, mZza, mInput->mJumping, mInput->mSneaking)));
            boost::shared_ptr<Entity> rootVehicle = getRootVehicle();
            if (rootVehicle != shared_from_this() && rootVehicle->isControlledByLocalInstance())
                mPacketListener->send(boost::shared_ptr<Packet>(new ServerboundMoveVehiclePacket(rootVehicle)));
        }
        else
        {
            sendPosition();
        }
    }
    else
    {
        if (!gApp.DebugDevToolsOn())
        {
            setPos(oldX, oldY, oldZ);
            double xa = newPos->x - mX;
            double ya = newPos->y - mY;
            double za = newPos->z - mZ;
            std::vector<AABB*>* boxes = mLevel->getCollisionAABBs(shared_from_this(), mBoundingBox->expand(xa, ya, za),
                                                                  false, true, false);
            if (!mLevel->mIsClientSide
                || mLevel->reallyHasChunk(static_cast<int>(std::floor(mX * 0.0625)),
                                          static_cast<int>(std::floor(mZ * 0.0625))))
            {
                for (std::vector<AABB*>::iterator it = boxes->begin(); it != boxes->end(); ++it)
                    ya = (*it)->clipYCollide(mBoundingBox, ya);
                setBoundingBox(mBoundingBox->move(0.0, ya, 0.0));
            }
            for (std::vector<AABB*>::iterator it = boxes->begin(); it != boxes->end(); ++it)
                xa = (*it)->clipXCollide(mBoundingBox, xa);
            setBoundingBox(mBoundingBox->move(xa, 0.0, 0.0));
            for (std::vector<AABB*>::iterator it = boxes->begin(); it != boxes->end(); ++it)
                za = (*it)->clipZCollide(mBoundingBox, za);
            setBoundingBox(mBoundingBox->move(0.0, 0.0, za));
            mX = (mBoundingBox->mMinX + mBoundingBox->mMaxX) * 0.5;
            mY = mBoundingBox->mMinY;
            mZ = (mBoundingBox->mMinZ + mBoundingBox->mMaxZ) * 0.5;
        }
        sendPosition();
    }
    mDamageIndicatorAllDirections = false;

    for (int segment = 0; segment < 8; ++segment)
        mDamageIndicatorSegments[segment] = false;

    if (!isAlive() || !isInWater() || !isUnderLiquid(Material::WATER))
    {
        mUnderwaterTicks[0] = 0;
        mUnderwaterTicks[1] = 0;
        if (mWaterVisionClarity > 0.0f)
            mWaterVisionClarity = Mth::clamp(mWaterVisionClarity - 0.5f, 0.0f, 10.0f);
    }
    else
    {
        ++mUnderwaterTicks[0];
        ++mUnderwaterTicks[1];
        if (mWaterVisionClarity < 10.0f)
            mWaterVisionClarity = Mth::clamp(mWaterVisionClarity + 0.05f, 0.0f, 10.0f);
    }
    updateProximityIndicator();
}

Vec3* LocalPlayer::getViewVector(float partialTick)
{
#ifdef MINECRAFT_PC
    double eye[3];
    double view[3];
    if (PcModViewsAimFor(this, eye, view))
        return Vec3::newTemp(view[0], view[1], view[2]);
#endif
    return calculateViewVector(mXRot, mYRot);
}

bool LocalPlayer::isInWall()
{
    if (mSleeping || mNoPhysics)
    {
        return false;
    }

    if (!mTickCache.mCacheChanged && mInWallCacheX == mX && mInWallCacheY == mY && mInWallCacheZ == mZ
        && mInWallCacheYRot == mYRot && mInWallCacheXRot == mXRot && mInWallThirdPersonView == mThirdPersonView)
    {
        return mInWallCacheResult;
    }

    mInWallThirdPersonView = mThirdPersonView;
    mInWallCacheYRot = mYRot;
    mInWallCacheXRot = mXRot;
    mInWallCacheX = mX;
    mInWallCacheY = mY;
    mInWallCacheZ = mZ;

    BlockPos lastPos(INT_MIN, INT_MIN, INT_MIN);
    double eyeHeight = getEyeHeight();
    int blockX = 0;
    int blockZ = 0;

    for (int i = 0; i < 8; ++i)
    {
        double y = mY + (double)(((float)((i >> 0) % 2) - 0.5f) * 0.1f) + eyeHeight;
        int blockY = Mth::floor(y);
        if ((i & 1) == 0)
        {
            double x = mX + (double)(((float)((i >> 1) % 2) - 0.5f) * mBbWidth * 0.8f);
            blockX = Mth::floor(x);
        }
        if ((i & 2) == 0)
        {
            double z = mZ + (double)(((float)((i >> 2) % 2) - 0.5f) * mBbWidth * 0.8f);
            blockZ = Mth::floor(z);
        }

        if (lastPos.x != blockX || lastPos.y != blockY || lastPos.z != blockZ)
        {
            lastPos = BlockPos(blockX, blockY, blockZ);
            const BlockState* state = mLevel->getBlockState(mTickCache, lastPos);
            if (state->isViewBlocking())
            {
                if (!Minecraft::InMiniGame(eMiniGameId_Any, true) || gApp.ShouldCollideWith(state->getBlock()))
                {
                    mInWallCacheResult = true;
                    return true;
                }
            }
        }
    }

    mInWallCacheResult = false;
    return false;
}

void LocalPlayer::rideTick()
{
    Player::rideTick();
    mHandsBusy = false;
    if (Entity::instanceof(getVehicle(), EntityType::BOAT))
    {
        boost::shared_ptr<Boat> boat = boost::static_pointer_cast<Boat>(getVehicle());
        if (boat->getControllingPassenger() == shared_from_this())
            boat->setInput(mInput->mLeft, mInput->mRight, mInput->mUp, mInput->mDown);
        mHandsBusy = mHandsBusy || mInput->mLeft || mInput->mRight || mInput->mUp || mInput->mDown;
    }
}

bool LocalPlayer::startRiding(boost::shared_ptr<Entity> vehicle, bool force)
{
    if (Entity::instanceof(vehicle, EntityType::HORSE))
    {
        mYRot = vehicle->mYRot;
        mXRot = vehicle->mXRot;
    }

    if (!Entity::startRiding(vehicle, force))
        return false;

    if (Entity::instanceof(vehicle, EntityType::MINECART))
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(shared_from_this());
        static_cast<ConsoleSoundEngine*>(mMinecraft->mSoundEngine)
            ->play(new RidingMinecartSoundInstance(player, boost::static_pointer_cast<Minecart>(vehicle)));
    }
    if (Entity::instanceof(vehicle, EntityType::BOAT))
    {
        mYRotO = vehicle->mYRot;
        mYRot = vehicle->mYRot;
        setYHeadRot(vehicle->mYRot);
    }
    return true;
}

void LocalPlayer::stopRiding()
{
    Player::stopRiding();
    mInput->mSneaking = false;
    mHandsBusy = false;
}

void LocalPlayer::handleEntityEvent(unsigned char eventId, int data)
{
    if (eventId >= 24 && eventId <= 28)
    {
        setPermissionLevel(eventId - 24);
        return;
    }
    Player::handleEntityEvent(eventId, data);
}

bool LocalPlayer::isSneaking()
{
    bool sneaking = mInput != NULL ? mInput->mSneaking : false;
    return sneaking && !isSleeping() && !mInput->mSwimming;
}

bool LocalPlayer::isSprinting()
{
    return mSprinting > 0;
}

void LocalPlayer::setSprinting(bool sprinting)
{
    LivingEntity::setSprinting(sprinting);
    mSprinting = sprinting ? 600 : 0;
}

boost::shared_ptr<Entity> LocalPlayer::changeDimension(int dimension)
{
    return shared_from_this();
}

void LocalPlayer::onSyncedDataUpdated(const EntityDataAccessor_Base* accessor)
{
    Player::onSyncedDataUpdated(accessor);
    if (DATA_LIVING_ENTITY_FLAGS->getId() != accessor->getId())
        return;

    bool usingItem = (1 & mEntityData->get(DATA_LIVING_ENTITY_FLAGS)) > 0;
    bool offhand = (2 & mEntityData->get(DATA_LIVING_ENTITY_FLAGS)) > 0;
    if (usingItem)
    {
        if (!mItemUsePending)
            startUsingItem(offhand ? InteractionHand::eInteractionHand_OffHand
                                   : InteractionHand::eInteractionHand_MainHand);
    }
    else if (mItemUsePending)
    {
        stopUsingItem();
    }
}

void LocalPlayer::closeContainer()
{
    mPacketListener->send(
        boost::shared_ptr<Packet>(new ServerboundContainerClosePacket(mContainerMenu->m_containerId)));
    LocalPlayer::clientSideCloseContainer();
}

boost::shared_ptr<ItemEntity> LocalPlayer::drop(bool dropAll)
{
    ServerboundPlayerActionPacket::Action action = dropAll ? ServerboundPlayerActionPacket::DROP_ALL_ITEMS
                                                           : ServerboundPlayerActionPacket::DROP_ITEM;
    mPacketListener->send(
        boost::shared_ptr<Packet>(new ServerboundPlayerActionPacket(action, BlockPos::ZERO, Direction::DOWN, 0)));
    return boost::shared_ptr<ItemEntity>();
}

not_null_ptr<ItemInstance> LocalPlayer::reallyDrop(boost::shared_ptr<ItemEntity> item)
{
    return ItemInstance::EMPTY;
}

void LocalPlayer::openTextEdit(boost::shared_ptr<SignBlockEntity> sign)
{
    if (gApp.LoadSignEntryMenu(mUserIndex, sign) != 0)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    }
}

void LocalPlayer::openCommandBlock(boost::shared_ptr<CommandBlockEntity> block)
{
}

int LocalPlayer::openTrading(boost::shared_ptr<Merchant> merchant, const std::wstring& name, int level)
{
    const int result = static_cast<int>(gApp.LoadTradingMenu(mUserIndex, mInventory, merchant, mLevel, name));
    if (result != 0)
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    return result;
}

int LocalPlayer::openContainer(boost::shared_ptr<Container> container)
{
    boost::shared_ptr<InteractionObject> interaction = boost::dynamic_pointer_cast<InteractionObject>(container);
    const int typeId = interaction != NULL ? interaction->getTypeId(false) : ClientboundContainerOpenPacket::CONTAINER;
    int result;
#ifdef MINECRAFT_PC
    if (PcNativeModsHideMenus() && mUserIndex == 0)
    {
        AbstractContainerMenu* menu = NULL;
        boost::shared_ptr<Player> self = boost::dynamic_pointer_cast<Player>(shared_from_this());
        if (typeId == ClientboundContainerOpenPacket::FURNACE)
            menu = new FurnaceMenu(mInventory, container);
        else if (typeId == ClientboundContainerOpenPacket::BREWING_STAND)
            menu = new BrewingStandMenu(mInventory, container);
        else if (typeId == ClientboundContainerOpenPacket::HOPPER)
            menu = new HopperMenu(mInventory, container, self);
        else if (typeId == ClientboundContainerOpenPacket::TRAP || typeId == ClientboundContainerOpenPacket::DROPPER)
            menu = new TrapMenu(mInventory, container);
        else if (typeId == ClientboundContainerOpenPacket::SHULKER_BOX)
            menu = new ShulkerBoxMenu(mInventory, container, self);
        else if (typeId != ClientboundContainerOpenPacket::BEACON)
            menu = new ChestMenu(mInventory, container, self);
        if (menu != NULL)
        {
            if (hiddenMenu != NULL && mContainerMenu == hiddenMenu)
                mContainerMenu = mInventoryMenu;
            delete hiddenMenu;
            hiddenMenu = menu;
            mContainerMenu = menu;
            return 1;
        }
    }
#endif
    if (typeId == ClientboundContainerOpenPacket::CONTAINER)
        result = static_cast<int>(gApp.LoadContainerMenu(mUserIndex, mInventory, container));
    else if (typeId == ClientboundContainerOpenPacket::HOPPER)
        result = static_cast<int>(gApp.LoadHopperMenu(mUserIndex, mInventory, container));
    else if (typeId == ClientboundContainerOpenPacket::FURNACE)
        result = static_cast<int>(gApp.LoadFurnaceMenu(mUserIndex, mInventory, container));
    else if (typeId == ClientboundContainerOpenPacket::BREWING_STAND)
        result = static_cast<int>(gApp.LoadBrewingStandMenu(mUserIndex, mInventory, container));
    else if (typeId == ClientboundContainerOpenPacket::BEACON)
        result = static_cast<int>(gApp.LoadBeaconMenu(mUserIndex, mInventory, container));
    else if (typeId == ClientboundContainerOpenPacket::TRAP || typeId == ClientboundContainerOpenPacket::DROPPER)
        result = static_cast<int>(gApp.LoadTrapMenu(mUserIndex, mInventory, container));
    else if (typeId == ClientboundContainerOpenPacket::SHULKER_BOX)
        result = static_cast<int>(gApp.LoadShulkerBoxMenu(mUserIndex, mInventory, container));
    else
        result = static_cast<int>(gApp.LoadContainerMenu(mUserIndex, mInventory, container));
    if (result != 0)
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    return result;
}

int LocalPlayer::openHorseInventory(boost::shared_ptr<AbstractHorse> horse, boost::shared_ptr<Container> container)
{
    const int result = (int)gApp.LoadHorseMenu(mUserIndex, mInventory, container, horse);
    if (result != 0)
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    return result;
}

int LocalPlayer::startBlockInteraction(InteractionObject& interaction)
{
    int result;
#ifdef MINECRAFT_PC
    if (PcNativeModsHideMenus() && mUserIndex == 0
        && interaction.getTypeId(false) == ClientboundContainerOpenPacket::WORKBENCH)
    {
        AbstractContainerMenu* menu = new CraftingMenu(mInventory, mLevel, BlockPos((int)mX, (int)mY, (int)mZ));
        if (hiddenMenu != NULL && mContainerMenu == hiddenMenu)
            mContainerMenu = mInventoryMenu;
        delete hiddenMenu;
        hiddenMenu = menu;
        mContainerMenu = menu;
        return 1;
    }
#endif
    if (interaction.getTypeId(false) == ClientboundContainerOpenPacket::WORKBENCH)
    {
        result = (int)gApp.LoadCrafting3x3Menu(mUserIndex, boost::dynamic_pointer_cast<LocalPlayer>(shared_from_this()),
                                               (int)mX, (int)mY, (int)mZ);
    }
    else if (interaction.getTypeId(false) == ClientboundContainerOpenPacket::ENCHANTMENT)
    {
        result = (int)gApp.LoadEnchantingMenu(mUserIndex, mInventory, (int)mX, (int)mY, (int)mZ, mLevel,
                                              interaction.getDisplayName());
    }
    else if (interaction.getTypeId(false) == ClientboundContainerOpenPacket::REPAIR_TABLE)
    {
        result = (int)gApp.LoadRepairingMenu(mUserIndex, mInventory, mLevel, (int)mX, (int)mY, (int)mZ);
    }
    else
    {
        return 0;
    }
    if (result != 0)
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    return result;
}

void LocalPlayer::openItemInstanceGui(not_null_ptr<ItemInstance> item, InteractionHand::EInteractionHand hand)
{
    bool opened = false;
    Item* book = item->getItem();
    if (book == Items::writtenBook)
    {
        opened = gApp.LoadWritingBookMenu(mUserIndex, item, false);
    }
    else if (book == Items::writableBook)
    {
        gApp.LoadWritingBookMenu(mUserIndex, item, true);
        opened = true;
    }

    if (opened)
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
}

void LocalPlayer::crit(boost::shared_ptr<Entity> entity)
{
    mMinecraft->mParticleEngine->createTrackingEmitter(entity, PARTICLE_CRIT);
}

void LocalPlayer::magicCrit(boost::shared_ptr<Entity> entity)
{
    mMinecraft->mParticleEngine->createTrackingEmitter(entity, PARTICLE_MAGIC_CRIT);
}

void LocalPlayer::serverAiStep()
{
    Player::serverAiStep();
    if (isControlledCamera())
    {
        float movement;
        if (mAbilities.mFlying && mAbilities.mMayfly
            && (movement = std::sqrt(mInput->mMoveStrafe * mInput->mMoveStrafe
                                     + mInput->mMoveForward * mInput->mMoveForward))
                   >= 0.1f)
        {
            const float angle = std::atan2(mInput->mMoveForward, mInput->mMoveStrafe) * 57.295776f;
            const float heading = angle + mYRot;
            const float snapped = angle + (std::floor(heading / 45.0f + 0.5f) * 45.0f - heading);
            const float radians = snapped * 0.017453292f;
            mXxa = std::cos(radians) * movement;
            mZza = std::sin(radians) * movement;
        }
        else
        {
            mXxa = mInput->mMoveStrafe;
            mZza = mInput->mMoveForward;
        }
        mJumping = mInput->mJumping;
        mViewXRotO = mViewXRot;
        mViewYRotO = mViewYRot;
        mViewXRot += (mXRot - mViewXRot) * 0.5f;
        mViewYRot += (mYRot - mViewYRot) * 0.5f;
    }
}

float LocalPlayer::tickHeadTurn(float yRot, float xRot)
{
    if (isFallFlying())
    {
        float bodyTurn = Mth::wrapDegrees(yRot - mYBodyRot);
        mYBodyRot = bodyTurn * 0.3f + mYBodyRot;
        float headDiff = Mth::wrapDegrees(mYRot - mYBodyRot);
        bool backwards = headDiff < -90.0f || headDiff >= 90.0f;
        if (headDiff < -50.0f)
            headDiff = -50.0f;
        else if (headDiff >= 50.0f)
            headDiff = 50.0f;
        mYBodyRot = mYRot - headDiff;
        if (backwards)
            xRot = -xRot;
        return xRot;
    }
    return LivingEntity::tickHeadTurn(yRot, xRot);
}

#ifdef MINECRAFT_PC
bool PcInputKeyboardStrafing();
#endif

void LocalPlayer::aiStep()
{
    if (isSprinting() && --mSprinting < 1)
        mSprinting = 1;

    if (mSprintDelay > 0)
        --mSprintDelay;

    mPortalEffectIntensityO = mPortalEffectIntensity;
    if (!mIsInsidePortal)
    {
        bool confusionIsLong = false;
        if (hasEffect(MobEffects::CONFUSION))
        {
            MobEffectInstance* confusion = getEffect(MobEffects::CONFUSION);
            confusionIsLong = confusion->getDuration() > 60;
        }
        if (confusionIsLong)
        {
            mPortalEffectIntensity = std::min(mPortalEffectIntensity + 0.006666667f, 1.0f);
        }
        else
        {
            if (mPortalEffectIntensity > 0.0f)
                mPortalEffectIntensity -= 0.05f;
            if (mPortalEffectIntensity < 0.0f)
                mPortalEffectIntensity = 0.0f;
        }
    }
    else
    {
        if (!mLevel->mIsClientSide && isPassenger())
            stopRiding();
        if (mMinecraft->mScreen != NULL)
            mMinecraft->setScreen(NULL);
        if (mPortalEffectIntensity == 0.0f)
            mMinecraft->mSoundEngine->playUI(SoundEvents::PORTAL_TRIGGER, 1.0f, mRandom->nextFloat() * 0.4f + 0.8f);
        mPortalEffectIntensity = std::min(mPortalEffectIntensity + 0.0125f, 1.0f);
        mIsInsidePortal = false;
    }

    if (mPortalCooldown > 0)
        --mPortalCooldown;

    const bool wasJumping = mInput->mJumping;
    const bool wasSneaking = mInput->mSneaking;
    const float previousForward = mInput->mMoveForward;
    mInput->tick(this);

    bool movingFreely = true;
    if (mInput->mMoveForward < 0.8f && mInput->mMoveForward > -0.8f && mInput->mMoveStrafe < 0.8f
        && mInput->mMoveStrafe > -0.8f)
    {
        movingFreely = false;
    }

    if (movingFreely)
        ++mThirdPersonViewTimer;
    else
        mThirdPersonViewTimer = 0;

    if (isUsingItem() && !isPassenger())
    {
        mSprintDelay = 0;
        mInput->mMoveStrafe *= 0.2f;
        mInput->mMoveForward *= 0.2f;
    }

    const int autoJumpTime = mAutoJumpTime;
    if (mAutoJumpTime > 0)
    {
        --mAutoJumpTime;
        mInput->mJumping = true;
    }

    const double edgeOffset = static_cast<double>(mBbWidth) * 0.35;
    const double edgeY = mBoundingBox->mMinY + 0.5;
    checkInBlock(mX - edgeOffset, edgeY, mZ + edgeOffset);
    checkInBlock(mX - edgeOffset, edgeY, mZ - edgeOffset);
    checkInBlock(mX + edgeOffset, edgeY, mZ - edgeOffset);
    checkInBlock(mX + edgeOffset, edgeY, mZ + edgeOffset);

    const bool canFly = mAbilities.mMayfly || isAllowedToFly();
    const int foodLevel = getFoodData()->getFoodLevel();
    const int sprintLevel = FoodConstants::SprintLevel();
    const bool foodAllowsSprinting = foodLevel > sprintLevel || canFly;

    bool forwardSprintInput = false;
    if (!((!mOnGround && !isInWater()) || isSprinting() || !foodAllowsSprinting || isUsingItem()
          || hasEffect(MobEffects::BLINDNESS) || isPassenger()))
    {
        forwardSprintInput = g_pInput->ButtonDown(mUserIndex, EControllerActions_Sprint);
        const float forward = mInput->mMoveForward;
        if (!forwardSprintInput || forward < 0.8f)
        {
            if (wasSneaking || previousForward >= 0.8f || forward < 0.8f)
            {
                if (mSprintDelay > 0 && forward == 0.0f)
                    mSprintPending = true;
            }
            else if (mSprintDelay == 0)
            {
                mSprintDelay = 9;
                mSprintPending = false;
            }
            else if (mSprintPending)
            {
                setSprinting(true);
                mSprintDelay = 0;
                mSprintPending = false;
            }
        }
        else
        {
            setSprinting(true);
            mSprintDelay = 0;
            mSprintPending = false;
        }
    }

    bool sidewaysAllowsSprint = std::fabs(mInput->getMoveVector()->x) <= 0.8f;
#ifdef MINECRAFT_PC
    if (mUserIndex == gProfile->GetPrimaryPad() && PcInputKeyboardStrafing())
        sidewaysAllowsSprint = true;
#endif
    bool stopForDirection = true;
    if (mInput->mMoveForward >= 0.8f && sidewaysAllowsSprint && foodAllowsSprinting)
        stopForDirection = false;

    bool keepSprintForInput = mInput->mMoveForward >= 0.8f && foodAllowsSprinting;
    if (isSprinting() && mOnGround && stopForDirection)
        setSprinting(false);

    Vec3* position = getPos(0.0f);
    const BlockPos currentBlock(position);
    Material* currentMaterial = mLevel->getBlockState(currentBlock)->getMaterial();
    const BlockPos aboveBlock(position->add(0.0, 1.0, 0.0));
    Material* aboveMaterial = mLevel->getBlockState(aboveBlock)->getMaterial();
    position = getPos(0.0f);
    Vec3* viewVector = getViewVector(1.0f);
    Vec3 horizontalView = *viewVector;
    horizontalView.y = 0.0;
    const float viewAngle = std::acos(static_cast<float>(horizontalView.dot(viewVector))) * 57.295776f;
    const BlockPos belowBlock(position->subtract(0.0, 1.1f, 0.0));
    Material* belowMaterial = mLevel->getBlockState(belowBlock)->getMaterial();

    if (isInWater() && !mAbilities.mFlying && !isSwimming()
        && ((belowMaterial != Material::AIR && aboveMaterial != Material::AIR) || viewVector->y < -0.15f)
        && mLevel->mIsClientSide && isSprinting() && !stopForDirection && !isPassenger())
    {
        startSwimming();
    }

    const bool stopSwimmingForBlocks = !keepSprintForInput && !aboveMaterial->blocksMotion()
                                       && !currentMaterial->blocksMotion();
    const bool stopSwimmingForView = belowMaterial == Material::AIR && viewAngle > 45.0f;
    if ((!isInWater() || stopSwimmingForBlocks || stopSwimmingForView) && mLevel->mIsClientSide && isSwimming())
    {
        stopSwimming();
    }

    if (isSprinting() && (stopForDirection || !foodAllowsSprinting) && !isSwimming())
    {
        setSprinting(false);
    }

    if (!isSleeping() && (mAbilities.mMayfly || isAllowedToFly()))
    {
        if (!mInput->mJumping)
        {
            if (mJumpTriggerTime > 0 && mFlyTogglePending && !isSpectator())
            {
                mAbilities.mFlying = !mAbilities.mFlying;
                onUpdateAbilities();
                mJumpTriggerTime = 0;
                mFlyTogglePending = false;
                if (mAbilities.mFlying)
                    mInput->mSneaking = false;
            }
        }
        else if (!wasJumping && autoJumpTime <= 0)
        {
            if (mJumpTriggerTime == 0)
            {
                mFlyTogglePending = false;
                mJumpTriggerTime = 10;
            }
            else
            {
                mFlyTogglePending = true;
            }
        }
    }
    else if (mAbilities.mFlying)
    {
        mAbilities.mFlying = false;
        onUpdateAbilities();
    }

    bool autoElytraRequested = true;
    if (!mInput->mJumping)
    {
        const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
        autoElytraRequested = miniGame->HasFeatureAutoElytraFly() && mFallDistance > 1.25f;
    }
    if (!isSleeping() && autoElytraRequested && !wasJumping && !mOnGround && mYd < 0.0 && !isFallFlying()
        && !mAbilities.mFlying)
    {
        not_null_ptr<ItemInstance> chestItem = getItemSlot(EquipmentSlot::CHEST);
        if (chestItem->getItem() == Items::elytra && ElytraItem::isFlyEnabled(chestItem))
        {
            boost::shared_ptr<Entity> self = shared_from_this();
            boost::shared_ptr<Packet> packet(
                new ServerboundPlayerCommandPacket(self, ServerboundPlayerCommandPacket::START_FALL_FLYING));
            mPacketListener->send(packet);
        }
    }

    const bool diving = isInWater() && (mButtonsDown & (1LL << EControllerActions_Sneak)) != 0;
    if (mInput->mSwimming)
    {
        if (diving && !mOnGround)
            goDownInWater();
    }
    else if (isInWater() && !mOnGround)
    {
        mInput->mSwimming = false;
        mInput->mSneaking = false;
    }

    const float creativeFlyVerticalSpeed = 0.57f;

    if (mAbilities.mFlying && isControlledCamera())
    {
        if ((mButtonsDown & (1LL << EControllerActions_Sneak)) != 0)
            mYd -= creativeFlyVerticalSpeed;
        if (mInput->mJumping)
        {
            mYd += creativeFlyVerticalSpeed;
            mNoJumpDelay = 0;
        }

        const float yawQuadrant = floorf(mYRot / 90.0f + 0.5f);
        if ((mFlyButtons[0] & 0x800) != 0)
        {
            const float radians = yawQuadrant * 90.0f * 3.1415927f / 180.0f;
            mXd = -0.15 * std::cos(radians);
            mZd = -0.15 * std::sin(radians);
        }
        else if ((mFlyButtons[0] & 0x400) != 0)
        {
            const float radians = yawQuadrant * 90.0f * 3.1415927f / 180.0f;
            mXd = 0.15 * std::cos(radians);
            mZd = 0.15 * std::sin(radians);
        }
    }

    if (!isFallFlying() || !isCreative() || !mInput->mJumping)
    {
        mGlideDistance = 0.0;
    }
    else
    {
        mGlideDistance = creativeFlyVerticalSpeed;
        mNoJumpDelay = 0;
    }

    if (isRidingJumpable())
    {
        boost::shared_ptr<Entity> vehicle = getVehicle();
        boost::shared_ptr<PlayerRideableJumping> rideable = boost::dynamic_pointer_cast<PlayerRideableJumping>(vehicle);

        if (mRideJumpTicks < 0 && ++mRideJumpTicks == 0)
            mRideJumpScale = 0.0f;

        if (wasJumping)
        {
            if (!mInput->mJumping)
            {
                mRideJumpTicks = -10;
                rideable->onPlayerJump(Mth::floor(getJumpRidingScale() * 100.0f));
                sendRidingJump();
            }
            else
            {
                ++mRideJumpTicks;
                if (mRideJumpTicks > 9)
                    mRideJumpScale = 2.0f / static_cast<float>(mRideJumpTicks - 8) * 0.1f + 0.8f;
                else
                    mRideJumpScale = static_cast<float>(mRideJumpTicks) * 0.1f;
            }
        }
        else if (mInput->mJumping)
        {
            mRideJumpTicks = 0;
            mRideJumpScale = 0.0f;
        }
    }
    else
    {
        mRideJumpScale = 0.0f;
    }

    Player::aiStep();

    if (mAbilities.mMayfly || isAllowedToFly() || !mOnGround)
    {
        if (mAbilities.mFlying)
        {
            Vec3* look = getViewVector(1.0f);
            if (!isSprinting() && !isSpectator())
            {
                mFlyMotionX = 0.0f;
                mFlyMotionZ = 0.0f;
                float verticalSpeed = 0.0f;
                if ((mFlyButtons[0] & 0x1000) != 0)
                    verticalSpeed = 0.1f;
                if ((mFlyButtons[0] & 0x2000) != 0)
                    verticalSpeed = -0.1f;
                mFlyMotionY = verticalSpeed;
                Entity::move(eMoverType_Self, 0.0, verticalSpeed, 0.0, false);
            }
            else
            {
                const float forward = mInput->mMoveForward;
                mFlyMotionX = static_cast<float>(look->x) * forward;
                mFlyMotionY = static_cast<float>(look->y) * forward;
                mFlyMotionZ = static_cast<float>(look->z) * forward;
                float speed = static_cast<float>(600 - mSprinting) / 10.0f;
                if (!isSpectator())
                {
                    speed *= speed;
                    if (speed > 1.0f)
                        speed = 1.0f;
                }
                else if (speed > 0.3f)
                {
                    speed = 0.3f;
                }
                mFlyMotionX *= speed;
                mFlyMotionY *= speed;
                mFlyMotionZ *= speed;
                Entity::move(eMoverType_Self, mFlyMotionX, mFlyMotionY, mFlyMotionZ, false);
            }

            mFallDistance = 0.0f;
            mYd = 0.0;
            mOnGround = true;
        }
    }
    else if (mAbilities.mFlying)
    {
        mAbilities.mFlying = false;
    }

    if (!mPresenceIdleSent)
    {
        if (g_pInput->GetIdleSeconds(mUserIndex) > 300.0)
        {
            gProfile->SetCurrentGameActivity(mUserIndex, 0, false);
            mPresenceIdleSent = true;
        }
    }
    else if (g_pInput->GetIdleSeconds(mUserIndex) < 300.0)
    {
        const int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
        const bool localGame = CGameNetworkManager::sInstance.IsLocalGame();
        if (playerCount < 2)
        {
            if (!localGame)
                gProfile->SetCurrentGameActivity(mUserIndex, 4, false);
            else
                gProfile->SetCurrentGameActivity(mUserIndex, 5, false);
            updateRichPresence();
        }
        else
        {
            if (!localGame)
                gProfile->SetCurrentGameActivity(mUserIndex, 2, false);
            else
                gProfile->SetCurrentGameActivity(mUserIndex, 3, false);
            updateRichPresence();
        }
        mPresenceIdleSent = false;
    }

    mFallFlyYawOld = mFallFlyYaw;
    mFallFlyingAlignmentO = mFallFlyingAlignment;
    if (isFallFlying())
    {
        Vec3* look = getViewVector(0.0f);
        const double horizontalMotion = mXd * mXd + mZd * mZd;
        const double horizontalLook = look->x * look->x + look->z * look->z;
        if (horizontalMotion > 0.01f && horizontalLook > 0.01f)
        {
            const double motionLength = Mth::sqrt(horizontalMotion);
            const double lookLength = Mth::sqrt(horizontalLook);
            const double lengthProduct = static_cast<double>(static_cast<float>(motionLength * lookLength));
            const double dot = (mXd * look->x + mZd * look->z) / lengthProduct;
            const double aligned = Mth::clamp(dot, 0.0, 1.0);
            const double cosine = Mth::clamp(dot, -1.0, 1.0);
            const double cross = mXd * look->z - mZd * look->x;
            const double sign = (0.0 < cross) - (cross < 0.0);
            mFallFlyingAlignment = static_cast<float>(aligned);
            const float signedAngle = static_cast<float>(sign * std::acos(cosine));
            mFallFlyYaw = signedAngle * 180.0f / 3.1415927f;
        }
        else
        {
            mFallFlyYaw = 0.0f;
        }
    }
    else
    {
        mFallFlyYaw = 0.0f;
    }
}

MobEffectInstance* LocalPlayer::removeEffectNoUpdate(MobEffect* effect)
{
    if (effect == MobEffects::CONFUSION)
    {
        mPortalEffectIntensityO = 0.0f;
        mPortalEffectIntensity = 0.0f;
    }
    return LivingEntity::removeEffectNoUpdate(effect);
}

void LocalPlayer::move(MoverType type, double xa, double ya, double za, bool collide)
{
    double xOld = mX;
    double zOld = mZ;
    Entity::move(type, xa, ya, za, collide);
    updateAutoJump(static_cast<float>(mX - xOld), static_cast<float>(mZ - zOld));
}

int LocalPlayer::openFireworks(int x, int y, int z)
{
    bool result = gApp.LoadFireworksMenu(mUserIndex, boost::static_pointer_cast<LocalPlayer>(shared_from_this()), x, y,
                                         z);
    if (result)
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    return result;
}

void LocalPlayer::displayClientMessage(int message, bool force)
{
    mMinecraft->mGui->displayClientMessage(message, mUserIndex);
}

void LocalPlayer::awardStat(Stat* stat, arrayWithLength<unsigned char> params)
{
    if (mUserIndex < 0)
        return;

    bool canRecord = true;
    if (!stat->isAwardedFromServer())
        canRecord = CanRecordStats(stat, params) != 0;
    bool hasProfile = true;
    if (gProfile->GetPrimaryPad() != mUserIndex && !gProfile->IsHasAchievementsFile(mUserIndex))
        hasProfile = false;
    CommonStats::readParam(params);
    unsigned int count = CommonStats::readParam(params);
    delete[] params.data;
    if (!(hasProfile & canRecord))
        return;

    static Stat* timePlayed = gStats->get_timePlayed();
    if (stat == timePlayed)
    {
        StatsCounter* stats = mMinecraft->mStats[mUserIndex];
        stats->award(stat, static_cast<StatsCounter::ESubTypes>(mLevel->getDifficulty()->getId()), count, true);
        if (stats->getTotalValue(stat) >= 2400000)
            awardStat(GenericStats::passingTheTime(), GenericStats::param_passingTheTime());
        return;
    }

    if (stat->isAchievement())
    {
        int achievementId = static_cast<Achievement*>(stat)->getAchievementID();
        if (gProfile->CanBeAwarded(mUserIndex, achievementId))
        {
            telemetryManagerInstance->RecordAchievementUnlocked(mUserIndex, achievementId, 0);
            if (CGameNetworkManager::sInstance.IsLocalGame() && CGameNetworkManager::sInstance.GetPlayerCount() == 1
                && gProfile->GetAwardType(achievementId))
            {
                gConsoleUIController.CloseUIScenes(mUserIndex, false);
                gConsoleUIController.NavigateToScene(mUserIndex, EUIScene_PauseMenu, NULL, EUILayer_Scene,
                                                     EUIGroup_PlayerDefault);
            }
        }

        unsigned char mask = static_cast<unsigned char>(1 << (achievementId & 7));
        unsigned char& awarded = mAwardedAchievements[achievementId >> 3];
        if ((awarded & mask) == 0)
        {
            gProfile->Award(mUserIndex, achievementId, false);
            if (gProfile->IsFullVersion())
                awarded |= mask;
        }
        mMinecraft->mStats[mUserIndex]->award(
            stat, static_cast<StatsCounter::ESubTypes>(mLevel->getDifficulty()->getId()), count, true);
        return;
    }

    StatsCounter* stats = mMinecraft->mStats[mUserIndex];
    stats->award(stat, static_cast<StatsCounter::ESubTypes>(mLevel->getDifficulty()->getId()), count, true);

    if (stat == gStats->get_tamedEntity(EntityType::WOLF))
    {
        if (stats->getTotalValue(gStats->get_tamedEntity(EntityType::WOLF)) > 4)
            awardStat(GenericStats::leaderOfThePack(), GenericStats::param_noArgs());
    }

    Stat* toolStats[4][5] = {
        {gStats->get_itemsCrafted(Item::VALIDgetId(Items::woodenShovel)),
         gStats->get_itemsCrafted(Item::VALIDgetId(Items::stoneShovel)),
         gStats->get_itemsCrafted(Item::VALIDgetId(Items::ironShovel)),
         gStats->get_itemsCrafted(Item::VALIDgetId(Items::diamondShovel)),
         gStats->get_itemsCrafted(Item::VALIDgetId(Items::goldenShovel))},
        {gStats->get_itemsCrafted(Item::VALIDgetId(Items::woodenPickaxe)),
         gStats->get_itemsCrafted(Item::VALIDgetId(Items::stonePickaxe)),
         gStats->get_itemsCrafted(Item::VALIDgetId(Items::ironPickaxe)),
         gStats->get_itemsCrafted(Item::VALIDgetId(Items::diamondPickaxe)),
         gStats->get_itemsCrafted(Item::VALIDgetId(Items::goldenPickaxe))},
        {gStats->get_itemsCrafted(Item::VALIDgetId(Items::woodenAxe)),
         gStats->get_itemsCrafted(Item::VALIDgetId(Items::stoneAxe)),
         gStats->get_itemsCrafted(Item::VALIDgetId(Items::ironAxe)),
         gStats->get_itemsCrafted(Item::VALIDgetId(Items::diamondAxe)),
         gStats->get_itemsCrafted(Item::VALIDgetId(Items::goldenAxe))},
        {gStats->get_itemsCrafted(Item::VALIDgetId(Items::woodenHoe)),
         gStats->get_itemsCrafted(Item::VALIDgetId(Items::stoneHoe)),
         gStats->get_itemsCrafted(Item::VALIDgetId(Items::ironHoe)),
         gStats->get_itemsCrafted(Item::VALIDgetId(Items::diamondHoe)),
         gStats->get_itemsCrafted(Item::VALIDgetId(Items::goldenHoe))},
    };
    bool craftedTool = false;
    for (int tool = 0; tool < 4; ++tool)
    {
        if (stat == toolStats[tool][0] || stat == toolStats[tool][1] || stat == toolStats[tool][2]
            || stat == toolStats[tool][3] || stat == toolStats[tool][4])
            craftedTool = true;
    }
    if (craftedTool)
    {
        bool craftedEveryTool = true;
        for (int tool = 0; tool < 4; ++tool)
        {
            if (stats->getTotalValue(toolStats[tool][0]) == 0 && stats->getTotalValue(toolStats[tool][1]) == 0
                && stats->getTotalValue(toolStats[tool][2]) == 0 && stats->getTotalValue(toolStats[tool][3]) == 0
                && stats->getTotalValue(toolStats[tool][4]) == 0)
            {
                craftedEveryTool = false;
                break;
            }
        }
        if (craftedEveryTool)
            awardStat(GenericStats::MOARTools(), GenericStats::param_noArgs());
    }

    static Stat* cookedPorkchopCrafted = gStats->get_itemsCrafted(Item::VALIDgetId(Items::cookedPorkchop));
    static Stat* cookedPorkchopUsed = gStats->get_itemsUsed(Item::VALIDgetId(Items::cookedPorkchop));
    if (stat == cookedPorkchopCrafted || stat == cookedPorkchopUsed)
    {
        if (stats->getTotalValue(cookedPorkchopCrafted) > 0 && stats->getTotalValue(cookedPorkchopUsed) > 0)
            awardStat(GenericStats::porkChop(), GenericStats::param_porkChop());
    }

    static Stat* emeraldOreMined = gStats->get_blocksMined(Block::getId(Blocks::emerald_ore));
    static Stat* emeraldsBought = gStats->get_itemsBought(Item::VALIDgetId(Items::emerald));
    if (stat == emeraldOreMined || stat == emeraldsBought)
    {
        if (stats->getTotalValue(emeraldOreMined) + stats->getTotalValue(emeraldsBought) >= 30)
            awardStat(GenericStats::theHaggler(), GenericStats::param_theHaggler());
    }

    static Stat* flowerPotCrafted = gStats->get_itemsCrafted(Item::VALIDgetId(Items::flowerPot));
    static Stat* flowerPotPlaced = gStats->get_blocksPlaced(Block::getId(Blocks::flower_pot));
    if (stat == flowerPotCrafted || stat == flowerPotPlaced)
    {
        if (stats->getTotalValue(flowerPotCrafted) != 0 && stats->getTotalValue(flowerPotPlaced) != 0)
            awardStat(GenericStats::potPlanter(), GenericStats::param_potPlanter());
    }

    static Stat* signCrafted = gStats->get_itemsCrafted(Item::VALIDgetId(Items::sign));
    static Stat* wallSignPlaced = gStats->get_blocksPlaced(Block::getId(Blocks::wall_sign));
    static Stat* standingSignPlaced = gStats->get_blocksPlaced(Block::getId(Blocks::standing_sign));
    if (stat == signCrafted || stat == wallSignPlaced || stat == standingSignPlaced)
    {
        if (stats->getTotalValue(signCrafted) > 0
            && stats->getTotalValue(wallSignPlaced) + stats->getTotalValue(standingSignPlaced) > 0)
            awardStat(GenericStats::itsASign(), GenericStats::param_itsASign());
    }

    static Stat* helmetDyed = gStats->get_armourDyed(Item::VALIDgetId(Items::leatherHelmet));
    static Stat* chestplateDyed = gStats->get_armourDyed(Item::VALIDgetId(Items::leatherChestplate));
    static Stat* leggingsDyed = gStats->get_armourDyed(Item::VALIDgetId(Items::leatherLeggings));
    static Stat* bootsDyed = gStats->get_armourDyed(Item::VALIDgetId(Items::leatherBoots));
    if (stat == helmetDyed || stat == chestplateDyed || stat == leggingsDyed || stat == bootsDyed)
    {
        unsigned char dyedPieces = (stats->getTotalValue(helmetDyed) != 0) + (stats->getTotalValue(chestplateDyed) != 0)
                                   + (stats->getTotalValue(leggingsDyed) != 0) + (stats->getTotalValue(bootsDyed) != 0);
        if (dyedPieces == 4)
            awardStat(GenericStats::tieDyeOutfit(), GenericStats::param_noArgs());
    }

    bool woolStat = false;
    for (int colour = 0; colour < 16; ++colour)
    {
        if (stat == gStats->get_itemsCollected(Item::VALIDgetId(Item::byBlock(Blocks::wool)), colour))
            woolStat = true;
    }
    if (woolStat)
    {
        unsigned int colours = 0;
        for (int colour = 0; colour < 16; ++colour)
        {
            if (stats->getTotalValue(gStats->get_itemsCollected(Item::VALIDgetId(Item::byBlock(Blocks::wool)), colour))
                != 0)
                ++colours;
        }
        if (colours >= 16)
            awardStat(GenericStats::rainbowCollection(), GenericStats::param_rainbowCollection());
    }

    bool biomeStat = false;
    for (int biome = 0; biome < 40; ++biome)
    {
        if (stat == gStats->get_enteredBiome(biome))
            biomeStat = true;
    }
    if (biomeStat)
    {
        unsigned int biomes = 0;
        for (int biome = 0; biome < 40; ++biome)
        {
            if (stats->getTotalValue(gStats->get_enteredBiome(biome)) != 0)
                ++biomes;
        }
        if (biomes >= 17)
            awardStat(GenericStats::adventuringTime(), GenericStats::param_adventuringTime());
    }
}

int LocalPlayer::GetThirdPersonView(bool firstPerson)
{
    if (IsSpectatingOtherPlayer() && !firstPerson)
    {
        return 1;
    }
    if (mDRCTouching)
    {
        if (isAlive())
        {
            return 2;
        }
    }
    return mThirdPersonView;
}

void LocalPlayer::SetThirdPersonView(int view)
{
    mThirdPersonView = view;
}

unsigned int LocalPlayer::CanChangeThirdPersonView()
{
    if (mThirdPersonViewTimer < 7 && mThirdPersonViewTimer != 0)
    {
        return 0;
    }
    return 1;
}

void LocalPlayer::onCrafted(not_null_ptr<ItemInstance> item)
{
    if (item->getItem()->IsArmor())
    {
        if (static_cast<ArmorItem*>(item->getItem())->hasCustomColor(item))
            awardStat(GenericStats::armourDyed(item->getItem()), GenericStats::param_noArgs());
    }

    if (mMinecraft->mGameModes[mUserIndex] != NULL)
    {
        Tutorial* tutorial = mMinecraft->mGameModes[mUserIndex]->getTutorial();
        if (tutorial != NULL)
            tutorial->onCrafted(item);
    }
}

void LocalPlayer::handleCollectItem(not_null_ptr<ItemInstance> item)
{
    if (!item->isEmpty())
    {
        unsigned int itemCount = 0;
        unsigned int auxValueCount = 0;
        for (unsigned int i = 0; i < mInventory->m_items.size(); ++i)
        {
            not_null_ptr<ItemInstance> slot = mInventory->m_items[i];
            if (!slot->isEmpty() && slot->getItem() == item->getItem())
            {
                int count = slot->getCount();
                itemCount += count;
                if (slot->getAuxValue() == item->getAuxValue())
                    auxValueCount += count;
            }
        }
        Tutorial* tutorial = mMinecraft->mGameModes[mUserIndex]->getTutorial();
        if (tutorial != NULL)
            tutorial->onTake(item, itemCount, auxValueCount);
    }
    if (gConsoleUIController.IsContainerMenuDisplayed(mUserIndex))
        gConsoleUIController.HandleInventoryUpdated(mUserIndex);
}

void LocalPlayer::SpectatePlayerNext()
{
    GetLocalGameMode()->Follow(true, false);
}

void LocalPlayer::SpectatePlayerPrev()
{
    GetLocalGameMode()->Follow(true, true);
}

void LocalPlayer::StopSpectatingPlayer()
{
    ClientMasterGameMode* masterGameMode = mMinecraft->GetClientMasterGameMode();
    if (!IsSpectatingOtherPlayer())
        return;
    if (masterGameMode->IsRoundEnding())
        return;

    GetLocalGameMode()->Follow(false, false);
    SetCamera(boost::shared_ptr<Entity>());
    if (SpectateAsOtherEntity() && mMinecraft->mGameModes[mUserIndex]->getTutorial() != NULL)
        mMinecraft->mGameModes[mUserIndex]->getTutorial()->onGameModeChange(eTutorial_GameMode_SpectatingAsMob);
}

const GameType* LocalPlayer::GetGameType()
{
    GameMode* gameMode = GetGameMode();
    return gameMode == 0 ? 0 : gameMode->GetGameType();
}

void LocalPlayer::SetGameType(const GameType* gameType)
{
    GameMode* gameMode = GetGameMode();
    if (gameMode != NULL)
    {
        gameMode->SetGameType(gameType);
        Tooltips::ResetAllTooltips(mUserIndex);
    }
}

unsigned long long LocalPlayer::GetType()
{
    return EntityType::LOCAL_PLAYER;
}

bool LocalPlayer::isHandsBusy()
{
    return mHandsBusy;
}

void LocalPlayer::SetSessionTimerStart()
{
    float now = static_cast<float>(gApp.getAppTime());
    mSessionTimerStart = now;
    mChangeDimensionTimerStart = now;
}

bool LocalPlayer::IsWaitingForScoreboard()
{
    return mWaitingForScoreboard;
}

bool LocalPlayer::doesLevelAllowBlockToBreak(const BlockPos& pos)
{
    const bool outsideNether = mLevel->mDimension->getType() != DimensionType::NETHER;
    const bool inEnd = mLevel->mDimension->getType() == DimensionType::THE_END;
    return (inEnd || pos.y > 0) && (outsideNether || pos.y < 127);
}

unsigned int LocalPlayer::pickBlock(bool select)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    HitResult* hitResult = minecraft->mHitResult;
    if (hitResult == NULL)
        return 0;
    if (hitResult->mType == HitResult::MISS)
        return 0;

    not_null_ptr<ItemInstance> pickedItem;
    boost::shared_ptr<BlockEntity> blockEntity;

    if (hitResult->mType == HitResult::BLOCK)
    {
        const BlockPos pos = hitResult->getBlockPos();
        const BlockState* state = mLevel->getBlockState(pos);
        Block* block = state->getBlock();
        if (state->getMaterial() == Material::AIR)
            return 0;
        pickedItem = block->getCloneItemInstance(mLevel, pos, state);
    }
    else if (hitResult->mType == HitResult::ENTITY && hitResult->mEntity)
    {
        if (eTYPE_DERIVED_FROM(EntityType::PAINTING, hitResult->mEntity->GetType()))
        {
            pickedItem = not_null_ptr<ItemInstance>(new ItemInstance(Items::painting));
        }
        else if (eTYPE_DERIVED_FROM(EntityType::LEASH_FENCE_KNOT_ENTITY, hitResult->mEntity->GetType()))
        {
            pickedItem = not_null_ptr<ItemInstance>(new ItemInstance(Items::lead));
        }
        else if (eTYPE_DERIVED_FROM(EntityType::ITEM_FRAME, hitResult->mEntity->GetType()))
        {
            boost::shared_ptr<ItemFrame> itemFrame = boost::static_pointer_cast<ItemFrame>(hitResult->mEntity);
            not_null_ptr<ItemInstance> frameItem = itemFrame->getItem();
            if (!frameItem->isEmpty())
                pickedItem = frameItem->copy();
            else
                pickedItem = not_null_ptr<ItemInstance>(new ItemInstance(Items::itemFrame));
        }
        else if (eTYPE_DERIVED_FROM(EntityType::MINECART, hitResult->mEntity->GetType()))
        {
            boost::shared_ptr<Minecart> minecart = boost::dynamic_pointer_cast<Minecart>(hitResult->mEntity);
            ItemInstance* minecartItem;
            switch (minecart->getType()->mId)
            {
            case Minecart::Type::eType_Chest:
                minecartItem = new ItemInstance(Items::chestMinecart);
                break;
            case Minecart::Type::eType_Furnace:
                minecartItem = new ItemInstance(Items::furnaceMinecart);
                break;
            case Minecart::Type::eType_TNT:
                minecartItem = new ItemInstance(Items::tntMinecart);
                break;
            case Minecart::Type::eType_Hopper:
                minecartItem = new ItemInstance(Items::hopperMinecart);
                break;
            case Minecart::Type::eType_CommandBlock:
                minecartItem = new ItemInstance(Items::commandBlockMinecart);
                break;
            default:
                minecartItem = new ItemInstance(Items::minecart);
                break;
            }
            pickedItem = not_null_ptr<ItemInstance>(minecartItem);
        }
        else if (eTYPE_DERIVED_FROM(EntityType::BOAT, hitResult->mEntity->GetType()))
        {
            boost::shared_ptr<Boat> boat = boost::static_pointer_cast<Boat>(hitResult->mEntity);
            pickedItem = not_null_ptr<ItemInstance>(new ItemInstance(boat->getDropItem()));
        }
        else if (eTYPE_DERIVED_FROM(EntityType::ARMOR_STAND, hitResult->mEntity->GetType()))
        {
            pickedItem = not_null_ptr<ItemInstance>(new ItemInstance(Items::armorStand));
        }
        else if (eTYPE_DERIVED_FROM(EntityType::ENDER_CRYSTAL, hitResult->mEntity->GetType()))
        {
            pickedItem = not_null_ptr<ItemInstance>(new ItemInstance(Items::endCrystal));
        }
        else
        {
            const ResourceLocation key = EntityIO::getKey(hitResult->mEntity);
            if (EntityIO::sSpawnInfo.find(key) == EntityIO::sSpawnInfo.end())
                return 0;
            pickedItem = not_null_ptr<ItemInstance>(new ItemInstance(Items::spawnEgg));
            SpawnEggItem::addSpawnEntity(pickedItem, key);
        }
    }
    else
    {
        return 0;
    }

    if (pickedItem->isEmpty())
        return 0;

    if (blockEntity && boost::dynamic_pointer_cast<FlowerPotBlockEntity>(blockEntity))
        blockEntity = boost::shared_ptr<BlockEntity>();

    boost::shared_ptr<Inventory> inventory = mInventory;
    if (blockEntity)
        Minecraft::addCustomNbtData(pickedItem, blockEntity);

    if (!select)
    {
        inventory->setPickedItem(pickedItem);
        minecraft->mGameMode->handlePickItem(pickedItem);
        return 1;
    }

    return mAbilities.mInstabuild || inventory->findSlotMatchingItem(pickedItem, false) != -1;
}

unsigned char LocalPlayer::handleMouseClick(int button)
{
    boost::shared_ptr<LocalPlayer> player = boost::static_pointer_cast<LocalPlayer>(shared_from_this());
    HitResult* hitResult = mMinecraft->mHitResult;
    MultiPlayerGameMode* gameMode = mMinecraft->mGameModes[mUserIndex];
    const InteractionHand::EInteractionHand hand = InteractionHand::eInteractionHand_MainHand;

    if (!isUsingItem())
    {
        if (button == 0)
        {
            if (mMissTime > 0)
                return 0;

            if (hitResult == NULL)
            {
                if (gameMode->hasMissTime())
                    mMissTime = 10;
                player->resetAttackStrengthTicker();
                if (swing(hand))
                {
                    playSound(SoundEvents::PLAYER_ATTACK_WEAK, 0.6f, 1.0f);
                    player->mPacketListener->send(boost::shared_ptr<Packet>(
                        new ClientboundSoundPacket(SoundEvents::PLAYER_ATTACK_WEAK, SoundSource::eSoundSource_Players,
                                                   player->mX, player->mY, player->mZ, 0.6f, 1.0f)));
                }
                return 0;
            }

            if (isHandsBusy())
                return 0;

            if (isSleeping() && mLevel != NULL && mLevel->mIsClientSide)
                return 0;

            switch (hitResult->mType)
            {
            case HitResult::MISS:
                if (gameMode->hasMissTime())
                    mMissTime = 10;
                player->resetAttackStrengthTicker();
                if (swing(hand))
                {
                    playSound(SoundEvents::PLAYER_ATTACK_WEAK, 0.6f, 1.0f);
                    player->mPacketListener->send(boost::shared_ptr<Packet>(
                        new ClientboundSoundPacket(SoundEvents::PLAYER_ATTACK_WEAK, SoundSource::eSoundSource_Players,
                                                   player->mX, player->mY, player->mZ, 0.6f, 1.0f)));
                }
                break;
            case HitResult::ENTITY:
                gameMode->attack(player, hitResult->mEntity);
                swing(hand);
                break;
            case HitResult::BLOCK:
            {
                const BlockPos pos = hitResult->getBlockPos();
                if (doesLevelAllowBlockToBreak(pos))
                {
                    gameMode->startDestroyBlock(pos, hitResult->mDirection);
                    swing(hand);
                }
                else
                {
                    if (gameMode->hasMissTime())
                        mMissTime = 10;
                    player->resetAttackStrengthTicker();
                    swing(hand);
                }
                break;
            }
            }
        }
        else if (button == 1)
        {
            mRightClickDelay = 4;
            if (isHandsBusy())
                return 0;

            if (isSleeping() && mLevel != NULL && mLevel->mIsClientSide)
            {
                if (mCreativeRepeatState == 4)
                    return 0;
                boost::shared_ptr<MultiplayerLocalPlayer> multiplayerPlayer =
                    boost::dynamic_pointer_cast<MultiplayerLocalPlayer>(shared_from_this());
                if (multiplayerPlayer && multiplayerPlayer->mPacketListener != NULL)
                    multiplayerPlayer->StopSleeping();
            }
            if (isSleeping() && mLevel != NULL && mLevel->mIsClientSide)
                return 0;

            if (Minecraft::InMiniGame(eMiniGameId_Glide, false))
            {
                const MiniGameDef* miniGame = mMinecraft->GetMiniGame();
                if (miniGame->ArePowerupsActive())
                {
                    boost::shared_ptr<MultiplayerLocalPlayer> multiplayerPlayer =
                        boost::dynamic_pointer_cast<MultiplayerLocalPlayer>(shared_from_this());
                    if (multiplayerPlayer)
                    {
                        not_null_ptr<ItemInstance> item = player->getMainHandItem();
                        if (!item->isEmpty())
                        {
                            PowerupItems::UsePowerupItem(multiplayerPlayer, Item::getId(item->getItem()));
                            boost::shared_ptr<ServerboundUseItemPacket> packet(new ServerboundUseItemPacket(hand));
                            multiplayerPlayer->mPacketListener->send(packet);
                            item->shrink(1);
                            if (item->getCount() == 0)
                                mMinecraft->mPlayer->setItemInHand(hand, ItemInstance::EMPTY);
                        }
                        return 1;
                    }
                }
                return 0;
            }

            for (int handIndex = 0; handIndex < 1; ++handIndex)
            {
                const InteractionHand::EInteractionHand useHand = static_cast<InteractionHand::EInteractionHand>(
                    handIndex);
                not_null_ptr<ItemInstance> heldItem = player->getItemInHand(useHand);

                if (hitResult != NULL)
                {
                    switch (hitResult->mType)
                    {
                    case HitResult::ENTITY:
                        if (gameMode->interact(player, hitResult->mEntity, player->getItemInHand(useHand), useHand)
                            == InteractionResult::eInteractionResult_Success)
                            return 0;
                        if (gameMode->interactAt(player, hitResult->mEntity, hitResult, player->getItemInHand(useHand),
                                                 useHand)
                            == InteractionResult::eInteractionResult_Success)
                            return 0;
                        break;
                    case HitResult::BLOCK:
                    {
                        const BlockPos pos = hitResult->getBlockPos();
                        if (mLevel->getBlockState(pos)->getMaterial() != Material::AIR)
                        {
                            bool used = false;
                            const int oldCount = heldItem->isEmpty() ? 0 : heldItem->getCount();
                            const InteractionResult::EInteractionResult result = gameMode->useItemOn(
                                player, mLevel, pos, hitResult->mDirection, hitResult->mLocation, useHand, false,
                                &used);
                            if (result == InteractionResult::eInteractionResult_Success)
                            {
                                swing(useHand);
                                if (!heldItem->isEmpty())
                                {
                                    if (heldItem->getCount() == 0)
                                        mMinecraft->mPlayer->setItemInHand(useHand, ItemInstance::EMPTY);
                                    else if (heldItem->getCount() != oldCount || gameMode->hasInfiniteItems())
                                        mMinecraft->mGameRenderer->mItemInHandRenderer->itemUsed(useHand);
                                }
                                return used;
                            }
                        }
                        break;
                    }
                    }
                }

                not_null_ptr<ItemInstance> item = getItemInHand(useHand);
                if (!item->isEmpty())
                {
                    if (gameMode->useItem(player, mLevel, useHand, false)
                        == InteractionResult::eInteractionResult_Success)
                    {
                        mMinecraft->mGameRenderer->mItemInHandRenderer->itemUsed(useHand);
                        return 0;
                    }
                }
            }
        }
        else if (button == 2 && !Minecraft::InMiniGame(eMiniGameId_Any, true))
        {
            pickBlock(false);
        }
    }
    return 0;
}

unsigned int LocalPlayer::creativeModeHandleMouseClick(int button, bool pressed)
{
    if (!pressed)
    {
        mCreativeRepeatState = 0;
        return 0;
    }

    if (mCreativeRepeatState == 4)
    {
        return 0;
    }

    if (mCreativeRepeatState == 0)
    {
        mCreativeMouseStartX = (float)mX;
        mCreativeMouseStartY = (float)mY;
        mCreativeMouseStartZ = (float)mZ;

        unsigned char handled = handleMouseClick(button);

        if (!isSprinting() && !isPassenger() && !isSleeping())
        {
            if (!handled)
            {
                mCreativeRepeatState = 4;
                return 1;
            }

            mCreativeMouseRepeatDelay = 0.0f;
            mCreativeRepeatState = 1;
            return 1;
        }

        mCreativeRepeatState = 5;
        return 1;
    }

    if (mCreativeRepeatState == 5)
    {
        return 0;
    }

    if (isSprinting())
    {
        mCreativeRepeatState = 5;
        return 0;
    }

    float deltaX = (float)mX - mCreativeMouseStartX;
    float deltaY = (float)mY - mCreativeMouseStartY;
    float deltaZ = (float)mZ - mCreativeMouseStartZ;
    float changeX = deltaX - mCreativeMouseDeltaX;
    float changeY = deltaY - mCreativeMouseDeltaY;
    float changeZ = deltaZ - mCreativeMouseDeltaZ;

    if (mCreativeRepeatState == 2)
    {
        if (sqrtf(changeX * changeX + changeY * changeY + changeZ * changeZ) < 0.01f)
        {
            mCreativeMouseRepeatDelay = 0.0f;
            mCreativeRepeatState = 3;
        }
    }
    else if (mCreativeRepeatState == 3)
    {
        if (sqrtf(changeX * changeX + changeY * changeY + changeZ * changeZ) >= 0.01f)
        {
            mCreativeMouseRepeatDelay = 0.0f;
            mCreativeRepeatState = 2;
        }
        else
        {
            mCreativeMouseRepeatDelay += 0.1f;

            if (mCreativeMouseRepeatDelay > 0.7f)
            {
                mCreativeMouseRepeatDelay = 0.0f;
                mCreativeRepeatState = 1;
            }
        }
    }

    mCreativeMouseDeltaX = deltaX;
    mCreativeMouseDeltaY = deltaY;
    mCreativeMouseDeltaZ = deltaZ;

    if (fabs(deltaX) >= 1.0f)
    {
        mCreativeMouseStartX += deltaX < 0.0f ? ceilf(deltaX) : floorf(deltaX);
        mCreativeMouseStartY += deltaY;
        mCreativeMouseStartZ += deltaZ;
    }
    else if (fabs(deltaY) >= 1.0f)
    {
        mCreativeMouseStartX += deltaX;
        mCreativeMouseStartY += deltaY < 0.0f ? ceilf(deltaY) : floorf(deltaY);
        mCreativeMouseStartZ += deltaZ;
    }
    else if (fabs(deltaZ) >= 1.0f)
    {
        mCreativeMouseStartX += deltaX;
        mCreativeMouseStartY += deltaY;
        mCreativeMouseStartZ += deltaZ < 0.0f ? ceilf(deltaZ) : floorf(deltaZ);
    }
    else
    {
        if (mCreativeMouseRepeatDelay <= 0.0f)
        {
            return 0;
        }

        float remaining = 1.0f - mCreativeMouseRepeatDelay;

        if (fabs(deltaY) < remaining && fabs(deltaZ) < remaining && fabs(deltaX) < remaining)
        {
            return 0;
        }

        mCreativeMouseStartX += deltaX;
        mCreativeMouseStartY += deltaY;
        mCreativeMouseStartZ += deltaZ;
    }

    double savedX = mX;
    double savedY = mY;
    double savedZ = mZ;

    mX = mCreativeMouseStartX;
    mY = mCreativeMouseStartY;
    mZ = mCreativeMouseStartZ;

    mMinecraft->mGameRenderer->pick(1.0f);

    mX = savedX;
    mY = savedY;
    mZ = savedZ;

    handleMouseClick(button);

    mCreativeMouseRepeatDelay = 0.0f;
    mCreativeRepeatState = mCreativeRepeatState == 3 ? 1 : 2;

    return 0;
}

void LocalPlayer::handleMouseDown(int button, bool pressed)
{
    if (button == 0 && pressed)
    {
        GetGameMode()->OnMouseDown();
    }

    if (isSpectator())
    {
        if (!Minecraft::GetInstance()->GetMiniGame()->HasSpectatorParticipation())
        {
            return;
        }
    }

    if (!isSleeping() || mLevel == 0 || !mLevel->mIsClientSide)
    {
        if (!pressed)
        {
            mMissTime = 0;
        }

        if (button != 0 || mMissTime < 1)
        {
            if (pressed)
            {
                if (mMinecraft->mHitResult == 0 || mMinecraft->mHitResult->mType != HitResult::BLOCK || button != 0)
                {
                    mMinecraft->mGameMode->stopDestroyBlock();
                    return;
                }

                if (!isUsingItem())
                {
                    BlockPos pos = mMinecraft->mHitResult->getBlockPos();
                    if (!doesLevelAllowBlockToBreak(pos))
                    {
                        return;
                    }
                    if (!mMinecraft->mGameMode->continueDestroyBlock(pos, mMinecraft->mHitResult->mDirection))
                    {
                        return;
                    }
                    mMinecraft->mParticleEngine->crack(pos, mMinecraft->mHitResult->mDirection);
                    swing(InteractionHand::eInteractionHand_MainHand);
                    return;
                }
            }
            mMinecraft->mGameMode->stopDestroyBlock();
        }
    }
    return;
}

void LocalPlayer::SetWaitingForScoreboard(bool waiting)
{
    mWaitingForScoreboard = waiting;
}

void LocalPlayer::updateRichPresence()
{
    if (mUserIndex == -1)
        return;

    not_null_ptr<ItemInstance> selected = mInventory->getSelected();
    if (!selected->isEmpty() && Item::getId(selected->getItem()) == ItemId::FISHING_ROD)
    {
        gApp.SetRichPresenceContext(mUserIndex, 4);
    }
    else if (!selected->isEmpty() && Item::getId(selected->getItem()) == ItemId::FILLED_MAP)
    {
        gApp.SetRichPresenceContext(mUserIndex, 9);
    }
    else
    {
        if (isPassenger() && eTYPE_DERIVED_FROM(EntityType::MINECART, getVehicle()->GetType()))
        {
            gApp.SetRichPresenceContext(mUserIndex, 2);
        }
        else if (isPassenger() && eTYPE_DERIVED_FROM(EntityType::BOAT, getVehicle()->GetType()))
        {
            gApp.SetRichPresenceContext(mUserIndex, 3);
        }
        else if (isPassenger() && eTYPE_DERIVED_FROM(EntityType::PIG, getVehicle()->GetType()))
        {
            gApp.SetRichPresenceContext(mUserIndex, 1);
        }
        else if (mDimension == -1)
        {
            gApp.SetRichPresenceContext(mUserIndex, 7);
        }
        else
        {
            int context = 0;
            if (mMinecraft->mSoundEngine->GetIsPlayingStreamingCDMusic())
                context = 8;
            gApp.SetRichPresenceContext(mUserIndex, context);
        }
    }
}

void LocalPlayer::ResetPercentageTraversed()
{
    mPercentageTraversed = -DBL_MAX;
}

void LocalPlayer::heal(float amount)
{
}

bool LocalPlayer::isInvisible()
{
    return Entity::isInvisible() || mRemoved;
}

bool LocalPlayer::canMoveOrBeMovedFromTo(double a, double b, double c, double d, double e, double f, Vec3* result)
{
    if (!mMinecraft->mGameModes[mUserIndex]->getGameModeConstraints()->canMoveToPosition(a, b, c, d, e, f, result))
    {
        if (!gApp.DebugDevToolsOn() && mLevel->mIsClientSide)
        {
            if (!mLevel->reallyHasChunk(Mth::floor(d * 0.0625), Mth::floor(f * 0.0625)))
                return false;
        }
        return true;
    }
    return false;
}

int LocalPlayer::getIndicatorSegment(float x, float z)
{
    Vec3* view = getViewVector(1.0f);
    double angle = std::atan2(view->x * z - view->z * x, view->x * x + view->z * z);
    int segment = static_cast<int>(Math::round((angle * Mth::RAD_TO_DEG + 180.0) / 45.0));
    return segment % 8;
}

void LocalPlayer::updateProximityIndicator()
{
    if (gApp.GetFirstMiniGameType() == eMiniGameId_Glide && gApp.getLevelGenerationOptions() != NULL
        && gApp.getLevelGenerationOptions()->getRequiredGameRules() != NULL && isFallFlying())
    {
        mElytraProximityWarning = false;
        AABB* searchBox = mBoundingBox->grow(2.0);
        std::vector<AABB*>* collisions = mLevel->getCollisionAABBs(shared_from_this(), searchBox, true, false, false);
        if (!collisions->empty())
        {
            AABB* closestBox = NULL;
            for (std::vector<AABB*>::iterator end = collisions->end(), it = collisions->begin(); it != end; ++it)
            {
                AABB* box = *it;
                double distance = getPos(1.0f)->distanceTo(box);
                if (distance <= mElytraCollisionDistance || closestBox == NULL)
                {
                    mElytraCollisionDistance = distance;
                    closestBox = box;
                }
            }
            Vec3* position = getPos(1.0f);
            Vec3* center = closestBox->getCenter();
            Vec3* direction = Vec3::newTemp(position->x - center->x, 0.0, position->z - center->z);
            direction->normalize();
            mDamageIndicatorSegments[getIndicatorSegment(static_cast<float>(direction->x),
                                                         static_cast<float>(direction->z))] = true;
            mElytraProximityWarning = true;
        }

        if (CGameNetworkManager::sInstance.GetPlayerCount() > 1)
        {
            Vec3* position = getPos(1.0f);
            std::vector<boost::shared_ptr<Player> >* nearby = mLevel->getNearbyPlayers(shared_from_this(), 3.0);
            for (std::vector<boost::shared_ptr<Player> >::iterator end = nearby->end(), it = nearby->begin(); it != end;
                 ++it)
            {
                boost::shared_ptr<Player>& player = *it;
                Vec3* direction = Vec3::newTemp(position->x - player->mX, 0.0, position->z - player->mZ);
                direction->normalize();
                mDamageIndicatorSegments[getIndicatorSegment(static_cast<float>(direction->x),
                                                             static_cast<float>(direction->z))] = true;
            }
            nearby->clear();
            delete nearby;
        }
        return;
    }
    mElytraProximityWarning = false;
}

bool LocalPlayer::isControlledCamera()
{
    return true;
}

unsigned long long LocalPlayer::sendPosition()
{
    bool sprinting = isSprinting();
    if (sprinting != mWasSprinting)
    {
        if (sprinting)
            mPacketListener->send(boost::shared_ptr<Packet>(new ServerboundPlayerCommandPacket(
                shared_from_this(), ServerboundPlayerCommandPacket::START_SPRINTING)));
        else
            mPacketListener->send(boost::shared_ptr<Packet>(new ServerboundPlayerCommandPacket(
                shared_from_this(), ServerboundPlayerCommandPacket::STOP_SPRINTING)));
        mWasSprinting = sprinting;
    }

    bool sneaking = isSneaking();
    if (sneaking != mWasSneaking)
    {
        if (sneaking)
            mPacketListener->send(boost::shared_ptr<Packet>(new ServerboundPlayerCommandPacket(
                shared_from_this(), ServerboundPlayerCommandPacket::PRESS_SHIFT_KEY)));
        else
            mPacketListener->send(boost::shared_ptr<Packet>(new ServerboundPlayerCommandPacket(
                shared_from_this(), ServerboundPlayerCommandPacket::RELEASE_SHIFT_KEY)));
        mWasSneaking = sneaking;
    }

    bool swimming = isSwimming();
    if (swimming != mWasSwimming)
    {
        if (swimming)
            mPacketListener->send(boost::shared_ptr<Packet>(new ServerboundPlayerCommandPacket(
                shared_from_this(), ServerboundPlayerCommandPacket::START_SWIMMING)));
        else
            mPacketListener->send(boost::shared_ptr<Packet>(
                new ServerboundPlayerCommandPacket(shared_from_this(), ServerboundPlayerCommandPacket::STOP_SWIMMING)));
        mWasSwimming = swimming;
    }

    bool damagingNearbyMobs = getDamageNearbyMobs();
    if (damagingNearbyMobs != mWasDamagingNearbyMobs)
    {
        if (damagingNearbyMobs)
            mPacketListener->send(boost::shared_ptr<Packet>(new ServerboundPlayerCommandPacket(
                shared_from_this(), ServerboundPlayerCommandPacket::START_SPIN_ATTACK)));
        else
            mPacketListener->send(boost::shared_ptr<Packet>(new ServerboundPlayerCommandPacket(
                shared_from_this(), ServerboundPlayerCommandPacket::STOP_SPIN_ATTACK)));
        mWasDamagingNearbyMobs = damagingNearbyMobs;
    }

    if (!isControlledCamera())
        return 0;

    float yRot = mYRot;
    float xRot = mXRot;
#ifdef MINECRAFT_PC
    PcModViewsLookRotation(this, &yRot, &xRot);
#endif
    AABB* box = mBoundingBox;
    double xd = mX - mLastSentX;
    double yd = box->mMinY - mLastSentY;
    double zd = mZ - mLastSentZ;
    ++mPositionSendCounter;
    bool move = xd * xd + yd * yd + zd * zd > 0.0009 || mPositionSendCounter >= 20 || mPreviousOnGround != mOnGround;
    bool rot = yRot - mLastSentYRot != 0.0 || xRot - mLastSentXRot != 0.0;
    bool flying = mAbilities.mFlying;

    if (isPassenger())
    {
        mPacketListener->send(boost::shared_ptr<Packet>(
            new ServerboundMovePlayerPacket::PosRot(mXd, -999.0, mZd, yRot, xRot, mOnGround, flying)));
    }
    else
    {
        if (move && rot)
        {
            mPacketListener->send(boost::shared_ptr<Packet>(
                new ServerboundMovePlayerPacket::PosRot(mX, box->mMinY, mZ, yRot, xRot, mOnGround, flying)));
        }
        else if (move)
        {
            mPacketListener->send(boost::shared_ptr<Packet>(
                new ServerboundMovePlayerPacket::Pos(mX, box->mMinY, box->mMinY, mZ, mOnGround, flying)));
        }
        else if (rot)
        {
            mPacketListener->send(
                boost::shared_ptr<Packet>(new ServerboundMovePlayerPacket::Rot(yRot, xRot, mOnGround, flying)));
        }
        else if (mPreviousOnGround != mOnGround)
        {
            mPacketListener->send(boost::shared_ptr<Packet>(new ServerboundMovePlayerPacket(mOnGround, flying)));
        }

        if (move)
        {
            mLastSentX = mX;
            mLastSentY = box->mMinY;
            mLastSentZ = mZ;
            mPositionSendCounter = 0;
        }
    }

    if (rot)
    {
        mLastSentYRot = yRot;
        mLastSentXRot = xRot;
    }
    mPreviousOnGround = mOnGround;
    mAutoJumpEnabled = GameSettings::GetGameSettings(mUserIndex, eGameSetting_AutoJump) != 0;
    return 0;
}

void LocalPlayer::chat(const std::wstring& message)
{
    mPacketListener->send(boost::shared_ptr<Packet>(new ServerboundChatPacket(message)));
}

bool LocalPlayer::swing(InteractionHand::EInteractionHand hand)
{
    bool result = LivingEntity::swing(hand);
    mPacketListener->send(boost::shared_ptr<Packet>(new ServerboundSwingPacket(hand)));
    return result;
}

void LocalPlayer::respawn()
{
    mPacketListener->send(
        boost::shared_ptr<Packet>(new ServerboundClientCommandPacket(ServerboundClientCommandPacket::PERFORM_RESPAWN)));
}

void LocalPlayer::actuallyHurt(const DamageSource* source, float amount)
{
    if (isInvulnerableTo(source))
        return;
    setHealth(getHealth() - amount);
    mDamageTaken = amount;
}

#ifdef MINECRAFT_PC
void LocalPlayer::PcCloseHiddenMenu()
{
    if (hiddenMenu == NULL || mContainerMenu != hiddenMenu)
        return;
    mPacketListener->send(boost::shared_ptr<Packet>(new ServerboundContainerClosePacket(hiddenMenu->m_containerId)));
    Player::closeContainer();
    delete hiddenMenu;
    hiddenMenu = NULL;
}
#endif

void LocalPlayer::clientSideCloseContainer()
{
    mInventory->setCarried(ItemInstance::EMPTY);
#ifdef MINECRAFT_PC
    if (hiddenMenu != NULL && mContainerMenu == hiddenMenu)
    {
        Player::closeContainer();
        delete hiddenMenu;
        hiddenMenu = NULL;
    }
#endif
    Player::closeContainer();
    mMinecraft->setScreen(NULL);
    if (gConsoleUIController.IsContainerMenuDisplayed(mUserIndex))
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_BACK);
        gConsoleUIController.CloseUIScenes(mUserIndex, false);
    }
}

void LocalPlayer::hurtTo(float amount, TelemetryDamageSource source)
{
    if (mHealthInitialized)
    {
        float delta = getHealth() - amount;
        if (delta <= 0.0f)
        {
            setHealth(amount);
            if (delta < 0.0f)
                mInvulnerableTime = mInvulnerableDuration / 2;
        }
        else
        {
            mLastHurt = delta;
            setHealth(getHealth());
            mInvulnerableTime = mInvulnerableDuration;
            actuallyHurt(DamageSource::GENERIC, delta);
            mHurtDuration = 10;
            mHurtTime = 10;
        }
    }
    else
    {
        setHealth(amount);
        mHealthInitialized = true;
    }

    if (getHealth() <= 0.0f)
    {
        telemetryManagerInstance->RecordPlayerDiedOrFailed(
            mUserIndex, (int)mX, (int)mY, (int)mZ, 0, source.playerWeaponId, source.causeOfDeathId, source.challenges);
        if (gConsoleUIController.GetMenuDisplayed(mUserIndex))
        {
            gConsoleUIController.CloseUIScenes(mUserIndex, false);
        }
    }
}

void LocalPlayer::onUpdateAbilities()
{
    mPacketListener->send(boost::shared_ptr<Packet>(new ServerboundPlayerAbilitiesPacket(&mAbilities)));
}

bool LocalPlayer::isLocalPlayer()
{
    return true;
}

void LocalPlayer::updateDamageIndicator(float x, float z, bool allDirections)
{
    if (allDirections)
    {
        mDamageIndicatorAllDirections = true;
        return;
    }
    int segment = getIndicatorSegment(x, z);
    mDamageIndicatorSegments[segment] = true;
}

float LocalPlayer::getJumpRidingScale()
{
    return mRideJumpScale;
}

void LocalPlayer::sendRidingJump()
{
    mPacketListener->send(boost::shared_ptr<Packet>(
        new ServerboundPlayerCommandPacket(shared_from_this(), ServerboundPlayerCommandPacket::RIDING_JUMP,
                                           static_cast<int>(getJumpRidingScale() * 100.0f))));
}

void LocalPlayer::sendOpenInventory()
{
    boost::shared_ptr<Entity> entity = shared_from_this();
    boost::shared_ptr<Packet> packet(
        new ServerboundPlayerCommandPacket(entity, ServerboundPlayerCommandPacket::OPEN_INVENTORY));
    mPacketListener->send(packet);
}

unsigned int LocalPlayer::getPermissionLevel()
{
    return (unsigned int)mPermissionLevel;
}

void LocalPlayer::setPermissionLevel(int level)
{
    mPermissionLevel = level;
}

bool LocalPlayer::freeAtAndAbove(const BlockPos& pos)
{
    const BlockState* state = mLevel->getBlockState(pos);
    const BlockState* aboveState = mLevel->getBlockState(pos.above());
    bool freeAtPos = !state->isSolidBlockingCubeAndNotSignalSource();
    bool freeAbove = !aboveState->isSolidBlockingCubeAndNotSignalSource();

    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        if (!freeAtPos && !gApp.ShouldCollideWith(state->getBlock()))
            freeAtPos = true;
        if (!freeAbove && !gApp.ShouldCollideWith(aboveState->getBlock()))
            freeAbove = true;
    }

    return freeAtPos && freeAbove;
}

unsigned char LocalPlayer::freeAt(const BlockPos& pos)
{
    const BlockState* state = mLevel->getBlockState(pos);
    unsigned int free = 1;
    if (state->isSolidBlockingCubeAndNotSignalSource())
    {
        if (!Minecraft::InMiniGame(eMiniGameId_Any, true) || gApp.ShouldCollideWith(state->getBlock()))
            free = 0;
    }
    return free;
}

bool LocalPlayer::checkInBlock(double x, double y, double z)
{
    if (!mNoPhysics)
    {
        BlockPos pos(x, y, z);
        double dx = x - pos.x;
        double dz = z - pos.z;
        double height = std::fabs(mBoundingBox->mMinY - mBoundingBox->mMaxY);
        unsigned int free;
        if (height > 1.0)
            free = freeAtAndAbove(pos);
        else
            free = freeAt(pos);
        if (!free)
        {
            int direction = -1;
            double best = 9999.0;
            if (freeAtAndAbove(pos.west()) && dx < best)
            {
                best = dx;
                direction = 0;
            }
            if (freeAtAndAbove(pos.east()) && 1.0 - dx < best)
            {
                best = 1.0 - dx;
                direction = 1;
            }
            if (freeAtAndAbove(pos.north()) && dz < best)
            {
                best = dz;
                direction = 4;
            }
            if (freeAtAndAbove(pos.south()) && 1.0 - dz < best)
            {
                best = 1.0 - dz;
                direction = 5;
            }
            float speed = 0.1f;
            if (direction == 0)
                mXd = -speed;
            if (direction == 1)
                mXd = speed;
            if (direction == 4)
                mZd = -speed;
            if (direction == 5)
                mZd = speed;
        }
    }
    return false;
}

void LocalPlayer::setExperienceValues(float progress, int total, int level)
{
    mExperienceProgress = progress;
    mTotalExperience = total;
    mExperienceLevel = level;
}

void LocalPlayer::playSound(const SoundEvent* sound, float volume, float pitch)
{
    mLevel->playLocalSound(mX, mY, mZ, sound, getSoundSource(), volume, pitch, false, 16.0f);
}

bool LocalPlayer::isEffectiveAi()
{
    return true;
}

void LocalPlayer::startUsingItem(InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> item = getItemInHand(hand);
    if (item->isEmpty() || isUsingItem())
        return;

    LivingEntity::startUsingItem(hand);
    mUsedItemHand = hand;
    mItemUsePending = true;
}

bool LocalPlayer::isUsingItem()
{
    return mItemUsePending;
}

void LocalPlayer::stopUsingItem()
{
    LivingEntity::stopUsingItem();
    mItemUsePending = false;
}

InteractionHand::EInteractionHand LocalPlayer::getUsedItemHand()
{
    return mUsedItemHand;
}

bool LocalPlayer::isRidingJumpable()
{
    boost::shared_ptr<PlayerRideableJumping> rideable = boost::dynamic_pointer_cast<PlayerRideableJumping>(
        getVehicle());
    return isPassenger() && rideable && rideable->canJump();
}

bool LocalPlayer::isAutoJumpEnabled()
{
    return mAutoJumpEnabled;
}

void LocalPlayer::updateAutoJump(float xMovement, float zMovement)
{
    if (!isAutoJumpEnabled() || mAutoJumpTime > 0)
        return;

    if (!mOnGround || isSneaking())
    {
        bool swimming = (!isUnderLiquid(Material::WATER) && isInWater()) || isInLava();
        if (!swimming || mAbilities.mFlying)
            return;
    }
    if (isPassenger())
        return;

    phys::Vec2* moveVector = mInput->getMoveVector();
    if (moveVector->x == 0.0f && moveVector->y == 0.0f)
        return;

    Vec3* start = Vec3::newTemp(mX, mBoundingBox->mMinY, mZ);
    Vec3* target = Vec3::newTemp(mX + xMovement, mBoundingBox->mMinY, mZ + zMovement);
    Vec3* movement = Vec3::newTemp(xMovement, 0.0, zMovement);
    float speed = getSpeed();
    float lengthSquared = (float)movement->lengthSqr();
    if (lengthSquared <= 0.001f)
    {
        float inputX = speed * moveVector->x;
        float inputZ = speed * moveVector->y;
        float sine = Mth::sin(mYRot * Mth::DEG_TO_RAD);
        float cosine = Mth::cos(mYRot * Mth::DEG_TO_RAD);
        movement = Vec3::newTemp(inputX * cosine - inputZ * sine, movement->y, inputZ * cosine + inputX * sine);
        lengthSquared = (float)movement->lengthSqr();
        if (lengthSquared <= 0.001f)
            return;
    }

    float inverseLength = (float)Mth::fastInvSqrt(lengthSquared);
    Vec3* direction = movement->scale(inverseLength);
    Vec3* forward = getForward();
    if ((float)(forward->x * direction->x + forward->z * direction->z) < -0.15f)
        return;

    if ((isInWater() || isInLava()) && !mAbilities.mFlying)
    {
        mAutoJumpTime = 1;
        return;
    }

    BlockPos headPos(mX, mBoundingBox->mMaxY, mZ);
    if (mLevel->getBlockState(headPos)->getClipAABB(mLevel, headPos) != NULL)
        return;
    headPos = headPos.above();
    if (mLevel->getBlockState(headPos)->getClipAABB(mLevel, headPos) != NULL)
        return;

    float jumpHeight = 1.2f;
    if (hasEffect(MobEffects::JUMP))
        jumpHeight += (float)(getEffect(MobEffects::JUMP)->getAmplifier() + 1) * 0.75f;

    const float sweptLength = speed * 7.0f;
    const float stepLength = 1.0f / inverseLength;
    const float reach = (sweptLength < stepLength) ? stepLength : sweptLength;
    target = target->add(direction->scale(reach));
    float width = mBbWidth;
    AABB* probe = AABB::newTemp(BlockPos(start), BlockPos(target->add(0.0, mBbHeight, 0.0)))->grow(width, 0.0, width);
    start = start->add(0.0, 0.51f, 0.0);
    target = target->add(0.0, 0.51f, 0.0);
    Vec3* side = direction->cross(Vec3::newTemp(0.0, 1.0, 0.0))->scale(width * 0.5f);
    Vec3* leftFrom = start->subtract(side);
    Vec3* leftTo = target->subtract(side);
    Vec3* rightFrom = start->add(side);
    Vec3* rightTo = target->add(side);

    std::vector<AABB*>* collisions = mLevel->getCollisionAABBs(shared_from_this(), probe, false, false, false);
    float highest = FLT_MIN;
    for (std::vector<AABB*>::iterator it = collisions->begin(); it != collisions->end(); ++it)
    {
        AABB* collision = *it;
        if (!collision->intersects(leftFrom, leftTo) && !collision->intersects(rightFrom, rightTo))
            continue;

        highest = (float)collision->mMaxY;
        BlockPos obstacle(collision->getCenter());
        for (int step = 1; (float)step < jumpHeight; ++step)
        {
            BlockPos above = obstacle.above(step);
            const AABB* aboveClip = mLevel->getBlockState(above)->getClipAABB(mLevel, above);
            if (aboveClip != NULL)
            {
                highest = (float)aboveClip->mMaxY + (float)above.y;
                if (highest - mBoundingBox->mMinY > jumpHeight)
                    return;
            }
            if (step > 1)
            {
                headPos = headPos.above();
                if (mLevel->getBlockState(headPos)->getClipAABB(mLevel, headPos) != NULL)
                    return;
            }
        }
        break;
    }

    if (highest == FLT_MIN)
        return;
    float height = (float)(highest - mBoundingBox->mMinY);
    if (height > 0.5f && height <= jumpHeight)
        mAutoJumpTime = 1;
}

void LocalPlayer::mapPlayerChunk(unsigned int targetBlockId)
{
    int centerChunkX = mChunkX;
    int centerChunkZ = mChunkZ;
    int localZ = (int)std::floor(mZ) % 16;
    int localX = (int)std::floor(mX) % 16;

    std::cout << "player in chunk (" << centerChunkX << "," << centerChunkZ << ") at (" << mX << "," << mY << "," << mZ
              << ")\n";

    for (int chunkZ = -1; chunkZ <= 1; ++chunkZ)
    {
        for (int z = 0; z < 16; ++z)
        {
            for (int chunkX = -1; chunkX <= 1; ++chunkX)
            {
                for (int x = 0; x < 16; ++x)
                {
                    LevelChunk* chunk = mLevel->getChunk(centerChunkX + chunkX, centerChunkZ + chunkZ);
                    if (x == localX && z == localZ && chunkX == 0 && chunkZ == 0)
                    {
                        std::cout << "O";
                        continue;
                    }

                    for (int y = 127; y > 0; --y)
                    {
                        int blockId = Block::getId(chunk->getBlock(BlockPos(x, y, z)));
                        if (targetBlockId != BlockId::AIR && blockId == (int)targetBlockId)
                        {
                            std::cout << "@";
                            break;
                        }
                        if (blockId != BlockId::AIR)
                        {
                            if (blockId < 10)
                            {
                                std::cout << blockId;
                            }
                            else
                            {
                                std::cout << "#";
                            }
                            break;
                        }
                    }
                }
            }
            std::cout << "\n";
        }
    }
    std::cout << "\n";
}

double LocalPlayer::getDamageTaken()
{
    return mDamageTaken;
}

bool LocalPlayer::isSolidBlock(int x, int y, int z)
{
    return mLevel->getBlockState(BlockPos(x, y, z))->isSolidBlockingCube();
}

not_null_ptr<ItemInstance> LocalPlayer::getCarriedItem()
{
    return mInventory->getSelected();
}

MultiPlayerGameMode* LocalPlayer::GetLocalGameMode()
{
    MultiPlayerGameMode* gameMode = 0;
    if ((unsigned int)mUserIndex < 4)
    {
        gameMode = mMinecraft->mGameModes[mUserIndex];
    }
    else
    {
        for (int index = 0; index < 4; ++index)
        {
            if (mMinecraft->mLocalPlayers[index] && mMinecraft->mLocalPlayers[index].get() == this)
            {
                gameMode = mMinecraft->mGameModes[index];
                break;
            }
        }
    }
    return gameMode;
}

GameMode* LocalPlayer::GetGameMode()
{
    return GetLocalGameMode();
}

void LocalPlayer::StartSpectating()
{
    GetLocalGameMode()->StartSpectating();
}

void LocalPlayer::OnEquipArmor(not_null_ptr<ItemInstance> item)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    MultiPlayerGameMode* gameMode = minecraft->mGameModes[mUserIndex];
    if (gameMode != 0)
    {
        Tutorial* tutorial = gameMode->getTutorial();
        if (tutorial != 0)
        {
            tutorial->onEquip(item);
        }
    }

    minecraft->mGui->ShowCharacterDisplay(mUserIndex);

    static Item* ironArmor[4] = {Items::ironBoots, Items::ironLeggings, Items::ironChestplate, Items::ironHelmet};
    int index = 0;
    for (int slot = 0; slot < 4; ++slot)
    {
        not_null_ptr<ItemInstance> armor = mInventory->m_armor[slot];
        if (armor->isEmpty() || armor->getItem() == NULL || armor->getItem() != ironArmor[index++])
        {
            return;
        }
    }

    awardStat(GenericStats::ironMan(), GenericStats::param_ironMan());
}

void LocalPlayer::OnTakeFromAnvil(not_null_ptr<ItemInstance> item)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    MultiPlayerGameMode* gameMode = minecraft->mGameModes[mUserIndex];
    if (gameMode != 0)
    {
        Tutorial* tutorial = gameMode->getTutorial();
        if (tutorial != 0)
        {
            tutorial->onTakeFromAnvil(item);
        }
    }
}

float LocalPlayer::getUnderwaterVisionClarity()
{
    float clarity;
    if (mWaterVisionClarity == mWaterVisionClarityO)
    {
        clarity = mWaterVisionClarityTarget;
    }
    else
    {
        clarity = 1.0f;
        mWaterVisionClarityTarget = clarity;
        if (mWaterVisionClarity < 5.0f)
        {
            clarity = Mth::Lerp(0.0f, 0.6f, mWaterVisionClarity / 5.0f);
            mWaterVisionClarityTarget = clarity;
        }
        else if (mWaterVisionClarity < 10.0f)
        {
            clarity = Mth::Lerp(0.6f, 1.0f, (mWaterVisionClarity - 5.0f) / 5.0f);
            mWaterVisionClarityTarget = clarity;
        }
        mWaterVisionClarityO = mWaterVisionClarity;
    }
    return clarity;
}

void LocalPlayer::DoAutoAttackOnTouch(boost::shared_ptr<Entity> entity)
{
    boost::shared_ptr<LocalPlayer> self = boost::static_pointer_cast<LocalPlayer>(shared_from_this());
    mMinecraft->mGameModes[mUserIndex]->attack(self, entity);
}

double LocalPlayer::GetPercentageTraversed() const
{
    return mPercentageTraversed;
}

void LocalPlayer::SetPercentageTraversed(double percentage)
{
    mPercentageTraversed = percentage;
}

void LocalPlayer::setDamageNearbyMobs(bool damage)
{
    mAutoSpinAttackTicks = 0;
    mDamageNearbyMobs = damage;
}

bool LocalPlayer::getDamageNearbyMobs()
{
    return mDamageNearbyMobs;
}

void LocalPlayer::setAndBroadcastCustomSkin(unsigned int skin)
{
    setCustomSkin(skin);
}

void LocalPlayer::setAndBroadcastCustomCape(unsigned int cape)
{
    setCustomCape(cape);
}
