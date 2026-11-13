#include "game/client/render/GameRenderer.hpp"
#include "game/core/EntityType.hpp"
#include "game/client/render/Font.hpp"
#include "game/client/render/MapRenderer.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/BufferedImage.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/Options.hpp"
#include "game/core/GameType.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Math.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/FloatBuffer.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Inventory.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/Player.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/generation/AbstractClientPlayer.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/HangingEntity.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/ThermalAreaRuleDefinition.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/AABB.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/client/render/ParticleEngine.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/Camera.hpp"
#include "game/core/CameraController.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/System.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/Options.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/LocalPlayer.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/core/Frustum.hpp"
#include "game/core/FrustumCuller.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/InfoBarManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/GhostController.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/ItemInHandRenderer.hpp"
#include "game/client/render/PlayerRenderer.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/Gui.hpp"
#include "game/core/BossHealthOverlay.hpp"
#include "game/data/ElementStorage.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/generation/Player.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "game/client/render/HumanoidModel.hpp"
#include "game/client/render/ElytraModel.hpp"
#include "game/item/Items.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/Inventory.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/core/Compression.hpp"
#include "game/core/IntCache.hpp"
#include "game/core/ShutdownManager.hpp"
#include "game/core/PostChain.hpp"
#include "game/world/block/CompressedBlockStorage.hpp"
#include "game/data/ElementStorage.hpp"
#include "game/data/SparseDataStorage.hpp"
#include "game/data/SparseLightStorage.hpp"
#include "game/ui/Screen.hpp"
#include "game/ui/ScreenSizeCalculator.hpp"
#include "platform/wiiu/C4JEventArrayImpl.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/SharedConstants.hpp"

#include <cfloat>
#ifdef MINECRAFT_PC
#include "platform/pc/PcDisplay.hpp"
#include "platform/pc/PcSettings.hpp"
#include "platform/pc/PcModViews.hpp"
#include "platform/pc/PcNativeMods.hpp"
#include "game/core/HumanoidArm.hpp"
#endif
#include <cmath>
#include <cstring>
#include <vector>

namespace
{
C4JEventArrayImpl* sRenderUpdateEvents = NULL;
C4JThreadImpl* sRenderUpdateThread = NULL;
OSFastMutex sDeleteMutex;
bool sRenderUpdateThreadActive = false;

std::vector<unsigned char*> sPendingByteDeletes;
std::vector<SparseLightStorage*> sPendingLightDeletes;
std::vector<CompressedBlockStorage*> sPendingCompressedDeletes;
std::vector<SparseDataStorage*> sPendingDataDeletes;
std::vector<LevelChunk*> sPendingChunkDeletes;

std::vector<GameRenderer::ElementStorageWithTick> sPendingElementStorageDeletes;

int sAnaglyphEyeIndex = 0;

static const BlockPos sNetherFogOffsets[] = {
    BlockPos(0, 0, 0),    BlockPos(-12, 0, 0),  BlockPos(-8, 0, -8),  BlockPos(0, 0, -12), BlockPos(8, 0, -8),
    BlockPos(12, 0, 0),   BlockPos(8, 0, 8),    BlockPos(0, 0, 12),   BlockPos(-8, 0, 8),  BlockPos(0, -3, 0),
    BlockPos(-12, -3, 0), BlockPos(-8, -3, -8), BlockPos(0, -3, -12), BlockPos(8, -3, -8), BlockPos(12, -3, 0),
    BlockPos(8, -3, 8),   BlockPos(0, -3, 12),  BlockPos(-8, -3, 8),  BlockPos(0, 3, 0),   BlockPos(-12, 3, 0),
    BlockPos(-8, 3, -8),  BlockPos(0, 3, -12),  BlockPos(8, 3, -8),   BlockPos(12, 3, 0),  BlockPos(8, 3, 8),
    BlockPos(0, 3, 12),   BlockPos(-8, 3, 8),
};

static const BlockPos sEndFogOffsets[] = {
    BlockPos(0, 0, 0),   BlockPos(-8, 0, 0),   BlockPos(-5, 0, -5), BlockPos(0, 0, -8),  BlockPos(5, 0, -5),
    BlockPos(8, 0, 0),   BlockPos(5, 0, 5),    BlockPos(0, 0, 8),   BlockPos(-5, 0, 5),  BlockPos(0, -2, 0),
    BlockPos(-8, -2, 0), BlockPos(-5, -2, -5), BlockPos(0, -2, -8), BlockPos(5, -2, -5), BlockPos(8, -2, 0),
    BlockPos(5, -2, 5),  BlockPos(0, -2, 8),   BlockPos(-5, -2, 5), BlockPos(0, 2, 0),   BlockPos(-8, 2, 0),
    BlockPos(-5, 2, -5), BlockPos(0, 2, -8),   BlockPos(5, 2, -5),  BlockPos(8, 2, 0),   BlockPos(5, 2, 5),
    BlockPos(0, 2, 8),   BlockPos(-5, 2, 5),
};

static const BlockPos sOverworldFogOffsets[] = {
    BlockPos(0, 0, 0),   BlockPos(-4, 0, 0),   BlockPos(-2, 0, -2), BlockPos(0, 0, -4),  BlockPos(2, 0, -2),
    BlockPos(4, 0, 0),   BlockPos(2, 0, 2),    BlockPos(0, 0, 4),   BlockPos(-2, 0, 2),  BlockPos(0, -1, 0),
    BlockPos(-4, -1, 0), BlockPos(-2, -1, -2), BlockPos(0, -1, -4), BlockPos(2, -1, -2), BlockPos(4, -1, 0),
    BlockPos(2, -1, 2),  BlockPos(0, -1, 4),   BlockPos(-2, -1, 2), BlockPos(0, 1, 0),   BlockPos(-4, 1, 0),
    BlockPos(-2, 1, -2), BlockPos(0, 1, -4),   BlockPos(2, 1, -2),  BlockPos(4, 1, 0),   BlockPos(2, 1, 2),
    BlockPos(0, 1, 4),   BlockPos(-2, 1, 2),
};

}

GameRenderer::GameRenderer(Minecraft* minecraft, ResourceManager* resourceManager)
    : mUpdateThreadDisableMask(0),
      mMinecraft(minecraft),
      mResourceManager(resourceManager),
      mRandom(),
      mFogDistance(0.0f),
      mItemInHandRenderer(minecraft->getItemInHandRenderer()),
      mMapRenderer(new MapRenderer(minecraft->mFont, minecraft->mOptions, minecraft->mTextures)),
      mTickCount(0),
      mPointedEntity(),
      mThirdPersonDistance(4.0f),
      mThirdPersonDistanceO(4.0f),
      mDebugCamYaw(0.0f),
      mDebugCamYawO(0.0f),
      mDebugCamPitch(0.0f),
      mDebugCamPitchO(0.0f),
      mSmoothCamYaw(0.0f),
      mSmoothCamPitch(0.0f),
      mSmoothCamFilterX(0.0f),
      mSmoothCamFilterY(0.0f),
      mSmoothCamPartialTicks(0.0f),
      mZeroVector(Vec3::newPermanent(0.0, 0.0, 0.0)),
      mFovSetting(70.0f),
      mDarkenWorldAmount(0.0f),
      mDarkenWorldAmountO(0.0f),
      mCloudFog(false),
      mRenderHand(true),
      mRenderBlockOutline(true),
      mLastActiveTime(Minecraft::currentTimeMillis()),
      mBlockLightRedFlicker(0.0f),
      mBlockLightRedFlickerSpeed(0.0f),
      mRainSoundTime(0),
      mFogBuffer(NULL),
      mFogRed(0.0f),
      mFogGreen(0.0f),
      mFogBlue(0.0f),
      mFogBrightnessO(0.0f),
      mFogBrightness(0.0f),
      mFogHeightLocked(false),
      mFogHeight(0.0),
      mPanoramicSection(0),
      mPanoramicMode(false),
      mZoom(1.0f),
      mZoomX(0.0f),
      mZoomY(0.0f),
      mPostEffect(NULL),
      mEffectIndex(24),
      mEffectActive(false),
      mFrameCount(0),
      mLastNanos(0),
      mLightTextureDirty(false),
      mUnusedBuffer(NULL),
      mDebugView(0)
{
    InitializeCriticalSection(&mUpdateThreadMutex);

    mUnusedBuffer = MemoryTracker::createFloatBuffer(16);
    mFogBuffer = MemoryTracker::createFloatBuffer(16);
    mOwnedArrays[0] = NULL;
    mOwnedArrays[1] = NULL;
    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
    {
        mFov[playerIndex] = 1.0f;
        mFovO[playerIndex] = 1.0f;
        mUnusedPlayerFloats[playerIndex] = 0.0f;
        mPlayerFogRed[playerIndex] = 0.0f;
        mPlayerFogGreen[playerIndex] = 0.0f;
        mPlayerFogBlue[playerIndex] = 0.0f;
        mPlayerFogDistance[playerIndex] = 0.0f;
        mTargetFogRed[playerIndex] = 0.0f;
        mTargetFogGreen[playerIndex] = 0.0f;
        mTargetFogBlue[playerIndex] = 0.0f;
        mTargetFogDistance[playerIndex] = 0.0f;
        mFogColourChangeTime[playerIndex] = 0;
        mFogDistanceChangeTime[playerIndex] = 0;
        mItemActivationItems[playerIndex] = ItemInstance::EMPTY;
    }

    for (int z = 0; z < 32; ++z)
    {
        for (int x = 0; x < 32; ++x)
        {
            const float dz = static_cast<float>(z - 16);
            const float dx = static_cast<float>(x - 16);
            const float length = std::sqrt(dx * dx + dz * dz);
            mRainSizeX[z << 5 | x] = -dz / length;
            mRainSizeZ[z << 5 | x] = dx / length;
        }
    }

    GlStateManager::enableLighting();
    mMinecraft->mLocalItemInHandRenderers[0] = new ItemInHandRenderer(minecraft);
    mMinecraft->mLocalItemInHandRenderers[1] = new ItemInHandRenderer(minecraft);
    mMinecraft->mLocalItemInHandRenderers[2] = new ItemInHandRenderer(minecraft);
    mMinecraft->mLocalItemInHandRenderers[3] = new ItemInHandRenderer(minecraft);
    GlStateManager::disableLighting();

    BufferedImage* lightImage = new BufferedImage(16, 16, 1);
    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
        mLightTextures[playerIndex] = mMinecraft->mTextures->getTexture(lightImage, C4JRender::eTextureFormat_RGBA8,
                                                                        true);
    delete lightImage;

    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
        mLightPixels[playerIndex] = arrayWithLength<int>(256, true);

    sRenderUpdateEvents = new C4JEventArrayImpl(2, C4JEventArray::Mode0);
    sRenderUpdateEvents->Set(1);
    InitializeCriticalSection(&sDeleteMutex);
    sRenderUpdateThread = new C4JThreadImpl(runUpdate, NULL, "Chunk update", 0);
    sRenderUpdateThread->SetProcessor(2);
    sRenderUpdateThread->Run();
}

GameRenderer::~GameRenderer()
{
    if (mOwnedArrays[0] != NULL)
        delete[] mOwnedArrays[0];
    if (mOwnedArrays[1] != NULL)
        delete[] mOwnedArrays[1];
    DeleteCriticalSection(&mUpdateThreadMutex);
}

const float GameRenderer::NAME_TAG_SCALE = 0.025f;
bool GameRenderer::anaglyph3d = false;

void GameRenderer::renderNameTagInWorld(Font* font, const std::wstring& text, float x, float y, float z, int nameOffset,
                                        float cameraYRot, float cameraXRot, bool thirdPerson, bool sneaking, int color,
                                        float alpha)
{
    static const float kZero = 0.0f;
    static const float kOne = 1.0f;
    static const float kNegOne = -1.0f;
    static const float k255 = 255.0f;
    static const float kNameTagScaleXY = -0.025f;
    static const float kNameTagScaleZ = 0.025f;
    static const float kBgAlpha = 0.25f;
    static const float kLineWidth = 2.0f;
    static const float kNameTagZOffset = -0.04f;

    GlStateManager::pushMatrix();
    GlStateManager::translatef(x, y, z);
    GlStateManager::rotatef(-cameraYRot, kZero, kOne, kZero);
    if (!thirdPerson)
        GlStateManager::rotatef(kOne * cameraXRot, kOne, kZero, kZero);
    else
        GlStateManager::rotatef(kNegOne * cameraXRot, kOne, kZero, kZero);
    GlStateManager::scalef(kNameTagScaleXY, kNameTagScaleXY, kNameTagScaleZ);
    GlStateManager::disableLighting();
    GlStateManager::depthMask(false);

    const int textWidth = font->width(text);
    const int halfWidth = (textWidth >> 1) + ((textWidth < 0 && (textWidth & 1) != 0) ? 1 : 0);

    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);

    if (!sneaking)
    {
        Tesselator* tesselator = Tesselator::getInstance();
        BufferBuilder* builder = tesselator->getBuilder();

        if (alpha > kZero)
        {
            GlStateManager::color4f(kOne, kOne, kOne, alpha);
            GlStateManager::depthMask(false);
            GlStateManager::disableDepthTest();
            GlStateManager::disableTexture();
            builder->begin();

            int bgAlphaByte;
            if (alpha >= kOne)
            {
                builder->color(kZero, kZero, kZero, kBgAlpha);
                bgAlphaByte = static_cast<int>(alpha * k255);
            }
            else
            {
                bgAlphaByte = static_cast<int>(k255 * alpha);
                builder->color(color, bgAlphaByte);
            }

            const float left = static_cast<float>(-(halfWidth + 1));
            const float right = static_cast<float>(halfWidth + 1);
            const float top = static_cast<float>(nameOffset - 1);
            const float bottom = static_cast<float>(nameOffset + 9);

            builder->vertex(left, top, kZero);
            builder->vertex(left, bottom, kZero);
            builder->vertex(right, bottom, kZero);
            builder->vertex(right, top, kZero);
            builder->end();

            GlStateManager::enableDepthTest();
            GlStateManager::depthMask(true);
            GlStateManager::depthFunc(7);
            GlStateManager::lineWidth(kLineWidth);
            builder->begin(5);
            builder->color(color, bgAlphaByte);
            builder->vertex(left, top, kZero);
            builder->vertex(left, bottom, kZero);
            builder->vertex(right, bottom, kZero);
            builder->vertex(right, top, kZero);
            builder->vertex(left, top, kZero);
            builder->end();
            GlStateManager::depthFunc(3);
            GlStateManager::depthMask(false);
            GlStateManager::disableDepthTest();
            GlStateManager::enableTexture();
            font->draw(text, -halfWidth, nameOffset, 0x20ffffff);
            GlStateManager::enableDepthTest();
            GlStateManager::depthMask(true);
        }

        if (alpha < kOne)
        {
            GlStateManager::color4f(kOne, kOne, kOne, kOne);
            GlStateManager::disableTexture();
            GlStateManager::depthFunc(7);
            builder->begin();
            builder->color(color, 0xff);
            const float left = static_cast<float>(-(halfWidth + 1));
            const float right = static_cast<float>(halfWidth + 1);
            const float top = static_cast<float>(nameOffset - 1);
            const float bottom = static_cast<float>(nameOffset + 8);
            builder->vertex(left, top, kZero);
            builder->vertex(left, bottom, kZero);
            builder->vertex(right, bottom, kZero);
            builder->vertex(right, top, kZero);
            builder->end();
            GlStateManager::depthFunc(3);
            GlStateManager::enableTexture();
            GlStateManager::translatef(kZero, kZero, kNameTagZOffset);
        }

        if (alpha > kZero)
        {
            const int textColor = (static_cast<int>(alpha * k255) << 24) | 0xffffff;
            font->draw(text, -halfWidth, 0, textColor);
        }
    }
    else
    {
        Tesselator* tesselator = Tesselator::getInstance();
        BufferBuilder* builder = tesselator->getBuilder();
        GlStateManager::disableTexture();
        builder->begin();
        builder->color(kZero, kZero, kZero, kBgAlpha);
        const float left = static_cast<float>(-(halfWidth + 1));
        const float right = static_cast<float>(halfWidth + 1);
        const float top = static_cast<float>(nameOffset - 1);
        const float bottom = static_cast<float>(nameOffset + 9);
        builder->vertex(left, top, kZero);
        builder->vertex(left, bottom, kZero);
        builder->vertex(right, bottom, kZero);
        builder->vertex(right, top, kZero);
        builder->end();
        GlStateManager::enableTexture();
        GlStateManager::depthMask(true);
        font->draw(text, -halfWidth, 0, 0x20ffffff);
    }

    GlStateManager::enableLighting();
    GlStateManager::disableBlend();
    GlStateManager::color4f(kOne, kOne, kOne, kOne);
    GlStateManager::popMatrix();
}

unsigned int GameRenderer::getLightTexture(int index, Level*)
{
    return mLightTextures[index];
}

int GameRenderer::runUpdate(void*)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    Vec3::CreateNewThreadStorage();
    AABB::CreateNewThreadStorage();
    IntCache::CreateNewThreadStorage();
    Tesselator::CreateNewThreadStorage(0x100000);
    Compression::UseDefaultThreadStorage();
    gRenderer->InitialiseContext();
    ElementStorage::CreateNewThreadStorage(ElementStorage::eHESThreadId_Renderer);
    Block::CreateNewThreadStorage();

    unsigned int* dirtyChunkBuffer = static_cast<unsigned int*>(operator new[](0x20000));

    ShutdownManager::HasStarted(ShutdownManager::eThreadId_RenderUpdate, sRenderUpdateEvents);

    while (ShutdownManager::ShouldRun(ShutdownManager::eThreadId_RenderUpdate))
    {
        sRenderUpdateEvents->WaitForAll(0xffffffff);
        if (!ShutdownManager::ShouldRun(ShutdownManager::eThreadId_RenderUpdate))
            break;

        sRenderUpdateEvents->Set(0);

        int passCount = 0;
        do
        {
            const int updated = minecraft->mLevelRenderer->updateDirtyChunks(dirtyChunkBuffer);
            ++passCount;
            if (updated == 0)
                break;
        } while (passCount < 10);

        gRenderer->CBuffDeferredModeEnd();
        minecraft->mLevelRenderer->fullyFlagRenderableBlockEntitiesToBeRemoved();

        EnterCriticalSection(&sDeleteMutex);

        for (size_t i = 0; i < sPendingChunkDeletes.size(); ++i)
        {
            LevelChunk* chunk = sPendingChunkDeletes[i];
            if (chunk != NULL)
                delete chunk;
        }
        sPendingChunkDeletes.clear();

        for (size_t i = 0; i < sPendingByteDeletes.size(); ++i)
            operator delete(static_cast<void*>(sPendingByteDeletes[i]));
        sPendingByteDeletes.clear();

        for (size_t i = 0; i < sPendingLightDeletes.size(); ++i)
            delete sPendingLightDeletes[i];
        sPendingLightDeletes.clear();

        for (size_t i = 0; i < sPendingCompressedDeletes.size(); ++i)
            delete sPendingCompressedDeletes[i];
        sPendingCompressedDeletes.clear();

        for (size_t i = 0; i < sPendingDataDeletes.size(); ++i)
            delete sPendingDataDeletes[i];
        sPendingDataDeletes.clear();

        for (std::vector<ElementStorageWithTick>::iterator entry = sPendingElementStorageDeletes.begin();
             entry != sPendingElementStorageDeletes.end();)
        {
            if (entry->tick == 0)
            {
                delete entry->storage;
                entry = sPendingElementStorageDeletes.erase(entry);
            }
            else
            {
                ++entry;
            }
        }

        LeaveCriticalSection(&sDeleteMutex);

        AABB::resetPool();
        Vec3::resetPool();
        IntCache::Reset();

        sRenderUpdateEvents->Set(1);
    }

    ShutdownManager::HasFinished(ShutdownManager::eThreadId_RenderUpdate);
    return 0;
}

