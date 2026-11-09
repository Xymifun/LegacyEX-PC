#include "game/core/Minecraft.hpp"
#include "game/core/StringIds.hpp"
#include "game/ui/DeathScreen.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/Mth.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"

#include <wchar.h>
#include <cmath>

#ifdef MINECRAFT_PC
#include "platform/pc/PcDisplay.hpp"
#include "platform/pc/PcGlide.hpp"
#include "platform/pc/PcGlideSections.hpp"
#include "platform/pc/PcInput.hpp"
#include "platform/pc/PcSettings.hpp"

static bool s_pcGlideWasDown[4][2];
static bool s_pcGlideHoldFired[4][2];
static unsigned long long s_pcGlideHoldStart[4][2];
static bool s_pcGlideHold[4][2];
static bool s_pcGlideBlocked[4][2];
#endif

#include "game/client/audio/ConsoleSoundEngine.hpp"
#include "game/client/audio/SoundEngine.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundType.hpp"
#include "game/client/render/BlockEntityRenderDispatcher.hpp"
#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/EntityBlockRenderer.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/Font.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/GuiParticles.hpp"
#include "game/client/render/ItemInHandRenderer.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/client/render/ParticleEngine.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/client/render/ProgressRenderer.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/TextureAtlas.hpp"
#include "game/client/render/TextureManager.hpp"
#include "game/client/render/DLCTexturePack.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BossHealthOverlay.hpp"
#include "game/core/AbstractIllager.hpp"
#include "game/core/ArmorStand.hpp"
#include "game/core/BannerPattern.hpp"
#include "game/core/Bat.hpp"
#include "game/core/Blaze.hpp"
#include "game/core/Boat.hpp"
#include "game/core/BuiltInLootTables.hpp"
#include "game/core/Chicken.hpp"
#include "game/core/CommonStats.hpp"
#include "game/core/CameraController.hpp"
#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/DefaultVertexFormat.hpp"
#include "game/core/DispenserBootstrap.hpp"
#include "game/core/EnderCrystal.hpp"
#include "game/core/EnderMan.hpp"
#include "game/core/FaceInfo.hpp"
#include "game/core/FontCharacterWidths.hpp"
#include "game/core/Ghast.hpp"
#include "game/core/Guardian.hpp"
#include "game/core/Llama.hpp"
#include "game/core/Minecart.hpp"
#include "game/core/Parrot.hpp"
#include "game/core/Pig.hpp"
#include "game/core/PolarBear.hpp"
#include "game/core/PrimedTnt.hpp"
#include "game/core/Salmon.hpp"
#include "game/core/SharedConstants.hpp"
#include "game/core/Sheep.hpp"
#include "game/core/Shulker.hpp"
#include "game/core/Slime.hpp"
#include "game/core/SnowMan.hpp"
#include "game/core/SpellcasterIllager.hpp"
#include "game/core/Spider.hpp"
#include "game/core/ThrownTrident.hpp"
#include "game/core/TropicalFish.hpp"
#include "game/core/Turtle.hpp"
#include "game/core/Vex.hpp"
#include "game/core/Witch.hpp"
#include "game/core/WitherBoss.hpp"
#include "game/core/WitherSkull.hpp"
#include "game/core/Wolf.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "vendor/_ConnectionProgressParams.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/FoliageColor.hpp"
#include "game/core/FrameTimer.hpp"
#include "game/core/GameModeConstraints.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GameType.hpp"
#include "game/core/GhostController.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/GrassColor.hpp"
#include "game/core/Gui.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Input.hpp"
#include "game/core/InfoBarManager.hpp"
#include "game/core/InteractionResult.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/Options.hpp"
#include "game/core/Stats.hpp"
#include "game/core/StatsCounter.hpp"
#include "game/core/StatsUID.hpp"
#include "game/core/System.hpp"
#include "game/core/Timer.hpp"
#include "game/core/TrialMode.hpp"
#include "game/core/User.hpp"
#include "game/core/UnityClient13.hpp"
#include "game/core/UnityClient6.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixers.hpp"
#include "game/command/CheckpointCommand.hpp"
#include "game/data/ListTag.hpp"
#include "game/data/McRegionLevelStorageSource.hpp"
#include "game/data/SparseDataStorage.hpp"
#include "game/data/SparseLightStorage.hpp"
#include "game/data/StringTag.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemColors.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/CheckpointRuleDefinition.hpp"
#include "game/minigame/MiniGameMedals.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/ClientPacketListener.hpp"
#include "game/network/PlayerReadyPacket.hpp"
#include "game/ui/Screen.hpp"
#include "game/ui/ScreenSizeCalculator.hpp"
#include "game/ui/TitleScreen.hpp"
#include "game/ui/Tooltips.hpp"
#include "game/ui/UIScene_CreativeMenu.hpp"
#include "game/ui/UIScene_LeaderboardsMenu.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockColors.hpp"
#include "game/world/block/CompressedBlockStorage.hpp"
#include "game/world/generation/AbstractClientPlayer.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/entity/mob/AbstractChestedHorse.hpp"
#include "game/entity/mob/AbstractHorse.hpp"
#include "game/entity/mob/AbstractSkeleton.hpp"
#include "game/entity/mob/AgableMob.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityIO.hpp"
#include "game/entity/mob/FireworksRocketEntity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/TamableAnimal.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/entity/mob/Zombie.hpp"
#include "game/network/Packet.hpp"
#include "game/entity/projectile/AbstractRotatedItemProjectile.hpp"
#include "game/entity/projectile/FishingHook.hpp"
#include "game/entity/projectile/TippableArrow.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/AxeItem.hpp"
#include "game/item/PickaxeItem.hpp"
#include "game/item/ShovelItem.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/Enchantments.hpp"
#include "game/item/FurnaceRecipes.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemIdFix.hpp"
#include "game/item/ItemPotionFix.hpp"
#include "game/item/MapExtendingRecipe.hpp"
#include "game/item/Potion.hpp"
#include "game/item/PotionBrewing.hpp"
#include "game/item/Potions.hpp"
#include "game/item/Recipes.hpp"
#include "game/item/ThrownPotion.hpp"
#include "game/ui/InventoryMenu.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/JungleBiome.hpp"
#include "game/world/biome/MesaBiome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/chunk/ChunkInfoMapManager.hpp"
#include "game/world/chunk/McRegionChunkStorage.hpp"
#include "game/world/LevelType.hpp"
#include "game/world/generation/BirchFeature.hpp"
#include "game/world/generation/BuriedTreasureFeature.hpp"
#include "game/world/generation/DesertWellFeature.hpp"
#include "game/world/generation/MegaPineTreeFeature.hpp"
#include "game/world/generation/OceanMonumentFeature.hpp"
#include "game/core/OceanMonumentPieces.hpp"
#include "game/world/generation/PineFeature.hpp"
#include "game/world/generation/RandomScatteredLargeFeature.hpp"
#include "game/world/generation/RoofTreeFeature.hpp"
#include "game/world/generation/SavannaTreeFeature.hpp"
#include "game/world/generation/ScatteredFeaturePieces.hpp"
#include "game/world/generation/SpruceFeature.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"
#include "game/world/generation/SwampTreeFeature.hpp"
#include "game/world/generation/TreeFeature.hpp"
#include "game/world/generation/VillageFeature.hpp"
#include "game/world/generation/VillagePieces.hpp"
#include "game/world/generation/Villager.hpp"
#include "game/world/generation/VillagerGolem.hpp"
#include "game/world/generation/WoodlandMansionFeature.hpp"
#include "game/world/generation/ZombieVillager.hpp"
#include "game/core/Creeper.hpp"
#include "game/core/Ocelot.hpp"
#include "game/core/StrongholdPieces.hpp"
#include "game/entity/mob/EnderDragon.hpp"
#include "game/core/MinecartFurnace.hpp"
#include "game/item/AreaEffectCloud.hpp"
#include "game/util/Rabbit.hpp"
#include "game/entity/mob/Horse.hpp"
#include "game/world/block/BeaconBlockEntity.hpp"
#include "game/world/block/BlockEntityIdFix.hpp"
#include "game/world/block/EatBlockGoal.hpp"
#include "game/world/block/FallingBlockEntity.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/world/block/ItemInstanceBlockEntityTagWalker.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MaterialColor.hpp"
#include "game/world/block/MinecartCommandBlock.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/CompressedBlockStorage.hpp"
#include "game/server/ServerLevel.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringUtils.hpp"
#include "game/core/Rarity.hpp"

void MinecraftWorld_RunStaticCtors();

template <typename T>
std::wstring _toString(T value);

static Minecraft* sInstance = NULL;
static int sAverageFps = 0;
static bool sStaticsInitialised = false;
static long long sFrameStartTime = 0;
static int sPressStartHoldOffFrames = 0;
static bool sCheckForNewLocalPlayers = true;
bool gFancyGraphics = true;
static File sWorkingDirectory;
static OSFastMutex sTextureRequestLock;

struct ScoreboardMenuData
{
    int pad;
    int type;
};

GenericStats* gStats = NULL;

Minecraft::Minecraft(Component* parent, Canvas* canvas, MinecraftApplet* applet, int width, int height, bool fullscreen)
    : mFrameTimer(new FrameTimer())
{
    mFixerUpper = DataFixers::createFixerUpper();
    mHasCrashed = false;
    mGameMode = NULL;
    mTimer = new Timer(20.0f);
    mPendingLevelChange = 0;
    mLevel = NULL;
    mLevels = arrayWithLength<MultiPlayerLevel*>(3, true);
    mLevelRenderer = NULL;
    mPlayer = boost::shared_ptr<MultiplayerLocalPlayer>();
    mCameraEntity = boost::shared_ptr<Entity>();
    mParticleEngine = NULL;
    mUser = NULL;
    mAppPaused = false;
    mTextures = NULL;
    mFont = NULL;
    mScreen = NULL;
    mLocalPlayerIdx = 0;

    InitializeCriticalSection(&sTextureRequestLock);
    InitializeCriticalSection(&mLevelLock);

    mProgressRenderer = NULL;
    mGameRenderer = NULL;
    mRightClickDelay = 0;
    mTicks = 0;
    mLastClickTick = 0;
    mDefaultHeight = 0;
    mGui = NULL;
    mNoRender = false;
    mHitResult = NULL;
    mOptions = NULL;

    mSoundEngine = new SoundEngine();
    mSoundReloadDelay = 0;
    mTexturePackRepository = NULL;
    mWorkingDirectory = File(L"");
    mLevelSource = NULL;
    mStats[0] = NULL;
    mStats[1] = NULL;
    mStats[2] = NULL;
    mStats[3] = NULL;
    mLevelId = 0;
    sWorkingDirectory = File(L"");
    mEmptyViewport = -1;
    mRunning = true;
    mLastRenderTime = -1;
    mFrameNumber = 0;
    Stats::init();

    mMouseGrabTicks = 0;
    mServerPort = 0;
    mItemColors = NULL;
    mBlockColors = NULL;
    mDefaultHeight = height;
    mFullscreen = fullscreen;
    if (gRenderer->IsWidescreen())
        mWidth = width;
    else
        mWidth = (width * 3) / 4;

    mPaused = false;
    mFullHeight = height;
    mFullWidth = width;
    mHeight = height;
    sInstance = this;
    mFullscreen = fullscreen;
    TextureManager::createInstance();

    for (int i = 0; i < 4; i++)
    {
        mGameModes[i] = NULL;
        mConnectionFailed[i] = false;
        mPendingConnections[i] = NULL;
    }
    mTutorialPlayers = 0;
    mAnimateTickLevel = NULL;
    mReloadTextures = false;
    mSoundEngine->init(NULL);

    mClientMasterGameMode = new ClientMasterGameMode();
    mGhostController = NULL;
    mMiniGame = NULL;
}

Minecraft* Minecraft::GetInstance()
{
    return sInstance;
}

long long Minecraft::currentTimeMillis()
{
    return System::processTimeInMilliSecs();
}

DataFixerUpper* Minecraft::getFixerUpper()
{
    return mFixerUpper;
}

BlockRenderDispatcher* Minecraft::getBlockRenderer()
{
    return mBlockRenderer;
}

ItemRenderer* Minecraft::getItemRenderer()
{
    return mItemRenderer;
}

ItemInHandRenderer* Minecraft::getItemInHandRenderer()
{
    return mItemInHandRenderer;
}

EntityRenderDispatcher* Minecraft::getEntityRenderDispatcher()
{
    return mEntityRenderDispatcher;
}

BlockColors* Minecraft::getBlockColors()
{
    return mBlockColors;
}

ItemColors* Minecraft::getItemColors()
{
    return mItemColors;
}

float Minecraft::getFrameTime()
{
    return mTimer->renderPartialTicks;
}

ColourTable* Minecraft::getColourTable()
{
    ColourTable* colourTable = mTexturePackRepository->getSelected()->getColourTable();
    if (colourTable == NULL)
        colourTable = TexturePackRepository::defaultTexturePack->getColourTable();
    return colourTable;
}

MaterialTable* Minecraft::getMaterialTable()
{
    MaterialTable* materialTable = mTexturePackRepository->getSelected()->getMaterialTable();
    if (materialTable == NULL)
        materialTable = TexturePackRepository::defaultTexturePack->getMaterialTable();
    return materialTable;
}

StructureManager* Minecraft::getStructureManager()
{
    StructureManager* structureManager = mTexturePackRepository->getSelected()->getStructureManager();
    if (structureManager == NULL)
        structureManager = TexturePackRepository::defaultTexturePack->getStructureManager();
    return structureManager;
}

unsigned int Minecraft::getCurrentTexturePackId()
{
    return mTexturePackRepository->getSelected()->getId();
}

bool Minecraft::isUsingDefaultSkin()
{
    return GetInstance()->mTexturePackRepository->getSelected() == TexturePackRepository::defaultTexturePack;
}

void Minecraft::setScreen(Screen* screen)
{
    if (mScreen != NULL)
        mScreen->removed();

    if (screen == NULL && mLevel == NULL)
    {
        screen = new TitleScreen();
    }
    else if (mPlayer && !gConsoleUIController.GetMenuDisplayed(mPlayer->mUserIndex))
    {
        if (mPlayer->getHealth() <= 0.0f)
        {
            if (mTicks == 0)
                mPlayer->respawn();
            else
                gConsoleUIController.NavigateToScene(mPlayer->mUserIndex, EUIScene_DeathMenu, NULL, EUILayer_Scene,
                                                     EUIGroup_PlayerDefault);
        }
    }

    if (screen != NULL && dynamic_cast<TitleScreen*>(screen) != NULL)
    {
        mOptions->mRenderDebug = 0;
        mGui->clearMessages(-1);
    }

    mScreen = screen;
    if (screen != NULL)
    {
        ScreenSizeCalculator sizeCalculator(mOptions, mWidth, mHeight, -1);
        screen->init(this, sizeCalculator.getWidth(), sizeCalculator.getHeight());
        mNoRender = false;
    }
}