int GameRenderer::getTickCount()
{
    return mTickCount;
}

unsigned int GameRenderer::postEffectActive()
{
    return 0;
}

void GameRenderer::shutdownEffect()
{
    if (mPostEffect != NULL)
        mPostEffect->destroy();

    mPostEffect = NULL;
    mEffectIndex = EFFECT_NONE;
}

void GameRenderer::checkEntityPostEffect(boost::shared_ptr<Entity>)
{
    return;
}

void GameRenderer::cycleEffect()
{
    return;
}

void GameRenderer::onResourceManagerReload(ResourceManager*)
{
    if (mPostEffect != NULL)
        mPostEffect->destroy();

    mPostEffect = NULL;

    if (mEffectIndex == EFFECT_NONE)
        checkEntityPostEffect(mMinecraft->getCameraEntity());
}

float GameRenderer::GetFovVal()
{
    return mFovSetting;
}

void GameRenderer::UpdateElementStorageDeleteTicks()
{
    EnterCriticalSection(&sDeleteMutex);
    for (std::vector<ElementStorageWithTick>::iterator entry = sPendingElementStorageDeletes.begin();
         entry != sPendingElementStorageDeletes.end(); ++entry)
    {
        if (entry->tick != 0)
            --entry->tick;
    }
    LeaveCriticalSection(&sDeleteMutex);
}

void GameRenderer::tickFov()
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mPlayer;
    const int playerIndex = player != NULL ? player->mUserIndex : 0;

    const float targetFov = player->getFieldOfViewModifier();

    mFovO[playerIndex] = mFov[playerIndex];
    mFov[playerIndex] += (targetFov - mFov[playerIndex]) * 0.5f;
    if (mFov[playerIndex] > 1.5f)
        mFov[playerIndex] = 1.5f;
    if (mFov[playerIndex] < 0.1f)
        mFov[playerIndex] = 0.1f;
}
void GameRenderer::tickLightTexture()
{
    mBlockLightRedFlickerSpeed = static_cast<float>((Math::random() - Math::random()) * Math::random() * Math::random()
                                                    + mBlockLightRedFlickerSpeed);
    mBlockLightRedFlickerSpeed *= 0.9f;
    mBlockLightRedFlicker += mBlockLightRedFlickerSpeed - mBlockLightRedFlicker;
    mLightTextureDirty = true;
}

boost::shared_ptr<Entity> GameRenderer::getCamera()
{
    return Minecraft::GetInstance()->getCameraEntity();
}

void GameRenderer::tickRain()
{
    float rainLevel = mMinecraft->mLevel->getRainLevel(1.0f);
    if (!mMinecraft->mOptions->mFancyGraphics)
        rainLevel *= 0.5f;
    if (rainLevel == 0.0f)
        return;

    const float rainIntensity = rainLevel / static_cast<float>(mMinecraft->mLevelRenderer->activePlayers() + 1);
    mRandom.setSeed(mTickCount * 312987231);
    boost::shared_ptr<Entity> camera = getCamera();
    Level* level = mMinecraft->mLevel;
    const BlockPos cameraPos(camera);

    int particleCount = static_cast<int>(100.0f * rainIntensity * rainIntensity);
    if (mMinecraft->mOptions->mParticles == 1)
        particleCount >>= 1;
    else if (mMinecraft->mOptions->mParticles == 2)
        particleCount = 0;

    int rainHits = 0;
    double soundX = 0.0;
    double soundY = 0.0;
    double soundZ = 0.0;
    for (int i = 0; i < particleCount; ++i)
    {
        const int offsetX = mRandom.nextInt(10) - mRandom.nextInt(10);
        const int offsetZ = mRandom.nextInt(10) - mRandom.nextInt(10);
        const BlockPos rainPos = level->getTopRainBlockPos(cameraPos.offset(offsetX, 0, offsetZ));
        Biome* biome = level->getBiome(rainPos);
        const BlockPos groundPos = rainPos.below();
        const BlockState* ground = level->getBlockState(groundPos);
        if (rainPos.y > cameraPos.y + 10 || rainPos.y < cameraPos.y - 10 || !biome->hasRain()
            || biome->getTemperature(rainPos) < 0.15f)
            continue;

        const double randomX = mRandom.nextDouble();
        const double randomZ = mRandom.nextDouble();
        const AABB* shape = ground->getShape(level, rainPos);
        if (ground->getMaterial() != Material::LAVA && ground->getBlock() != Blocks::magma)
        {
            if (ground->getMaterial() != Material::AIR)
            {
                const double dropX = groundPos.x + randomX;
                const double dropY = static_cast<float>(groundPos.y) + 0.1f + shape->mMaxY;
                const double dropZ = groundPos.z + randomZ;
                if (mRandom.nextInt(++rainHits) == 0)
                {
                    soundX = dropX;
                    soundY = dropY - 1.0;
                    soundZ = dropZ;
                }
                mMinecraft->mLevel->addParticle(PARTICLE_DROPLET, dropX, dropY, dropZ, 0.0, 0.0, 0.0, arrayWithLength<int>());
            }
        }
        else
        {
            mMinecraft->mLevel->addParticle(PARTICLE_SMOKE, rainPos.x + randomX, static_cast<float>(rainPos.y) + 0.1f - shape->mMinY,
                               rainPos.z + randomZ, 0.0, 0.0, 0.0, arrayWithLength<int>());
        }
    }

    if (rainHits > 0 && mRandom.nextInt(3) < mRainSoundTime++)
    {
        mRainSoundTime = 0;
        MemSect(0x18);
        if (soundY > cameraPos.y + 1 && level->getTopRainBlockPos(cameraPos).y > cameraPos.y)
            mMinecraft->mLevel->playLocalSound(soundX, soundY, soundZ, SoundEvents::WEATHER_RAIN_ABOVE,
                                  SoundSource::eSoundSource_Weather, 0.1f, 0.5f, false, 16.0f);
        else
            mMinecraft->mLevel->playLocalSound(soundX, soundY, soundZ, SoundEvents::WEATHER_RAIN, SoundSource::eSoundSource_Weather,
                                  0.2f, 1.0f, false, 16.0f);
        MemSect(0);
    }
}

void GameRenderer::tick(bool tickWorld)
{
    if (tickWorld)
        UpdateElementStorageDeleteTicks();

    tickFov();
    tickLightTexture();

    mFogBrightnessO = mFogBrightness;
    mThirdPersonDistanceO = mThirdPersonDistance;

    if (!mMinecraft->mOptions->mUnknownFlag4)
    {
        mSmoothCamFilterX = 0.0f;
        mSmoothCamFilterY = 0.0f;
    }
    else
    {
        const float sensitivity = mMinecraft->mOptions->mSensitivity * 0.6f + 0.2f;
        const float smoothDelta = 0.05f * (sensitivity * sensitivity * sensitivity * 8.0f);
        mSmoothCamFilterX = mSmoothTurnX.getNewDeltaValue(mSmoothCamYaw, smoothDelta);
        mSmoothCamFilterY = mSmoothTurnY.getNewDeltaValue(mSmoothCamPitch, smoothDelta);
        mSmoothCamPartialTicks = 0.0f;
        mSmoothCamPitch = 0.0f;
        mSmoothCamYaw = 0.0f;
    }

    boost::shared_ptr<Entity> cameraEntity = mMinecraft->getCameraEntity();
    cameraEntity->mEyeHeightO = cameraEntity->mEyeHeight;
    cameraEntity->mEyeHeight += (mMinecraft->getCameraEntity()->getEyeHeight() - cameraEntity->mEyeHeight) * 0.5f;

    float blockBrightness;
    {
        boost::shared_ptr<Entity> cameraForBlockPosition = mMinecraft->getCameraEntity();
        blockBrightness = mMinecraft->mLevel->getBrightness(BlockPos(cameraForBlockPosition));
    }
    const float targetFogBrightness = static_cast<float>(blockBrightness * 0.0f + 1.0f);
    mFogBrightness += (targetFogBrightness - mFogBrightness) * 0.1f;

    mItemInHandRenderer->tick();

    tickRain();

    mDarkenWorldAmountO = mDarkenWorldAmount;

    BossHealthOverlay* bossOverlay = mMinecraft->mGui->getBossOverlay(mMinecraft->mPlayer->mUserIndex);
    if (bossOverlay->shouldDarkenScreen())
    {
        mDarkenWorldAmount += 0.05f;
        if (mDarkenWorldAmount > 1.0f)
            mDarkenWorldAmount = 1.0f;
    }
    else if (mDarkenWorldAmount > 0.0f)
    {
        mDarkenWorldAmount -= 0.0125f;
    }

    const int playerIndex = mMinecraft->mPlayer->mUserIndex;
    if (mItemActivationTicks[playerIndex] > 0)
    {
        --mItemActivationTicks[playerIndex];
        if (mItemActivationTicks[playerIndex] == 0)
            mItemActivationItems[playerIndex] = ItemInstance::EMPTY;
    }

    const int primaryPad = gProfile->GetPrimaryPad();
    if (mMinecraft->mPlayer.get() == mMinecraft->mLocalPlayers[primaryPad].get())
    {
        ++mTickCount;
    }
}

static bool isPickable_apply(boost::shared_ptr<Entity> entity)
{
    return entity->isPickable() || eTYPE_DERIVED_FROM(EntityType::ITEM_ENTITY, entity->GetType());
}

void GameRenderer::pick(float partialTick)
{
    static const double kPickDistanceCompare = 3.0;
    static const double kCreativePickRange = 6.0;
    static const double kNoHitDistance = 0.0;

    boost::shared_ptr<Entity> camera = getCamera();
    if (camera.get() == NULL)
    {
        return;
    }

    Level* level = mMinecraft->mLevel;
    if (level == NULL)
    {
        return;
    }

    boost::shared_ptr<Entity>& pointedEntity = mPointedEntity;
    boost::shared_ptr<Entity>& minecraftPointedEntityShared = mMinecraft->mSpectateEntity;

    minecraftPointedEntityShared.reset();

    const double pickRange = mMinecraft->mGameMode->getPickRange();

    delete mMinecraft->mHitResult;
    mMinecraft->mHitResult = NULL;

#ifdef MINECRAFT_PC
    struct ModAim
    {
        HitResult** hit;
        ModAim(Entity* camera, float partialTick, HitResult** result) : hit(result)
        {
            PcModViewsAimBegin(camera, partialTick);
        }
        ~ModAim() { PcModViewsAimEnd(*hit); }
    } modAim(camera.get(), partialTick, &mMinecraft->mHitResult);
#endif
    MemSect(0x1f);
    mMinecraft->mHitResult = camera->pick(pickRange, partialTick);
    MemSect(0);

    Dimension* dimension = level->mDimension;
    if (mMinecraft->mHitResult != NULL && dimension->getType() != DimensionType::THE_END)
    {
        BlockPos blockPos = mMinecraft->mHitResult->getBlockPos();
        int adjustedX = blockPos.x;
        int adjustedZ = blockPos.z;
        const int stepX = mMinecraft->mHitResult->mDirection->getStepX();
        const int stepZ = mMinecraft->mHitResult->mDirection->getStepZ();
        if ((adjustedX < 0) == (stepX < 0))
        {
            adjustedX += stepX;
        }
        if ((adjustedZ < 0) == (stepZ < 0))
        {
            adjustedZ += stepZ;
        }

        const AABB* bounds = dimension->getPlayerConstraints(blockPos);
        if (adjustedX < static_cast<int>(bounds->mMinX) + 1 || static_cast<int>(bounds->mMaxX) - 2 < adjustedX
            || adjustedZ < static_cast<int>(bounds->mMinZ) + 1 || static_cast<int>(bounds->mMaxZ) - 2 < adjustedZ)
        {
            delete mMinecraft->mHitResult;
            mMinecraft->mHitResult = NULL;
        }
    }

    Entity* cameraRaw = camera.get();
    Vec3* eyePosition = cameraRaw->getEyePosition(partialTick);

    const bool beyondSurvivalReach = pickRange > kPickDistanceCompare;
    const bool farPickRange = mMinecraft->mGameMode->hasFarPickRange();
    const double entityReach = farPickRange ? kCreativePickRange : pickRange;

    double blockHitDistance = entityReach;
    if (mMinecraft->mHitResult != NULL)
    {
        blockHitDistance = eyePosition->distanceTo(mMinecraft->mHitResult->mLocation);
    }

    Vec3* viewVector = cameraRaw->getViewVector(1.0f);
    Vec3* rayEnd = eyePosition->add(viewVector->x * entityReach, viewVector->y * entityReach,
                                    viewVector->z * entityReach);

    pointedEntity.reset();

    static Predicate<boost::shared_ptr<Entity> >* sIsPickablePredicate =
        new Predicates<boost::shared_ptr<Entity> >::FunctionPredicate(isPickable_apply);
    const Predicate<boost::shared_ptr<Entity> >* predicates[2] = {Predicates<boost::shared_ptr<Entity> >::alwaysTrue,
                                                                  sIsPickablePredicate};
    static Predicate<boost::shared_ptr<Entity> >* sPickPredicate =
        new Predicates<boost::shared_ptr<Entity> >::ConnectivePredicate(
            Predicates<boost::shared_ptr<Entity> >::ConnectivePredicate::eOperator_And, predicates, 2);
    const Predicate<boost::shared_ptr<Entity> >* entityPredicate = sPickPredicate;
    if (mMinecraft->mOptions->mRenderDebug != 0)
    {
        entityPredicate = NULL;
    }

    if (Minecraft::InMiniGame(eMiniGameId_Tumble, false))
    {
        return;
    }

    AABB* searchBox = cameraRaw->mBoundingBox
                          ->expand(viewVector->x * entityReach, viewVector->y * entityReach,
                                   viewVector->z * entityReach)
                          ->grow(1.0, 1.0, 1.0);

    std::vector<boost::shared_ptr<Entity> >* entities = level->getEntities(camera, searchBox, entityPredicate, NULL);

    Vec3* entityHitLocation = NULL;
    double closestDistance = blockHitDistance;

    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities->begin(); it != entities->end(); ++it)
    {
        boost::shared_ptr<Entity> candidate = *it;
        Entity* candidateRaw = candidate.get();

        AABB* grownBox = candidateRaw->mBoundingBox->grow(candidateRaw->getPickRadius());
        HitResult* clipHit = grownBox->clip(eyePosition, rayEnd);
        if (grownBox->contains(eyePosition))
        {
            if (closestDistance >= kNoHitDistance)
            {
                pointedEntity = candidate;
                entityHitLocation = clipHit == NULL ? eyePosition : clipHit->mLocation;
                closestDistance = kNoHitDistance;
            }
        }
        else if (clipHit != NULL)
        {
            const double hitDistance = eyePosition->distanceTo(clipHit->mLocation);
            if (hitDistance < closestDistance || closestDistance == kNoHitDistance)
            {
                boost::shared_ptr<Entity> candidateRoot = candidateRaw->getRootVehicle();
                boost::shared_ptr<Entity> cameraRoot = cameraRaw->getRootVehicle();
                if (candidateRoot.get() == cameraRoot.get())
                {
                    if (closestDistance == kNoHitDistance)
                    {
                        pointedEntity = candidate;
                        entityHitLocation = clipHit->mLocation;
                    }
                }
                else
                {
                    pointedEntity = candidate;
                    entityHitLocation = clipHit->mLocation;
                    closestDistance = hitDistance;
                }
            }
        }
        if (clipHit != NULL)
        {
            delete clipHit;
        }
    }

    if (pointedEntity.get() == NULL)
    {
        return;
    }

    if (!farPickRange && beyondSurvivalReach && eyePosition->distanceTo(entityHitLocation) > kPickDistanceCompare)
    {
        pointedEntity.reset();
        delete mMinecraft->mHitResult;
        mMinecraft->mHitResult = new HitResult(HitResult::MISS, entityHitLocation, NULL, BlockPos(entityHitLocation));
        return;
    }

    if (closestDistance < blockHitDistance || mMinecraft->mHitResult == NULL)
    {
        delete mMinecraft->mHitResult;
        mMinecraft->mHitResult = new HitResult(pointedEntity, entityHitLocation);

        const unsigned long long entityType = pointedEntity->GetType();
        if ((entityType & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY
            || entityType == EntityType::ITEM_FRAME)
        {
            minecraftPointedEntityShared = pointedEntity;
        }
    }
}

float GameRenderer::getFov(float amount, bool useSmoothing)
{
    if (mPanoramicMode)
        return 90.0f;

    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mPlayer;
    const int playerIndex = player != NULL ? player->mUserIndex : 0;

#ifdef MINECRAFT_PC
    float fov = useSmoothing ? static_cast<float>(PcSettingsFov()) : mFovSetting;
#else
    float fov = mFovSetting;
#endif
    if (useSmoothing)
    {
        fov = (fov + mMinecraft->mOptions->mFov)
              * ((mFov[playerIndex] - mFovO[playerIndex]) * amount + mFovO[playerIndex]);
    }

    if (player->getHealth() <= 0.0f)
    {
        float deathTime = static_cast<float>(player->mDeathTime) + amount;
        if (deathTime > 20.0f)
            deathTime = 20.0f;
        const float divisor = 1.0f - 500.0f / (deathTime + 500.0f);
        fov = fov / (divisor + divisor + 1.0f);
    }

    const BlockState* cameraBlock = Camera::getBlockAt(mMinecraft->mLevel, player, amount);
    if (cameraBlock->getMaterial()->isUnderWater())
        fov = fov * 60.0f / 70.0f;

    return fov;
}

void GameRenderer::bobHurt(float partialTick)
{
    static const float kZero = 0.0f;
    static const float kOne = 1.0f;
    static const float kNauseaMax = 20.0f;
    static const float kNauseaBase = 40.0f;
    static const float kNauseaDiv = 8000.0f;
    static const float kNauseaAdd = 200.0f;
    static const float kHurtBobScale = 14.0f;
    static const float kPi = 3.1415927f;

    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, getCamera()->GetType()))
    {
        boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(getCamera());

        float hurt = static_cast<float>(living->mHurtTime) - partialTick;
        if (living->getHealth() <= kZero)
        {
            float deathTime = static_cast<float>(living->mDeathTime) + partialTick;
            if (deathTime > kNauseaMax)
                deathTime = kNauseaMax;
            GlStateManager::rotatef(kNauseaBase - kNauseaDiv / (deathTime + kNauseaAdd), kZero, kZero, kOne);
        }

        if (hurt < kZero)
            return;

        hurt = hurt / static_cast<float>(living->mHurtDuration);
        hurt = Mth::sin(hurt * hurt * hurt * hurt * kPi);
        const float hurtDir = living->mHurtDir;
        GlStateManager::rotatef(-hurtDir, kZero, kOne, kZero);
        GlStateManager::rotatef(-hurt * kHurtBobScale, kZero, kZero, kOne);
        GlStateManager::rotatef(hurtDir, kZero, kOne, kZero);
    }
}

void GameRenderer::bobView(float partialTick)
{
    if (!eTYPE_DERIVED_FROM(EntityType::PLAYER, getCamera()->GetType()))
        return;

    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(getCamera());
    const float walkDelta = player->mWalkDist - player->mWalkDistO;
    const float walk = -(player->mWalkDist + walkDelta * partialTick);
    const float dist = player->mPrevCameraYaw + (player->mCameraYaw - player->mPrevCameraYaw) * partialTick;
    const float roll = player->mOBob + (player->mBob - player->mOBob) * partialTick;

    GlStateManager::translatef(Mth::sin(walk * 3.1415927f) * dist * 0.5f,
                               -std::fabs(Mth::cos(walk * 3.1415927f) * dist), 0.0f);
    GlStateManager::rotatef(Mth::sin(walk * 3.1415927f) * dist * 3.0f, 0.0f, 0.0f, 1.0f);
    GlStateManager::rotatef(std::fabs(Mth::cos(walk * 3.1415927f - 0.2f) * dist) * 5.0f, 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(roll, 1.0f, 0.0f, 0.0f);
}

unsigned int GameRenderer::getCameraThirdPerson()
{
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(mMinecraft->mPlayer);
    return player->GetThirdPersonView(false);
}

float GameRenderer::getCameraYRot(float amount)
{
    return (mMinecraft->mPlayer->mYRot - mMinecraft->mPlayer->mYRotO) * amount + mMinecraft->mPlayer->mYRotO;
}

float GameRenderer::getCameraXRot(float amount)
{
    return (mMinecraft->mPlayer->mXRot - mMinecraft->mPlayer->mXRotO) * amount + mMinecraft->mPlayer->mXRotO;
}

void GameRenderer::moveCameraToPlayer(float partialTick)
{
    static const float kZero = 0.0f;
    static const float kOne = 1.0f;
    static const float kNegOne = -1.0f;
    static const float k180 = 180.0f;
    static const float kFirstPersonNudge = 0.05f;
    static const float kSleepingYOffset = 0.3f;
    static const float kSleepingHeightAdd = 1.0f;
    static const float kBedFacingRotation = 90.0f;
    static const float kCollisionProbe = 0.1f;
    static const float kFallFlightYawScale = 0.5f;
    static const float kFallFlightAlignmentScale = 0.25f;

    boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(getCamera());

    const unsigned int thirdPerson = getCameraThirdPerson();
    float eyeHeight = (living->mEyeHeight - living->mEyeHeightO) * partialTick + living->mEyeHeightO;
    double x = (living->mX - living->mXo) * partialTick + living->mXo;
    double yEye = (living->mY - living->mYo) * partialTick + living->mYo + living->getEyeHeight();
    double z = (living->mZ - living->mZo) * partialTick + living->mZo;

    if (thirdPerson == 3)
    {
        CameraController* controller = boost::static_pointer_cast<Player>(mMinecraft->mPlayer)->GetCameraController();
        if (controller != NULL)
        {
            GlStateManager::translatef(static_cast<float>(controller->getCurrentPos()->x),
                                       static_cast<float>(controller->getCurrentPos()->y),
                                       static_cast<float>(controller->getCurrentPos()->z));
            GlStateManager::rotatef(static_cast<float>(controller->getCurrentRot()->x), kOne, kZero, kZero);
            GlStateManager::rotatef(static_cast<float>(controller->getCurrentRot()->y), kZero, kOne, kZero);
            GlStateManager::rotatef(static_cast<float>(controller->getCurrentRot()->z), kZero, kZero, kOne);
        }
        else
        {
            mMinecraft->mPlayer->SetThirdPersonView(1);
        }
    }
    else
    {
        if (living->isSleeping())
        {
            eyeHeight = eyeHeight + kSleepingHeightAdd;
            GlStateManager::translatef(kZero, kSleepingYOffset, kZero);
            if (mMinecraft->mOptions->mUnknownFlag5 == 0)
            {
                const BlockPos cameraBlockPos(living);
                const BlockState* bedState = mMinecraft->mLevel->getBlockState(cameraBlockPos);
                if (bedState->getBlock() == Blocks::bed)
                {
                    const Direction* facing = bedState->getValue<const Direction*>(
                        HorizontalDirectionalBlock::FACING_PROPERTY);
                    GlStateManager::rotatef(static_cast<float>(facing->get2DDataValue()) * kBedFacingRotation, kZero,
                                            kOne, kZero);
                }
                GlStateManager::rotatef(static_cast<float>(getCameraYRot(partialTick) + k180), kZero, kNegOne, kZero);
                GlStateManager::rotatef(static_cast<float>(getCameraXRot(partialTick)), kNegOne, kZero, kZero);
            }
        }
        else
        {
            if (thirdPerson != 0)
            {
                double distance = static_cast<double>((mThirdPersonDistance - mThirdPersonDistanceO) * partialTick
                                                      + mThirdPersonDistanceO);

                if (mMinecraft->mOptions->mUnknownFlag5 != 0)
                {
                    GlStateManager::translatef(kZero, kZero, -static_cast<float>(distance));
                }
                else
                {
                    const float cameraYaw = static_cast<float>(getCameraYRot(partialTick));
                    const float cameraPitch = static_cast<float>(getCameraXRot(partialTick));
                    float yaw = cameraYaw;
                    float pitch = cameraPitch;
                    if (thirdPerson == 2)
                        pitch = pitch + k180;

                    double offsetX = static_cast<float>(-Mth::sin(yaw * Mth::DEG_TO_RAD)
                                                        * Mth::cos(pitch * Mth::DEG_TO_RAD))
                                     * distance;
                    double offsetZ = static_cast<float>(Mth::cos(yaw * Mth::DEG_TO_RAD)
                                                        * Mth::cos(pitch * Mth::DEG_TO_RAD))
                                     * distance;
                    double offsetY = -Mth::sin(pitch * Mth::DEG_TO_RAD) * distance;

                    for (int corner = 0; corner < 8; ++corner)
                    {
                        const float probeY = static_cast<float>((corner & 2) - 1) * kCollisionProbe;
                        const float probeZ = static_cast<float>((corner >> 1 & 2) - 1) * kCollisionProbe;
                        const float probeX = static_cast<float>((corner & 1) * 2 - 1) * kCollisionProbe;

                        Vec3* from = Vec3::newTemp(x + probeX, yEye + probeY, z + probeZ);
                        Vec3* to = Vec3::newTemp(x - offsetX + probeX, yEye - offsetY + probeY, z - offsetZ + probeZ);

                        HitResult* hitForward = mMinecraft->mLevel->clip(from, to, false, true, false, false, NULL,
                                                                         false);
                        HitResult* hitReverse = mMinecraft->mLevel->clip(to, from, false, true, false, false, NULL,
                                                                         false);

                        if (hitForward != NULL)
                        {
                            if (hitReverse != NULL)
                            {
                                Block* forwardBlock = mMinecraft->mLevel->getBlock(hitForward->getBlockPos());
                                Block* reverseBlock = mMinecraft->mLevel->getBlock(hitReverse->getBlockPos());
                                if ((forwardBlock->mMaterial->blocksMotion()
                                     && (!Minecraft::InMiniGame(eMiniGameId_Any, true)
                                         || gApp.ShouldCollideWith(forwardBlock)))
                                    || (reverseBlock->mMaterial->blocksMotion()
                                        && (!Minecraft::InMiniGame(eMiniGameId_Any, true)
                                            || gApp.ShouldCollideWith(reverseBlock))))
                                {
                                    const double hitDistance = hitForward->mLocation->distanceTo(
                                        Vec3::newTemp(x, yEye, z));
                                    if (hitDistance < distance)
                                        distance = hitDistance;
                                }
                                delete hitForward;
                                delete hitReverse;
                            }
                            else
                            {
                                Block* block = mMinecraft->mLevel->getBlock(hitForward->getBlockPos());
                                if (block->mMaterial->blocksMotion())
                                {
                                    if (!Minecraft::InMiniGame(eMiniGameId_Any, true) || gApp.ShouldCollideWith(block))
                                    {
                                        const double hitDistance = hitForward->mLocation->distanceTo(
                                            Vec3::newTemp(x, yEye, z));
                                        if (hitDistance < distance)
                                            distance = hitDistance;
                                    }
                                }
                                delete hitForward;
                            }
                        }
                        else if (hitReverse != NULL)
                        {
                            delete hitReverse;
                        }
                    }

                    if (thirdPerson == 2)
                        GlStateManager::rotatef(k180, kZero, kOne, kZero);

                    GlStateManager::rotatef(cameraPitch - pitch, kOne, kZero, kZero);
                    GlStateManager::rotatef(cameraYaw - yaw, kZero, kOne, kZero);
                    GlStateManager::translatef(kZero, kZero, -static_cast<float>(distance));
                    GlStateManager::rotatef(yaw - cameraYaw, kZero, kOne, kZero);
                    GlStateManager::rotatef(pitch - cameraPitch, kOne, kZero, kZero);
                }
            }
            else
            {
#ifdef MINECRAFT_PC
                if (!PcModViewsInView())
                    GlStateManager::translatef(kZero, kZero, kFirstPersonNudge);
#else
                GlStateManager::translatef(kZero, kZero, kFirstPersonNudge);
#endif
            }
        }

        if (living->isFallFlying())
        {
            if (GameSettings::GetGameSettings(mMinecraft->mPlayer->mUserIndex, eGameSetting_FlyingViewRolling) == 0)
            {
                const float fallFlightRotation =
                    ((mMinecraft->mPlayer->mFallFlyYaw - mMinecraft->mPlayer->mFallFlyYawOld) * partialTick
                     + mMinecraft->mPlayer->mFallFlyYawOld)
                    * ((mMinecraft->mPlayer->mFallFlyingAlignment - mMinecraft->mPlayer->mFallFlyingAlignmentO)
                           * partialTick
                       + mMinecraft->mPlayer->mFallFlyingAlignmentO);
                GlStateManager::rotatef(fallFlightRotation * kFallFlightYawScale, kZero, kZero, kOne);
                GlStateManager::rotatef(fallFlightRotation * kFallFlightAlignmentScale, kZero, kOne, kZero);
            }
        }
    }

    if (mMinecraft->mOptions->mUnknownFlag5 == 0)
    {
        GlStateManager::rotatef(static_cast<float>(getCameraXRot(partialTick)), kOne, kZero, kZero);
        if (eTYPE_DERIVED_FROM(EntityType::ANIMAL, living->GetType()))
        {
            boost::shared_ptr<Animal> animal = boost::static_pointer_cast<Animal>(living);
            GlStateManager::rotatef((animal->mYHeadRot - animal->mYHeadRotO) * partialTick + animal->mYHeadRotO + k180,
                                    kZero, kOne, kZero);
        }
        else
        {
            GlStateManager::rotatef(static_cast<float>(getCameraYRot(partialTick) + k180), kZero, kOne, kZero);
        }
    }

    GlStateManager::translatef(kZero, -eyeHeight, kZero);

    mCloudFog = mMinecraft->mLevelRenderer->isInCloud(
        (living->mX - living->mXo) * partialTick + living->mXo,
        (living->mY - living->mYo) * partialTick + living->mYo + eyeHeight,
        (living->mZ - living->mZo) * partialTick + living->mZo, partialTick);
}

void GameRenderer::getFovAndAspect(float& fov, float& aspect, float partialTick, bool useSmoothing)
{
    aspect = static_cast<float>(mMinecraft->mWidth) / static_cast<float>(mMinecraft->mHeight);
#ifdef MINECRAFT_PC
    aspect = PcGetWorldAspect();
#endif
    fov = getFov(partialTick, useSmoothing);

    if (mMinecraft->mPlayer->mViewport == 1 || mMinecraft->mPlayer->mViewport == 2)
    {
        aspect = aspect + aspect;
        fov = fov * 0.7f;
    }
    else if (mMinecraft->mPlayer->mViewport == 3 || mMinecraft->mPlayer->mViewport == 4)
    {
        aspect = aspect * 0.5f;
        fov = fov * 1.1f;
    }
}

void GameRenderer::setupCamera(float partialTick, int eyeIndex)
{
    static const float kZero = 0.0f;
    static const float kOne = 1.0f;
    static const float kDefaultFog = 256.0f;
    static const float kProjectionEyeOffset = 0.07f;
    static const float kModelEyeOffset = 0.1f;
    static const float kNearPlane = 0.05f;
    static const float kSpectatorZoomScale = 0.04f;
    static const float kSpectatorZoomDenom = 5.0f;
    static const float kRotate90 = 90.0f;
    static const float kRotateMinus90 = -90.0f;
    static const float kRotate180 = 180.0f;
    static const unsigned int kAnimBitNoViewBob = 9;

#ifdef MINECRAFT_PC
    mFogDistance = PcSettingsFogDistance();
#else
    mFogDistance = kDefaultFog;
#endif

    Level* level = Minecraft::GetInstance()->mLevels[0];
    if (level != NULL && level->getFogDistance() > 0)
        mFogDistance = static_cast<float>(level->getFogDistance());

    GlStateManager::matrixMode(1);
    GlStateManager::loadIdentity();
#ifdef MINECRAFT_PC
    if (PcModViewsInView())
    {
        float matrix[16];
        PcModViewsProjection(kNearPlane, mFogDistance + mFogDistance, matrix);
        GlStateManager::glMultMatrixf(matrix);
        GlStateManager::matrixMode(0);
        GlStateManager::loadIdentity();
        memcpy(matrix, PcModViewsMatrix(), sizeof(matrix));
        GlStateManager::glMultMatrixf(matrix);
        moveCameraToPlayer(partialTick);
        return;
    }
#endif
    if (mMinecraft->mOptions->mAnaglyph3d)
    {
        const float eyeOffset = static_cast<float>(-(eyeIndex * 2 - 1));
        GlStateManager::translatef(eyeOffset * kProjectionEyeOffset, kZero, kZero);
    }

    float aspect;
    float fov;
    getFovAndAspect(fov, aspect, partialTick, true);

    if (mZoom != kOne)
    {
        GlStateManager::translatef(mZoomX, -mZoomY, kZero);
        GlStateManager::scaled(mZoom, mZoom, kOne);
    }

    const float farPlane = mFogDistance + mFogDistance;
    GlStateManager::perspective(fov, aspect, kNearPlane, farPlane);

    GlStateManager::matrixMode(0);
    GlStateManager::loadIdentity();
    if (mMinecraft->mOptions->mAnaglyph3d)
    {
        const float eyeOffset = static_cast<float>(eyeIndex * 2 - 1);
        GlStateManager::translatef(eyeOffset * kModelEyeOffset, kZero, kZero);
    }

    bobHurt(partialTick);

    _SkinAdjustments skinAdjustments = mMinecraft->mPlayer->getSkinAdjustments();

    const bool legsStill = ((skinAdjustments.mAnimFlags >> HumanoidModel::eAnimBit_LegsStill) & 1u) != 0;
    const bool noViewBob = ((skinAdjustments.mAnimFlags >> kAnimBitNoViewBob) & 1u) != 0;

    if (GameSettings::GetGameSettings(mMinecraft->mPlayer->mUserIndex, eGameSetting_ViewBob) != 0)
    {
        if (!mMinecraft->mPlayer->mAbilities.mFlying && !legsStill && !noViewBob)
        {
            if (!mMinecraft->mPlayer->isSwimming())
                bobView(partialTick);
        }
    }

    const float interpolatedPortalIntensity =
        mMinecraft->mPlayer->mPortalEffectIntensityO
        + (mMinecraft->mPlayer->mPortalEffectIntensity - mMinecraft->mPlayer->mPortalEffectIntensityO) * partialTick;
    if (interpolatedPortalIntensity > kZero)
    {
        int rotateAmount = 20;
        if (mMinecraft->mPlayer->hasEffect(MobEffects::CONFUSION))
            rotateAmount = 7;
        const float zoom = -(interpolatedPortalIntensity * kSpectatorZoomScale
                             - (kSpectatorZoomDenom
                                / (interpolatedPortalIntensity * interpolatedPortalIntensity + kSpectatorZoomDenom)));
        const float rotateScale = static_cast<float>(rotateAmount);
        const float frame = static_cast<float>(mTickCount) + partialTick;
        GlStateManager::rotatef(frame * rotateScale, kZero, kOne, kOne);
        GlStateManager::scalef(kOne / (zoom * zoom), kOne, kOne);
        GlStateManager::rotatef(-(frame * rotateScale), kZero, kOne, kOne);
    }

    moveCameraToPlayer(partialTick);

    if (!mPanoramicMode)
        return;

    switch (mPanoramicSection)
    {
    case 0:
        GlStateManager::rotatef(kRotate90, kZero, kOne, kZero);
        break;
    case 1:
        GlStateManager::rotatef(kRotate180, kZero, kOne, kZero);
        break;
    case 2:
        GlStateManager::rotatef(kRotateMinus90, kZero, kOne, kZero);
        break;
    case 3:
        GlStateManager::rotatef(kRotate90, kOne, kZero, kZero);
        break;
    case 4:
        GlStateManager::rotatef(kRotateMinus90, kOne, kZero, kZero);
        break;
    }
}

void GameRenderer::turnOnLightLayer(double)
{
    gRenderer->TextureBindVertex(
        static_cast<int>(getLightTexture(mMinecraft->mPlayer->mUserIndex, mMinecraft->mLevel)));
    GlStateManager::texParameteri(1, 1, 1);
    GlStateManager::texParameteri(1, 2, 1);
    GlStateManager::texParameteri(1, 3, 0);
    GlStateManager::texParameteri(1, 4, 0);
}

void GameRenderer::turnOffLightLayer(double)
{
    gRenderer->TextureBindVertex(-1);
}

void GameRenderer::renderItemInHand(float partialTick, int eyeIndex)
{
    static const float kZero = 0.0f;
    static const float kOne = 1.0f;
    static const float kProjectionEyeOffset = 0.07f;
    static const float kModelEyeOffset = 0.1f;
    static const float kNearPlane = 0.05f;

    const unsigned int thirdPerson = getCameraThirdPerson();
    if (mPanoramicMode)
    {
        return;
    }

    mRenderHand = true;

    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mPlayer;
    bool holdingMap = false;
    const bool cameraIsLocalPlayer = eTYPE_DERIVED_FROM(EntityType::LOCAL_PLAYER, getCamera()->GetType());
    if (cameraIsLocalPlayer)
    {
        not_null_ptr<ItemInstance> selected = localPlayer->mInventory->getSelected();
        ItemInstance* item = selected.get();
        if (item != NULL && item->getItem() == Items::filledMap)
            holdingMap = true;

        if (GameSettings::GetGameSettings(localPlayer->mUserIndex, eGameSetting_DisplayHand) == 0)
            mRenderHand = false;
    }

    GlStateManager::matrixMode(1);
    GlStateManager::loadIdentity();
    if (mMinecraft->mOptions->mAnaglyph3d)
    {
        const float eyeOffset = static_cast<float>(-(eyeIndex * 2 - 1));
        GlStateManager::translatef(eyeOffset * kProjectionEyeOffset, kZero, kZero);
    }

    float fov = partialTick;
    float aspect = partialTick;
    getFovAndAspect(fov, aspect, partialTick, false);

    if (mZoom != kOne)
    {
        GlStateManager::translatef(mZoomX, -mZoomY, kZero);
        GlStateManager::scaled(mZoom, mZoom, kOne);
    }

    const float farPlane = mFogDistance + mFogDistance;
    GlStateManager::perspective(fov, aspect, kNearPlane, farPlane);

    GlStateManager::matrixMode(0);
    GlStateManager::loadIdentity();
    if (mMinecraft->mOptions->mAnaglyph3d)
    {
        const float eyeOffset = static_cast<float>(eyeIndex * 2 - 1);
        GlStateManager::translatef(eyeOffset * kModelEyeOffset, kZero, kZero);
    }

    GlStateManager::pushMatrix();
    bobHurt(partialTick);

    _SkinAdjustments skinAdjustments = localPlayer->getSkinAdjustments();
    const unsigned int skinFlags = skinAdjustments.mAnimFlags;
    const unsigned int skinMask = skinFlags & 0x204u;
    const unsigned int skinAlt = (skinMask - 1u) + (skinMask == 0u ? 1u : 0u);

    if (GameSettings::GetGameSettings(localPlayer->mUserIndex, eGameSetting_ViewBob) != 0)
    {
        if (!localPlayer->mAbilities.mFlying && skinMask == skinAlt)
        {
            if (!localPlayer->isSwimming())
                bobView(partialTick);
        }
    }

    const bool cameraSleeping = eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, getCamera()->GetType())
                                && boost::static_pointer_cast<LivingEntity>(getCamera())->isSleeping();

    const bool skipFirstPersonHand = (!mRenderHand && !holdingMap) || thirdPerson != 0 || cameraSleeping;

    if (skipFirstPersonHand)
    {
        GlStateManager::popMatrix();
    }
    else
    {
        if (!localPlayer->isSpectator() && !mMinecraft->mOptions->mHideGui && !mMinecraft->mGameMode->isCutScene())
        {
            turnOnLightLayer(static_cast<double>(partialTick));
            mItemInHandRenderer->render(partialTick);
            turnOffLightLayer(static_cast<double>(partialTick));
        }
        GlStateManager::popMatrix();
    }

    if (localPlayer != NULL && getCameraThirdPerson() == 0 && !cameraSleeping)
    {
        mItemInHandRenderer->renderScreenEffect(partialTick);
        bobHurt(partialTick);
    }

    if (GameSettings::GetGameSettings(localPlayer->mUserIndex, eGameSetting_ViewBob) != 0)
    {
        if (!localPlayer->mAbilities.mFlying && skinMask == skinAlt)
        {
            if (!localPlayer->isSwimming())
                bobView(partialTick);
        }
    }
}