void Minecraft::init()
{
    mWorkingDirectory = File(L"");
    mLevelSource = new McRegionLevelStorageSource(File(mWorkingDirectory, L"saves"), getFixerUpper());
    mOptions = new Options(this, mWorkingDirectory);
    mTexturePackRepository = new TexturePackRepository(mWorkingDirectory, this);
    mTexturePackRepository->addDebugPacks();
    mTextures = new Textures(mTexturePackRepository, mOptions);

    mFont = new Font(mOptions, L"font/Default", mTextures, false, &UnityClient13TextureLocation_0x7A, 0x17, 0x1c, 0x10,
                     0x10, fontCharacterWidthTable);
    mAlternateFont = new Font(mOptions, L"font/alternate", mTextures, false, &UnityClient13TextureLocation_0x7B, 0x10,
                              0x10, 8, 8, NULL);

    GrassColor::init(mTextures->loadTexturePixels(TN_MISC_GRASSCOLOR, L"misc/grasscolor"));
    FoliageColor::init(mTextures->loadTexturePixels(TN_MISC_FOLIAGECOLOR, L"misc/foliagecolor"));
    Biome::generateColoursDebugOutput();

    mTerrainAtlas = new TextureAtlas(0, L"terrain", L"textures/blocks/", NULL, true);
    mBlockColors = BlockColors::createDefault();
    mItemColors = ItemColors::createDefault(mBlockColors);
    mItemRenderer = new ItemRenderer(mEntityRenderDispatcher, mTextures, mItemColors);
    mEntityRenderDispatcher = new EntityRenderDispatcher(mTextures, mItemRenderer);

    GlStateManager::enableLighting();
    mItemInHandRenderer = new ItemInHandRenderer(this);
    GlStateManager::disableLighting();

    mGameRenderer = new GameRenderer(this, NULL);
    mBlockRenderer = new BlockRenderDispatcher(mBlockColors);
    BlockEntityRenderDispatcher::instance->mTextures = mTextures;
    EntityBlockRenderer::instance = new EntityBlockRenderer();

    mStats[0] = new StatsCounter();
    mStats[1] = new StatsCounter();
    mStats[2] = new StatsCounter();
    mStats[3] = new StatsCounter();

    MemSect(0x1f);
    MemSect(0);

    GlStateManager::enableTexture();
    GlStateManager::shadeModel(0);
    GlStateManager::clearDepth(1.0);
    GlStateManager::enableDepthTest();
    GlStateManager::depthFunc(3);
    GlStateManager::enableAlphaTest();
    GlStateManager::alphaFunc(4, 0.1f);
    GlStateManager::cullFace(1);
    GlStateManager::matrixMode(1);
    GlStateManager::loadIdentity();
    GlStateManager::matrixMode(0);

    MemSect(0x1f);
    MemSect(0);

    mLevelRenderer = new LevelRenderer(this, mTextures);
    mTextures->stitch();
    GlStateManager::viewport(0, 0, mWidth, mHeight);
    mParticleEngine = new ParticleEngine(mLevel, mTextures);

    MemSect(0x1f);
    MemSect(0);

    mGui = new Gui(this);
    if (mLevelName.compare(0, mLevelName.size(), L"", wcslen(L"")) == 0)
        setScreen(new TitleScreen());

    mProgressRenderer = new ProgressRenderer(this);
    gRenderer->CBuffLockStaticCreations();
}

void Minecraft::run()
{
    mRunning = true;
    init();
}

void Minecraft::stop()
{
    mRunning = false;
}

void Minecraft::destroy()
{
    setLevel(NULL, -1, boost::shared_ptr<Player>(), true, false);
    MemoryTracker::release();
    mSoundEngine->destroy();
}

void Minecraft::emergencySave()
{
    mLevelRenderer->clear();
    AABB::clearPool();
    Vec3::clearPool();
    setLevel(NULL, -1, boost::shared_ptr<Player>(), true, false);
}

void Minecraft::reloadSound()
{
    mSoundEngine = new SoundEngine();
    mSoundEngine->init(mOptions);
}

bool Minecraft::isClientSide()
{
    return mLevel != NULL && mLevel->mIsClientSide;
}

bool Minecraft::isTutorial()
{
    return mTutorialPlayers != 0;
}

bool Minecraft::showOnlyReducedInfo()
{
    return mPlayer && mPlayer->isReducedDebugInfo();
}

LevelStorageSource* Minecraft::getLevelSource()
{
    return mLevelSource;
}

#ifdef MINECRAFT_PC
static void PcApplySessionName(const boost::shared_ptr<MultiplayerLocalPlayer>& player, int userIndex)
{
    if (player == NULL)
        return;
    INetworkPlayer* networkPlayer = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(userIndex);
    if (networkPlayer == NULL || networkPlayer->GetOnlineName() == NULL || networkPlayer->GetOnlineName()[0] == 0)
        return;
    const std::wstring sessionName(networkPlayer->GetOnlineName());
    if (sessionName == player->getName())
        return;
    player->setName(sessionName);
    if (!player->mIsGuest)
        player->setUUID(sessionName);
}
#endif

MultiPlayerLevel* Minecraft::getLevel(int dimension)
{
    if (dimension == -1)
        return mLevels[1];
    if (dimension == 1)
        return mLevels[2];
    return mLevels[0];
}

void Minecraft::forceaddLevel(MultiPlayerLevel* level)
{
    const DimensionType* type = level->mDimension->getType();
    if (type == DimensionType::NETHER)
        mLevels[1] = level;
    else if (type == DimensionType::THE_END)
        mLevels[2] = level;
    else
        mLevels[0] = level;
}

void Minecraft::releaseLevel(int dimension)
{
    setLevel(NULL, dimension, boost::shared_ptr<Player>(), true, false);
}

void Minecraft::prepareLevel(int dimension)
{
    if (mProgressRenderer != NULL)
    {
        mProgressRenderer->progressStart(dimension);
        mProgressRenderer->progressStage(IDS_PROGRESS_BUILDING_TERRAIN);
    }

    int radius = mGameMode->isCutScene() ? 0x40 : 0x80;
    int step = (radius * 2) / 16 + 1;
    int total = step * step;
    int done = 0;
    for (int x = -radius; x <= radius; x += 16)
    {
        for (int z = -radius; z <= radius; z += 16)
        {
            if (mProgressRenderer != NULL)
            {
                mProgressRenderer->progressStagePercentage((done * 100) / total);
                ++done;
            }
        }
    }

    if (!mGameMode->isCutScene() && mProgressRenderer != NULL)
        mProgressRenderer->progressStage(IDS_PROGRESS_SIMULATING_WORLD);
}

void Minecraft::setLevel(MultiPlayerLevel* level, int dimension, boost::shared_ptr<Player> player,
                         bool doForceStatsSave, bool primaryPlayerSignedOut)
{
    EnterCriticalSection(&mLevelLock);

    mCameraEntity = boost::shared_ptr<Entity>();

    if (mProgressRenderer != NULL)
    {
        mProgressRenderer->progressStart(dimension);
        mProgressRenderer->progressStage(-1);
    }

    mGameRenderer->DisableUpdateThread(0);

    for (int i = 0; i < mLevels.length; ++i)
    {
        if (mLevels[i] != NULL && level == NULL)
        {
            if (doForceStatsSave && mPlayer)
                forceStatsSave(mPlayer->mUserIndex);

            if (mLevelRenderer != NULL)
            {
                mLevelRenderer->setLevel(0, NULL);
                mLevelRenderer->setLevel(1, NULL);
                mLevelRenderer->setLevel(2, NULL);
                mLevelRenderer->setLevel(3, NULL);
            }
            if (mParticleEngine != NULL)
                mParticleEngine->setLevel(NULL);
        }
    }

    if (level == NULL)
    {
        mGui->onDisconnected();
        mSoundEngine->stopAll();

        if (mLevels[0] != NULL)
        {
            delete mLevels[0];
            mLevels[0] = NULL;
            if (mLevels[1] != NULL)
                mLevels[1]->setSavedDataStorage(NULL);
            if (mLevels[2] != NULL)
                mLevels[2]->setSavedDataStorage(NULL);
        }
        if (mLevels[1] != NULL)
            delete mLevels[1];
        mLevels[1] = NULL;
        if (mLevels[2] != NULL)
            delete mLevels[2];
        mLevels[2] = NULL;

        for (int i = 0; i < 4; ++i)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = mLocalPlayers[i];
            if (localPlayer && localPlayer->mPacketListener != NULL)
            {
                delete localPlayer->mPacketListener;
                localPlayer->mPacketListener = NULL;
            }
            if (mGameModes[i] != NULL)
            {
                delete mGameModes[i];
                mGameModes[i] = NULL;
            }
            if (mPendingConnections[i] != NULL)
            {
                delete mPendingConnections[i];
                mPendingConnections[i] = NULL;
            }
            mLocalPlayers[i] = boost::shared_ptr<MultiplayerLocalPlayer>();
        }

        mGameMode = NULL;
        mPlayer = boost::shared_ptr<MultiplayerLocalPlayer>();
        mCameraEntity = boost::shared_ptr<Entity>();
        mEntityRenderDispatcher->mCamera = boost::shared_ptr<Entity>();
        BlockEntityRenderDispatcher::instance->mCamera = boost::shared_ptr<Entity>();
    }

    mLevel = level;

    if (level != NULL)
    {
        const DimensionType* type = level->mDimension->getType();
        if (type == DimensionType::NETHER)
            mLevels[1] = level;
        else if (type == DimensionType::THE_END)
            mLevels[2] = level;
        else
            mLevels[0] = level;

        if (mPlayer == NULL)
        {
            int userIndex = gProfile->GetPrimaryPad();
            mPlayer = mGameMode->createPlayer(level);

            PlayerUID xuid(0);
            PlayerUID onlineXuid(0);
            gProfile->GetXUID(userIndex, &xuid, false);
            gProfile->GetXUID(userIndex, &onlineXuid, true);
            mPlayer->setXuid(xuid);
            mPlayer->mOnlineXuid = onlineXuid;

            mPlayer->setName(convStringToWstring(gProfile->GetGamertag(userIndex)));
            mPlayer->mDisplayName = gProfile->GetDisplayName(userIndex);
            if (!mPlayer->mIsGuest)
                mPlayer->setUUID(mPlayer->mDisplayName);
#ifdef MINECRAFT_PC
            PcApplySessionName(mPlayer, userIndex);
#endif

            mPlayer->mUserIndex = userIndex;
            mPlayer->resetPos();
            mGameMode->initPlayer(mPlayer);

            for (int i = 0; i < 4; ++i)
            {
                mPendingConnections[i] = NULL;
                if (userIndex != i)
                    mGameModes[i] = NULL;
            }
        }

        if (mPlayer != NULL)
        {
            mPlayer->resetPos();
            if (level != NULL)
                level->addEntity(mPlayer);
        }

        if (mPlayer->mInput != NULL)
            delete mPlayer->mInput;
        mPlayer->mInput = new Input();

        if (mLevelRenderer != NULL)
            mLevelRenderer->setLevel(mPlayer->mUserIndex, level);
        if (mParticleEngine != NULL)
            mParticleEngine->setLevel(level);

        mGameMode->adjustPlayer(mPlayer);

        for (int i = 0; i < 4; ++i)
            mPendingConnections[i] = NULL;

        updatePlayerViewportAssignments();
        mCameraEntity = mPlayer;
        mGameRenderer->EnableUpdateThread(0);
    }
    else
    {
        mLevelSource->clearAll();
        mPlayer = boost::shared_ptr<MultiplayerLocalPlayer>();

        for (int i = 0; i < 4; ++i)
        {
            if (mPendingConnections[i] != NULL)
                mPendingConnections[i]->close();
            mPendingConnections[i] = NULL;
            mLocalPlayers[i] = boost::shared_ptr<MultiplayerLocalPlayer>();
            mGameModes[i] = NULL;
        }
    }

    LeaveCriticalSection(&mLevelLock);
}

void Minecraft::levelTickUpdateFunc(void* level)
{
    ((MultiPlayerLevel*)level)->tick();
}

void Minecraft::checkViewportAssignment(int playerCount)
{
    int primary = gProfile->GetPrimaryPad();
    if (playerCount < 2)
        return;

    if (playerCount == 2)
    {
        bool verticalSplit = GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_SplitScreenVertical)
                             != 0;
        if (mLocalPlayers[primary]->mViewport == 3 || !verticalSplit)
        {
            if (mLocalPlayers[primary]->mViewport == 1)
                return;
            if (verticalSplit)
                return;
        }

        for (int i = 0; i < 4; ++i)
        {
            if (mLocalPlayers[i] && i != primary)
            {
                int viewport = mLocalPlayers[primary]->mViewport;
                mLocalPlayers[primary]->mViewport = mLocalPlayers[i]->mViewport;
                mLocalPlayers[i]->mViewport = viewport;
                return;
            }
        }
        return;
    }

    if (playerCount > 4)
        return;

    if (mLocalPlayers[primary]->mViewport == 5)
        return;

    for (int i = 0; i < 4; ++i)
    {
        if (mLocalPlayers[i] && i != primary && mLocalPlayers[i]->mViewport == 5)
        {
            int viewport = mLocalPlayers[primary]->mViewport;
            mLocalPlayers[primary]->mViewport = mLocalPlayers[i]->mViewport;
            mLocalPlayers[i]->mViewport = viewport;
            return;
        }
    }
}

void Minecraft::updatePlayerViewportAssignments()
{
    mEmptyViewport = -1;

    int playerCount = 0;
    for (int i = 0; i < 4; ++i)
    {
        if (mLocalPlayers[i])
            ++playerCount;
    }

    if (playerCount == 1)
    {
        for (int i = 0; i < 4; ++i)
        {
            if (mLocalPlayers[i])
                mLocalPlayers[i]->mViewport = 0;
        }
    }
    else if (playerCount == 2)
    {
        int viewport = 0;
        for (int i = 0; i < 4; ++i)
        {
            if (!mLocalPlayers[i])
                continue;
            if (GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_SplitScreenVertical) == 0)
            {
                ++viewport;
                mLocalPlayers[i]->mViewport = viewport;
            }
            else
            {
                mLocalPlayers[i]->mViewport = viewport + 3;
                ++viewport;
            }
        }
    }
    else if (playerCount > 2)
    {
        bool quadrantUsed[4];
        quadrantUsed[0] = false;
        quadrantUsed[1] = false;
        quadrantUsed[2] = false;
        quadrantUsed[3] = false;

        for (int i = 0; i < 4; ++i)
        {
            if (!mLocalPlayers[i])
                continue;
            if (!gApp.GetGameStarted())
            {
                mLocalPlayers[i]->mViewport = 0;
                continue;
            }
            int viewport = mLocalPlayers[i]->mViewport;
            if (viewport >= 5 && viewport <= 8)
                quadrantUsed[viewport - 5] = true;
        }

        for (int i = 0; i < 4; ++i)
        {
            if (!mLocalPlayers[i])
                continue;
            int viewport = mLocalPlayers[i]->mViewport;
            if (viewport >= 5 && viewport <= 8)
                continue;
            for (int quadrant = 0; quadrant < 4; ++quadrant)
            {
                if (!quadrantUsed[quadrant])
                {
                    quadrantUsed[quadrant] = true;
                    mLocalPlayers[i]->mViewport = quadrant + 5;
                    break;
                }
            }
        }

        for (int quadrant = 0; quadrant < 4; ++quadrant)
        {
            if (!quadrantUsed[quadrant])
                mEmptyViewport = quadrant;
        }
    }

    checkViewportAssignment(playerCount);

    if (gApp.GetGameStarted())
        gConsoleUIController.UpdatePlayerBasePositions();
}

bool Minecraft::addLocalPlayer(int playerIdx)
{
    if (mPendingConnections[playerIdx] != NULL)
        mPendingConnections[playerIdx]->close();
    mConnectionFailed[playerIdx] = false;
    mPendingConnections[playerIdx] = NULL;

    bool added = CGameNetworkManager::sInstance.AddLocalPlayerByUserIndex(playerIdx);
    gApp.SetDRCCheckPlayerCountEnabled(true);
    if (!added)
    {
        gProfile->SignOutUser(playerIdx);
        return false;
    }

    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer(new MultiplayerLocalPlayer(this, mLevel, mUser, NULL));
    mLocalPlayers[playerIdx] = localPlayer;
    mGameModes[playerIdx] = NULL;
    updatePlayerViewportAssignments();

    _ConnectionProgressParams* params = new _ConnectionProgressParams();
    params->m_playerId = playerIdx;
    params->m_stringId = IDS_PROGRESS_CONNECTING;
    params->m_setFailTimer = true;
    params->m_timerTime = 500;
    gConsoleUIController.NavigateToScene(playerIdx, EUIScene_ConnectingProgress, params, EUILayer_Scene,
                                         EUIGroup_PlayerDefault);
    return true;
}