float GameRenderer::getNightVisionScale(boost::shared_ptr<LivingEntity> living, float partialTick)
{
    MobEffectInstance* nightVision = living->getEffect(MobEffects::NIGHT_VISION);
    MobEffectInstance* conduitPower = living->getEffect(MobEffects::CONDUIT_POWER);
    if (nightVision == NULL && conduitPower == NULL)
        return 0.0f;
    int duration = 0;
    if (nightVision != NULL)
        duration = nightVision->getDuration();
    if (conduitPower != NULL)
    {
        const int conduitDuration = conduitPower->getDuration();
        if (duration < conduitDuration)
            duration = conduitDuration;
    }
    if (duration > 200)
        return 1.0f;
    float remaining = static_cast<float>(duration) - partialTick;
    if (remaining < 0.0f)
        remaining = 0.0f;
    return Mth::sin(remaining * 3.1415927f * 0.05f) * 0.3f + 0.7f;
}

float GameRenderer::getUnderWaterVisionScale(const boost::shared_ptr<Player>& player, float partialTick)
{
    static const float kMinSkyDarken = 0.13333334f;
    static const float kMinBrightness = 2.0f;
    static const float kLightWeight = 0.9f;
    static const float kBrightnessWeight = 0.1f;
    static const float kBrightnessDivisor = 15.0f;
    static const float kPowBase = 0.4f;
    static const float kPowExp = 0.2f;
    static const float kOne = 1.0f;

    const float blend = powf(kPowBase, kPowExp);
    Player* entity = player.get();

    const double eyeHeight = entity->getEyeHeight();
    BlockPos eyePos(entity->mX, entity->mY + eyeHeight, entity->mZ);
    const float skyDarken = entity->mLevel->getSkyDarken(partialTick);
    float brightness = static_cast<float>(entity->mLevel->getBrightnessPropagate(LightLayer::SKY, eyePos));
    float underwaterLight = static_cast<float>(entity->getUnderwaterLightLevel());

    const float clampedSky = skyDarken - kMinSkyDarken >= 0.0f ? skyDarken : kMinSkyDarken;
    brightness = brightness - kMinBrightness >= 0.0f ? brightness : kMinBrightness;
    if (clampedSky < kOne)
        brightness *= clampedSky;

    const float lightLevel = kLightWeight * underwaterLight + kBrightnessWeight * brightness;
    const float normalized = lightLevel / kBrightnessDivisor;
    const float currentScale = static_cast<float>(entity->getUnderwaterVisionScale());
    const float result = (normalized - currentScale) * (kOne - blend) + currentScale;

    entity->setUnderwaterVisionScale(result);
    entity->setUnderwaterLightLevel(lightLevel);
    return result;
}

void GameRenderer::updateLightTexture(float partialTick)
{
    if (!mLightTextureDirty || mMinecraft->mLevel == NULL)
        return;

    for (int playerIndex = 0; playerIndex < 4; ++playerIndex)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[playerIndex];
        if (player == NULL)
            continue;

        Level* level = player->mLevel;
        const float skyDarken = level->getSkyDarken(1.0f);

        for (int lightIndex = 0; lightIndex < 0x100; ++lightIndex)
        {
            const int blockLight = lightIndex / 16;
            const int skyLight = lightIndex % 16;

            float blockBrightness = level->mDimension->getBrightnessRamp()[blockLight] * (skyDarken * 0.95f + 0.05f);
            const float skyBrightness = level->mDimension->getBrightnessRamp()[skyLight]
                                        * (mBlockLightRedFlicker * 0.1f + 1.5f);
            if (level->getSkyFlashTime() > 0)
                blockBrightness = level->mDimension->getBrightnessRamp()[blockLight];

            const float greenBase = blockBrightness * (skyDarken * 0.65f + 0.35f);
            const float greenFactor = skyBrightness * ((skyBrightness * 0.6f + 0.4f) * 0.6f + 0.4f);
            const float blueFactor = skyBrightness * (skyBrightness * skyBrightness * 0.6f + 0.4f);
            const unsigned int blockLightColor = level->mDimension->getBlockLightColour();
            const float lightRed = static_cast<float>((blockLightColor >> 16) & 0xff) / 255.0f;
            const float lightGreen = static_cast<float>((blockLightColor >> 8) & 0xff) / 255.0f;
            const float lightBlue = static_cast<float>(blockLightColor & 0xff) / 255.0f;
            const float redLight = skyBrightness * lightRed;
            const float blueLight = blueFactor * lightBlue;
            const float greenLight = greenFactor * lightGreen;

            float red = (greenBase + redLight) * 0.96f + 0.03f;
            float green = (greenBase + greenLight) * 0.96f + 0.03f;
            float blue = (blockBrightness + blueLight) * 0.96f + 0.03f;

            if (mDarkenWorldAmount > 0.0f)
            {
                const float flashAmount = (mDarkenWorldAmount - mDarkenWorldAmountO) * partialTick
                                          + mDarkenWorldAmountO;
                const float keep = 1.0f - flashAmount;
                red = red * keep + red * 0.7f * flashAmount;
                green = green * keep + green * 0.6f * flashAmount;
                blue = blue * keep + blue * 0.6f * flashAmount;
            }

            if (level->mDimension->getType() == DimensionType::THE_END)
            {
                red = redLight * 0.75f + 0.22f;
                green = greenLight * 0.75f + 0.28f;
                blue = blueLight * 0.75f + 0.25f;
            }

            if (player->hasEffect(MobEffects::NIGHT_VISION) || player->hasEffect(MobEffects::CONDUIT_POWER))
            {
                const float nightVision = getNightVisionScale(player, partialTick);
                float minScale = 1.0f / red;
                if (minScale > 1.0f / green)
                    minScale = 1.0f / green;
                if (minScale > 1.0f / blue)
                    minScale = 1.0f / blue;
                const float keep = 1.0f - nightVision;
                red = red * keep + red * minScale * nightVision;
                green = green * keep + green * minScale * nightVision;
                blue = blue * keep + blue * minScale * nightVision;
            }

            if (player->isUnderLiquid(Material::WATER))
            {
                const float underwaterScale = getUnderWaterVisionScale(player, partialTick);
                float minScale = 1.0f / red;
                if (minScale > 1.0f / green)
                    minScale = 1.0f / green;
                if (minScale > 1.0f / blue)
                    minScale = 1.0f / blue;
                const float keep = 1.0f - underwaterScale;
                red = red * keep + red * minScale * underwaterScale;
                green = green * keep + green * minScale * underwaterScale;
                blue = blue * keep + blue * minScale * underwaterScale;
            }

            if (red > 1.0f)
                red = 1.0f;
            if (green > 1.0f)
                green = 1.0f;
            if (blue > 1.0f)
                blue = 1.0f;

            const float redCurve = 1.0f - red;
            const float greenCurve = 1.0f - green;
            const float blueCurve = 1.0f - blue;
            red = ((1.0f - redCurve * redCurve * redCurve * redCurve) * 0.0f + red) * 0.96f + 0.03f;
            green = ((1.0f - greenCurve * greenCurve * greenCurve * greenCurve) * 0.0f + green) * 0.96f + 0.03f;
            blue = ((1.0f - blueCurve * blueCurve * blueCurve * blueCurve) * 0.0f + blue) * 0.96f + 0.03f;

            if (red > 1.0f)
                red = 1.0f;
            if (green > 1.0f)
                green = 1.0f;
            if (blue > 1.0f)
                blue = 1.0f;
            if (red < 0.0f)
                red = 0.0f;
            if (green < 0.0f)
                green = 0.0f;
            if (blue < 0.0f)
                blue = 0.0f;

            mLightPixels[playerIndex].data[lightIndex] = static_cast<int>(red * 255.0f) << 24
                                                         | static_cast<int>(green * 255.0f) << 16
                                                         | static_cast<int>(blue * 255.0f) << 8 | 0xff;
        }

        const unsigned int texture = getLightTexture(playerIndex, level);
        mMinecraft->mTextures->replaceTextureDirect(mLightPixels[playerIndex], 16, 16, texture);
    }
}

unsigned int GameRenderer::fjGetFpsCap(int value)
{
    static const unsigned int kFpsCapHigh = 120;
    static const unsigned int kFpsCapLow = 35;
    static const unsigned int kFpsCapDefault = 200;

    switch (value)
    {
    case 1:
        return kFpsCapHigh;
    case 2:
        return kFpsCapLow;
    default:
        return kFpsCapDefault;
    }
}

const BlockPos* GameRenderer::getFogOffsets()
{
    const BlockPos* offsets;
    switch (mMinecraft->mLevel->getLevelData()->getBiomeScale())
    {
    case 1:
        offsets = sEndFogOffsets;
        break;
    case 2:
        offsets = sNetherFogOffsets;
        break;
    default:
        offsets = sOverworldFogOffsets;
        break;
    }
    return offsets;
}

float GameRenderer::getTargetFogDistance(const BlockPos& pos)
{
    float distanceSum = 0.0f;
    float weightSum = 0.0f;
    const BlockPos* table = getFogOffsets();

    for (int i = 0; i < 27; ++i)
    {
        BlockPos samplePos = pos.offset(table[i].x, table[i].y, table[i].z);
        distanceSum += mMinecraft->mLevel->getBiome(samplePos)->getFogDistance();
        weightSum += 1.0f;
    }

    return distanceSum / weightSum;
}

void GameRenderer::getTargetFogColor(const BlockPos& pos, float& red, float& green, float& blue)
{
    float redSum = 0.0f;
    float greenSum = 0.0f;
    float blueSum = 0.0f;
    float weight = 0.0f;
    const BlockPos* table = getFogOffsets();

    for (int i = 0; i < 27; ++i)
    {
        BlockPos samplePos = pos.offset(table[i].x, table[i].y, table[i].z);
        const unsigned int colour = mMinecraft->mLevel->getBiome(samplePos)->getUnderWaterFogColor();
        redSum = static_cast<float>((colour >> 16) & 255u) * 0.00390625f + redSum;
        greenSum = static_cast<float>((colour >> 8) & 255u) * 0.00390625f + greenSum;
        blueSum = static_cast<float>(colour & 255u) * 0.00390625f + blueSum;
        weight += 1.0f;
    }

    red = redSum / weight;
    green = greenSum / weight;
    blue = blueSum / weight;
}

float GameRenderer::modifyFogDistance(const BlockPos& pos, float distance, float value)
{
    const float deltaY = distance - static_cast<float>(pos.getY());
    if (deltaY >= 0.0f)
    {
        if (deltaY >= 40.0f)
            value *= 0.33333334f;
        else
            value *= Mth::Lerp(1.0f, 0.33333334f, deltaY / 40.0f);
    }
    return value;
}

void GameRenderer::setupClearColor(float partialTick)
{
    Level* level = mMinecraft->mLevel;
    boost::shared_ptr<Entity> camera = getCamera();
    boost::shared_ptr<LivingEntity> living;
    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, camera->GetType()))
        living = boost::static_pointer_cast<LivingEntity>(camera);

    const float skyBlend = 1.0f - static_cast<float>(std::pow(0.25, 0.25));
    Vec3* skyColor = level->getSkyColor(getCamera(), partialTick);
    const float skyRed = static_cast<float>(skyColor->x);
    const float skyGreen = static_cast<float>(skyColor->y);
    const float skyBlue = static_cast<float>(skyColor->z);

    Vec3* fogColor = level->getFogColor(getCamera(), partialTick);
    mFogRed = static_cast<float>(fogColor->x);
    mFogGreen = static_cast<float>(fogColor->y);
    mFogBlue = static_cast<float>(fogColor->z);

    Vec3* sunDirection = Mth::sin(level->getSunAngle(partialTick)) > 0.0f ? Vec3::newTemp(-1.0, 0.0, 0.0)
                                                                          : Vec3::newTemp(1.0, 0.0, 0.0);
    const float sunDot = static_cast<float>(camera->getViewVector(partialTick)->dot(sunDirection));
    if (sunDot > 0.0f)
    {
        float* sunriseColor = level->mDimension->getSunriseColor(level->getTimeOfDay(partialTick), partialTick);
        if (sunriseColor != NULL)
        {
            const float sunriseBlend = sunDot * sunriseColor[3];
            mFogRed = mFogRed * (1.0f - sunriseBlend) + sunriseColor[0] * sunriseBlend;
            mFogGreen = mFogGreen * (1.0f - sunriseBlend) + sunriseColor[1] * sunriseBlend;
            mFogBlue = mFogBlue * (1.0f - sunriseBlend) + sunriseColor[2] * sunriseBlend;
        }
    }

    mFogRed += (skyRed - mFogRed) * skyBlend;
    mFogGreen += (skyGreen - mFogGreen) * skyBlend;
    mFogBlue += (skyBlue - mFogBlue) * skyBlend;

    const float rainLevel = level->getRainLevel(partialTick);
    if (rainLevel > 0.0f)
    {
        const float rainDarken = 1.0f - rainLevel * 0.5f;
        mFogRed *= rainDarken;
        mFogGreen *= rainDarken;
        mFogBlue *= 1.0f - rainLevel * 0.4f;
    }

    const float thunderLevel = level->getThunderLevel(partialTick);
    if (thunderLevel > 0.0f)
    {
        const float thunderDarken = 1.0f - thunderLevel * 0.5f;
        mFogRed *= thunderDarken;
        mFogGreen *= thunderDarken;
        mFogBlue *= thunderDarken;
    }

    const BlockState* cameraBlock = Camera::getBlockAt(mMinecraft->mLevel, camera, partialTick);
    if (mCloudFog)
    {
        Vec3* cloudColor = level->getCloudColor(partialTick);
        mFogRed = static_cast<float>(cloudColor->x);
        mFogGreen = static_cast<float>(cloudColor->y);
        mFogBlue = static_cast<float>(cloudColor->z);
    }

    if (!cameraBlock->getMaterial()->isUnderWater())
    {
        mFogHeightLocked = false;
    }
    else
    {
        boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mPlayer;
        const int playerIndex = player != NULL ? player->mUserIndex : 0;
        const long long now = System::processTimeInMilliSecs();
        BlockPos cameraPos(getCamera());

        float targetRed = 0.0f;
        float targetGreen = 0.0f;
        float targetBlue = 0.0f;
        getTargetFogColor(cameraPos, targetRed, targetGreen, targetBlue);
        if (targetRed != mTargetFogRed[playerIndex] || targetGreen != mTargetFogGreen[playerIndex]
            || targetBlue != mTargetFogBlue[playerIndex])
        {
            mTargetFogRed[playerIndex] = targetRed;
            mTargetFogGreen[playerIndex] = targetGreen;
            mTargetFogBlue[playerIndex] = targetBlue;
            mFogColourChangeTime[playerIndex] = now;
        }

        if (!mFogHeightLocked)
        {
            const double cameraY = getCamera()->mY;
            if (cameraY <= level->getSeaLevel())
                mFogHeight = level->getSeaLevel();
            else
                mFogHeight = static_cast<float>(cameraY);
        }

        const float fogDistance = modifyFogDistance(cameraPos, static_cast<float>(mFogHeight),
                                                    getTargetFogDistance(cameraPos));
        if (!mFogHeightLocked)
        {
            mPlayerFogRed[playerIndex] = mTargetFogRed[playerIndex];
            mPlayerFogGreen[playerIndex] = mTargetFogGreen[playerIndex];
            mPlayerFogBlue[playerIndex] = mTargetFogBlue[playerIndex];
            mPlayerFogDistance[playerIndex] = fogDistance;
        }

        const float colourProgress = Mth::clamp(static_cast<float>(now - mFogColourChangeTime[playerIndex]) / 4000.0f,
                                                0.0f, 1.0f);
        mPlayerFogRed[playerIndex] = Mth::Lerp(mPlayerFogRed[playerIndex], mTargetFogRed[playerIndex], colourProgress);
        mPlayerFogGreen[playerIndex] = Mth::Lerp(mPlayerFogGreen[playerIndex], mTargetFogGreen[playerIndex],
                                                 colourProgress);
        mPlayerFogBlue[playerIndex] = Mth::Lerp(mPlayerFogBlue[playerIndex], mTargetFogBlue[playerIndex],
                                                colourProgress);
        mFogRed = mPlayerFogRed[playerIndex];
        mFogGreen = mPlayerFogGreen[playerIndex];
        mFogBlue = mPlayerFogBlue[playerIndex];

        const float targetDistance = modifyFogDistance(cameraPos, static_cast<float>(mFogHeight),
                                                       getTargetFogDistance(cameraPos));
        if (targetDistance == mTargetFogDistance[playerIndex])
        {
            const float distanceProgress = Mth::clamp(
                static_cast<float>(now - mFogDistanceChangeTime[playerIndex]) / 4000.0f, 0.0f, 1.0f);
            mPlayerFogDistance[playerIndex] = Mth::Lerp(mPlayerFogDistance[playerIndex],
                                                        mTargetFogDistance[playerIndex], distanceProgress);
        }
        else
        {
            mTargetFogDistance[playerIndex] = targetDistance;
            mFogDistanceChangeTime[playerIndex] = now;
            mPlayerFogDistance[playerIndex] = Mth::Lerp(mPlayerFogDistance[playerIndex],
                                                        mTargetFogDistance[playerIndex], 0.0f);
        }

        float fogStart = 0.0f;
        float fogEnd = mPlayerFogDistance[playerIndex];
        if (eTYPE_DERIVED_FROM(EntityType::LOCAL_PLAYER, getCamera()->GetType()))
        {
            const float clarity = boost::static_pointer_cast<LocalPlayer>(getCamera())->getUnderwaterVisionClarity();
            if (clarity < 1.0f)
            {
                const float clarityScale = 0.6f * clarity + 0.4f;
                fogEnd *= clarityScale;
                fogStart *= clarityScale;
            }
        }

        GlStateManager::fogStart(fogStart + 4.0f);
        fogEnd += 4.0f;
        GlStateManager::fogEnd(fogEnd + 8.0f);
        GlStateManager::fogDensity(0.05f);
        mFogHeightLocked = true;
    }

    const float fogBrightness = (mFogBrightness - mFogBrightnessO) * partialTick + mFogBrightnessO;
    if (cameraBlock->getMaterial() == Material::LAVA)
    {
        const unsigned int lavaColour = Minecraft::GetInstance()->getColourTable()->getColour(
            eMinecraftColour_Under_Lava_Clear_Colour);
        mFogRed = static_cast<float>((lavaColour >> 16) & 0xff) / 256.0f * fogBrightness;
        mFogGreen = static_cast<float>((lavaColour >> 8) & 0xff) / 256.0f * fogBrightness;
        mFogBlue = static_cast<float>(lavaColour & 0xff) / 256.0f * fogBrightness;
    }
    else
    {
        mFogRed *= fogBrightness;
        mFogGreen *= fogBrightness;
        mFogBlue *= fogBrightness;
    }

    double voidBrightness = ((camera->mY - camera->mYOld) * partialTick + camera->mYOld)
                            * level->mDimension->getClearColorScale();
    if (living != NULL && living->hasEffect(MobEffects::BLINDNESS))
    {
        const int duration = living->getEffect(MobEffects::BLINDNESS)->getDuration();
        if (duration < 20)
            voidBrightness *= 1.0f - static_cast<float>(duration) / 20.0f;
        else
            voidBrightness = 0.0;
    }
    if (voidBrightness < 1.0)
    {
        if (voidBrightness < 0.0)
            voidBrightness = 0.0;
        voidBrightness *= voidBrightness;
        mFogRed = static_cast<float>(mFogRed * voidBrightness);
        mFogGreen = static_cast<float>(mFogGreen * voidBrightness);
        mFogBlue = static_cast<float>(mFogBlue * voidBrightness);
    }

    if (mDarkenWorldAmount > 0.0f)
    {
        const float darken = (mDarkenWorldAmount - mDarkenWorldAmountO) * partialTick + mDarkenWorldAmountO;
        mFogRed = mFogRed * (1.0f - darken) + mFogRed * 0.7f * darken;
        mFogGreen = mFogGreen * (1.0f - darken) + mFogGreen * 0.6f * darken;
        mFogBlue = mFogBlue * (1.0f - darken) + mFogBlue * 0.6f * darken;
    }

    if (living != NULL && (living->hasEffect(MobEffects::NIGHT_VISION) || living->hasEffect(MobEffects::CONDUIT_POWER)))
    {
        const float nightVision = static_cast<float>(getNightVisionScale(living, partialTick));
        float brighten = FLT_MAX;
        if (mFogRed > 0.0f && 1.0f / mFogRed < brighten)
            brighten = 1.0f / mFogRed;
        if (mFogGreen > 0.0f && 1.0f / mFogGreen < brighten)
            brighten = 1.0f / mFogGreen;
        if (mFogBlue > 0.0f && 1.0f / mFogBlue < brighten)
            brighten = 1.0f / mFogBlue;
        mFogRed = mFogRed * (1.0f - nightVision) + mFogRed * brighten * nightVision;
        mFogGreen = mFogGreen * (1.0f - nightVision) + mFogGreen * brighten * nightVision;
        mFogBlue = mFogBlue * (1.0f - nightVision) + mFogBlue * brighten * nightVision;
    }

    if (mMinecraft->mOptions->mAnaglyph3d)
    {
        const float red = (mFogRed * 30.0f + mFogGreen * 59.0f + mFogBlue * 11.0f) / 100.0f;
        const float green = (mFogRed * 30.0f + mFogGreen * 70.0f) / 100.0f;
        const float blue = (mFogRed * 30.0f + mFogBlue * 70.0f) / 100.0f;
        mFogRed = red;
        mFogGreen = green;
        mFogBlue = blue;
    }

    GlStateManager::clearColor(mFogRed, mFogGreen, mFogBlue, 1.0f);
}

FloatBuffer* GameRenderer::getBuffer(float x, float y, float z, float w)
{
    mFogBuffer->clear();
    mFogBuffer->put(x)->put(y)->put(z)->put(w);
    mFogBuffer->flip();
    return mFogBuffer;
}

void GameRenderer::resetFogColor(bool useBlack)
{
    if (useBlack)
    {
        FloatBuffer* buffer = getBuffer(0.0f, 0.0f, 0.0f, 1.0f);
        GlStateManager::glFog(5, buffer);
    }
    else
    {
        const float red = mFogRed;
        const float green = mFogGreen;
        const float blue = mFogBlue;
        FloatBuffer* buffer = getBuffer(red, green, blue, 1.0f);
        GlStateManager::glFog(5, buffer);
    }
}

void GameRenderer::setupFog(int, float partialTick)
{
    static const float kOne = 1.0f;
    static const float kFogStartScale = 0.25f;
    static const float kFogStartTightScale = 0.05f;
    static const float kFogEndCap = 192.0f;
    static const float kFogEndScale = 0.5f;
    static const float kExpFogDensity = 0.1f;
    static const float kLavaFogDensity = 2.0f;
    static const float kBlindnessFogEnd = 5.0f;
    static const float kBlindnessInterpRange = 20.0f;
    static const int kBlindnessInterpTicks = 20;

    boost::shared_ptr<Entity> camera = getCamera();
    resetFogColor(false);
    GlStateManager::color4f(kOne, kOne, kOne, kOne);

    const BlockState* blockState = Camera::getBlockAt(mMinecraft->mLevel, camera, partialTick);

    bool hasBlindness = false;
    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, camera->GetType()))
    {
        boost::shared_ptr<LivingEntity> living = boost::static_pointer_cast<LivingEntity>(camera);
        if (living->hasEffect(MobEffects::BLINDNESS))
            hasBlindness = true;
    }

    if (hasBlindness)
    {
        float fogEnd = kBlindnessFogEnd;
        const int duration =
            boost::static_pointer_cast<LivingEntity>(camera)->getEffect(MobEffects::BLINDNESS)->getDuration();
        if (duration < kBlindnessInterpTicks)
        {
            const float t = kOne - static_cast<float>(duration) / kBlindnessInterpRange;
            fogEnd = (mFogDistance - kBlindnessFogEnd) * t + kBlindnessFogEnd;
        }
        const float fogStart = fogEnd * kFogStartScale;
        GlStateManager::fogMode(1);
        GlStateManager::fogStart(fogStart);
        GlStateManager::fogEnd(fogEnd);
        gRenderer->StateSetVertexAlphaDistance(fogStart);
        GlStateManager::enableColorMaterial();
        GlStateManager::colorMaterial(0, 2);
    }
    else if (mCloudFog)
    {
        GlStateManager::fogMode(2);
        GlStateManager::fogDensity(kExpFogDensity);
        GlStateManager::enableColorMaterial();
        GlStateManager::colorMaterial(0, 2);
    }
    else
    {
        if (!blockState->getMaterial()->isUnderWater())
        {
            if (blockState->getMaterial() == Material::LAVA)
            {
                GlStateManager::fogMode(2);
                GlStateManager::fogDensity(kLavaFogDensity);
                GlStateManager::enableColorMaterial();
                GlStateManager::colorMaterial(0, 2);
            }
            else
            {
                float fogEnd = mFogDistance;
                GlStateManager::fogMode(1);
                float fogStart = fogEnd * kFogStartScale;
                GlStateManager::fogStart(fogStart);
                GlStateManager::fogEnd(fogEnd);

                bool tightenFog = mMinecraft->mLevel
                                      ->getBiome(
                                          BlockPos(static_cast<int>(camera->mX), 0, static_cast<int>(camera->mZ)))
                                      ->isFoggy();
                if (!tightenFog)
                    tightenFog =
                        mMinecraft->mGui->getBossOverlay(mMinecraft->mPlayer->mUserIndex)->shouldCreateWorldFog();
                if (tightenFog)
                {
                    fogStart = fogEnd * kFogStartTightScale;
                    GlStateManager::fogStart(fogStart);
                    const float cappedEnd = fogEnd - kFogEndCap >= 0.0f ? kFogEndCap : fogEnd;
                    GlStateManager::fogEnd(cappedEnd * kFogEndScale);
                }

                gRenderer->StateSetVertexAlphaDistance(fogStart);
                GlStateManager::enableColorMaterial();
                GlStateManager::colorMaterial(0, 2);
            }
        }
        else
        {
            GlStateManager::enableColorMaterial();
            GlStateManager::colorMaterial(0, 2);
        }
    }
}

void GameRenderer::prepareAndRenderClouds(LevelRenderer* levelRenderer, float partialTick, int eyeIndex, double x,
                                          double y, double z)
{
    const int playerIndex = gProfile->GetPrimaryPad();
    if (GameSettings::GetGameSettings(playerIndex, eGameSetting_Clouds) == 0)
        return;

    if (!Minecraft::GetInstance()->GetMiniGame()->HasFeatureClouds())
        return;

    GlStateManager::pushMatrix();
    setupFog(-1, partialTick);
    GlStateManager::enableFog();
    levelRenderer->renderClouds(partialTick, eyeIndex, x, y, z);
    GlStateManager::disableFog();
    setupFog(0, partialTick);
    GlStateManager::popMatrix();
}

static const int FULL_BRIGHT_LIGHT = 0xF000F0;
static const int COLUMN_SEED_X_SQUARED = 3121;
static const int COLUMN_SEED_X = 45238971;
static const int COLUMN_SEED_Z_SQUARED = 418711;
static const int COLUMN_SEED_Z = 13761;
static ResourceLocation sRainTexture(TN_ENVIRONMENT_RAIN);
static ResourceLocation sSnowTexture(TN_ENVIRONMENT_SNOW);
static ResourceLocation sThermalTexture(TN_ENVIRONMENT_THERMAL_1);
static ResourceLocation sThermalTintedTexture(TN_ENVIRONMENT_THERMAL_2);
static ResourceLocation sBoostArrowTexture(TN_ENVIRONMENT_BOOSTARROW);
static float sOutlineX[8];
static float sOutlineY[8];
static float sOutlineZ[8];
static float sHeadX[8];
static float sHeadY[8];
static float sHeadZ[8];
static double sEdgeLengths[8];
static const float COLUMN_ALPHAS[4] = {0.0f, 1.0f, 1.0f, 0.0f};
static const float COLUMN_BAND_HEIGHTS[3] = {0.125f, 0.5f, 0.375f};