void Minecraft::addPendingLocalConnection(int playerIdx, ClientPacketListener* connection)
{
    mPendingConnections[playerIdx] = connection;
}

const MiniGameDef* Minecraft::GetMiniGame()
{
    return mMiniGame;
}

ClientMasterGameMode* Minecraft::GetClientMasterGameMode()
{
    return mClientMasterGameMode;
}

bool Minecraft::InMiniGame(EMiniGameId miniGameId, bool includeNextGameMode)
{
    if (miniGameId == eMiniGameId_Any)
        return gApp.GetGameHostOption(eGameHostOption_MiniGame) != 0;

    const MiniGameDef* miniGame = GetInstance()->GetMiniGame();
    if (includeNextGameMode && miniGameId != eMiniGameId_Lobby && miniGame->GetId() == eMiniGameId_Lobby)
        return GetInstance()->GetClientMasterGameMode()->GetNextGameMode() == miniGameId;

    return miniGame->GetId() == miniGameId;
}

EMiniGameId Minecraft::GetMiniGameId()
{
    const MiniGameDef* miniGame = GetInstance()->GetMiniGame();
    if (miniGame->GetId() == eMiniGameId_Lobby)
        return GetInstance()->GetClientMasterGameMode()->GetNextGameMode();
    return miniGame->GetId();
}

void Minecraft::SetupMiniGameInstance(const MiniGameDef& miniGame, int variant)
{
    mMiniGame = &miniGame;
}

const MiniGameDef* Minecraft::GetFirstMiniGame()
{
    return MiniGameDef::GetCustomGameModeById(gApp.GetFirstMiniGameType(), true);
}

void Minecraft::playerStartedTutorial(int playerIdx)
{
    if (gApp.GetTutorialMode(boost::shared_ptr<Player>()))
        mTutorialPlayers = (unsigned char)(mTutorialPlayers | (1 << playerIdx));
}

void Minecraft::playerLeftTutorial(int playerIdx)
{
    if (mTutorialPlayers == 0)
    {
        gApp.SetTutorialMode(false, boost::shared_ptr<Player>());
        return;
    }

    mTutorialPlayers = (unsigned char)(mTutorialPlayers & ~(1 << playerIdx));
    if (mTutorialPlayers != 0)
        return;

    gApp.SetTutorialMode(false, boost::shared_ptr<Player>());

    for (int i = 0; i < 4; ++i)
    {
        if (!mLocalPlayers[i])
            continue;
        telemetryManagerInstance->RecordLevelStart(i, eSen_FriendOrMatch_Friend, eSen_CompeteOrCoop_Both,
                                                   mLevel->getDifficulty()->getId(), gApp.GetLocalPlayerCount(),
                                                   CGameNetworkManager::sInstance.GetOnlinePlayerCount());
    }
}

boost::shared_ptr<Entity> Minecraft::getCameraEntity()
{
    return mCameraEntity;
}

void Minecraft::setCameraEntity(boost::shared_ptr<Entity> entity)
{
    mCameraEntity = entity;
}

boost::shared_ptr<MultiplayerLocalPlayer> Minecraft::createExtraLocalPlayer(int playerIdx, const std::wstring& name,
                                                                            int userIndex, int dimension,
                                                                            ClientPacketListener* connection,
                                                                            MultiPlayerLevel* level, int gameTypeId,
                                                                            int gameModeUserIndex)
{
    if (connection == NULL)
        return boost::shared_ptr<MultiplayerLocalPlayer>();

    if (connection == mPendingConnections[playerIdx])
    {
        int viewport = 0;
        if (mLocalPlayers[playerIdx] && mGameModes[playerIdx] == NULL)
            viewport = mLocalPlayers[playerIdx]->mViewport;

        std::wstring previousName = mUser->name;
        mUser->name = name;
        mPendingConnections[playerIdx] = NULL;

        if (level == NULL)
        {
            mLevel = getLevel(dimension);
            getLevel(dimension)->mConnections.push_back(connection);
        }
        else
        {
            mLevel = level;
        }

        PlayerUID xuid(0);
        PlayerUID onlineXuid(0);
        gProfile->GetXUID(playerIdx, &xuid, false);
        gProfile->GetXUID(playerIdx, &onlineXuid, true);

        bool tutorialForUid = gApp.GetTutorialMode(onlineXuid != PlayerUID(0) ? onlineXuid : xuid);
        bool tutorialMode = gApp.GetTutorialMode(boost::shared_ptr<Player>());

        if (tutorialMode && tutorialForUid)
            mGameModes[playerIdx] = new MultiPlayerGameMode(playerIdx, connection, gameModeUserIndex, true, false);
        else if (!gProfile->IsFullVersion())
            mGameModes[playerIdx] = new TrialMode(playerIdx, connection, gameModeUserIndex);
        else
            mGameModes[playerIdx] = new MultiPlayerGameMode(playerIdx, connection, gameModeUserIndex, false,
                                                            GameType::byId(gameTypeId) == GameType::LOBBY);
        mLocalPlayers[playerIdx] = mGameModes[playerIdx]->createPlayer(mLevel);

        mLocalPlayers[playerIdx]->mIsGuest = gProfile->IsGuest(playerIdx);
        mLocalPlayers[playerIdx]->setXuid(xuid);
        mLocalPlayers[playerIdx]->mOnlineXuid = onlineXuid;
        mLocalPlayers[playerIdx]->setName(convStringToWstring(gProfile->GetGamertag(playerIdx)));
        mLocalPlayers[playerIdx]->mDisplayName = gProfile->GetDisplayName(playerIdx);
        if (!mLocalPlayers[playerIdx]->mIsGuest && (xuid.flags & PlayerUID::FlagMiiPrincipal) == 0)
            mLocalPlayers[playerIdx]->setUUID(mLocalPlayers[playerIdx]->mDisplayName);
#ifdef MINECRAFT_PC
        PcApplySessionName(mLocalPlayers[playerIdx], playerIdx);
#endif

        mLocalPlayers[playerIdx]->mViewport = viewport;

        if (level == NULL)
            mLevel->addEntity(mLocalPlayers[playerIdx]);

        mLocalPlayers[playerIdx]->mUserIndex = userIndex;

        if (mLocalPlayers[playerIdx]->mInput != NULL)
            delete mLocalPlayers[playerIdx]->mInput;
        mLocalPlayers[playerIdx]->mInput = new Input();

        if (GetInstance()->GetClientMasterGameMode()->IsInActiveRoundState())
            mLocalPlayers[playerIdx]->SetGameType(GameType::SPECTATOR);

        mLocalPlayers[playerIdx]->resetPos();
        mLevelRenderer->setLevel(playerIdx, mLevel);
        mLocalPlayers[playerIdx]->mLevel = mLevel;

        mUser->name = previousName;
        updatePlayerViewportAssignments();

        if (tutorialMode && !tutorialForUid)
        {
            if (mGameModes[playerIdx]->getGameModeConstraints()->canMoveToPosition(
                    mLocalPlayers[playerIdx]->mX, mLocalPlayers[playerIdx]->mY, mLocalPlayers[playerIdx]->mZ,
                    mLocalPlayers[playerIdx]->mX, mLocalPlayers[playerIdx]->mY, mLocalPlayers[playerIdx]->mZ, NULL))
            {
                boost::shared_ptr<MultiplayerLocalPlayer> primary =
                    GetInstance()->mLocalPlayers[gProfile->GetPrimaryPad()];
                mLocalPlayers[playerIdx]->teleportTo(primary->mX, primary->mY, primary->mZ);
            }
        }
    }

    return mLocalPlayers[playerIdx];
}

void Minecraft::createPrimaryLocalPlayer(int playerIdx)
{
    mGameModes[playerIdx] = mGameMode;
    mLocalPlayers[playerIdx] = mPlayer;
    mLocalItemInHandRenderers[playerIdx]->Reset();

    if (gProfile->IsSignedIn(gProfile->GetPrimaryPad()))
        mUser->name = convStringToWstring(gProfile->GetGamertag(gProfile->GetPrimaryPad()));
}

bool Minecraft::setLocalPlayerIdx(int playerIdx)
{
    mLocalPlayerIdx = playerIdx;
    if (!mLocalPlayers[playerIdx] || mGameModes[playerIdx] == NULL)
        return false;

    mGameMode = mGameModes[playerIdx];
    mPlayer = mLocalPlayers[playerIdx];
    mCameraEntity = mLocalPlayers[playerIdx]->GetCamera();
    mGameRenderer->mItemInHandRenderer = mLocalItemInHandRenderers[playerIdx];
    mLevel = getLevel(mLocalPlayers[playerIdx]->mDimension);
    mParticleEngine->setLevel(mLevel);
    return true;
}

int Minecraft::getLocalPlayerIdx()
{
    return mLocalPlayerIdx;
}

void Minecraft::removeLocalPlayerIdx(int playerIdx)
{
    if (mGameModes[playerIdx] == NULL)
    {
        if (mPendingConnections[playerIdx] != NULL)
        {
            mPendingConnections[playerIdx]->sendAndDisconnect(
                boost::shared_ptr<Packet>(new DisconnectPacket(DisconnectPacket::eDisconnectReason_Quit)));

            if (gApp.GetGameStarted())
            {
                CGameNetworkManager::sInstance.RemoveCreatedConnection(mPendingConnections[playerIdx]);
                delete mPendingConnections[playerIdx];
            }
            mPendingConnections[playerIdx] = NULL;
            CGameNetworkManager::sInstance.RemoveLocalPlayerByUserIndex(playerIdx);
        }
    }
    else
    {
        if (getLevel(mLocalPlayers[playerIdx]->mDimension)->mIsClientSide)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = mLocalPlayers[playerIdx];
            getLevel(mLocalPlayers[playerIdx]->mDimension)
                ->removeClientPacketListener(localPlayer->mPacketListener, true);
            CGameNetworkManager::sInstance.RemoveCreatedConnection(localPlayer->mPacketListener);
            delete localPlayer->mPacketListener;
            localPlayer->mPacketListener = NULL;
            CGameNetworkManager::sInstance.RemoveLocalPlayerByUserIndex(playerIdx);
        }

        getLevel(mLocalPlayers[playerIdx]->mDimension)->removeEntity(mLocalPlayers[playerIdx]);
        playerLeftTutorial(playerIdx);
        delete mGameModes[playerIdx];
        mGameModes[playerIdx] = NULL;
    }

    mLocalPlayers[playerIdx].reset();

    if (playerIdx != gProfile->GetPrimaryPad())
    {
        mGameRenderer->DisableUpdateThread(5);
        mLevelRenderer->setLevel(playerIdx, NULL);
        mGameRenderer->EnableUpdateThread(5);
        gConsoleUIController.CloseUIScenes(playerIdx, true);
        updatePlayerViewportAssignments();
        mGui->getBossOverlay(playerIdx)->reset();
    }
}

void Minecraft::storeExtraLocalPlayer(int playerIdx)
{
    mLocalPlayers[playerIdx] = mPlayer;

    if (mLocalPlayers[playerIdx]->mInput != NULL)
        delete mLocalPlayers[playerIdx]->mInput;
    mLocalPlayers[playerIdx]->mInput = new Input();

    if (gProfile->IsSignedIn(playerIdx))
        mLocalPlayers[playerIdx]->setName(convStringToWstring(gProfile->GetGamertag(playerIdx)));
#ifdef MINECRAFT_PC
    PcApplySessionName(mLocalPlayers[playerIdx], playerIdx);
#endif

    mLocalItemInHandRenderers[playerIdx]->Reset();
}

void Minecraft::clearConnectionFailed()
{
    for (int i = 0; i < 4; ++i)
    {
        mConnectionFailed[i] = false;
        mDisconnectReasons[i] = DisconnectPacket::eDisconnectReason_None;
    }
    gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_None);
}

void Minecraft::forceStatsSave(int playerIdx)
{
    mStats[playerIdx]->save(playerIdx, true, NULL);
    if (gProfile->IsSignedInLive(playerIdx))
    {
        int lockedProfile = gProfile->GetLockedProfile();
        gProfile->SetLockedProfile(playerIdx);
        mStats[playerIdx]->saveLeaderboards();
        gProfile->SetLockedProfile(lockedProfile);
    }
}

void Minecraft::forceLeaderboardSave(int playerIdx)
{
    if (gProfile->IsSignedInLive(playerIdx))
    {
        int lockedProfile = gProfile->GetLockedProfile();
        gProfile->SetLockedProfile(playerIdx);
        mStats[playerIdx]->saveLeaderboards();
        gProfile->SetLockedProfile(lockedProfile);
    }
}

int Minecraft::InGame_SignInReturned(void* userData, bool signedIn, int userIndex, int playerIdx)
{
    Minecraft* minecraft = (Minecraft*)userData;

    if (CGameNetworkManager::sInstance.IsInSession())
        XEnableGuestSignin(0);

    if (signedIn && CGameNetworkManager::sInstance.IsInSession() && !minecraft->mLocalPlayers[userIndex]
        && !gConsoleUIController.IsIgnorePlayerJoinMenuDisplayed(gProfile->GetPrimaryPad()))
    {
        if (!gProfile->IsSignedIn(userIndex))
            return 0;

        if (!CGameNetworkManager::sInstance.SessionHasSpace(1))
        {
            int messageParameters[1];
            messageParameters[0] = IDS_CONFIRM_OK;
            gConsoleUIController.RequestErrorMessage(0x1AFA54B5, 0xF18FCCD0, messageParameters, 1, 0xff, NULL, NULL,
                                                     NULL, true);
            gProfile->SignOutUser(userIndex);
            gApp.SetDRCCheckPlayerCountEnabled(true);
            return 0;
        }

        if (CGameNetworkManager::sInstance.SessionHasMii(userIndex))
        {
            int messageParameters[1];
            messageParameters[0] = IDS_CONFIRM_OK;
            gConsoleUIController.RequestErrorMessage(STRING_MII_IN_USE_TITLE, STRING_MII_ALREADY_IN_USE,
                                                     messageParameters, 1, 0xff, NULL, NULL, NULL, true);
            gProfile->SignOutUser(userIndex);
            gApp.SetDRCCheckPlayerCountEnabled(true);
            return 0;
        }

        if (!CGameNetworkManager::sInstance.IsLocalGame()
            && (!gProfile->IsSignedInLive(userIndex) || !gProfile->AllowedToPlayMultiplayer(userIndex)))
        {
            if (!gProfile->IsSignedInLive(gProfile->GetPrimaryPad()))
                return 0;
            if (gProfile->AllowedToPlayMultiplayer(userIndex))
                return 0;

            int messageParameters[1];
            messageParameters[0] = IDS_CONFIRM_OK;
            gConsoleUIController.RequestErrorMessage(0x252AB13F, 0xCC5A590F, messageParameters, 1, userIndex, NULL,
                                                     NULL, NULL, true);
            gProfile->SignOutUser(userIndex);
            gApp.SetDRCCheckPlayerCountEnabled(true);
            return 0;
        }

        if (minecraft->mLevel->mIsClientSide)
        {
            minecraft->addLocalPlayer(userIndex);
            return 0;
        }

        boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[userIndex];
        if (!player)
        {
            player = minecraft->createExtraLocalPlayer(
                userIndex, convStringToWstring(gProfile->GetGamertag(userIndex)).c_str(), userIndex,
                minecraft->mLevel->mDimension->getType()->getId(), NULL, NULL, 0, 0);
        }
        return 0;
    }

    gApp.SetDRCCheckPlayerCountEnabled(true);
    return 0;
}