void GameRenderer::renderThermal(Culler* culler, float partialTick)
{
    LevelRuleset* ruleset = gApp.getGameRuleDefinitions();
    if (!Minecraft::InMiniGame(eMiniGameId_Glide, true) || ruleset == NULL)
    {
        return;
    }

    FrustumCuller* frustumCuller = reinterpret_cast<FrustumCuller*>(culler);
    const double negXOffset = -frustumCuller->xOffset;
    const double negYOffset = -frustumCuller->yOffset;
    const double negZOffset = -frustumCuller->zOffset;
    float clipPlanes[24];
    for (int planeIndex = 0; planeIndex < 6; ++planeIndex)
    {
        const float* frustumPlane = frustumCuller->frustumData->frustum[planeIndex];
        clipPlanes[planeIndex * 4 + 0] = frustumPlane[0];
        clipPlanes[planeIndex * 4 + 1] = frustumPlane[1];
        clipPlanes[planeIndex * 4 + 2] = frustumPlane[2];
        clipPlanes[planeIndex * 4 + 3] = static_cast<float>(frustumPlane[3] + frustumPlane[0] * negXOffset
                                                            + frustumPlane[1] * negYOffset + frustumPlane[2] * negZOffset);
    }

    const ResourceLocation* boundTexture = &sThermalTexture;
    mMinecraft->mTextures->bindTexture(&sThermalTexture);
    GlStateManager::disableFog();
    GlStateManager::disableLighting();
    GlStateManager::disableCull();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::depthMask(false);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    boost::shared_ptr<Entity> camera = getCamera();
    builder->offset(-static_cast<float>((camera->mX - camera->mXOld) * partialTick + camera->mXOld),
                    -static_cast<float>((camera->mY - camera->mYOld) * partialTick + camera->mYOld),
                    -static_cast<float>((camera->mZ - camera->mZOld) * partialTick + camera->mZOld));

    std::vector<ThermalAreaRuleDefinition*> thermalAreas;
    ruleset->getThermalAreas(&thermalAreas);

    for (std::vector<ThermalAreaRuleDefinition*>::iterator it = thermalAreas.begin(); it != thermalAreas.end(); ++it)
    {
        ThermalAreaRuleDefinition* rule = *it;
        if (!rule->isEnabled() || !rule->getConditionsMet(mMinecraft->mPlayer))
        {
            continue;
        }

        AABB* area = rule->getArea();
        if (!clip(area, clipPlanes))
        {
            continue;
        }

        if (rule->getElytraSpeedBoost() > 0.0)
        {
            const float width = static_cast<float>((area->mMaxX - area->mMinX) - 0.0625);
            const float halfHeight = static_cast<float>((area->mMaxY - area->mMinY) - 0.0625) * 0.5f;
            const float depth = static_cast<float>((area->mMaxZ - area->mMinZ) - 0.0625);
            const float halfDepth = depth * 0.5f;
            const float halfWidth = width * 0.5f;
            const float centerX = static_cast<float>(area->getCenter()->x);
            const float centerY = static_cast<float>(area->getCenter()->y);
            const float centerZ = static_cast<float>(area->getCenter()->z);
            const float arrowAlphas[4] = {0.0f, 1.0f, 0.5f, 0.0f};
            const unsigned int boostDirection = rule->getBoostDirection();
            const bool alongX = boostDirection <= 1 || (boostDirection >= 4 && boostDirection <= 5);
            const bool alongZ = (boostDirection >= 2 && boostDirection <= 3)
                                || (boostDirection >= 6 && boostDirection <= 7);

            if (alongX || alongZ)
            {
                sOutlineY[0] = centerY - halfHeight;
                sOutlineY[1] = centerY - halfHeight;
                sOutlineY[2] = centerY + halfHeight;
                sOutlineY[3] = centerY + halfHeight;
                sHeadY[0] = centerY - halfHeight;
                sHeadY[1] = centerY - halfHeight;
                sHeadY[2] = halfHeight * 1.5f + centerY;
                sHeadY[3] = halfHeight * 1.5f + centerY;
            }
            if (alongX)
            {
                sOutlineZ[0] = centerZ - halfDepth;
                sOutlineZ[1] = centerZ + halfDepth;
                sOutlineZ[2] = centerZ + halfDepth;
                sOutlineZ[3] = centerZ - halfDepth;
                sHeadZ[0] = centerZ - halfDepth * 1.5f;
                sHeadZ[1] = centerZ + halfDepth * 1.5f;
                sHeadZ[2] = centerZ + halfDepth * 1.5f;
                sHeadZ[3] = centerZ - halfDepth * 1.5f;
            }
            else if (alongZ)
            {
                sOutlineX[0] = centerX - halfWidth;
                sOutlineX[1] = centerX + halfWidth;
                sOutlineX[2] = centerX + halfWidth;
                sOutlineX[3] = centerX - halfWidth;
                sHeadX[0] = centerX - halfWidth * 1.5f;
                sHeadX[1] = centerX + halfWidth * 1.5f;
                sHeadX[2] = centerX + halfWidth * 1.5f;
                sHeadX[3] = centerX - halfWidth * 1.5f;
            }

            double perimeter = 0.0;
            for (int side = 0; side < 4; ++side)
            {
                unsigned int next = side + 1;
                if (next > 3)
                    next = 0;
                if (boostDirection < 2 || (boostDirection >= 6 && boostDirection < 8))
                {
                    const double dy = sOutlineY[next] - sOutlineY[side];
                    const double dz = sOutlineZ[next] - sOutlineZ[side];
                    sEdgeLengths[side] = sqrt(dy * dy + dz * dz);
                }
                else if (boostDirection >= 2 && boostDirection < 6)
                {
                    const double dx = sOutlineX[next] - sOutlineX[side];
                    const double dy = sOutlineY[next] - sOutlineY[side];
                    sEdgeLengths[side] = sqrt(dx * dx + dy * dy);
                }
                perimeter += sEdgeLengths[side];
            }
            Math::round(perimeter * 0.25);

            const int seedX = static_cast<int>(area->mMinX + 0.03125);
            const int seedZ = static_cast<int>(area->mMinZ + 0.03125);
            mRandom.setSeed(static_cast<long long>((seedX * seedX * COLUMN_SEED_X_SQUARED + seedX * COLUMN_SEED_X)
                                                   ^ (seedZ * seedZ * COLUMN_SEED_Z_SQUARED + seedZ * COLUMN_SEED_Z)));
            const float time = static_cast<float>(static_cast<int>(mTickCount)) + partialTick;
            const double scroll = mRandom.nextDouble() + time * 0.033333335f;
            builder->begin();

            ColourTable* colourTable = Minecraft::GetInstance()->getColourTable();
            int colour;
            if (boostDirection >= 4 && boostDirection <= 7)
            {
                colour = colourTable->getColour(eMinecraftColour_Thermal_Speedboost2);
            }
            else
            {
                colour = colourTable->getColour(eMinecraftColour_Thermal_Speedboost);
            }
            if (boundTexture != &sBoostArrowTexture)
            {
                boundTexture = &sBoostArrowTexture;
                mMinecraft->mTextures->bindTexture(&sBoostArrowTexture);
            }
            const float red = static_cast<float>((colour >> 16) & 0xFF) / 255.0f;
            const float green = static_cast<float>((colour >> 8) & 0xFF) / 255.0f;
            const float blue = static_cast<float>(colour & 0xFF) / 255.0f;

            const int lightColor = mMinecraft->mLevel->getLightColor(BlockPos(area->mMinX, area->mMinY, area->mMinZ), 0,
                                                                     -1);
            const int light = (lightColor * 3 + FULL_BRIGHT_LIGHT) / 4;
            const bool forwards = boostDirection == 1 || boostDirection == 3 || boostDirection == 5
                                  || boostDirection == 7;
            const float scrollStep = forwards ? 1.0f : -1.0f;
            const bool headAtStart = boostDirection == 0 || boostDirection == 2
                                     || (boostDirection >= 4 && boostDirection <= 7);
            const bool headAtEnd = boostDirection == 1 || boostDirection == 3
                                   || (boostDirection >= 4 && boostDirection <= 7);

            const float length = (alongX ? width : depth) + 8.0f;
            if (alongX || alongZ)
            {
                for (int side = 0; side < 4; ++side)
                {
                    unsigned int next = side + 1;
                    if (next > 3)
                        next = 0;
                    float start = static_cast<float>((alongX ? area->mMinX : area->mMinZ) - 4.0);
                    float v0 = static_cast<float>(scroll);
                    for (int segment = 0; segment < 3; ++segment)
                    {
                        const float end = start + (segment == 1 ? length - 8.0f : 4.0f);
                        const float v1 = v0 + scrollStep;
                        const int alphaStart = forwards ? 3 - segment : segment;
                        const int alphaEnd = forwards ? 3 - (segment + 1) : segment + 1;
                        const bool headStart = headAtStart && segment == 0;
                        const bool headEnd = headAtEnd && segment == 2;
                        if (alongX)
                        {
                            float endY0 = sOutlineY[side];
                            float endZ0 = sOutlineZ[side];
                            float endY1 = sOutlineY[next];
                            float endZ1 = sOutlineZ[next];
                            if (headStart)
                            {
                                builder->blockRainQuad(start, sHeadY[side], sHeadZ[side], 0.0f, v0, start, sHeadY[next],
                                                       sHeadZ[next], 1.0f, v0, end, endY1, endZ1, 1.0f, v1, end, endY0,
                                                       endZ0, 0.0f, v1, red, green, blue, arrowAlphas[alphaStart], red,
                                                       green, blue, arrowAlphas[alphaEnd], light);
                            }
                            else
                            {
                                if (headEnd)
                                {
                                    endY0 = sHeadY[side];
                                    endZ0 = sHeadZ[side];
                                    endY1 = sHeadY[next];
                                    endZ1 = sHeadZ[next];
                                }
                                builder->blockRainQuad(start, sOutlineY[side], sOutlineZ[side], 0.0f, v0, start,
                                                       sOutlineY[next], sOutlineZ[next], 1.0f, v0, end, endY1, endZ1,
                                                       1.0f, v1, end, endY0, endZ0, 0.0f, v1, red, green, blue,
                                                       arrowAlphas[alphaStart], red, green, blue, arrowAlphas[alphaEnd],
                                                       light);
                            }
                        }
                        else
                        {
                            float endX0 = sOutlineX[side];
                            float endY0 = sOutlineY[side];
                            float endX1 = sOutlineX[next];
                            float endY1 = sOutlineY[next];
                            if (headStart)
                            {
                                builder->blockRainQuad(
                                    sHeadX[side], sHeadY[side], start, 0.0f, v0, sHeadX[next], sHeadY[next], start,
                                    1.0f, v0, endX1, endY1, end, 1.0f, v1, endX0, endY0, end, 0.0f, v1, red, green,
                                    blue, arrowAlphas[alphaStart], red, green, blue, arrowAlphas[alphaEnd], light);
                            }
                            else
                            {
                                if (headEnd)
                                {
                                    endX0 = sHeadX[side];
                                    endY0 = sHeadY[side];
                                    endX1 = sHeadX[next];
                                    endY1 = sHeadY[next];
                                }
                                builder->blockRainQuad(sOutlineX[side], sOutlineY[side], start, 0.0f, v0,
                                                       sOutlineX[next], sOutlineY[next], start, 1.0f, v0, endX1, endY1,
                                                       end, 1.0f, v1, endX0, endY0, end, 0.0f, v1, red, green, blue,
                                                       arrowAlphas[alphaStart], red, green, blue, arrowAlphas[alphaEnd],
                                                       light);
                            }
                        }
                        start = end;
                        v0 = v1;
                    }
                }
            }
            builder->end();
        }
        else if (rule->getLiftSpeed() < 0.0)
        {
            continue;
        }
        else
        {
            const float radiusX = static_cast<float>(static_cast<int>(area->mMaxX - area->mMinX)) * 0.5f;
            const float radiusZ = static_cast<float>(static_cast<int>(area->mMaxZ - area->mMinZ)) * 0.5f;
            const float centerX = static_cast<float>(area->mMinX + radiusX);
            const float centerZ = static_cast<float>(area->mMinZ + radiusZ);
            for (unsigned int corner = 0; corner < 8; ++corner)
            {
                const float angle = Mth::PI * 2.0f * static_cast<float>(corner) * 0.125f;
                const float cosine = cosf(angle);
                const float sine = sinf(angle);
                sOutlineX[corner] = radiusX * cosine + centerX;
                sOutlineZ[corner] = radiusZ * sine + centerZ;
            }

            double perimeter = 0.0;
            for (int corner = 0; corner < 8; ++corner)
            {
                unsigned int next = corner + 1;
                if (next > 7)
                    next = 0;
                const double dx = sOutlineX[next] - sOutlineX[corner];
                const double dz = sOutlineZ[next] - sOutlineZ[corner];
                sEdgeLengths[corner] = sqrt(dx * dx + dz * dz);
                perimeter += sEdgeLengths[corner];
            }
            const double segments = static_cast<double>(Math::round(perimeter / 3.059999942779541));
            const int seedX = static_cast<int>(area->mMinX);
            const int seedZ = static_cast<int>(area->mMinZ);
            mRandom.setSeed(static_cast<long long>((seedX * seedX * COLUMN_SEED_X_SQUARED + seedX * COLUMN_SEED_X)
                                                   ^ (seedZ * seedZ * COLUMN_SEED_Z_SQUARED + seedZ * COLUMN_SEED_Z)));
            const float segmentLength = static_cast<float>(perimeter / segments);
            const float time = static_cast<float>(static_cast<int>(mTickCount)) + partialTick;
            double u = mRandom.nextDouble() + time * 0.011111111f;
            const double vStart = -(mRandom.nextDouble() + time * 0.0055555557f);
            builder->begin();

            int colour = 0;
            const ResourceLocation* texture = &sThermalTexture;
            if (rule->getLiftForceModifier() > 1.0)
            {
                colour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Thermal_Lift);
            }
            else if (rule->getMaxHeight() > 0.0)
            {
                colour = Minecraft::GetInstance()->getColourTable()->getColour(
                    eMinecraftColour_Thermal_ConstantToHeight);
                texture = &sThermalTintedTexture;
            }
            else if (rule->getLiftSpeed() != 0.0)
            {
                colour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Thermal_Constant);
                texture = &sThermalTintedTexture;
            }
            if (texture != boundTexture)
            {
                boundTexture = texture;
                mMinecraft->mTextures->bindTexture(texture);
            }
            const float red = static_cast<float>((colour >> 16) & 0xFF) / 255.0f;
            const float green = static_cast<float>((colour >> 8) & 0xFF) / 255.0f;
            const float blue = static_cast<float>(colour & 0xFF) / 255.0f;

            const int lightColor = mMinecraft->mLevel->getLightColor(BlockPos(area->mMinX, area->mMinY, area->mMinZ), 0,
                                                                     -1);
            const int light = (lightColor * 3 + FULL_BRIGHT_LIGHT) / 4;
            const double vSpan = ((area->mMaxY - area->mMinY) * 0.0625 + vStart) - vStart;
            const float height = static_cast<float>(static_cast<int>(area->mMaxY - area->mMinY));

            for (int corner = 0; corner < 8; ++corner)
            {
                unsigned int next = corner + 1;
                if (next > 7)
                    next = 0;
                const double nextU = u + sEdgeLengths[corner] / segmentLength;
                const float u0 = static_cast<float>(u);
                const float u1 = static_cast<float>(nextU);
                float y0 = static_cast<float>(area->mMinY);
                float v0 = static_cast<float>(vStart);
                for (int band = 0; band < 3; ++band)
                {
                    const float y1 = height * COLUMN_BAND_HEIGHTS[band] + y0;
                    const float v1 = v0 + static_cast<float>(vSpan * COLUMN_BAND_HEIGHTS[band]);
                    builder->blockRainQuad(sOutlineX[corner], y0, sOutlineZ[corner], u0, v0, sOutlineX[next], y0,
                                           sOutlineZ[next], u1, v0, sOutlineX[next], y1, sOutlineZ[next], u1, v1,
                                           sOutlineX[corner], y1, sOutlineZ[corner], u0, v1, red, green, blue,
                                           COLUMN_ALPHAS[band], red, green, blue, COLUMN_ALPHAS[band + 1], light);
                    y0 = y1;
                    v0 = v1;
                }
                u = nextU;
            }
            builder->end();
        }
    }

    builder->offset(0.0f, 0.0f, 0.0f);
    GlStateManager::disableLighting();
    GlStateManager::enableTexture();
    GlStateManager::depthMask(true);
    GlStateManager::disableBlend();
    GlStateManager::enableFog();
}

void GameRenderer::renderGhost(float partialTick, boost::shared_ptr<Entity> entity)
{
    static const float kZero = 0.0f;
    static const float kOne = 1.0f;
    static const float kHalf = 0.5f;
    static const float kGhostScale = 0.9375f;
    static const float kModelScale = 0.0625f;
    static const float kDepthOffset = 0.125f;
    static const double kNeg180 = -180.0;
    static const double kNeg90 = -90.0;
    static const float kNegOne = -1.0f;
    static const float kPowBase = 1.5f;
    static const float kColourDivisor = 255.0f;
    static const float kPolygonOffset = -0.3f;

    const int startColour = mMinecraft->getColourTable()->getColour(eMinecraftColour_Glide_Ghost_Slow);
    const int middleColour = mMinecraft->getColourTable()->getColour(eMinecraftColour_Glide_Ghost_Medium);
    const int endColour = mMinecraft->getColourTable()->getColour(eMinecraftColour_Glide_Ghost_Fast);

    GhostController* ghostController = mMinecraft->GetGhostController();
    LevelGenerationOptions* generationOptions = gApp.getLevelGenerationOptions();
    if (ghostController != NULL && generationOptions != NULL)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> player = mMinecraft->mPlayer;

        GhostController::GhostRecord* record = ghostController->mLoaded[player->mUserIndex];
        if (record != NULL && record->HasRecording() && !ghostController->HasFinished(record))
        {
            if (ghostController->mPlaybackActive[player->mUserIndex])
            {
                float playbackFraction = kOne;
                unsigned int sampleIndex = ghostController->getPlaybackIndex(record, playbackFraction);

                Vec3* samplePos = record->GetPosAtIndex(sampleIndex);
                Vec3* renderRot = record->GetRotAtIndex(sampleIndex);
                float sampleVelocity = record->GetVelocityAtIndex(sampleIndex);

                Vec3* renderPos = samplePos;
                float flyAmount = kOne;
                if (sampleIndex + 1U < static_cast<unsigned int>(record->NumSamples()))
                {
                    Vec3* nextRot = record->GetRotAtIndex(sampleIndex + 1U);
                    const float nextVelocity = record->GetVelocityAtIndex(sampleIndex + 1U);
                    Vec3* nextPos = record->GetPosAtIndex(sampleIndex + 1U);

                    renderPos = samplePos->interpolateTo(nextPos, static_cast<double>(playbackFraction));
                    if (renderPos->y < samplePos->y)
                    {
                        Vec3* direction = Vec3::newTemp(renderPos->x - samplePos->x, renderPos->y - samplePos->y,
                                                        renderPos->z - samplePos->z)
                                              ->normalize();
                        flyAmount = kOne - static_cast<float>(std::pow(-direction->y, static_cast<double>(kPowBase)));
                    }
                    renderRot = renderRot->interpolateTo(nextRot, static_cast<double>(playbackFraction));
                    sampleVelocity = Mth::Lerp(sampleVelocity, nextVelocity, playbackFraction);
                }

                const double cameraX = (player->mX - player->mXOld) * partialTick + player->mXOld;
                const double cameraY = (player->mY - player->mYOld) * partialTick + player->mYOld;
                const double cameraZ = (player->mZ - player->mZOld) * partialTick + player->mZOld;

                const float velocityAlpha = Mth::clamp(sampleVelocity * kDepthOffset, kZero, kOne);
                const unsigned int blendedColour = ColourTable::LinearBlend(startColour, middleColour, endColour,
                                                                            velocityAlpha);

                const float distanceAlpha = Mth::clamp(
                    static_cast<float>(Vec3::newTemp(player->mX, player->mY, player->mZ)->distanceTo(renderPos))
                        * kHalf,
                    kZero, kOne);

                PlayerRenderer* playerRenderer = mMinecraft->getEntityRenderDispatcher()->getPlayerRenderer(
                    EPLAYER_MODEL_SLIM);
                if (playerRenderer != NULL)
                {
                    HumanoidModel* humanoidModel = static_cast<HumanoidModel*>(playerRenderer->getModel());
                    GlStateManager::pushMatrix();
                    GlStateManager::translatef(-static_cast<float>(cameraX), -static_cast<float>(cameraY),
                                               -static_cast<float>(cameraZ));
                    GlStateManager::translatef(static_cast<float>(renderPos->x), static_cast<float>(renderPos->y),
                                               static_cast<float>(renderPos->z));
                    GlStateManager::rotatef(static_cast<float>(kNeg180 - renderRot->y), kZero, kOne, kZero);
                    GlStateManager::rotatef(static_cast<float>(kNeg90 - renderRot->x), kOne, kZero, kZero);
                    GlStateManager::rotatef(static_cast<float>(renderRot->z), kZero, kZero, kOne);
                    GlStateManager::enableRescaleNormal();
                    GlStateManager::scalef(kNegOne, kNegOne, kOne);
                    GlStateManager::scalef(kGhostScale, kGhostScale, kGhostScale);

                    humanoidModel->setAllVisible(true);
                    ElytraModel* elytraModel = playerRenderer->getElytraModel();
                    ResourceLocation ghostBodyTexture(TN_MOB_GLIDE_GHOST);
                    ResourceLocation ghostElytraTexture(TN_ITEM_ELYTRA);
                    GlStateManager::translatef(kZero, kZero, kDepthOffset);
                    humanoidModel->overrideAnim();
                    humanoidModel->clearAllOffsets();
                    elytraModel->overrideAnim(flyAmount, static_cast<float>(renderRot->y),
                                              static_cast<float>(renderRot->x), kModelScale);

                    const float red = static_cast<float>(static_cast<int>((blendedColour >> 16) & 0xFF))
                                      / kColourDivisor;
                    const float green = static_cast<float>(static_cast<int>((blendedColour >> 8) & 0xFF))
                                        / kColourDivisor;
                    const float blue = static_cast<float>(static_cast<int>(blendedColour & 0xFF)) / kColourDivisor;
                    const float alpha = kHalf * distanceAlpha;
                    GlStateManager::color4f(red, green, blue, alpha);

                    Lighting::turnOn();
                    GlStateManager::cullFace(1);
                    gRenderer->StateSetWriteEnable(false, false, false, false);
                    Minecraft::GetInstance()->mTextures->bindTexture(&ghostBodyTexture);
                    humanoidModel->overrideRender(kModelScale, true);
                    Minecraft::GetInstance()->mTextures->bindTexture(&ghostElytraTexture);
                    boost::shared_ptr<Entity> noEntity;
                    elytraModel->render(noEntity, kZero, kZero, kZero, static_cast<float>(renderRot->y),
                                        static_cast<float>(renderRot->x), kModelScale, true);

                    gRenderer->StateSetWriteEnable(true, true, true, true);
                    GlStateManager::color4f(red, green, blue, alpha);
                    GlStateManager::enableBlend();
                    GlStateManager::blendFuncSeparate(4, 5, 1, 0);
                    GlStateManager::polygonOffset(kPolygonOffset, kPolygonOffset);

                    Minecraft::GetInstance()->mTextures->bindTexture(&ghostBodyTexture);
                    humanoidModel->overrideRender(kModelScale, true);
                    Minecraft::GetInstance()->mTextures->bindTexture(&ghostElytraTexture);
                    elytraModel->render(noEntity, kZero, kZero, kZero, static_cast<float>(renderRot->y),
                                        static_cast<float>(renderRot->x), kModelScale, true);

                    Lighting::turnOff();
                    GlStateManager::polygonOffset(kZero, kZero);
                    GlStateManager::disableBlend();
                    GlStateManager::popMatrix();
                }
            }
        }
    }
}

void GameRenderer::renderGhostPath(float partialTick, boost::shared_ptr<Entity> entity)
{
    const int startColour = mMinecraft->getColourTable()->getColour(eMinecraftColour_Glide_Ghost_Slow);
    const int middleColour = mMinecraft->getColourTable()->getColour(eMinecraftColour_Glide_Ghost_Medium);
    const int endColour = mMinecraft->getColourTable()->getColour(eMinecraftColour_Glide_Ghost_Fast);

    GhostController* ghostController = mMinecraft->GetGhostController();
    if (ghostController == NULL)
        return;

    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = mMinecraft->mPlayer;
    GhostController::GhostRecord* record = ghostController->mLoaded[localPlayer->mUserIndex];
    if (record == NULL || !record->HasRecording() || !ghostController->mPlaybackActive[localPlayer->mUserIndex])
        return;

    std::vector<PositionStruct> positions;
    std::vector<int> colours;
    std::vector<int> alphas;
    positions.reserve(record->NumSamples());
    colours.reserve(record->NumSamples());
    alphas.reserve(record->NumSamples());

    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
    double cameraX = (player->mX - player->mXOld) * partialTick + player->mXOld;
    double cameraY = (player->mY - player->mYOld) * partialTick + player->mYOld;
    double cameraZ = (player->mZ - player->mZOld) * partialTick + player->mZOld;
    GlStateManager::pushMatrix();
    GlStateManager::translatef(-(float)cameraX, -(float)cameraY, -(float)cameraZ);

    for (unsigned int i = 0; i < record->NumSamples(); ++i)
    {
        float velocity = Mth::clamp((float)(record->GetVelocityAtIndex(i) * 0.125f), 0.0f, 1.0f);
        int colour = ColourTable::LinearBlend(startColour, middleColour, endColour, velocity);
        Vec3* position = record->GetPosAtIndex(i);
        double distance = Mth::clamp(position->distanceTo(player->getPos(1.0f)), 0.0, 10.0);
        int alpha = (int)(255.0 * distance * 0.04);
        if (i != 0 && i + 1 < record->NumSamples())
        {
            const bool farFromPrevious = record->GetPosAtIndex(i - 1)->distanceTo(position) > 20.0;
            const bool farFromNext = position->distanceTo(record->GetPosAtIndex(i + 1)) > 20.0;
            if (farFromPrevious || farFromNext)
                alpha = 0;
        }
        alphas.push_back(alpha);
        colours.push_back(colour);
        PositionStruct linePosition;
        linePosition.x = (float)position->x;
        linePosition.y = (float)position->y;
        linePosition.z = (float)position->z;
        positions.push_back(linePosition);

        if (positions.size() == 1000)
        {
            LevelRenderer::renderLine(&positions, &colours, &alphas, 0.5f);
            colours.clear();
            alphas.clear();
            positions.clear();
            --i;
        }
    }

    if (positions.size() > 1)
        LevelRenderer::renderLine(&positions, &colours, &alphas, 0.5f);
    GlStateManager::popMatrix();
}

void GameRenderer::renderSnowAndRain(float partialTick)
{
    Level* level = mMinecraft->mLevel;
    float rainLevel = level->getRainLevel(partialTick);

    bool tumbleWeather = false;
    int columnStep = 1;
    if (Minecraft::InMiniGame(eMiniGameId_Tumble, false) && SharedConstants::IS_CHRISTMAS)
    {
        tumbleWeather = true;
        columnStep = 2;
        rainLevel = 0.55f;
    }

    if (!tumbleWeather && rainLevel <= 0.0f)
    {
        return;
    }

    gRenderer->StateSetEnableViewportClipPlanes(true);

    turnOnLightLayer(partialTick);
    boost::shared_ptr<Entity> camera = getCamera();

    const int cameraBlockX = Mth::floor(camera->mX);
    const int cameraBlockY = Mth::floor(camera->mY);
    const int cameraBlockZ = Mth::floor(camera->mZ);

    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();

    GlStateManager::disableCull();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    const float alphaRef = 0.01f;
    GlStateManager::alphaFunc(4, alphaRef);

    MemSect(0x1f);
    mMinecraft->mTextures->bindTexture(&sSnowTexture);
    MemSect(0);

    const double cameraOldX = camera->mXOld;
    const double cameraOldY = camera->mYOld;
    const double cameraOldZ = camera->mZOld;
    const float cameraX = static_cast<float>((camera->mX - cameraOldX) * partialTick + cameraOldX);
    const double interpolatedY = (camera->mY - cameraOldY) * partialTick + cameraOldY;
    const float cameraY = static_cast<float>(interpolatedY);
    const float cameraZ = static_cast<float>((camera->mZ - cameraOldZ) * partialTick + cameraOldZ);
    const int interpolatedBlockY = Mth::floor(interpolatedY);

    const int rainTime = static_cast<int>(mTickCount);

    const float negCameraX = -cameraX;
    const float negCameraY = -cameraY;
    const float negCameraZ = -cameraZ;
    builder->offset(negCameraX, negCameraY, negCameraZ);

    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);

    unsigned char activePlayerCount = mMinecraft->mLevelRenderer->activePlayers();
    int renderRadius = 9;
    if (activePlayerCount >= 2 && activePlayerCount <= 4)
    {
        static const unsigned char sRadiusByPlayerCount[5] = {0, 0, 7, 5, 5};
        renderRadius = sRadiusByPlayerCount[activePlayerCount];
    }

    int precipitationMode = -1;
    const int startBlockX = cameraBlockX - renderRadius;
    const int startBlockZ = cameraBlockZ - renderRadius;
    int xSeedAccumulator = startBlockX * COLUMN_SEED_X;

    for (int blockX = startBlockX; blockX <= cameraBlockX + renderRadius; blockX += columnStep)
    {
        const int xInnerSeed = blockX * blockX * COLUMN_SEED_X_SQUARED;
        int zSeedAccumulator = startBlockZ * COLUMN_SEED_Z;

        for (int blockZ = startBlockZ; blockZ <= cameraBlockZ + renderRadius; blockZ += columnStep)
        {
            const int offsetIndex = (blockZ - cameraBlockZ) * 32 + (blockX - cameraBlockX) + 528;
            const float offsetX = mRainSizeX[offsetIndex] * 0.5f;
            const float offsetZ = mRainSizeZ[offsetIndex] * 0.5f;

            BlockPos samplePos(blockX, 0, blockZ);
            Biome* biome = level->getBiome(samplePos);
            if (!tumbleWeather && !biome->hasRain() && !biome->hasSnow())
            {
                zSeedAccumulator += columnStep * COLUMN_SEED_Z;
                continue;
            }

            BlockPos topRainPos = level->getTopRainBlockPos(samplePos);
            const int topY = topRainPos.y;
            int minY = cameraBlockY - renderRadius;
            if (minY < topY)
            {
                minY = topY;
            }

            int maxY = cameraBlockY + renderRadius;
            if (maxY < topY)
            {
                maxY = topY;
            }

            const int renderY = topY < interpolatedBlockY ? interpolatedBlockY : topY;
            if (minY == maxY)
            {
                zSeedAccumulator += columnStep * COLUMN_SEED_Z;
                continue;
            }

            const int zInnerSeed = blockZ * blockZ * COLUMN_SEED_Z_SQUARED;
            mRandom.setSeed(static_cast<long long>((xInnerSeed + xSeedAccumulator) ^ (zInnerSeed + zSeedAccumulator)));

            const float temperature = biome->getTemperature(BlockPos(blockX, minY, blockZ));
            if (!tumbleWeather)
            {
                const float scaledTemp = level->getBiomeSource()->scaleTemp(temperature, topY);
                if (scaledTemp >= 0.15f)
                {
                    if (precipitationMode < 0)
                    {
                        precipitationMode = 0;
                        mMinecraft->mTextures->bindTexture(&sRainTexture);
                        builder->begin();
                        builder->offset(negCameraX, negCameraY, negCameraZ);
                    }
                    else if (precipitationMode > 0)
                    {
                        builder->end();
                        precipitationMode = 0;
                        mMinecraft->mTextures->bindTexture(&sRainTexture);
                        builder->begin();
                        builder->offset(negCameraX, negCameraY, negCameraZ);
                    }

                    const int dropSeed = (rainTime + xInnerSeed + xSeedAccumulator + zInnerSeed + zSeedAccumulator)
                                         & 31;
                    const double dropPhase = (static_cast<double>(dropSeed) + static_cast<double>(partialTick))
                                             * 0.03125 * (mRandom.nextDouble() + 3.0);

                    const double dx = static_cast<double>(blockX + 0.5f) - camera->mX;
                    const double dz = static_cast<double>(blockZ + 0.5f) - camera->mZ;
                    const float dist = Mth::sqrt(dx * dx + dz * dz) / static_cast<float>(renderRadius);
                    const float alpha = ((1.0f - dist * dist) * 0.5f + 0.5f) * rainLevel * rainLevel;

                    BlockPos lightPos(blockX, renderY, blockZ);
                    builder->tex2(level->getLightColor(lightPos, 0, -1));
                    builder->color(1.0f, 1.0f, 1.0f, alpha);

                    const float yMin = static_cast<float>(minY);
                    const float yMax = static_cast<float>(maxY);
                    const float minYPhaseBase = static_cast<float>(static_cast<double>(static_cast<float>(minY))
                                                                   * 0.25);
                    const float maxYPhaseBase = static_cast<float>(static_cast<double>(static_cast<float>(maxY))
                                                                   * 0.25);
                    const float vTop = static_cast<float>(static_cast<double>(minYPhaseBase) + dropPhase);
                    const float vBottom = static_cast<float>(static_cast<double>(maxYPhaseBase) + dropPhase);
                    const float xLeft = blockX - offsetX + 0.5;
                    const float xRight = blockX + offsetX + 0.5;
                    const float zFront = blockZ - offsetZ + 0.5;
                    const float zBack = blockZ + offsetZ + 0.5;

                    builder->vertexUV(xLeft, yMin, zFront, 0.0f, vTop);
                    builder->vertexUV(xRight, yMin, zBack, 1.0f, vTop);
                    builder->color(1.0f, 1.0f, 1.0f, 0.0f);
                    builder->vertexUV(xRight, yMax, zBack, 1.0f, vBottom);
                    builder->vertexUV(xLeft, yMax, zFront, 0.0f, vBottom);
                    continue;
                }
            }

            if (precipitationMode != 1)
            {
                if (precipitationMode >= 0)
                {
                    builder->end();
                }
                precipitationMode = 1;
                mMinecraft->mTextures->bindTexture(&sSnowTexture);
                builder->begin();
            }

            const float rainTickPhase = static_cast<float>(static_cast<double>(static_cast<float>(rainTime))
                                                           + static_cast<double>(partialTick));
            const float rainScroll = (static_cast<float>(rainTime & 0x1ff) + partialTick) * 0.001953125f;
            const float rainGaussianScale = rainTickPhase * 0.01f;
            const double rainRandomA = mRandom.nextDouble();
            const double rainGaussianA = mRandom.nextGaussian();
            const double rainRandomB = mRandom.nextDouble();
            const double rainGaussianB = mRandom.nextGaussian();
            const double rainOffsetA = rainRandomA
                                       + static_cast<double>(rainGaussianScale * static_cast<float>(rainGaussianA));
            const double rainOffsetB =
                rainRandomB + static_cast<double>(rainTickPhase * static_cast<float>(rainGaussianB) * 0.001f);

            const double dx = static_cast<double>(blockX + 0.5f) - camera->mX;
            const double dz = static_cast<double>(blockZ + 0.5f) - camera->mZ;
            const float dist = static_cast<float>(std::sqrt(dx * dx + dz * dz)) / static_cast<float>(renderRadius);
            builder->offset(negCameraX, negCameraY, negCameraZ);

            BlockPos lightPos(blockX, renderY, blockZ);
            builder->tex2((level->getLightColor(lightPos, 0, -1) * 3 + FULL_BRIGHT_LIGHT) / 4);
            const float alpha = ((1.0f - dist * dist) * 0.3f + 0.5f) * rainLevel * rainLevel;
            builder->color(1.0f, 1.0f, 1.0f, alpha);

            const float xLeft = blockX - offsetX + 0.5;
            const float xRight = blockX + offsetX + 0.5;
            const float yMin = static_cast<float>(minY);
            const float yMax = static_cast<float>(maxY);
            const float zFront = blockZ - offsetZ + 0.5;
            const float zBack = blockZ + offsetZ + 0.5;
            const float minYPhaseBase = static_cast<float>(static_cast<double>(static_cast<float>(minY)) * 0.25);
            const float maxYPhaseBase = static_cast<float>(static_cast<double>(static_cast<float>(maxY)) * 0.25);
            const float vTop = static_cast<float>(static_cast<double>(minYPhaseBase) + static_cast<double>(rainScroll)
                                                  + rainOffsetB);
            const float vBottom = static_cast<float>(static_cast<double>(maxYPhaseBase)
                                                     + static_cast<double>(rainScroll) + rainOffsetB);

            builder->vertexUV(xLeft, yMin, zFront, static_cast<float>(rainOffsetA), vTop);
            builder->vertexUV(xRight, yMin, zBack, static_cast<float>(1.0 + rainOffsetA), vTop);
            builder->vertexUV(xRight, yMax, zBack, static_cast<float>(1.0 + rainOffsetA), vBottom);
            builder->vertexUV(xLeft, yMax, zFront, static_cast<float>(rainOffsetA), vBottom);
            builder->offset(0.0f, 0.0f, 0.0f);

            zSeedAccumulator += columnStep * COLUMN_SEED_Z;
        }

        xSeedAccumulator += columnStep * COLUMN_SEED_X;
    }

    if (precipitationMode >= 0)
    {
        builder->end();
    }

    builder->offset(0.0f, 0.0f, 0.0f);
    GlStateManager::enableCull();
    GlStateManager::disableBlend();
    GlStateManager::alphaFunc(4, alphaRef);
    turnOffLightLayer(partialTick);

    gRenderer->StateSetEnableViewportClipPlanes(false);
}

void GameRenderer::renderEyePass(int eyeIndex, float partialTick, long long, bool sortTransparent)
{
    LevelRenderer* levelRenderer = mMinecraft->mLevelRenderer;
    gHangingEntityRenderCount = 0;
    ParticleEngine* particleEngine = mMinecraft->mParticleEngine;
    GlStateManager::enableCull();
    Camera::UpdatePlayerOffsets(mMinecraft->mPlayer);

    setupClearColor(partialTick);
    GlStateManager::clear(3);
    setupCamera(partialTick, eyeIndex);

    Camera::prepare(mMinecraft->mPlayer, getCameraThirdPerson() == 2);

    Frustum::getFrustum();
    setupFog(-1, partialTick);
    levelRenderer->renderSky(partialTick);

    if (mMinecraft->mTexturePackRepository->getSelected()->getId() == 0x402)
    {
        levelRenderer->renderHaloRing(partialTick);
    }

    GlStateManager::enableFog();
    setupFog(0, partialTick);

    MemSect(0x1f);
    FrustumCuller frustumCuller(NULL);
    MemSect(0);

    boost::shared_ptr<Entity> cameraEntity = mMinecraft->getCameraEntity();
    const double cameraX = (cameraEntity->mX - cameraEntity->mXOld) * partialTick + cameraEntity->mXOld;
    const double cameraY = (cameraEntity->mY - cameraEntity->mYOld) * partialTick + cameraEntity->mYOld;
    const double cameraZ = (cameraEntity->mZ - cameraEntity->mZOld) * partialTick + cameraEntity->mZOld;
    Culler* culler = &frustumCuller;
    culler->prepare(cameraX, cameraY, cameraZ);
    mMinecraft->mLevelRenderer->cull(culler, partialTick);

    if (cameraEntity->mY + cameraEntity->getEyeHeight() < static_cast<double>(mMinecraft->mLevel->getCloudHeight()))
    {
        prepareAndRenderClouds(levelRenderer, partialTick, eyeIndex, cameraX, cameraY, cameraZ);
    }

    GlStateManager::disableBlend();
    Frustum::getFrustum();
    setupFog(0, partialTick);
    GlStateManager::enableFog();

    MemSect(0x1f);
    mMinecraft->mTextures->bindTexture(&BLOCKS_LOCATION);
    MemSect(0);
    Lighting::turnOff();

    InfoBarManager::StartTimingCategory(InfoBarManager::eTimingCagegory_Terrain);
    levelRenderer->render(cameraEntity, 0, partialTick, sortTransparent);
    InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_Terrain);

    GlStateManager::alphaFunc(4, 0.01f);

    if (mDebugView == 0)
    {
        Lighting::turnOn();
        culler->prepare(cameraEntity->mX, cameraEntity->mY, cameraEntity->mZ);
        levelRenderer->renderEntities(cameraEntity, culler, partialTick);

        InfoBarManager::StartTimingCategory(InfoBarManager::eTimingCagegory_Particles);
        turnOnLightLayer(partialTick);
        particleEngine->renderLit(cameraEntity, partialTick, 1);
        Lighting::turnOff();
        setupFog(0, partialTick);
        particleEngine->render(cameraEntity, partialTick, 1);
        turnOffLightLayer(partialTick);

        if (mMinecraft->mHitResult != NULL && cameraEntity->isUnderLiquid(Material::WATER))
        {
            if (eTYPE_DERIVED_FROM(EntityType::PLAYER, cameraEntity->GetType()))
            {
                boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(cameraEntity);
                GlStateManager::disableAlphaTest();
                levelRenderer->renderHit(player, mMinecraft->mHitResult, 0, player->mInventory->getSelected(),
                                         partialTick);
                GlStateManager::enableAlphaTest();
            }
        }

        InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_Particles);
    }

    InfoBarManager::StartTimingCategory(InfoBarManager::eTimingCagegory_TerrainTransparent);
    GlStateManager::disableBlend();
    GlStateManager::enableCull();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::depthMask(true);
    setupFog(0, partialTick);
    GlStateManager::enableBlend();
    GlStateManager::disableCull();
    MemSect(0x1f);
    mMinecraft->mTextures->bindTexture(&BLOCKS_LOCATION);
    MemSect(0);
    GlStateManager::blendFunc(0, 1);
    GlStateManager::enableCull();
    levelRenderer->render(cameraEntity, 2, partialTick, sortTransparent);
    GlStateManager::blendFunc(4, 5);
    levelRenderer->render(cameraEntity, 2, partialTick, sortTransparent);
    gRenderer->BeginOrderIndependentAlpha();
    levelRenderer->render(cameraEntity, 1, partialTick, sortTransparent);
    InfoBarManager::EndTimingCategory(InfoBarManager::eTimingCagegory_TerrainTransparent);

    Lighting::turnOn();
    turnOnLightLayer(partialTick);
    particleEngine->renderLit(cameraEntity, partialTick, 0);
    Lighting::turnOff();
    setupFog(0, partialTick);
    particleEngine->render(cameraEntity, partialTick, 0);
    turnOffLightLayer(partialTick);

    gRenderer->EndOrderIndependentAlpha();
    GlStateManager::depthMask(true);
    GlStateManager::enableCull();
    GlStateManager::disableBlend();
    if (mZoom == 1.0f)
    {
        if (eTYPE_DERIVED_FROM(EntityType::PLAYER, cameraEntity->GetType()) && mMinecraft->mHitResult != NULL)
        {
            boost::shared_ptr<Player> outlinePlayer = boost::static_pointer_cast<Player>(cameraEntity);
            GlStateManager::disableAlphaTest();
            levelRenderer->renderHitOutline(outlinePlayer, mMinecraft->mHitResult, 0, partialTick);
            GlStateManager::enableAlphaTest();
        }
    }

    renderThermal(culler, partialTick);

    if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
    {
        if (Minecraft::GetInstance()->GetMiniGame()->GetHostOption(eCustomGameModeHostOption_TimeTrial) != 0)
        {
            boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = mMinecraft->mPlayer;
#ifdef MINECRAFT_PC
            const bool ghostShown = !PcSettingsGlideGhostHidden();
#else
            const bool ghostShown = true;
#endif
            if (GameSettings::GetGameSettings(localPlayer->mUserIndex, eGameSetting_ShowGlideGhost) != 0
                && gApp.IsAppPaused() == 0 && !localPlayer->isSpectator() && ghostShown)
            {
                renderGhost(partialTick, cameraEntity);
            }

            if (GameSettings::GetGameSettings(localPlayer->mUserIndex, eGameSetting_ShowGlideGhostPath) != 0
                && !localPlayer->isSpectator() && ghostShown)
            {
                renderGhostPath(partialTick, cameraEntity);
            }
        }
    }

    levelRenderer->renderDestroyAnimation(Tesselator::getInstance(), boost::static_pointer_cast<Player>(cameraEntity),
                                          partialTick);

    if (static_cast<double>(mMinecraft->mLevel->getCloudHeight()) <= cameraEntity->mY + cameraEntity->getEyeHeight())
    {
        prepareAndRenderClouds(levelRenderer, partialTick, eyeIndex, cameraX, cameraY, cameraZ);
    }

    setupFog(0, partialTick);
    GlStateManager::enableFog();
    renderSnowAndRain(partialTick);
    GlStateManager::disableFog();
#ifdef MINECRAFT_PC
    if (PcModViewsDrawsInWorld())
        renderModHands(partialTick);
    if (!PcModViewsShowHand())
        return;
#endif
    GlStateManager::clear(1);
    renderItemInHand(partialTick, eyeIndex);
}