boost::shared_ptr<ItemInstance> Minecraft::addCustomNbtData(boost::shared_ptr<ItemInstance> item,
                                                            boost::shared_ptr<BlockEntity> blockEntity)
{
    CompoundTag* data = new CompoundTag();
    blockEntity->save(data);

    if (item->getItem() == Items::skull && data->contains(L"Owner"))
    {
        CompoundTag* owner = data->getCompound(L"Owner");
        CompoundTag* tag = new CompoundTag();
        tag->put(L"SkullOwner", owner);
        item->setTag(tag);
        return item;
    }

    item->addTagElement(L"BlockEntityTag", data);

    CompoundTag* display = new CompoundTag();
    ListTag* lore = new ListTag();
    lore->add(new StringTag(L"(+NBT)"));
    display->put(L"Lore", lore);
    item->addTagElement(L"display", display);
    return item;
}

void Minecraft::tickAllConnections()
{
    int previousIdx = getLocalPlayerIdx();

    for (int i = 0; i < 4; ++i)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = mLocalPlayers[i];
        if (localPlayer && localPlayer->mPacketListener != NULL)
        {
            setLocalPlayerIdx(i);
            localPlayer->mPacketListener->tick();
        }
    }

    setLocalPlayerIdx(previousIdx);
}

void Minecraft::tick(bool handleInput, bool lastTickOfFrame)
{
    int playerIdx = mPlayer->mUserIndex;

    mStats[playerIdx]->tick(playerIdx);
    gApp.TickOpacityTimer(playerIdx);

    if (handleInput)
        mLevelRenderer->mDestroyedBlocks->tick();

    for (std::list<GhostController*>::iterator it = mFinishedGhostControllers.begin();
         it != mFinishedGhostControllers.end();)
    {
        if ((*it)->canBeDeleted())
        {
            delete *it;
            it = mFinishedGhostControllers.erase(it);
        }
        else
        {
            ++it;
        }
    }

    if (InMiniGame(eMiniGameId_Glide, true) && mGhostController != NULL)
        mGhostController->tick();

    mGui->tick();
    mGameRenderer->pick(1.0f);

    if (!mAppPaused && mLevel != NULL)
        mGameMode->tick();

    MemSect(0x1f);
    GlStateManager::bindTexture(mTextures->loadTexture(0xf1));
    MemSect(0);

    if (handleInput)
    {
        PIXBeginNamedEvent(0.0f, "Texture tick");
        mTextures->tick(lastTickOfFrame, mAppPaused, true);
        PIXEndNamedEvent();
    }

    if (mScreen == NULL && mPlayer)
    {
        if (mPlayer->getHealth() <= 0.0f && !gConsoleUIController.GetMenuDisplayed(playerIdx))
            setScreen(NULL);
        else
            mPlayer->isSleeping();
    }

    if (mScreen != NULL)
    {
        mPlayer->mMissTime = 10000;
        mPlayer->mLastAttackTick = mTicks + 10000;
        mPlayer->mLastUseTick = mTicks + 10000;
        mScreen->updateEvents();
        if (mScreen != NULL)
        {
            mScreen->mGuiParticles->tick();
            mScreen->tick();
        }
    }

    if (mScreen == NULL)
    {
        if (!gConsoleUIController.GetMenuDisplayed(playerIdx) && !gApp.DebugUIFocussed())
        {
            if ((mPlayer->mButtonsDown & (1LL << EControllerActions_ChangeCameraMode)) != 0
                && mGameMode->isInputAllowed(EControllerActions_ChangeCameraMode)
                && mPlayer->CanChangeThirdPersonView())
            {
                mPlayer->SetThirdPersonView((mPlayer->GetThirdPersonView(false) + 1) % 3);
            }

            if (mGameMode->isInputAllowed(EControllerActions_Action))
            {
                if ((mPlayer->mButtonsDown & (1LL << EControllerActions_Action)) != 0)
                {
                    mPlayer->handleMouseClick(0);
                    mPlayer->mLastAttackTick = mTicks;
                }

                if (g_pInput->ButtonDown(playerIdx, EControllerActions_Action)
                    && (float)(mTicks - mPlayer->mLastAttackTick) >= mTimer->ticksPerSecond * 0.25f)
                {
                    mPlayer->handleMouseClick(0);
                    mPlayer->mLastAttackTick = mTicks;
                }

                if (g_pInput->ButtonDown(playerIdx, EControllerActions_Action))
                    mPlayer->handleMouseDown(0, true);
                else
                    mPlayer->handleMouseDown(0, false);
            }

            if ((mPlayer->mButtonsDown & (1LL << EControllerActions_Pause)) != 0)
            {
                gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
                gConsoleUIController.NavigateToScene(playerIdx, EUIScene_PauseMenu, NULL, EUILayer_Scene,
                                                     EUIGroup_PlayerDefault);
            }

            bool waitingForScoreboard = mPlayer->IsWaitingForScoreboard();
            bool showScoreboard = false;
            bool scoreboardPending = true;

            if ((!mPlayer->mHasOpenedInventory || gConsoleUIController.getCleanupOnReload())
                && (mClientMasterGameMode == NULL
                    || (!mClientMasterGameMode->IsRoundPlaying() && !mClientMasterGameMode->IsWaitingToStart()
                        && !mClientMasterGameMode->IsRoundStarting())))
            {
                scoreboardPending = false;
            }

            if (!gConsoleUIController.IsSceneInStack(0xff, EUIScene_FullscreenProgress))
            {
                if (waitingForScoreboard && scoreboardPending)
                    showScoreboard = true;
                else if ((mPlayer->mButtonsDown & (1LL << EControllerActions_Scoreboard)) != 0
                         && mGameMode->isInputAllowed(EControllerActions_Scoreboard))
                    showScoreboard = true;
            }

            if (showScoreboard)
            {
                mPlayer->SetWaitingForScoreboard(false);
                if (InMiniGame(eMiniGameId_Any, true))
                {
                    if (!gConsoleUIController.IsSceneInStack(playerIdx, EUIScene_GameModeScoreboardMenu))
                    {
                        ScoreboardMenuData* sceneData = new ScoreboardMenuData();
                        sceneData->pad = playerIdx;
                        sceneData->type = mPlayer->GetGameType() == GameType::LOBBY;
                        gConsoleUIController.NavigateToScene(playerIdx, EUIScene_GameModeScoreboardMenu, sceneData,
                                                             EUILayer_Scene, EUIGroup_PlayerDefault);

                        if (mGameModes[playerIdx]->getTutorial() != NULL)
                            mGameModes[playerIdx]->getTutorial()->onGameModeChange(eTutorial_GameMode_Scoreboard);

                        if (InMiniGame(eMiniGameId_Glide, true))
                        {
                            bool playWinnerSound = false;
                            {
                                std::wstring winner =
                                    static_cast<ClientScoreboard*>(mLevel->GetScoreboard())->GetWinnerName(-1);
                                std::wstring displayName = mPlayer->getDisplayName();
                                if (winner.compare(0, winner.size(), displayName.c_str(), displayName.size()) == 0
                                    && waitingForScoreboard)
                                    playWinnerSound = true;
                            }

                            if (playWinnerSound)
                                mSoundEngine->playUI(SoundEvents::MG03_WINNER, 1.0f, 1.0f);
                        }
                    }
                }
                else
                {
                    gConsoleUIController.NavigateToScene(playerIdx, EUIScene_InGameInfoMenu, NULL, EUILayer_Scene,
                                                         EUIGroup_PlayerDefault);
                }
                gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
            }

            if (!mPlayer->isSpectator())
            {
                Tooltips::ResetTooltip(playerIdx, EControllerActions_SpectateCancel);
                Tooltips::ResetTooltip(playerIdx, EControllerActions_SpectateNext);
                Tooltips::ResetTooltip(playerIdx, EControllerActions_SpectatePrevious);
                Tooltips::ResetTooltip(playerIdx, EControllerActions_SpectateNoise);

                InteractionResult::EInteractionResult interaction = InteractionResult::eInteractionResult_Fail;
                if (mHitResult == NULL || mHitResult->mType == 0)
                {
                    interaction = mGameMode->useItem(mPlayer, mLevel, InteractionHand::eInteractionHand_MainHand, true);
                }
                else if (mHitResult->mType == 1)
                {
                    const BlockPos blockPos = mHitResult->getBlockPos();
                    const BlockState* blockState = mLevel->getBlockState(blockPos);
                    Block* block = blockState->getBlock();
                    interaction = mGameMode->useItemOn(mPlayer, mLevel, blockPos, mHitResult->mDirection,
                                                       mHitResult->mLocation,
                                                       InteractionHand::eInteractionHand_MainHand, true, NULL);

                    if (mGameMode != NULL && mGameMode->getTutorial() != NULL)
                    {
                        if (block == Blocks::skull)
                            mGameMode->getTutorial()->onLookAtSkull(
                                mLevel, blockPos, block->fillVirtualBlockStateProperties(blockState, mLevel, blockPos));
                        else
                            mGameMode->getTutorial()->onLookAt(
                                block->fillVirtualBlockStateProperties(blockState, mLevel, blockPos));
                    }
                }
                else if (mHitResult->mType == 2)
                {
                    if (mGameMode != NULL && mGameMode->getTutorial() != NULL)
                        mGameMode->getTutorial()->onLookAtEntity(mHitResult->mEntity);
                    interaction = mGameMode->useItem(mPlayer, mLevel, InteractionHand::eInteractionHand_MainHand, true);
                }

                Tooltips::SetInteractTooltips(mPlayer, mLevel, mHitResult, interaction);

                int hotbarScroll = 0;
                if (g_pInput->GetValue(playerIdx, EControllerActions_CycleHeldItemLeft, true)
                    && mGameMode->isInputAllowed(EControllerActions_CycleHeldItemLeft))
                    hotbarScroll = 1;
#ifdef MINECRAFT_PC
                else if (g_pInput->GetValue(playerIdx, EControllerActions_CycleHeldItemRight, true)
                         && mGameMode->isInputAllowed(EControllerActions_CycleHeldItemRight) && !PcInputGlideKeys())
                    hotbarScroll = -1;
#else
                else if (g_pInput->GetValue(playerIdx, EControllerActions_CycleHeldItemRight, true)
                         && mGameMode->isInputAllowed(EControllerActions_CycleHeldItemRight))
                    hotbarScroll = -1;
#endif
#ifdef MINECRAFT_PC
                if (hotbarScroll == 0 && playerIdx == gProfile->GetPrimaryPad()
                    && mGameMode->isInputAllowed(EControllerActions_CycleHeldItemLeft)
                    && mGameMode->isInputAllowed(EControllerActions_CycleHeldItemRight))
                    hotbarScroll = PcInputConsumeHotbarWheel();
#endif

                if (hotbarScroll != 0)
                {
                    for (int step = hotbarScroll < 0 ? -hotbarScroll : hotbarScroll; step > 0; --step)
                        mPlayer->mInventory->swapPaint(hotbarScroll);

                    if (mGameMode != NULL && mGameMode->getTutorial() != NULL)
                        mGameMode->getTutorial()->onSelectedItemChanged(mPlayer->mInventory->getSelected());

                    mPlayer->updateRichPresence();

                    if (mOptions->mScrollHotbar)
                    {
                        if (hotbarScroll < 0)
                            hotbarScroll = -1;
                        else if (hotbarScroll > 0)
                            hotbarScroll = 1;

                        mOptions->mHotbarScroll += (float)hotbarScroll * 0.25f;
                    }
                }

                bool pickBlock = g_pInput->ButtonDown(playerIdx, EControllerActions_PickBlock);
#ifdef MINECRAFT_PC
                if (playerIdx == gProfile->GetPrimaryPad() && PcInputBindingHeld(ePcKey_PickBlock)
                    && PcInputActionButtons(EControllerActions_PickBlock) == 0)
                    pickBlock = true;
#endif
                if (pickBlock)
                    mPlayer->handleMouseClick(2);

                if (mPlayer->isUsingItem())
                {
                    if (!g_pInput->ButtonDown(playerIdx, EControllerActions_Use))
                        mGameMode->releaseUsingItem(mPlayer);
                }
                else if (mGameMode->isInputAllowed(EControllerActions_Use)
                         && !g_pInput->ButtonDown(playerIdx, EControllerActions_Action))
                {
                    if (mPlayer->mAbilities.mInstabuild)
                    {
                        unsigned int used = mPlayer->creativeModeHandleMouseClick(
                            1, g_pInput->ButtonDown(playerIdx, EControllerActions_Use));
                        if (mPlayer->mCreativeRepeatState == 4)
                        {
                            if (used)
                            {
                                mPlayer->mLastUseTick = mTicks;
                            }
                            else if (g_pInput->ButtonDown(playerIdx, EControllerActions_Use)
                                     && (float)(mTicks - mPlayer->mLastUseTick) >= mTimer->ticksPerSecond * 0.25f)
                            {
                                mPlayer->handleMouseClick(1);
                                mPlayer->mLastUseTick = mTicks;
                            }
                        }
                    }
                    else
                    {
                        bool neverUsed = mPlayer->mLastUseTick == 0;
                        bool useDelayElapsed = (float)(mTicks - mPlayer->mLastUseTick)
                                               >= mTimer->ticksPerSecond * 0.25f;
                        if (mPlayer->isPassenger() || mPlayer->isSprinting() || mPlayer->isSleeping())
                            useDelayElapsed = false;

                        if (g_pInput->ButtonDown(playerIdx, EControllerActions_Use))
                        {
                            if (mPlayer->isSleeping())
                                mPlayer->mLastUseTick = (int)((float)mTicks + mTimer->ticksPerSecond * 2);

                            if (neverUsed || useDelayElapsed)
                            {
                                bool sleeping = mPlayer->isSleeping();
                                mPlayer->handleMouseClick(1);
                                if (sleeping)
                                    mPlayer->mLastUseTick = (int)((float)mTicks + mTimer->ticksPerSecond * 2);
                                else
                                    mPlayer->mLastUseTick = mTicks;
                            }
                        }
                        else
                        {
                            mPlayer->mLastUseTick = 0;
                        }
                    }
                }

                if (mPlayer->mMissTime > 0)
                    --mPlayer->mMissTime;

                bool achievementsIdle = false;
                if ((mPlayer->mButtonsDown & (1LL << EControllerActions_Inventory)) != 0)
                {
                    achievementsIdle = !gConsoleUIController.GetHoldToShowAchievementsActive(true)
                                       && !gConsoleUIController.GetAchievementToastShowing(playerIdx);
                }
                bool openInventory = gConsoleUIController.GetHoldActionCancelled(playerIdx) || achievementsIdle;

                if (openInventory
                    && !(mGameMode->isInputAllowed(EControllerActions_Use)
                         && g_pInput->ButtonDown(playerIdx, EControllerActions_Use)))
                {
                    if (mPlayer->mHasOpenedInventory && mGameMode->isInputAllowed(EControllerActions_Inventory))
                    {
                        boost::shared_ptr<MultiplayerLocalPlayer> player = GetInstance()->mPlayer;
                        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
                        if (mGameMode->isServerControlledInventory())
                            player->sendOpenInventory();
                        else
                            gApp.LoadInventoryMenu(playerIdx, player, false);
                    }
                }

                if (!InMiniGame(eMiniGameId_Any, true) || GetInstance()->GetMiniGame()->CanCraft())
                {
                    if ((mPlayer->mButtonsDown & (1LL << EControllerActions_Crafting)) != 0
                        && mGameMode->isInputAllowed(EControllerActions_Crafting))
                    {
                        boost::shared_ptr<MultiplayerLocalPlayer> player = GetInstance()->mPlayer;
                        if (mGameMode->hasInfiniteItems())
                        {
                            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
                            gApp.LoadCreativeMenu(playerIdx, player, false);
                        }
                        else if (mHitResult != NULL && mHitResult->mType == 1
                                 && mLevel->getBlockState(mHitResult->getBlockPos())->getBlock()
                                        == Blocks::crafting_table)
                        {
                            bool cancelled = false;
                            mGameMode->useItemOn(player, mLevel, mHitResult->getBlockPos(), Direction::UP,
                                                 mHitResult->mLocation, InteractionHand::eInteractionHand_MainHand,
                                                 false, &cancelled);
                        }
                        else
                        {
                            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
                            gApp.LoadCrafting2x2Menu(playerIdx, player);
                        }
                    }
                }
                else if ((mPlayer->mButtonsDown & (1LL << EControllerActions_ConfirmReady)) != 0
                         && mGameMode->isInputAllowed(EControllerActions_ConfirmReady)
                         && mPlayer->GetGameType() == GameType::LOBBY)
                {
                    mPlayer->mPacketListener->send(boost::shared_ptr<PlayerReadyPacket>(new PlayerReadyPacket(
                        mPlayer->mPlayerIndex,
                        !GetInstance()->GetClientMasterGameMode()->IsPlayerReady(mPlayer->mPlayerIndex))));
                }

                if (!gConsoleUIController.GetMenuDisplayed(playerIdx))
                {
                    ClientMasterGameMode* masterGameMode = GetInstance()->GetClientMasterGameMode();
                    if (!masterGameMode->IsInScoringState()
                        && (mPlayer->mButtonsDown & (1LL << EControllerActions_Vote)) != 0
                        && mGameMode->isInputAllowed(EControllerActions_Vote))
                    {
                        std::vector<ClientMasterGameMode::MapSelectInfo> playlist;
                        masterGameMode->GetUniquePlaylist(playlist);

                        if (mPlayer->GetGameType() == GameType::LOBBY
                            && (playlist.size() >= 2
                                || (!InMiniGame(eMiniGameId_Battle, true) && GetMiniGame()->HasMultipleVariants())))
                        {
                            int primaryPad = gProfile->GetPrimaryPad();
                            if (CGameNetworkManager::sInstance.IsInPublicJoinableGame())
                            {
                                if (GetMiniGame()->HostOption_Bool(eCustomGameModeHostOption_EnableVoting))
                                {
                                    gConsoleUIController.NavigateToScene(playerIdx, EUIScene_MapVoteMenu, NULL,
                                                                         EUILayer_Scene, EUIGroup_PlayerDefault);
                                }
                            }
                            else if (primaryPad == playerIdx && CGameNetworkManager::sInstance.IsHost())
                            {
                                Minecraft* minecraft = GetInstance();
                                if (minecraft != NULL
                                    && minecraft->mLocalPlayers[playerIdx]->GetGameType() == GameType::LOBBY
                                    && GetMiniGame()->HostOption_Bool(eCustomGameModeHostOption_EnableVoting))
                                {
                                    gConsoleUIController.NavigateToScene(playerIdx, EUIScene_MapVoteMenu, (void*)1,
                                                                         EUILayer_Scene, EUIGroup_PlayerDefault);
                                }
                            }
                        }
                    }
                }

                if ((mPlayer->mButtonsDown & (1LL << EControllerActions_Drop)) != 0
                    && mGameMode->isInputAllowed(EControllerActions_Drop)
                    && !gConsoleUIController.GetMenuDisplayed(playerIdx))
                {
                    mPlayer->drop();
                }

                int touchX = 0;
                int touchY = 0;
                bool touching = false;
                unsigned short* touchData = g_pInput->GetTouchPadData(playerIdx, C_4JInput::ETouchType_0, true);
                if (touchData != NULL && touchData[2] == 1 && touchData[3] == 0)
                {
                    touching = gConsoleUIController.GetDRCEnabled();
                    touchX = touchData[0];
                    touchY = touchData[1];
                }

                InMiniGame(eMiniGameId_Tumble, true);
                bool quickSelected = false;
                if (touching && gConsoleUIController.GetQuickSelectVisible(playerIdx))
                {
                    int hotBarX = gConsoleUIController.GetHotBarX(playerIdx);
                    int hotBarY = gConsoleUIController.GetHotBarY(playerIdx);
                    int hotBarWidth = gConsoleUIController.GetHotBarWidth(playerIdx);
                    int hotBarHeight = gConsoleUIController.GetHotBarHeight(playerIdx);
                    if (touchX > hotBarX && touchX < hotBarX + hotBarWidth && touchY > hotBarY
                        && touchY < hotBarY + hotBarHeight)
                    {
                        int slot = (touchX - hotBarX) / (hotBarWidth / 9);
                        if (slot > 8)
                            slot = 8;
                        mPlayer->mInventory->m_selected = slot;
                        quickSelected = true;
                    }
                }

                if (quickSelected || hotbarScroll != 0
                    || (mPlayer->mButtonsDown & (1LL << EControllerActions_Drop)) != 0
                    || (pickBlock && mPlayer->mInventory->hasPickedItem()))
                {
                    std::wstring itemName(L"");
                    const boost::shared_ptr<ItemInstance>& mainHandItem = mPlayer->getMainHandItem();
                    if (!mainHandItem->isEmpty())
                    {
                        if ((mPlayer->mButtonsDown & (1LL << EControllerActions_Drop)) == 0
                            || mainHandItem->getCount() != 1)
                            itemName = mainHandItem->getHoverName();
                    }

                    if ((mPlayer->mButtonsDown & (1LL << EControllerActions_Drop)) == 0
                        || (!mainHandItem->isEmpty() && mainHandItem->getCount() < 2))
                    {
                        if (!mainHandItem->isEmpty())
                        {
                            std::vector<HtmlString>* tooltipLines = mainHandItem->getTooltipLines(mPlayer, false);
                            if (!tooltipLines->empty())
                            {
                                tooltipLines->at(0).mColour = mainHandItem->getRarity()->colour;
                                if (mainHandItem->hasCustomHoverName())
                                    tooltipLines->at(0).mColour = eMinecraftColour_Color_RenamedItemTitle;
                            }

                            for (int line = (int)tooltipLines->size() - 1; line >= 0; --line)
                            {
                                tooltipLines->at(line).mIndent = false;
                                stripWhitespaceForHtml(tooltipLines->at(line).mText, true);
                                if (tooltipLines->at(line).mText == L"")
                                    tooltipLines->erase(tooltipLines->begin() + line);
                            }

                            if (tooltipLines->size() > 4)
                            {
                                HtmlString ellipsis(L"...", eMinecraftColour_NOTSET, false, false, 0);
                                while (tooltipLines->size() > 4)
                                    tooltipLines->pop_back();
                                tooltipLines->push_back(ellipsis);
                            }

                            gConsoleUIController.SetSelectedItem(playerIdx, HtmlString::Compose(tooltipLines), L"");
                            delete tooltipLines;
                        }
                        else
                        {
                            gConsoleUIController.SetSelectedItem(playerIdx, L"", L"");
                        }
                    }
                }

                if (InMiniGame(eMiniGameId_Glide, false))
                {
                    MultiPlayerLevel* level = mPlayer->GetLevel();
                    ClientScoreboard* scoreboard = level->GetClientScoreboard();
                    int playerCount = 0;
                    for (int i = 0; i < 4; ++i)
                    {
                        if (mLocalPlayers[i]
                            && (!mLocalPlayers[i]->isSpectator()
                                || scoreboard->GetEndZoneEntered(mLocalPlayers[i]->getSUID(), -1)))
                        {
                            ++playerCount;
                        }
                    }

                    if (playerCount == 1)
                    {
                        ClientMasterGameMode* masterGameMode = GetClientMasterGameMode();
#ifdef MINECRAFT_PC
                        if ((mPlayer->mButtonsDown & (1LL << EControllerActions_CycleHeldItemRight)) != 0
                            && GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
                        {
                            PcSettingsToggleGlideGhost();
                            printf("[pc] glide: ghost %s\n", PcSettingsGlideGhostHidden() ? "hidden" : "shown");
                        }
#endif
                        if (GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0
                            && masterGameMode->IsRoundPlaying()
                            && !scoreboard->GetEndZoneEntered(mPlayer->getSUID(), -1))
                        {
#ifdef MINECRAFT_PC
                            if (s_pcGlideHold[playerIdx][0])
                            {
                                UIScene_PcGlideSections::BackTwoSections(playerIdx);
                                mPlayer->ResetPercentageTraversed();
                                mPlayer->ResetThermalVariables();
                            }
                            if (s_pcGlideHold[playerIdx][1])
                            {
                                gConsoleUIController.NavigateToScene(playerIdx, EUIScene_PcGlideSections, NULL,
                                                                     EUILayer_Scene, EUIGroup_PlayerDefault);
                            }
#endif
                            bool resetTraversal = false;
                            if ((mPlayer->mButtonsDown & (1LL << EControllerActions_RestartSection)) != 0)
                            {
                                boost::shared_ptr<Packet> packet = CheckpointCommand::preparePacket(mPlayer->getSUID(),
                                                                                                    -1);
                                mPlayer->mPacketListener->send(packet);
                                resetTraversal = true;
                            }
                            if ((mPlayer->mButtonsDown & (1LL << EControllerActions_RestartRace)) != 0)
                            {
                                mPlayer->mPacketListener->send(CheckpointCommand::preparePacket(mPlayer->getSUID(), 0));
                                resetTraversal = true;
                            }
                            if (resetTraversal)
                            {
                                mPlayer->ResetPercentageTraversed();
                                mPlayer->ResetThermalVariables();
                            }
                        }
                    }
                }
            }
            else
            {
                MultiPlayerLevel* level = mLocalPlayers[playerIdx]->GetLevel();
                ClientScoreboard* scoreboard = NULL;
                if (level != NULL)
                    scoreboard = level->GetClientScoreboard();
                if (scoreboard == NULL || !scoreboard->HasWinner(-1))
                {
                    int activePlayers = GetInstance()->mLevel->GetClientScoreboard()->GetActivePlayerCount(
                        eMiniGameId_Any);
                    if (!GetClientMasterGameMode()->IsRoundEnding() && activePlayers > 1)
                    {
                        Tooltips::OverwriteTooltip(playerIdx, EControllerActions_SpectatePrevious, 0x2ECE48BA);
                        Tooltips::OverwriteTooltip(playerIdx, EControllerActions_SpectateNext,
                                                   IDS_TOOLTIPS_SPECTATE_NEXT);

#ifdef MINECRAFT_PC
                        const int wheel = playerIdx == gProfile->GetPrimaryPad() ? PcInputConsumeHotbarWheel() : 0;
#else
                        const int wheel = 0;
#endif
                        if ((g_pInput->GetValue(playerIdx, EControllerActions_SpectatePrevious, true) || wheel > 0)
                            && mGameMode->isInputAllowed(EControllerActions_SpectatePrevious))
                        {
                            mPlayer->SpectatePlayerPrev();
                        }
                        else if ((g_pInput->GetValue(playerIdx, EControllerActions_SpectateNext, true) || wheel < 0)
                                 && mGameMode->isInputAllowed(EControllerActions_SpectateNext))
                        {
                            mPlayer->SpectatePlayerNext();
                        }
                    }
                    else
                    {
                        Tooltips::ResetTooltip(playerIdx, EControllerActions_SpectatePrevious);
                        Tooltips::ResetTooltip(playerIdx, EControllerActions_SpectateNext);
                    }

                    if (mPlayer->IsSpectatingOtherPlayer() && !GetMiniGame()->ForceSpectateModeFollow())
                    {
                        Tooltips::OverwriteTooltip(playerIdx, EControllerActions_SpectateCancel, IDS_TOOLTIPS_BACK);
                        Tooltips::ResetTooltip(playerIdx, EControllerActions_Action);
                        Tooltips::ResetTooltip(playerIdx, EControllerActions_SpectateNoise);
                        if (g_pInput->GetValue(playerIdx, EControllerActions_SpectateCancel, true)
                            && mGameMode->isInputAllowed(EControllerActions_SpectateCancel))
                        {
                            mGui->clearSingleMessages(playerIdx);
                            mPlayer->StopSpectatingPlayer();
                        }
                    }
                    else if (mPlayer->SpectateAsOtherEntity() || mPlayer->SpectateAsInvisible()
                             || mPlayer->SpectateAsFloatingHead())
                    {
                        Tooltips::ResetTooltip(playerIdx, EControllerActions_SpectateCancel);

                        if (GetInstance()->GetMiniGame()->HasSpectatorParticipation() && mHitResult != NULL
                            && mHitResult->mType == 1
                            && GetInstance()->mGameModes[playerIdx]->MayDestroy(mHitResult->getBlockPos())
                            && mPlayer->isAllowedToMine())
                        {
                            Tooltips::OverwriteTooltip(playerIdx, EControllerActions_Action, IDS_TOOLTIPS_MINE);
                            Tooltips::ResetTooltip(playerIdx, EControllerActions_SpectateNoise);
                        }
                        else if (GetInstance()->mGameModes[playerIdx]->SpectateSoundEnabled())
                        {
                            Tooltips::ResetTooltip(playerIdx, EControllerActions_Action);
                            long long spectateEntity = mPlayer->GetSpectateEntity();
                            if (spectateEntity == EntityType::VEX)
                                Tooltips::OverwriteTooltip(playerIdx, EControllerActions_SpectateNoise, 0x9D34B5A5);
                            else if (spectateEntity == EntityType::PARROT)
                                Tooltips::OverwriteTooltip(playerIdx, EControllerActions_SpectateNoise, 0xC1781F1C);
                            else if (spectateEntity == EntityType::BAT)
                                Tooltips::OverwriteTooltip(playerIdx, EControllerActions_SpectateNoise, 0x162E9829);
                            else
                                Tooltips::ResetTooltip(playerIdx, EControllerActions_SpectateNoise);

                            if (GetInstance()->mGameModes[playerIdx]->CanMakeSpectateSound()
                                && g_pInput->GetValue(playerIdx, EControllerActions_SpectateNoise, true)
                                && mGameMode->isInputAllowed(EControllerActions_SpectateNoise))
                            {
                                GetInstance()->mGameModes[playerIdx]->MakeSpectateSound();
                            }
                        }
                    }
                    else
                    {
                        Tooltips::ResetTooltip(playerIdx, EControllerActions_SpectateCancel);
                        Tooltips::ResetTooltip(playerIdx, EControllerActions_Action);
                        Tooltips::ResetTooltip(playerIdx, EControllerActions_SpectateNoise);
                    }
                }
                else
                {
                    Tooltips::ResetAllTooltips(playerIdx);
                }
            }

            Tooltips::DisplayTooltips(playerIdx);
        }
    }
    else
    {
        if (g_pInput->GetValue(playerIdx, EControllerActions_MenuCancel, false)
            && mGameMode->isInputAllowed(EControllerActions_MenuCancel))
            setScreen(NULL);

        mGameMode->stopDestroyBlock();

        if (mPlayer->isUsingItem())
            mGameMode->releaseUsingItem(mPlayer);
    }

    if (mLevel != NULL)
    {
        if (mPlayer)
        {
            if (++mFrameNumber == 30)
                mFrameNumber = 0;
            else if (mFrameNumber % 30 == playerIdx * 7 + 7)
                mLevel->ensureAdded(mPlayer);
        }

        PIXBeginNamedEvent(0.0f, "Game renderer tick");
        if (!mAppPaused)
            mGameRenderer->tick(handleInput);
        PIXEndNamedEvent();

        static unsigned int levelTickMask = 0;
        if (handleInput)
        {
            levelTickMask = 0;
            SparseLightStorage::tick();
            CompressedBlockStorage::tick();
            SparseDataStorage::tick();
            mClientMasterGameMode->Tick();
        }

        for (int i = 0; i < mLevels.length; ++i)
        {
            if (mPlayer->mLevel != mLevels[i])
                continue;

            if (!mAppPaused && mLevels[i] != NULL)
            {
                int x = Mth::floor(mPlayer->mX);
                int y = Mth::floor(mPlayer->mY);
                int z = Mth::floor(mPlayer->mZ);
                mLevels[i]->animateTick(x, y, z);
            }

            unsigned int levelMask = 1u << i;
            if ((levelTickMask & levelMask) != 0)
                continue;
            levelTickMask |= levelMask;

            PIXBeginNamedEvent(0.0f, "Level renderer tick");
            if (!mAppPaused)
                mLevelRenderer->tick();
            PIXEndNamedEvent();

            if (mLevels[i] != NULL)
            {
                if (!mAppPaused)
                {
                    int skyFlashTime = mLevels[i]->getSkyFlashTime();
                    if (skyFlashTime > 0)
                        mLevels[i]->setSkyFlashTime(skyFlashTime - 1);

                    PIXBeginNamedEvent(0.0f, "Level entity tick");
                    mLevels[i]->tickEntities();
                    PIXEndNamedEvent();
                }

                if (!mAppPaused)
                {
                    mLevels[i]->setSpawnSettings(mLevel->getDifficulty() != Difficulty::PEACEFUL, true);
                    PIXBeginNamedEvent(0.0f, "Level tick");
                    mLevels[i]->tick();
                    PIXEndNamedEvent();
                }
            }
            else
            {
                if (mGameRenderer->postEffectActive())
                    mGameRenderer->shutdownEffect();
            }
        }
        if (handleInput)
        {
            PIXBeginNamedEvent(0.0f, "Particle tick");
            if (!mAppPaused)
                mParticleEngine->tick();
            PIXEndNamedEvent();
        }

        if (mAppPaused)
            tickAllConnections();
    }
}

void Minecraft::renderInfoBars()
{
    InfoBarManager::Render((float)mWidth, (float)mHeight);
}

void Minecraft::renderChunkInfoMap()
{
    ChunkInfoMapManager::Render((float)mWidth, (float)mHeight);
}

void Minecraft::run_middle()
{
    if (sFrameStartTime == 0)
        sFrameStartTime = System::processTimeInNanoSecs();

    EnterCriticalSection(&mLevelLock);
    if (!mRunning)
    {
        LeaveCriticalSection(&mLevelLock);
        return;
    }

#ifdef MINECRAFT_PC
    {
        const int primary = gProfile->GetPrimaryPad();
        PcInputSetGlideKeys(mLevel != NULL && InMiniGame(eMiniGameId_Glide, false)
                            && GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0
                            && gApp.GetLocalPlayerCount() == 1 && primary >= 0 && primary < 4
                            && mLocalPlayers[primary] != NULL);
        const bool local = primary >= 0 && primary < 4 && mLocalPlayers[primary] != NULL;
        PcInputSetMiniGameKeys(local && mLocalPlayers[primary]->GetGameType() == GameType::LOBBY,
                               local && mLocalPlayers[primary]->isSpectator(),
                               local && mLevel != NULL && InMiniGame(eMiniGameId_Glide, false));
    }
#endif

    if (mReloadTextures)
    {
        mReloadTextures = false;
        mTextures->reloadAll();
    }

    AABB::resetPool();
    Vec3::resetPool();

    if (mLevel != NULL && gProfile->IsFullVersion() && CGameNetworkManager::sInstance.IsHost()
        && !InMiniGame(eMiniGameId_Any, true) && !gStorage->GetSaveDisabled())
    {
        if (gApp.GetXuiAction(gProfile->GetPrimaryPad()) == eXuiAction_Idle)
        {
            if (gConsoleUIController.IsPauseMenuDisplayed(gProfile->GetPrimaryPad())
                || gConsoleUIController.IsIgnoreAutosaveMenuDisplayed(gProfile->GetPrimaryPad()))
            {
                gConsoleUIController.ShowAutosaveCountdownTimer(false);
            }
            else
            {
                bool autosaveAllowed = GameSettings::GetGameSettings(gProfile->GetPrimaryPad(), eGameSetting_Autosave)
                                       != 0;
                if (GetInstance()->mTexturePackRepository->getSelected() != TexturePackRepository::defaultTexturePack)
                {
                    DLCPack* parentPack = static_cast<DLCTexturePack*>(
                                              GetInstance()->mTexturePackRepository->getSelected())
                                              ->getDLCInfoParentPack();
                    if (parentPack != NULL && !parentPack->hasPurchasedPack())
                    {
                        autosaveAllowed = false;
                    }
                }

                if (autosaveAllowed)
                {
                    if (!gApp.AutosaveDue())
                    {
                        unsigned int seconds = gApp.SecondsToAutosave();
                        if (seconds <= 5)
                        {
                            gConsoleUIController.ShowAutosaveCountdownTimer(true);
                            gConsoleUIController.UpdateAutosaveCountdownTimer(seconds);
                        }
                    }
                    else
                    {
                        gConsoleUIController.ShowAutosaveCountdownTimer(false);
                        gApp.SetAction(gProfile->GetPrimaryPad(), eXuiAction_CheckSaveExists, NULL);
                    }
                }
            }
        }
    }

    for (int i = 0; i < 4; ++i)
    {
        if (mLocalPlayers[i] && !gApp.GetBanListCheck(i) && !GetInstance()->isTutorial() && gProfile->IsSignedInLive(i)
            && !gProfile->IsGuest(i) && !gProfile->IsSystemUIDisplayed())
        {
            gApp.SetBanListCheck(i, true);
        }
    }

    if (!gProfile->IsSystemUIDisplayed() && gApp.DLCInstallProcessCompleted() && !gApp.DLCInstallPending()
        && gApp.mDLCManager.m_bCorruptDLCFound)
    {
        gApp.mDLCManager.checkForCorruptDLCAndAlert(true);
    }

    if (!gApp.IsDrcPreventingSplitscreen() && mLevel != NULL && sCheckForNewLocalPlayers)
    {
        if (CGameNetworkManager::sInstance.SessionHasSpace(1))
        {
            if (sPressStartHoldOffFrames != 0)
            {
                --sPressStartHoldOffFrames;
            }
            else
            {
                sCheckForNewLocalPlayers = false;
                if (gApp.IsLocalMultiplayerAvailable())
                {
                    for (int i = 0; i < 4; ++i)
                    {
                        if (!mLocalPlayers[i] && g_pInput->IsPadConnected(i)
                            && !gConsoleUIController.PressStartPlaying(i))
                        {
                            gConsoleUIController.ShowPressStart(i);
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < 4; ++i)
    {
        if (!mLocalPlayers[i])
        {
            bool joinPressed = !mAppPaused
                               && !gConsoleUIController.IsIgnorePlayerJoinMenuDisplayed(gProfile->GetPrimaryPad())
                               && CGameNetworkManager::sInstance.SessionHasSpace(1) && gRenderer->IsHiDef()
                               && g_pInput->ButtonPressed(i, 0xff, 0);
            if (gRenderer->IsWidescreen() && joinPressed)
            {
                if (!gConsoleUIController.PressStartPlaying(i))
                {
                    gConsoleUIController.ShowPressStart(i);
                }
                else if (g_pInput->ButtonPressed(i, EControllerActions_Pause, 0) && !gApp.IsDrcPreventingSplitscreen())
                {
                    if (gProfile->IsSignedIn(i))
                    {
                        if (!CGameNetworkManager::sInstance.IsLocalGame()
                            && (!gProfile->IsSignedInLive(i) || !gProfile->AllowedToPlayMultiplayer(i)))
                        {
                            if (gProfile->IsSignedInLive(gProfile->GetPrimaryPad())
                                && !gProfile->AllowedToPlayMultiplayer(i))
                            {
                                gProfile->RequestConvertOfflineToGuestUI(InGame_SignInReturned, this, i);
                                gConsoleUIController.HidePressStart();
                                int messageParameters[1];
                                messageParameters[0] = IDS_CONFIRM_OK;
                                gConsoleUIController.RequestErrorMessage(0x252AB13F, 0xCC5A590F, messageParameters, 1,
                                                                         i, NULL, NULL, NULL, true);
                            }
                            gApp.SetDRCIconEnabled(false);
                            gApp.SetDRCCheckPlayerCountEnabled(false);
                            gProfile->RequestSignInUI(0, CGameNetworkManager::sInstance.IsLocalGame(), 1, 0, 1,
                                                      InGame_SignInReturned, this, i);
                        }
                        else
                        {
                            gApp.SetDRCIconEnabled(false);
                            gApp.SetDRCCheckPlayerCountEnabled(false);
                            if (!mLevel->mIsClientSide)
                            {
                                boost::shared_ptr<MultiplayerLocalPlayer> player = mLocalPlayers[i];
                                if (player == NULL)
                                {
                                    player = createExtraLocalPlayer(
                                        i, convStringToWstring(gProfile->GetGamertag(i)).c_str(), i,
                                        mLevel->mDimension->getType()->getId(), NULL, NULL, 0, 0);
                                }
                            }
                            else if (!addLocalPlayer(i))
                            {
                                gProfile->RequestSignInUI(0, CGameNetworkManager::sInstance.IsLocalGame(), 1, 0, 1,
                                                          InGame_SignInReturned, this, i);
                            }
                        }
                    }
                    else
                    {
                        gApp.SetDRCIconEnabled(false);
                        gApp.SetDRCCheckPlayerCountEnabled(false);
                        gProfile->RequestSignInUI(0, CGameNetworkManager::sInstance.IsLocalGame(), 1, 0, 1,
                                                  InGame_SignInReturned, this, i);
                    }
                }
            }
            continue;
        }

        gApp.ProcessFakePadValueA();

        if (g_pInput->ButtonPressed(i, EControllerActions_Jump,
                                    GameSettings::GetInputButton(i, true, EControllerActions_Jump)))
            mLocalPlayers[i]->mButtonsDown |= 1LL << EControllerActions_Jump;
        if (g_pInput->ButtonPressed(i, EControllerActions_Use,
                                    GameSettings::GetInputButton(i, true, EControllerActions_Use)))
            mLocalPlayers[i]->mButtonsDown |= 1LL << EControllerActions_Use;
        if (g_pInput->ButtonPressed(i, EControllerActions_Inventory,
                                    GameSettings::GetInputButton(i, true, EControllerActions_Inventory)))
            mLocalPlayers[i]->mButtonsDown |= 1LL << EControllerActions_Inventory;
        if (g_pInput->ButtonPressed(i, EControllerActions_Action,
                                    GameSettings::GetInputButton(i, true, EControllerActions_Action)))
            mLocalPlayers[i]->mButtonsDown |= 1LL << EControllerActions_Action;
        if (g_pInput->ButtonPressed(i, EControllerActions_Crafting,
                                    GameSettings::GetInputButton(i, true, EControllerActions_Crafting)))
            mLocalPlayers[i]->mButtonsDown |= 1LL << EControllerActions_Crafting;
        if (g_pInput->ButtonPressed(i, EControllerActions_Pause, 0))
            mLocalPlayers[i]->mButtonsDown |= 1LL << EControllerActions_Pause;
        if (g_pInput->ButtonPressed(i, EControllerActions_Drop,
                                    GameSettings::GetInputButton(i, true, EControllerActions_Drop)))
            mLocalPlayers[i]->mButtonsDown |= 1LL << EControllerActions_Drop;
        if ((!mLocalPlayers[i]->mAbilities.mFlying && (!mLocalPlayers[i]->isInWater() || mLocalPlayers[i]->mOnGround))
                ? g_pInput->ButtonPressed(i, EControllerActions_Sneak,
                                          GameSettings::GetInputButton(i, true, EControllerActions_Sneak))
                : g_pInput->ButtonDown(i, EControllerActions_Sneak))
        {
            mLocalPlayers[i]->mButtonsDown |= 1LL << EControllerActions_Sneak;
        }
        if (g_pInput->ButtonPressed(i, EControllerActions_ChangeCameraMode,
                                    GameSettings::GetInputButton(i, true, EControllerActions_ChangeCameraMode)))
            mLocalPlayers[i]->mButtonsDown |= 1LL << EControllerActions_ChangeCameraMode;
        if (g_pInput->ButtonPressed(i, EControllerActions_Scoreboard, 0))
            mLocalPlayers[i]->mButtonsDown |= 1LL << EControllerActions_Scoreboard;
        if (g_pInput->ButtonPressed(i, EControllerActions_ConfirmReady,
                                    GameSettings::GetInputButton(i, true, EControllerActions_ConfirmReady)))
            mLocalPlayers[i]->mButtonsDown |= 1LL << EControllerActions_ConfirmReady;
        if (g_pInput->ButtonPressed(i, EControllerActions_Vote,
                                    GameSettings::GetInputButton(i, true, EControllerActions_Vote)))
            mLocalPlayers[i]->mButtonsDown |= 1LL << EControllerActions_Vote;
#ifdef MINECRAFT_PC
        {
            static const EControllerActions kRestartActions[2] = {EControllerActions_RestartSection,
                                                                  EControllerActions_RestartRace};
            const unsigned long long now = System::processTimeInMilliSecs();
            for (int restart = 0; restart < 2; ++restart)
            {
                const bool down = g_pInput->ButtonDown(i, kRestartActions[restart]);
                if (gConsoleUIController.GetMenuDisplayed(i) || s_pcGlideBlocked[i][restart])
                {
                    s_pcGlideBlocked[i][restart] = down || gConsoleUIController.GetMenuDisplayed(i);
                    s_pcGlideWasDown[i][restart] = false;
                    continue;
                }
                if (down && !s_pcGlideWasDown[i][restart])
                {
                    s_pcGlideHoldStart[i][restart] = now;
                    s_pcGlideHoldFired[i][restart] = false;
                }
                else if (down && !s_pcGlideHoldFired[i][restart]
                         && now - s_pcGlideHoldStart[i][restart] >= kPcGlideHoldMilliseconds)
                {
                    s_pcGlideHoldFired[i][restart] = true;
                    s_pcGlideHold[i][restart] = true;
                }
                else if (!down && s_pcGlideWasDown[i][restart] && !s_pcGlideHoldFired[i][restart])
                {
                    mLocalPlayers[i]->mButtonsDown |= 1LL << kRestartActions[restart];
                }
                s_pcGlideWasDown[i][restart] = down;
            }
            if (PcInputGlideKeys() && g_pInput->ButtonPressed(i, EControllerActions_CycleHeldItemRight, 0))
                mLocalPlayers[i]->mButtonsDown |= 1LL << EControllerActions_CycleHeldItemRight;
        }
#else
        if (g_pInput->ButtonPressed(i, EControllerActions_RestartSection,
                                    GameSettings::GetInputButton(i, true, EControllerActions_RestartSection)))
            mLocalPlayers[i]->mButtonsDown |= 1LL << EControllerActions_RestartSection;
        if (g_pInput->ButtonPressed(i, EControllerActions_RestartRace,
                                    GameSettings::GetInputButton(i, true, EControllerActions_RestartRace)))
            mLocalPlayers[i]->mButtonsDown |= 1LL << EControllerActions_RestartRace;
#endif

        if (GetMiniGame()->HasFeatureLookBack() && mLocalPlayers[i]->mThirdPersonView != 2)
            mLocalPlayers[i]->mDRCTouching = g_pInput->ButtonDown(i, EControllerActions_LookBehind);

        int moveButtons = 0;
        mLocalPlayers[i]->mFlyButtonsDown[0] = 0;
        mLocalPlayers[i]->mFlyButtonsDown[1] = 0;
        if (g_pInput->ButtonDown(i, EControllerActions_FlyLeft))
        {
            ++moveButtons;
            mLocalPlayers[i]->mFlyButtonsDown[0] |= 0x400;
        }
        if (g_pInput->ButtonDown(i, EControllerActions_FlyRight))
        {
            ++moveButtons;
            mLocalPlayers[i]->mFlyButtonsDown[0] |= 0x800;
        }
        if (g_pInput->ButtonDown(i, EControllerActions_FlyUp))
        {
            ++moveButtons;
            mLocalPlayers[i]->mFlyButtonsDown[0] |= 0x1000;
        }
        if (g_pInput->ButtonDown(i, EControllerActions_FlyDown))
        {
            ++moveButtons;
            mLocalPlayers[i]->mFlyButtonsDown[0] |= 0x2000;
        }

        if (moveButtons < 2)
        {
            mLocalPlayers[i]->mLastSingleFlyButtons[0] = mLocalPlayers[i]->mFlyButtonsDown[0];
            mLocalPlayers[i]->mLastSingleFlyButtons[1] = mLocalPlayers[i]->mFlyButtonsDown[1];
            mLocalPlayers[i]->mFlyButtons[0] = mLocalPlayers[i]->mFlyButtonsDown[0];
            mLocalPlayers[i]->mFlyButtons[1] = mLocalPlayers[i]->mFlyButtonsDown[1];
        }
        else
        {
            mLocalPlayers[i]->mFlyButtons[0] = mLocalPlayers[i]->mLastSingleFlyButtons[0];
            mLocalPlayers[i]->mFlyButtons[1] = mLocalPlayers[i]->mLastSingleFlyButtons[1];
        }

        if (g_pInput->ButtonPressed(i, EControllerActions_CycleHeldItemLeft, 0)
            || g_pInput->ButtonPressed(i, EControllerActions_CycleHeldItemRight, 0))
            gApp.SetOpacityTimer(i);
    }

    if (!mAppPaused && !gConsoleUIController.IsIgnorePlayerJoinMenuDisplayed(gProfile->GetPrimaryPad())
        && CGameNetworkManager::sInstance.SessionHasSpace(1) && gRenderer->IsHiDef())
    {
        int freeSlot = 0;
        if (mLocalPlayers[0])
        {
            if (!mLocalPlayers[1])
                freeSlot = 1;
            else if (!mLocalPlayers[2])
                freeSlot = 2;
            else if (!mLocalPlayers[3])
                freeSlot = 3;
        }

        int lockedPads = 0;
        for (int pad = 4; pad < 9; ++pad)
        {
            if (g_pInput->IsPadLocked(pad))
                ++lockedPads;
        }

        if (lockedPads < 4)
        {
            for (int pad = 4; pad < 9; ++pad)
            {
                bool locked = g_pInput->IsPadLocked(pad);
                bool connected = g_pInput->IsPadConnected(pad);
                bool pressed = g_pInput->ButtonPressed(pad, 0xff, 0);
                if (!locked && connected && pressed)
                {
                    if (!gConsoleUIController.PressStartPlaying(freeSlot))
                    {
                        gConsoleUIController.ShowPressStart(freeSlot);
                    }
                    else if (!gApp.IsDrcPreventingSplitscreen()
                             && g_pInput->ButtonPressed(pad, EControllerActions_Pause, 0))
                    {
                        gApp.SetDRCIconEnabled(false);
                        gApp.SetDRCCheckPlayerCountEnabled(false);
                        gProfile->RequestSignInUI(0, CGameNetworkManager::sInstance.IsLocalGame(), 1, 0, 1,
                                                  InGame_SignInReturned, this, pad);
                        break;
                    }
                }
            }
        }
    }

    long long tickStartTime = System::processTimeInNanoSecs();

    if (mAppPaused && mLevel != NULL)
    {
        float partialTicks = mTimer->renderPartialTicks;
        mTimer->advanceTime();
        mTimer->renderPartialTicks = partialTicks;
    }
    else
    {
        mTimer->advanceTime();
    }

    for (int step = 0; step < mTimer->elapsedTicks; ++step)
    {
        bool lastStep = step == mTimer->elapsedTicks - 1;
        if (step != 0)
        {
            g_pInput->Tick();
            gApp.HandleButtonPresses();
        }

        ++mTicks;
        bool handleInput = true;
        for (int i = 0; i < 4; ++i)
        {
            if (mPendingConnections[i] != NULL)
                mPendingConnections[i]->tick();

            bool active = true;
            if (mLocalPlayers[i])
            {
                if (mLocalPlayers[i]->mButtonsDown == 0 && g_pInput->GetJoypadStick_LX(i, false) == 0.0f
                    && g_pInput->GetJoypadStick_LY(i, false) == 0.0f && g_pInput->GetJoypadStick_RX(i, false) == 0.0f
                    && g_pInput->GetJoypadStick_RY(i, false) == 0.0f)
                {
                    if (mLocalPlayers[i]->mIdleTicks < 0xff)
                        ++mLocalPlayers[i]->mIdleTicks;
                }
                else
                {
                    mLocalPlayers[i]->mIdleTicks = 0;
                }

                if (mLocalPlayers[i]->mIdleTicks < 0xc9)
                {
                    if (mLocalPlayers[i]->isIdle())
                        mLocalPlayers[i]->setIsIdle(false);
                }
                else if (!mLocalPlayers[i]->isIdle() && mLocalPlayers[i]->mOnGround)
                {
                    mLocalPlayers[i]->setIsIdle(true);
                }
            }

            active = setLocalPlayerIdx(i);
            if (active)
            {
                tick(handleInput, lastStep);
                handleInput = false;
                mPlayer->mButtonsDown = 0;
#ifdef MINECRAFT_PC
                s_pcGlideHold[i][0] = false;
                s_pcGlideHold[i][1] = false;
#endif
            }
        }

        gConsoleUIController.HandleGameTick();
        setLocalPlayerIdx(gProfile->GetPrimaryPad());

        for (int i = 0; i < mLevels.length; ++i)
        {
            if (mLevels[i] != NULL)
                mLevels[i]->animateTickDoWork();
        }
    }

    MemSect(0x1f);
    MemSect(0);

    gFancyGraphics = mOptions->mFancyGraphics;

    PIXBeginNamedEvent(0.0f, "Sound engine update");
    mSoundEngine->tick(reinterpret_cast<boost::shared_ptr<Mob>*>(mLocalPlayers), mTimer->renderPartialTicks);
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Light update");
    GlStateManager::enableTexture();
    PIXEndNamedEvent();

    for (int i = 0; i < 4; ++i)
    {
        if (!setLocalPlayerIdx(i))
            continue;
        if (mPlayer != NULL && mPlayer->isInWall())
            mPlayer->SetThirdPersonView(0);
        CameraController* cameraController = mPlayer->GetCameraController();
        if (cameraController != NULL)
            cameraController->update((float)mTimer->elapsedTicks + mTimer->renderPartialTicks);
    }

    if (!mNoRender)
    {
        int primary = gProfile->GetPrimaryPad();
        bool clearScreen = true;
        for (int i = 0; i < 4; ++i)
        {
            if (!setLocalPlayerIdx(i))
                continue;

            PIXBeginNamedEvent(0.0f, "Game render player idx %d", i);
            gRenderer->StateSetViewport((C4JRender::eViewportType)mPlayer->mViewport);
            mGameRenderer->render(mTimer->renderPartialTicks, tickStartTime, clearScreen);
            clearScreen = false;
            PIXEndNamedEvent();

            if (primary == i)
            {
                const eXuiAction action = gApp.GetXuiAction(i);
                if (action == eXuiAction_SaveGameNow || action == eXuiAction_ExitWorldNow
                    || action == eXuiAction_AutoSave)
                    gApp.CaptureSaveThumbnail();
            }
        }

        if (mEmptyViewport >= 0)
        {
            gRenderer->StateSetViewport((C4JRender::eViewportType)(mEmptyViewport + 5));
#ifdef MINECRAFT_PC
            PcSetWideWorldPass(true);
#endif
            GlStateManager::clearColor(0.0f, 0.0f, 0.0f, 0.0f);
            GlStateManager::clear(2);
#ifdef MINECRAFT_PC
            PcSetWideWorldPass(false);
#endif
            gConsoleUIController.SetEmptyQuadrantLogo(mEmptyViewport + 5);
        }

        setLocalPlayerIdx(primary);
        gRenderer->StateSetViewport(C4JRender::eViewportType_FullScreen);
    }

    GlStateManager::flush();

    if (mOptions->mShowInfoBars)
        renderInfoBars();
    if (mOptions->mShowChunkInfoMap)
        renderChunkInfoMap();

    if (mOptions->mRenderDebug != 0)
        CGameNetworkManager::sInstance.renderQueueMeter();
    else
        mLastRenderTime = System::processTimeInNanoSecs();

    PIXBeginNamedEvent(0.0f, "Sleeping");
    Sleep(0);
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Display update");
    PIXEndNamedEvent();

    MemSect(0x1f);
    MemSect(0);

    mAppPaused = gApp.IsAppPaused();

    long long now = System::processTimeInNanoSecs();
    mFrameTimer->logFrameDuration(now - mLastFrameTime);
    mLastFrameTime = now;

    LeaveCriticalSection(&mLevelLock);
}

void Minecraft::respawnPlayer(int playerIdx, int dimension, int entityId, bool alive)
{
    mGameRenderer->DisableUpdateThread(6);

    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = mLocalPlayers[playerIdx];
    mLevel->validateSpawn();
    mLevel->removeAllPendingEntityRemovals();
    if (localPlayer)
        mLevel->removeEntity(localPlayer);

    boost::shared_ptr<Player> oldPlayer = localPlayer;
    mCameraEntity = boost::shared_ptr<Entity>();

    int userIndex = localPlayer->mUserIndex;
    unsigned int viewport = localPlayer->mViewport;
    EDefaultSkins defaultSkin = static_cast<EDefaultSkins>(localPlayer->mField0x7d8);

    mPlayer = mGameModes[playerIdx]->createPlayer(mLevel);

    mPlayer->setName(convStringToWstring(gProfile->GetGamertag(playerIdx)));
    if (alive)
        mPlayer->getEntityData()->assignValues(oldPlayer->getEntityData()->getAll());

    mPlayer->setSharedFlag(Entity::FLAG_ONFIRE, false);
    if (InMiniGame(eMiniGameId_Glide, false) && gApp.getLevelGenerationOptions() != NULL
        && gApp.getLevelGenerationOptions()->getRequiredGameRules() != NULL)
    {
        AABB* jumpOffArea = gApp.getLevelGenerationOptions()->getRequiredGameRules()->getNamedArea(L"JumpOffPoint");
        if (jumpOffArea == NULL || !jumpOffArea->containsIncludingLowerBound(mPlayer->getPos(1.0f)))
            mPlayer->setSharedFlag(Entity::FLAG_FALL_FLYING, true);
    }

    PlayerUID xuid(0);
    PlayerUID onlineXuid(0);
    gProfile->GetXUID(userIndex, &xuid, false);
    gProfile->GetXUID(userIndex, &onlineXuid, true);
    mPlayer->mIsGuest = gProfile->IsGuest(userIndex);
    mPlayer->setXuid(xuid);
    mPlayer->mOnlineXuid = onlineXuid;
    mPlayer->mDisplayName = gProfile->GetDisplayName(playerIdx);

    if (oldPlayer->IsSpectatingOtherPlayer())
        mPlayer->SetCamera(oldPlayer->GetCamera());

    if (!mPlayer->mIsGuest)
        mPlayer->setUUID(mPlayer->mDisplayName);
#ifdef MINECRAFT_PC
    PcApplySessionName(mPlayer, userIndex);
#endif

    mPlayer->mUserIndex = userIndex;
    mPlayer->mViewport = viewport;
    mPlayer->mPlayerIndex = localPlayer->mPlayerIndex;

    mPlayer->setCustomSkin(localPlayer->mCustomSkin);
    mPlayer->setPlayerDefaultSkin(defaultSkin);
    mPlayer->setCustomCape(localPlayer->mCustomCape);
    mPlayer->mSessionTimerStart = localPlayer->mSessionTimerStart;
    mPlayer->mChangeDimensionTimerStart = localPlayer->mChangeDimensionTimerStart;
    mPlayer->setPlayerGamePrivilege(Player::ePlayerGamePrivilege_All,
                                    localPlayer->getPlayerGamePrivilege(Player::ePlayerGamePrivilege_All));
    mPlayer->SetThirdPersonView(oldPlayer->GetThirdPersonView(true));
    if (localPlayer->getHealth() > 0.0f && localPlayer->mY > -64.0 && !InMiniGame(eMiniGameId_Any, true))
        mPlayer->mInventory->m_selected = localPlayer->mInventory->m_selected;

    int skinId = static_cast<int>(CMinecraftApp::getSkinIdFromPath(mPlayer->mSkinLocation));
    if (skinId < 0)
        mPlayer->setSkinAdjustments(Player::getSkinAdjustmentsById(skinId));
    mPlayer->mDimension = dimension;
    mCameraEntity = mPlayer;

    if (playerIdx == gProfile->GetPrimaryPad())
    {
        createPrimaryLocalPlayer(playerIdx);
        GameSettings::SetGameSettingsDebugMask(gProfile->GetPrimaryPad(),
                                               GameSettings::GetGameSettingsDebugMask(-1, true));
    }
    else
    {
        storeExtraLocalPlayer(playerIdx);
    }

    mPlayer->mShowOnMaps = gApp.GetGameHostOption(eGameHostOption_Gamertags) != 0;

    mPlayer->resetPos();
    mLevel->addEntity(mPlayer);

    mGameMode->initPlayer(mPlayer);

    ClientMasterGameMode* masterGameMode = GetClientMasterGameMode();

    if (InMiniGame(eMiniGameId_Glide, true) && CommonMasterGameMode::GetMiniGame()->IsCheckpointBasedSpawn())
    {
        int checkpoint = mPlayer->GetLevel()->GetClientScoreboard()->GetLastCheckpointID(mPlayer->getSUID(), -1);

        if (gApp.getLevelGenerationOptions() != NULL)
        {
            const std::vector<CheckpointRuleDefinition*> checkpoints =
                *gApp.getLevelGenerationOptions()->getRequiredGameRules()->getCheckpoints();

            if (checkpoint >= 0 && static_cast<unsigned int>(checkpoint) < checkpoints.size())
            {
                double speedBoost = gApp.getLevelGenerationOptions()
                                        ->getRequiredGameRules()
                                        ->getCheckpoints()
                                        ->at(checkpoint)
                                        ->getElytraBoost();
                if (speedBoost > 0.0)
                {
                    mPlayer->activateElytraSpeedBoost(speedBoost);
                }
            }
        }

        if (masterGameMode->IsRoundPlaying())
        {
            if (mPlayer->isLocalPlayer())
            {
                mSoundEngine->playUI(SoundEvents::MG03_RESPAWN, 1.0f, 1.0f);
            }
            else
            {
                mSoundEngine->play(SoundEvents::MG03_RESPAWN, (float)mPlayer->mX, (float)mPlayer->mY,
                                   (float)mPlayer->mZ, 1.0f, 1.0f, NULL);
            }
        }
    }

    if (mPlayer->mInput != NULL)
        delete mPlayer->mInput;
    mPlayer->mInput = new Input();
    mPlayer->setId(entityId);
    mGameMode->adjustPlayer(mPlayer);

    if (!mLevel->mIsClientSide)
        prepareLevel((int)IDS_PROGRESS_RESPAWNING);

    mPlayer->mHasOpenedInventory = true;
    if (gConsoleUIController.IsSceneInStack(playerIdx, EUIScene_ConnectingProgress)
        || gApp.GetXuiAction(playerIdx) == eXuiAction_RespawnProgress)
    {
        if (gConsoleUIController.IsSceneInStack(playerIdx, EUIScene_EndPoem))
        {
            gConsoleUIController.NavigateBack(playerIdx, false, EUIScene_EndPoem, EUILayer_Count);
        }
        else if (gApp.GetXuiAction(playerIdx) != eXuiAction_CloseScenesOnConnect)
        {
            gConsoleUIController.CloseUIScenes(playerIdx, false);
            gConsoleUIController.UpdatePlayerBasePositions();
        }
    }

    mPlayer->mLastPoints = oldPlayer->mLastPoints;
    mPlayer->SetWaitingForScoreboard(localPlayer->IsWaitingForScoreboard());

    if (dynamic_cast<DeathScreen*>(mScreen) != NULL)
        setScreen(NULL);

    mGameRenderer->EnableUpdateThread(6);
}

void Minecraft::startAndConnectTo(const std::wstring& userName, const std::wstring& sessionId,
                                  const std::wstring& server)
{
    Minecraft* minecraft = new Minecraft(NULL, NULL, NULL, 0x500, 0x2d0, false);
    minecraft->mServer = L"www.minecraft.net";

    if (userName.compare(0, userName.size(), L"", wcslen(L"")) != 0
        && sessionId.compare(0, sessionId.size(), L"", wcslen(L"")) != 0)
    {
        minecraft->mUser = new User(userName, sessionId);
        minecraft->run();
        return;
    }

    std::wstring generatedName = std::wstring(L"Player") + _toString(System::processTimeInMilliSecs() % 1000);
    minecraft->mUser = new User(generatedName, L"");
    minecraft->run();
}

void Minecraft::start(const std::wstring& userName, const std::wstring& sessionId)
{
    startAndConnectTo(userName, sessionId, L"");
}

void MinecraftWorld_RunStaticCtors()
{
    Direction::staticCtor();
    Packet::staticCtor();
    SoundEvents::staticCtor();
    SoundType::StaticCtor();
    MaterialColor::staticCtor();
    DyeColor::staticCtor();
    MaterialColor::staticCtorAfterDyeColor();
    Material::staticCtor();
    Block::staticCtor();
    Blocks::staticInit();
    AxeItem::staticCtor();
    PickaxeItem::staticCtor();
    ShovelItem::staticCtor();
    JungleBiome::staticCtor();
    MesaBiome::staticCtor();
    Biome::staticCtor();
    MobEffect::staticCtor();
    MobEffects::staticCtor();
    EquipmentSlot::staticCtor();
    InventoryMenu::staticCtor();
    ArmorItem::staticCtor();
    Item::staticCtor();
    FurnaceRecipes::staticCtor();
    MapExtendingRecipe::StaticCtor();
    BannerPattern::staticCtor();
    Potion::StaticCtor();
    Potions::StaticCtor();
    Recipes::staticCtor();
    gStats = new CommonStats();
    Stats::staticCtor();
    BlockEntity::staticCtor();
    EntityIO::bootStrap();
    MobCategory::staticCtor();
    BlockEntityIdFix::staticCtor();
    ItemPotionFix::staticCtor();
    Item::staticInit();
    LevelChunk::staticCtor();
    LevelType::staticCtor();
    StructureFeatureIO::staticCtor();
    VillagePieces::Smithy::staticCtor();
    VillageFeature::staticCtor();
    OceanMonumentFeature::staticCtor();
    RandomScatteredLargeFeature::staticCtor();
    OceanMonumentPieces::OceanMonumentPiece::staticCtor();
    DesertWellFeature::staticCtor();
    ScatteredFeaturePieces::JunglePyramidPiece::staticCtor();
    SwampTreeFeature::staticCtor();
    StrongholdPieces::staticCtor();
    WoodlandMansionFeature::staticCtor();
    BuriedTreasureFeature::staticCtor();
    Entity::staticCtor();
    LivingEntity::staticCtor();
    ArmorStand::staticCtor();
    Mob::staticCtor();

    SynchedEntityData::defineId<int>(EntityType::PATHFINDER_MOB, EntityType::MOB, NULL);
    AgableMob::staticCtor();
    Animal::staticCtor();
    Villager::staticCtor();
    TamableAnimal::staticCtor();
    Ocelot::staticCtor();
    Wolf::staticCtor();

    SynchedEntityData::defineId<int>(EntityType::ABSTRACT_HORSE, EntityType::ANIMAL, NULL);
    AbstractHorse::staticCtor();
    SynchedEntityData::defineId<int>(EntityType::ABSTRACT_CHESTED_HORSE, EntityType::ABSTRACT_HORSE, NULL);
    AbstractChestedHorse::staticCtor();
    Horse::staticCtor();
    Llama::staticCtor();

    SynchedEntityData::defineId<int>(EntityType::TAMABLE_ANIMAL, EntityType::ANIMAL, NULL);
    Parrot::staticCtor();
    Sheep::staticCtor();
    Pig::staticCtor();
    Chicken::staticCtor();
    Rabbit::staticCtor();
    PolarBear::staticCtor();
    Turtle::staticCtor();

    SynchedEntityData::defineId<int>(EntityType::GOLEM, EntityType::PATHFINDER_MOB, NULL);
    SnowMan::staticCtor();
    VillagerGolem::staticCtor();
    Shulker::staticCtor();

    SynchedEntityData::defineId<int>(EntityType::MONSTER, EntityType::PATHFINDER_MOB, NULL);
    Spider::staticCtor();
    Zombie::staticCtor();
    ZombieVillager::staticCtor();
    Creeper::staticCtor();
    AbstractSkeleton::staticCtor();
    EnderMan::staticCtor();
    Blaze::staticCtor();
    Witch::staticCtor();
    WitherBoss::staticCtor();
    Guardian::staticCtor();
    Vex::staticCtor();

    SynchedEntityData::defineId<int>(EntityType::ABSTRACT_ILLAGER, EntityType::MONSTER, NULL);
    AbstractIllager::staticCtor();
    SynchedEntityData::defineId<int>(EntityType::SPELLCASTER_ILLAGER, EntityType::ABSTRACT_ILLAGER, NULL);
    SpellcasterIllager::staticCtor();
    SynchedEntityData::defineId<int>(EntityType::WATER_ANIMAL, EntityType::MOB, NULL);
    TropicalFish::staticCtor();
    Salmon::staticCtor();
    SynchedEntityData::defineId<int>(EntityType::AMBIENT_CREATURE, EntityType::MOB, NULL);
    Bat::staticCtor();
    SynchedEntityData::defineId<int>(EntityType::FLYING_MOB, EntityType::MOB, NULL);
    Ghast::staticCtor();
    Slime::staticCtor();
    EnderDragon::staticCtor();
    Player::staticCtor();
    Minecart::staticCtor();
    MinecartCommandBlock::staticCtor();
    MinecartFurnace::staticCtor();
    AbstractRotatedItemProjectile::staticCtor();
    TippableArrow::staticCtor();
    SynchedEntityData::defineId<int>(EntityType::FIREBALL, EntityType::ENTITY, NULL);
    WitherSkull::staticCtor();
    SynchedEntityData::defineId<int>(EntityType::THROWABLE_PROJECTILE, EntityType::ENTITY, NULL);
    ThrownPotion::staticCtor();
    SynchedEntityData::defineId<int>(EntityType::HANGING_ENTITY, EntityType::ENTITY, NULL);
    ItemFrame::staticCtor();
    PrimedTnt::staticCtor();
    FireworksRocketEntity::staticCtor();
    EnderCrystal::staticCtor();
    FishingHook::staticCtor();
    FallingBlockEntity::staticCtor();
    ItemEntity::staticCtor();
    Boat::staticCtor();
    AreaEffectCloud::staticCtor();
    ThrownTrident::staticCtor();
    PotionBrewing::StaticCtor();
    Enchantment::staticCtor();
    SharedConstants::staticCtor();
    ServerLevel::staticCtor();
    SparseLightStorage::staticCtor();
    CompressedBlockStorage::staticCtor();
    SparseDataStorage::staticCtor();
    McRegionChunkStorage::staticCtor();
    GameType::staticCtor();
    BeaconBlockEntity::staticCtor();
    DefaultVertexFormat::staticCtor();
    FaceInfo::staticCtor();
    EatBlockGoal::staticCtor();
    BirchFeature::staticCtor();
    MegaPineTreeFeature::staticCtor();
    PineFeature::staticCtor();
    RoofTreeFeature::staticCtor();
    SavannaTreeFeature::staticCtor();
    SpruceFeature::staticCtor();
    SwampTreeFeature::staticCtor();
    TreeFeature::staticCtor();
    BuiltInLootTables::staticCtor();
    Enchantments::staticCtor();
    DispenserBootstrap::StaticCtor();
    ItemIdFix::staticCtor();
    ItemInstanceBlockEntityTagWalker::staticCtor();
}

ClientPacketListener* Minecraft::getConnection(int playerIdx)
{
    return mLocalPlayers[playerIdx]->mPacketListener;
}

void Minecraft::main()
{
    sStaticsInitialised = true;

    MinecraftWorld_RunStaticCtors();
    User::staticCtor();
    Tutorial::staticCtor();
    ColourTable::staticCtor();
    StructureManager::staticCtor();
    Tooltips::staticCtor();
    GlStateManager::staticCtor();
    gApp.loadDefaultGameRules();

    std::wstring userName = std::wstring(L"Player") + _toString(System::processTimeInMilliSecs() % 1000);

    UIScene_CreativeMenu::staticCtor();
    UIScene_LeaderboardsMenu::staticCtor();
    BlockEntityRenderDispatcher::staticCtor();

    start(userName, L"-");

    ParticleType::staticCtor();
    MasterGameMode::StaticCtor();
    MiniGameMedals::staticCtor();
    CMinecraftApp::StaticCtor();
    MiniGameDef::StaticCtor();

    GetInstance()->mMiniGame = MiniGameDef::GetCustomGameModeById(eMiniGameId_Any, true);
    ClientPacketListener::staticCtor();
}

bool Minecraft::renderNames()
{
    if (sInstance != NULL && sInstance->mOptions->mHideGui)
        return false;
    return true;
}

bool Minecraft::useFancyGraphics()
{
    return sInstance != NULL && sInstance->mOptions->mFancyGraphics;
}

bool Minecraft::useAmbientOcclusion()
{
    return sInstance != NULL && sInstance->mOptions->mAmbientOcclusion;
}

bool Minecraft::renderDebug()
{
    return sInstance != NULL && sInstance->mOptions->mRenderDebug != 0;
}

int Minecraft::getAverageFps()
{
    return sAverageFps;
}

bool Minecraft::addPendingClientTextureRequest(const std::wstring& textureName)
{
    std::vector<std::wstring>::iterator it = mPendingClientTextureRequests.begin();
    while (it != mPendingClientTextureRequests.end())
    {
        if (it->compare(0, it->size(), textureName.c_str(), textureName.size()) == 0)
            break;
        ++it;
    }

    if (it == mPendingClientTextureRequests.end())
    {
        mPendingClientTextureRequests.push_back(textureName);
        return true;
    }
    return false;
}

void Minecraft::handleClientTextureReceived(const std::wstring& textureName)
{
    std::vector<std::wstring>::iterator it = mPendingClientTextureRequests.begin();
    while (it != mPendingClientTextureRequests.end())
    {
        if (it->compare(0, it->size(), textureName.c_str(), textureName.size()) == 0)
            break;
        ++it;
    }

    if (it != mPendingClientTextureRequests.end())
        mPendingClientTextureRequests.erase(it);
}

void Minecraft::fjReloadTexturePack()
{
    getEntityRenderDispatcher()->setupForTexturePack(mTexturePackRepository->getSelected());
    BlockEntityRenderDispatcher::instance->setupForTexturePack(mTexturePackRepository->getSelected());
    mTextures->reloadAll();
    mTexturePackRepository->updateUI();

    TexturePack* selected = mTexturePackRepository->getSelected();
    if (selected->hasAudio() || selected->getId() == 0x801 || selected->getId() == 0x803 || selected->getId() == 0x806
        || selected->getId() == 0x808)
    {
        const MiniGameDef* miniGame = GetMiniGame();
        ClientMasterGameMode* masterGameMode = GetInstance()->GetClientMasterGameMode();
        unsigned int miniGameId = miniGame->GetId();
        if (miniGameId != eMiniGameId_Lobby && miniGameId < 4)
        {
            mSoundEngine->playStreaming(NULL, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, false,
                                        masterGameMode->IsWaitingForMusicStartEvent(),
                                        ConsoleSoundEngine::eAudioDelayType_None);
        }
        else
        {
            mSoundEngine->playStreaming(NULL, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, false, false,
                                        ConsoleSoundEngine::eAudioDelayType_None);
        }
    }
}

boost::shared_ptr<AbstractClientPlayer> Minecraft::GetLocalPlayerByName(const std::wstring& name)
{
    boost::shared_ptr<Player> found;
    for (int i = 0; i < mLevels.length; ++i)
    {
        if (mLevels[i] == NULL)
            continue;
        found = mLevels[i]->getPlayerByName(name);
        if (found)
            break;
    }
    return boost::dynamic_pointer_cast<AbstractClientPlayer>(found);
}

boost::shared_ptr<Player> Minecraft::GetPlayerByPlayerIndex(int playerIndex)
{
    for (int i = 0; i < 3; ++i)
    {
        MultiPlayerLevel* level = mLevels[i];
        if (level == NULL)
            continue;

        for (std::vector<boost::shared_ptr<Player> >::iterator it = level->mPlayers.begin();
             it != level->mPlayers.end(); ++it)
        {
            if ((*it)->mPlayerIndex == playerIndex)
                return *it;
        }
    }
    return boost::shared_ptr<Player>();
}

int Minecraft::GetSpectateMode()
{
    if (InMiniGame(eMiniGameId_Any, true))
        return GetMiniGame()->GetSpectateMode();
    return 0;
}

unsigned long long Minecraft::GetSpectateEntity()
{
    if (InMiniGame(eMiniGameId_Any, true))
        return GetMiniGame()->GetSpectateEntity();

    MiniGameDef defaultMiniGame(eMiniGameId_Lobby);
    return defaultMiniGame.GetSpectateEntity();
}

GhostController* Minecraft::GetGhostController()
{
    return mGhostController;
}

void Minecraft::SetGhostController(GhostController* ghostController)
{
    if (mGhostController != NULL)
    {
        mGhostController->setDone();
        mFinishedGhostControllers.push_back(mGhostController);
    }
    mGhostController = ghostController;
}

void Minecraft::SetJoiningFromFriendsTabOrInvite(bool joining)
{
    mJoiningFromFriendsTabOrInvite = joining;
}

bool Minecraft::IsJoiningFromFriendsTabOrInvite() const
{
    return mJoiningFromFriendsTabOrInvite;
}