#ifdef MINECRAFT_PC
void GameRenderer::renderModHands(float partialTick)
{
    if (!PcModViewsInView())
        PcModViewsSetGameView(gRenderer->MatrixGet(0));
    float hands[2][16];
    const bool left = PcModViewsArm(0, hands[0]);
    const bool right = PcModViewsArm(1, hands[1]);
    const int items = PcModViewsItemCount();
    const int boxes = PcModViewsBoxCount();
    const int texts = PcModViewsTextCount();
    const bool draws = PcNativeModsWantViewDraw() && PcModViewsFromCamera() != 0;
    if (!left && !right && items == 0 && boxes == 0 && texts == 0 && !draws)
        return;
    boost::shared_ptr<MultiplayerLocalPlayer> player = mMinecraft->mPlayer;
    if (player != NULL && (player->isSpectator() || mMinecraft->mOptions->mHideGui))
        return;
    GlStateManager::enableDepthTest();
    GlStateManager::depthMask(true);
    GlStateManager::matrixMode(0);
    if (player != NULL)
    {
        Lighting::turnOn();
        turnOnLightLayer(static_cast<double>(partialTick));
        GlStateManager::enableRescaleNormal();
        if (left)
            mItemInHandRenderer->renderModHand(HumanoidArm::LEFT, hands[0]);
        if (right)
            mItemInHandRenderer->renderModHand(HumanoidArm::RIGHT, hands[1]);
        for (int index = 0; index < items; ++index)
        {
            float viewFromItem[16];
            const void* item = PcModViewsItem(index, viewFromItem);
            if (item != 0)
                mItemInHandRenderer->renderModItem(*static_cast<const boost::shared_ptr<ItemInstance>*>(item),
                                                  viewFromItem);
        }
        GlStateManager::disableRescaleNormal();
        turnOffLightLayer(static_cast<double>(partialTick));
        Lighting::turnOff();
    }

    if (draws)
    {
        Lighting::turnOn();
        if (player != NULL)
        {
            turnOnLightLayer(static_cast<double>(partialTick));
            mItemInHandRenderer->setLightValue(player);
        }
        GlStateManager::enableRescaleNormal();
        GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
        GlStateManager::pushMatrix();
        GlStateManager::loadIdentity();
        GlStateManager::glMultMatrixf(const_cast<float*>(PcModViewsFromCamera()));
        PcModViewsDraw();
        GlStateManager::popMatrix();
        GlStateManager::disableRescaleNormal();
        if (player != NULL)
            turnOffLightLayer(static_cast<double>(partialTick));
        Lighting::turnOff();
        GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    }

    static const float faces[6][4][3] = {
        {{-1, 1, -1}, {-1, 1, 1}, {1, 1, 1}, {1, 1, -1}},     {{-1, -1, 1}, {-1, -1, -1}, {1, -1, -1}, {1, -1, 1}},
        {{1, -1, -1}, {1, 1, -1}, {1, 1, 1}, {1, -1, 1}},     {{-1, -1, 1}, {-1, 1, 1}, {-1, 1, -1}, {-1, -1, -1}},
        {{-1, -1, -1}, {-1, 1, -1}, {1, 1, -1}, {1, -1, -1}}, {{1, -1, 1}, {1, 1, 1}, {-1, 1, 1}, {-1, -1, 1}}};
    static const float shades[6] = {1.0f, 0.64f, 0.86f, 0.86f, 0.77f, 0.77f};
    bool started = false;
    for (int index = 0; index < boxes * 2; ++index)
    {
        float viewFromBox[16];
        float colour[4];
        const int kind = PcModViewsWorldBox(index % boxes, viewFromBox, colour);
        if (kind == 0 || (kind == 3) != (index >= boxes))
            continue;
        if (index >= boxes)
            GlStateManager::disableDepthTest();
        if (!started)
        {
            GlStateManager::disableTexture();
            GlStateManager::disableLighting();
            GlStateManager::disableCull();
            GlStateManager::enableBlend();
            GlStateManager::blendFunc(4, 5);
            started = true;
        }
        GlStateManager::depthMask(colour[3] >= 0.99f);
        GlStateManager::pushMatrix();
        GlStateManager::loadIdentity();
        GlStateManager::glMultMatrixf(viewFromBox);
        BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
        for (int face = 0; face < 6; ++face)
        {
            const float shade = kind == 2 ? 1.0f : shades[face];
            GlStateManager::color4f(colour[0] * shade, colour[1] * shade, colour[2] * shade, colour[3]);
            builder->begin();
            for (int corner = 0; corner < 4; ++corner)
                builder->vertex(faces[face][corner][0], faces[face][corner][1], faces[face][corner][2]);
            builder->end();
        }
        GlStateManager::popMatrix();
    }
    if (started)
    {
        GlStateManager::enableDepthTest();
        GlStateManager::depthMask(true);
        GlStateManager::enableCull();
        GlStateManager::disableBlend();
        GlStateManager::enableTexture();
        GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    }

    if (texts == 0)
        return;
    GlStateManager::disableLighting();
    GlStateManager::disableCull();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::depthMask(false);
    for (int index = 0; index < texts; ++index)
    {
        float viewFromText[16];
        unsigned int colour = 0;
        unsigned int background = 0;
        const char* utf8 = PcModViewsText(index, viewFromText, &colour, &background);
        if (utf8 == 0 || utf8[0] == 0)
            continue;
        std::wstring text;
        for (const unsigned char* c = (const unsigned char*)utf8; *c != 0;)
        {
            unsigned int code = *c++;
            int more = 0;
            if (code >= 0xF0)
            {
                code &= 0x07;
                more = 3;
            }
            else if (code >= 0xE0)
            {
                code &= 0x0F;
                more = 2;
            }
            else if (code >= 0xC0)
            {
                code &= 0x1F;
                more = 1;
            }
            for (; more > 0 && (*c & 0xC0) == 0x80; --more)
                code = (code << 6) | (*c++ & 0x3F);
            text += (wchar_t)code;
        }
        const int width = mMinecraft->mFont->width(text);
        GlStateManager::pushMatrix();
        GlStateManager::loadIdentity();
        GlStateManager::glMultMatrixf(viewFromText);
        GlStateManager::scalef(1.0f, -1.0f, 1.0f);
        if ((background >> 24) != 0)
        {
            GlStateManager::disableTexture();
            GlStateManager::color4f((float)((background >> 16) & 0xFF) / 255.0f, (float)((background >> 8) & 0xFF) / 255.0f,
                                    (float)(background & 0xFF) / 255.0f, (float)(background >> 24) / 255.0f);
            BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
            builder->begin();
            builder->vertex((float)(-width / 2 - 1), -5.0f, 0.0f);
            builder->vertex((float)(-width / 2 - 1), 4.0f, 0.0f);
            builder->vertex((float)(width - width / 2 + 1), 4.0f, 0.0f);
            builder->vertex((float)(width - width / 2 + 1), -5.0f, 0.0f);
            builder->end();
            GlStateManager::enableTexture();
        }
        GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
        mMinecraft->mFont->draw(text, -width / 2, -4, (int)colour);
        GlStateManager::popMatrix();
    }
    GlStateManager::depthMask(true);
    GlStateManager::enableCull();
    GlStateManager::disableBlend();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
}
#endif

#ifdef MINECRAFT_PC
void GameRenderer::renderModThingsInView()
{
    unsigned int scissor[4];
    gRenderer->StateGetScissorRect(scissor[0], scissor[1], scissor[2], scissor[3]);
    PcSetWideWorldPass(true);
    gRenderer->StateSetViewport(C4JRender::eViewportType_FullScreen);
    gRenderer->StateSetScissorRect(0, 0, 1280, 720);
    float matrix[16];
    GlStateManager::matrixMode(1);
    GlStateManager::loadIdentity();
    PcModViewsProjection(0.05f, 64.0f, matrix);
    GlStateManager::glMultMatrixf(matrix);
    GlStateManager::matrixMode(0);
    GlStateManager::loadIdentity();
    GlStateManager::disableFog();
    GlStateManager::clear(1);
    renderModHands(0.0f);
    PcSetWideWorldPass(false);
    gRenderer->StateSetScissorRect(scissor[0], scissor[1], scissor[2], scissor[3]);
}
#endif

void GameRenderer::renderLevel(float partialTick, long long targetNanoTime)
{
    static const float kAlphaRef = 0.5f;

    Minecraft* minecraft = mMinecraft;
    const int playerIndex = gProfile->GetPrimaryPad();
    const bool sortTransparent = minecraft->mPlayer.get() == minecraft->mLocalPlayers[playerIndex].get();

    pick(partialTick);
    GlStateManager::enableDepthTest();
    GlStateManager::enableAlphaTest();
    GlStateManager::alphaFunc(4, kAlphaRef);
#ifdef MINECRAFT_PC
    PcSetWideWorldPass(true);
    if (PcModViewsBeginFrame(mMinecraft->getCameraEntity().get(), partialTick))
    {
        GlStateManager::colorMask(true, true, true, false);
        for (int i = 0; i < PcModViewsCount(); ++i)
        {
            PcModViewsBeginView(i);
            renderEyePass(2, partialTick, targetNanoTime, sortTransparent);
            PcModViewsEndView();
        }
        if (PcModViewsKeepWindowView())
            renderEyePass(2, partialTick, targetNanoTime, sortTransparent);
        PcModViewsFinishFrame();
        GlStateManager::colorMask(true, true, true, true);
        PcSetWideWorldPass(false);
        return;
    }
#endif

    if (mMinecraft->mOptions->mAnaglyph3d)
    {
        sAnaglyphEyeIndex = 0;
        GlStateManager::colorMask(false, true, true, false);
        renderEyePass(0, partialTick, targetNanoTime, sortTransparent);
        sAnaglyphEyeIndex = 1;
        GlStateManager::colorMask(true, false, false, false);
        renderEyePass(1, partialTick, targetNanoTime, sortTransparent);
    }
    else
    {
        GlStateManager::colorMask(true, true, true, false);
        renderEyePass(2, partialTick, targetNanoTime, sortTransparent);
    }
#ifdef MINECRAFT_PC
    PcSetWideWorldPass(false);
#endif
}

void GameRenderer::setupGuiScreen(int guiScale)
{
    ScreenSizeCalculator screenSize(mMinecraft->mOptions, guiScale);

    GlStateManager::matrixMode(1);
    GlStateManager::loadIdentity();
    GlStateManager::ortho(0.0, static_cast<float>(screenSize.mWidthScale), static_cast<float>(screenSize.mHeightScale),
                          0.0, 1000.0, 3000.0);
    GlStateManager::matrixMode(0);
    GlStateManager::loadIdentity();
    GlStateManager::translatef(0.0f, 0.0f, -2000.0f);
}

void GameRenderer::renderItemActivationAnimation(int width, int height, float partialTick)
{
    static const float kPi = 3.1415927f;

    const int playerIndex = mMinecraft->mPlayer->mUserIndex;
    if (mItemActivationItems[playerIndex]->isEmpty() || mItemActivationTicks[playerIndex] <= 0)
        return;

    const float progress = (static_cast<float>(40 - mItemActivationTicks[playerIndex]) + partialTick) / 40.0f;
    const float progressSquared = progress * progress;
    const float progressCubed = progress * progressSquared;
    const float curve = 10.25f * progressCubed * progressSquared + -24.95f * progressSquared * progressSquared
                        + 25.5f * progressCubed + -13.8f * progressSquared + 4.0f * progress;
    const float angle = curve * kPi;
    const float offsetX = mItemActivationOffX[playerIndex];
    const float offsetY = mItemActivationOffY[playerIndex];

    GlStateManager::enableAlphaTest();
    GlStateManager::pushMatrix();
    GlStateManager::enableDepthTest();
    GlStateManager::disableCull();
    Lighting::turnOn();

    const float swingX = sinf(angle * 2.0f);
    const float swingY = sinf(angle * 2.0f);
    GlStateManager::translatef(
        offsetX * static_cast<float>(width / 4) * Mth::abs(swingX) + static_cast<float>(width / 2),
        offsetY * static_cast<float>(height / 4) * Mth::abs(swingY) + static_cast<float>(height / 2), -50.0f);
    const float scale = 50.0f + 175.0f * Mth::sin(angle);
    GlStateManager::scalef(scale, -scale, scale);
    GlStateManager::rotatef(900.0f * Mth::abs(sinf(angle)), 0.0f, 1.0f, 0.0f);
    GlStateManager::rotatef(6.0f * cosf(progress * 8.0f), 1.0f, 0.0f, 0.0f);
    GlStateManager::rotatef(6.0f * cosf(progress * 8.0f), 0.0f, 0.0f, 1.0f);

    TextureAtlasSprite* sprite = NULL;
    mMinecraft->getItemRenderer()->renderStatic(mItemActivationItems[playerIndex], boost::shared_ptr<LivingEntity>(),
                                                ItemTransforms::FIXED, 1.0f, false, sprite);

    GlStateManager::popMatrix();
    Lighting::turnOff();
    GlStateManager::enableCull();
    GlStateManager::disableDepthTest();
}

void GameRenderer::render(float partialTick, long long frameNanoTime, bool refreshLightTexture)
{
    if (mLightTextureDirty && refreshLightTexture)
        this->updateLightTexture(partialTick);

    mLastActiveTime = System::processTimeInMilliSecs();

    if (mMinecraft->mNoRender)
        return;

    anaglyph3d = mMinecraft->mOptions->mAnaglyph3d;

    ScreenSizeCalculator calculator(mMinecraft->mOptions, -1);
    calculator.getWidth();
    const int screenHeight = static_cast<int>(calculator.getHeight());

    int fpsCap = fjGetFpsCap(mMinecraft->mOptions->mFpsLimit);

    if (mMinecraft->mLevel != NULL)
    {
        const int averageFps = Minecraft::getAverageFps();
        if (averageFps < fpsCap)
            fpsCap = averageFps;
        if (fpsCap < 60)
            fpsCap = 60;

        const long long startNano = System::processTimeInNanoSecs();
        int remainingNanos = 1000000000 / fpsCap / 4 - static_cast<int>(startNano - frameNanoTime);
        if (remainingNanos < 0)
            remainingNanos = 0;
        renderLevel(partialTick, System::processTimeInNanoSecs() + remainingNanos);

        mLastNanos = System::processTimeInNanoSecs();

        if (!mMinecraft->mOptions->mHideGui || mMinecraft->mScreen != NULL)
        {
            GlStateManager::alphaFunc(4, 0.01f);
            mMinecraft->mGui->render(partialTick, mMinecraft->mScreen != NULL, 0, screenHeight - 1);
        }
    }
    else
    {

        GlStateManager::matrixMode(1);
        GlStateManager::loadIdentity();
        GlStateManager::matrixMode(0);
        GlStateManager::loadIdentity();
        setupGuiScreen(-1);
        mLastNanos = System::processTimeInNanoSecs();
    }
    if (mMinecraft->mScreen != NULL)
    {
        GlStateManager::clear(1);
        mMinecraft->mScreen->render(0, screenHeight - 1, partialTick);
    }
}

int GameRenderer::shouldRenderBlockOutline()
{
    if (!mRenderBlockOutline)
        return 0;

    boost::shared_ptr<Entity> camera = mMinecraft->getCameraEntity();
    bool render = eTYPE_DERIVED_FROM(EntityType::PLAYER, camera->GetType()) && !mMinecraft->mOptions->mHideGui;
    boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(camera);
    if (render && !player->mAbilities.mMayBuild)
    {
        not_null_ptr<ItemInstance> item = player->getMainHandItem();
        const HitResult* hit = mMinecraft->mHitResult;
        if (hit != NULL && hit->mType == HitResult::BLOCK)
        {
            BlockPos pos = hit->getBlockPos();
            const BlockState* state = mMinecraft->mLevel->getBlockState(pos);
            Block* block = state->getBlock();
            if (mMinecraft->mGameMode->GetGameType() == GameType::SPECTATOR)
            {
                render = block->mBlockEntity
                         && eTYPE_DERIVED_FROM(BlockEntityType::CONTAINER,
                                               mMinecraft->mLevel->getBlockEntity(pos)->GetType());
            }
            else
            {
                render = !item->isEmpty()
                         && (item->hasAdventureModeBreakTagForBlock(block)
                             || item->hasAdventureModePlaceTagForBlock(block));
            }
        }
    }
    return render ? 1 : 0;
}

void GameRenderer::resetData()
{
    mItemActivationItems[0] = ItemInstance::EMPTY;
    mItemActivationItems[1] = ItemInstance::EMPTY;
    mItemActivationItems[2] = ItemInstance::EMPTY;
    mItemActivationItems[3] = ItemInstance::EMPTY;
    mMapRenderer->resetData();
}

MapRenderer* GameRenderer::getMapRenderer()
{
    return mMapRenderer;
}

void GameRenderer::AddForDelete(SparseLightStorage* storage)
{
    EnterCriticalSection(&sDeleteMutex);
    sPendingLightDeletes.push_back(storage);
}

void GameRenderer::AddForDelete(LevelChunk* chunk)
{
    EnterCriticalSection(&sDeleteMutex);
    sPendingChunkDeletes.push_back(chunk);
}

void GameRenderer::AddForDelete(ElementStorage* storage)
{
    EnterCriticalSection(&sDeleteMutex);
    ElementStorageWithTick entry;
    entry.tick = 2;
    entry.storage = storage;
    sPendingElementStorageDeletes.push_back(entry);
}

void GameRenderer::FinishedReassigning()
{
    LeaveCriticalSection(&sDeleteMutex);
}

void GameRenderer::EnableUpdateThread(int reason)
{
    EnterCriticalSection(&mUpdateThreadMutex);

    mUpdateThreadDisableMask &= ~(1 << reason);

    if (mUpdateThreadDisableMask == 0 && !sRenderUpdateThreadActive)
    {
        sRenderUpdateThreadActive = true;
        sRenderUpdateEvents->Set(0);
        sRenderUpdateEvents->Set(1);
    }

    LeaveCriticalSection(&mUpdateThreadMutex);
}

void GameRenderer::DisableUpdateThread(int reason)
{
    EnterCriticalSection(&mUpdateThreadMutex);

    mUpdateThreadDisableMask |= (1 << reason);

    if (mUpdateThreadDisableMask != 0 && sRenderUpdateThreadActive)
    {
        sRenderUpdateThreadActive = false;
        sRenderUpdateEvents->Clear(0);
        sRenderUpdateEvents->WaitForSingle(1, 0xffffffff);
    }

    LeaveCriticalSection(&mUpdateThreadMutex);
}

void GameRenderer::EnterDeleteThreadCS()
{
    EnterCriticalSection(&sDeleteMutex);
}

void GameRenderer::LeaveDeleteThreadCS()
{
    LeaveCriticalSection(&sDeleteMutex);
}

void GameRenderer::displayItemActivation(not_null_ptr<ItemInstance> item, int playerIndex)
{
    mItemActivationItems[playerIndex] = item;
    mItemActivationTicks[playerIndex] = 40;
    mItemActivationOffX[playerIndex] = mRandom.nextFloat() * 2.0f - 1.0f;
    mItemActivationOffY[playerIndex] = mRandom.nextFloat() * 2.0f - 1.0f;
}
