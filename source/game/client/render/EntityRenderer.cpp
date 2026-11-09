#include "game/client/render/EntityRenderer.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cmath>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/Culler.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Mth.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Options.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/Animal.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/util/AABB.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/FireBlock.hpp"
#include "game/world/block/Iterator.hpp"
#include "game/world/generation/Player.hpp"

static ResourceLocation sShadowTexture(TN_CLAMP_MISC_SHADOW);

EntityRenderer::EntityRenderer(EntityRenderDispatcher* dispatcher)
    : mDispatcher(dispatcher),
      mShadowRadius(0.0F),
      mShadowStrength(1.0F),
      mModel(NULL),
      mOriginalModel(NULL)
{
}

bool EntityRenderer::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double x, double y, double z)
{
    AABB* box = entity->getBoundingBoxForCulling()->grow(0.5);
    if (box->hasNaN() || box->getSize() == 0.0)
    {
        box = AABB::newTemp(entity->mX - 2.0, entity->mY - 2.0, entity->mZ - 2.0, entity->mX + 2.0, entity->mY + 2.0,
                            entity->mZ + 2.0);
    }

    return entity->shouldRender(x, y, z) && (entity->mNoCulling || culler->isVisible(entity->mBoundingBox));
}

bool EntityRenderer::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double distance)
{
    AABB* box = entity->getBoundingBoxForCulling()->grow(0.5);
    if (box->hasNaN() || box->getSize() == 0.0)
    {
        box = AABB::newTemp(entity->mX - 2.0, entity->mY - 2.0, entity->mZ - 2.0, entity->mX + 2.0, entity->mY + 2.0,
                            entity->mZ + 2.0);
    }

    return entity->shouldRender(distance) && (entity->mNoCulling || culler->isVisible(entity->mBoundingBox));
}

void EntityRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float, float)
{
    if (!mSolidRender)
        renderName(entity, x, y, z);
}

void EntityRenderer::render(AABB* box, double x, double y, double z)
{
    GlStateManager::disableTexture();
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    builder->begin();
    builder->offset((float)x, (float)y, (float)z);

    builder->normal(0.0F, 0.0F, -1.0F);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->normal(0.0F, 0.0F, 1.0F);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->normal(0.0F, -1.0F, 0.0F);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMaxZ);
    builder->normal(0.0F, 1.0F, 0.0F);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->normal(-1.0F, 0.0F, 0.0F);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMinX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMinX, (float)box->mMinY, (float)box->mMinZ);
    builder->normal(1.0F, 0.0F, 0.0F);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMinZ);
    builder->vertex((float)box->mMaxX, (float)box->mMaxY, (float)box->mMaxZ);
    builder->vertex((float)box->mMaxX, (float)box->mMinY, (float)box->mMaxZ);

    builder->offset(0.0F, 0.0F, 0.0F);
    builder->end();
    GlStateManager::enableTexture();
}

void EntityRenderer::renderName(boost::shared_ptr<Entity>& entity, double x, double y, double z)
{
    if (shouldShowName(entity))
        renderNameTag(entity, entity->getDisplayName(), x, y, z, 64, 0xFF000000);
}

bool EntityRenderer::shouldShowName(boost::shared_ptr<Entity>& entity)
{
    return entity->shouldShowName() && entity->hasCustomName();
}

void EntityRenderer::renderNameTags(boost::shared_ptr<Entity>& entity, double x, double y, double z,
                                    const std::wstring& name, double, int color)
{
    renderNameTag(entity, name, x, y, z, 64, color);
}

bool EntityRenderer::bindTexture(boost::shared_ptr<Entity>& entity)
{
    const ResourceLocation* location = getTextureLocation(entity);
    if (location == NULL)
        return false;
    bindTexture(location);
    return true;
}

void EntityRenderer::bindTexture(const ResourceLocation* location)
{
    mDispatcher->mTextures->bindTexture(location);
}

bool EntityRenderer::bindTexture(const std::wstring& path, int index)
{
    Textures* textures = mDispatcher->mTextures;
    int texture = textures->loadMemTexture(path, index);
    if (texture < 0)
        return false;
    GlStateManager::bindTexture(texture);
    textures->clearLastBoundId();
    return true;
}

bool EntityRenderer::bindTexture(const std::wstring& path, const std::wstring& alternate)
{
    Textures* textures = mDispatcher->mTextures;
    int texture = textures->loadMemTexture(path, alternate);
    if (texture < 0)
        return false;
    GlStateManager::bindTexture(texture);
    textures->clearLastBoundId();
    return true;
}

const ResourceLocation* EntityRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return NULL;
}

void EntityRenderer::renderFlame(boost::shared_ptr<Entity>& entity, double x, double y, double z, float)
{
    GlStateManager::disableLighting();
    TextureAtlasSprite* fire0 = static_cast<FireBlock*>(Blocks::fire)->getTextureLayer(0);
    TextureAtlasSprite* fire1 = static_cast<FireBlock*>(Blocks::fire)->getTextureLayer(1);
    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y, (float)z);
    float scale = entity->mBbWidth * 1.4F;
    GlStateManager::scalef(scale, scale, scale);
    MemSect(0x1f);
    bindTexture(&BLOCKS_LOCATION);
    MemSect(0);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    float halfWidth = 0.5F;
    float height = entity->mBbHeight / scale;
    float yOffset = (float)(entity->mY - entity->mBoundingBox->mMinY);
    GlStateManager::rotatef(-mDispatcher->mCameraYRot, 0.0F, 1.0F, 0.0F);
    GlStateManager::translatef(0.0F, 0.0F, (float)(int)height * 0.02F - 0.3F);
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);

    float depth = 0.0F;
    int layer = 0;
    builder->begin();
    while (height > 0.0F)
    {
        TextureAtlasSprite* sprite = (layer & 1) == 0 ? fire0 : fire1;
        float u0 = (float)sprite->getU0(false);
        float v0 = (float)sprite->getV0(false);
        float u1 = (float)sprite->getU1(false);
        float v1 = (float)sprite->getV1(false);
        if (((layer / 2) & 1) == 0)
            std::swap(u0, u1);
        builder->vertexUV(halfWidth, -yOffset, depth, u1, v1);
        builder->vertexUV(-halfWidth, -yOffset, depth, u0, v1);
        builder->vertexUV(-halfWidth, 1.4F - yOffset, depth, u0, v0);
        builder->vertexUV(halfWidth, 1.4F - yOffset, depth, u1, v0);
        height -= 0.45F;
        depth += 0.03F;
        yOffset -= 0.45F;
        halfWidth *= 0.9F;
        ++layer;
    }
    builder->end();
    GlStateManager::popMatrix();
    GlStateManager::enableLighting();
}

void EntityRenderer::renderShadow(boost::shared_ptr<Entity>& entity, double x, double y, double z, float alpha,
                                  float partialTick)
{
    GlStateManager::disableLighting();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::polygonOffset(-3.0F, -0.3F);
    MemSect(0x1f);
    mDispatcher->mTextures->bindTexture(&sShadowTexture);
    MemSect(0);
    Level* level = getLevel();
    GlStateManager::depthMask(false);

    float radius = mShadowRadius;
    if (eTYPE_DERIVED_FROM(EntityType::MOB, entity->GetType()))
    {
        boost::shared_ptr<Mob> mob = boost::static_pointer_cast<Mob>(entity);
        radius *= (float)mob->getSizeScale();
        if (eTYPE_DERIVED_FROM(EntityType::AGABLE_MOB, mob->GetType()))
        {
            boost::shared_ptr<Animal> animal = boost::static_pointer_cast<Animal>(mob);
            if (animal->isBaby())
                radius *= 0.5F;
        }
    }

    double entityX = entity->mXOld + (entity->mX - entity->mXOld) * partialTick;
    double entityY = entity->mYOld + (entity->mY - entity->mYOld) * partialTick;
    double entityZ = entity->mZOld + (entity->mZ - entity->mZOld) * partialTick;
    int minX = Mth::floor(entityX - radius);
    int maxX = Mth::floor(entityX + radius);
    int minY = Mth::floor(entityY - radius);
    int maxY = Mth::floor(entityY);
    int minZ = Mth::floor(entityZ - radius);
    int maxZ = Mth::floor(entityZ + radius);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->begin();
    Iterator<BlockPos*>* positions = BlockPos::between(BlockPos(minX, minY, minZ), BlockPos(maxX, maxY, maxZ));
    while (positions->hasNext())
    {
        BlockPos pos = *positions->next();
        const BlockState* state = level->getBlockState(pos.below());
        if (state->getRenderShape() != BLOCK_RENDER_INVISIBLE && level->getRawBrightness(pos) > 3)
            renderBlockShadow(state, x, y, z, pos, alpha, radius, x - entityX, y - entityY, z - entityZ);
    }
    delete positions;
    builder->end();
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    GlStateManager::disableBlend();
    GlStateManager::depthMask(true);
    GlStateManager::enableLighting();
    GlStateManager::polygonOffset(0.0F, 0.0F);
}

Level* EntityRenderer::getLevel()
{
    return mDispatcher->mLevel;
}

void EntityRenderer::renderBlockShadow(const BlockState* state, double x, double y, double z, const BlockPos& pos,
                                       float alpha, float radius, double offsetX, double offsetY, double offsetZ)
{
    if (!state->isCubeShaped())
        return;
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    Level* level = getLevel();
    double shade = (alpha - (y - ((double)pos.y + offsetY)) * 0.5) * 0.5;
    shade *= level->getBrightness(pos);
    if (shade < 0.0)
        return;
    if (shade > 1.0)
        shade = 1.0;

    builder->color(1.0F, 1.0F, 1.0F, (float)shade);
    const AABB* shape = state->getShape(getLevel(), pos);
    double minX = (double)pos.x + shape->mMinX + offsetX;
    double maxX = (double)pos.x + shape->mMaxX + offsetX;
    double minY = (double)pos.y + shape->mMinY + offsetY + 0.015625;
    double minZ = (double)pos.z + shape->mMinZ + offsetZ;
    double maxZ = (double)pos.z + shape->mMaxZ + offsetZ;
    float minU = (float)((x - minX) * 0.5 / radius + 0.5);
    float maxU = (float)((x - maxX) * 0.5 / radius + 0.5);
    float minV = (float)((z - minZ) * 0.5 / radius + 0.5);
    float maxV = (float)((z - maxZ) * 0.5 / radius + 0.5);
    builder->vertexUV((float)minX, (float)minY, (float)minZ, minU, minV);
    builder->vertexUV((float)minX, (float)minY, (float)maxZ, minU, maxV);
    builder->vertexUV((float)maxX, (float)minY, (float)maxZ, maxU, maxV);
    builder->vertexUV((float)maxX, (float)minY, (float)minZ, maxU, minV);
}

void EntityRenderer::postRender(boost::shared_ptr<Entity>& entity, double x, double y, double z, float,
                                float partialTick, bool renderShadowFlag)
{
    if (mDispatcher->mOptions != NULL)
    {
        if (!mDispatcher->mRenderingGuiPlayer && renderShadowFlag && mDispatcher->mOptions->mFancyGraphics
            && mShadowRadius > 0.0F && !entity->isInvisible() && mDispatcher->shouldRenderShadow())
        {
            double distance = mDispatcher->distanceToSqr(entity->mX, entity->mY, entity->mZ);
            float alpha = (float)((1.0 - distance / 256.0) * mShadowStrength);
            if (alpha > 0.0F)
                renderShadow(entity, x, y, z, alpha, partialTick);
        }

        if (entity->displayFireAnimation())
        {
            if (!(eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType())
                  && boost::static_pointer_cast<Player>(entity)->isSpectator()))
                renderFlame(entity, x, y, z, partialTick);
        }
    }
}

Font* EntityRenderer::getFont()
{
    return mDispatcher->getFont();
}

void EntityRenderer::renderNameTag(boost::shared_ptr<Entity>& entity, const std::wstring& name, double x, double y,
                                   double z, int maxDistance, int color)
{
    double distance = entity->distanceToSqr(mDispatcher->mCamera);
    if (distance > (double)(maxDistance * maxDistance))
        return;

    bool sneaking = entity->isSneaking();
    float yOffset = entity->mBbHeight + 0.5F - (sneaking ? 0.25F : 0.0F);
    std::wstring displayName(name);
    int nameOffset = 0;
    if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType()))
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
        if (gApp.isXuidDeadmau5(player->mXuid))
            nameOffset = -10;
        displayName = name;
    }

    int fadeDistance = 256;
    if (!gRenderer->IsHiDef() || gApp.GetLocalPlayerCount() > 2)
        fadeDistance = 64;
    float alpha = 1.0F;
    if (distance >= (double)fadeDistance)
    {
        int fade = (int)std::sqrt(distance - fadeDistance);
        alpha = 1.0F / (fade / 2);
        if (fade > fadeDistance || alpha < 0.0F)
            alpha = 0.0F;
        else if (alpha > 1.0F)
            alpha = 1.0F;
    }

    bool renderSneaking = sneaking && !Minecraft::InMiniGame(eMiniGameId_Any, true);
    GameRenderer::renderNameTagInWorld(getFont(), displayName, (float)x, (float)y + yOffset, (float)z, nameOffset,
                                       mDispatcher->mCameraYRot, mDispatcher->mCameraXRot,
                                       mDispatcher->mOptions->mThirdPersonView == 2, renderSneaking, color, alpha);
}

bool EntityRenderer::hasSecondPass()
{
    return false;
}

void EntityRenderer::renderSecondPass(boost::shared_ptr<Entity>&, double, double, double, float, float)
{
}

void EntityRenderer::setupForTexturePack(TexturePack* pack)
{
    std::wstring modelName(L"");
    std::wstring originalModelName(L"");
    Model* model = NULL;
    if (mModel != NULL)
    {
        modelName = getModelName();
        originalModelName = getOriginalModelName();
        model = mModel;
    }
    if (mOriginalModel == NULL)
        mOriginalModel = model;
    if (model != mOriginalModel)
    {
        if (model != NULL)
            delete model;
        mModel = NULL;
    }
    if (pack->HasCustomModel(modelName))
        mModel = createModel(*pack->GetCustomModel(modelName));
    else if (pack->HasCustomModel(originalModelName))
        mModel = createModel(*pack->GetCustomModel(originalModelName));
    else
        mModel = mOriginalModel;
}

void EntityRenderer::SetItemFrame(bool)
{
}

Model* EntityRenderer::getModel()
{
    return mModel;
}

std::wstring EntityRenderer::getModelName()
{
    return L"";
}

Model* EntityRenderer::createModel(const ModelDefinition&)
{
    return NULL;
}

EntityRenderDispatcher* EntityRenderer::getDispatcher()
{
    return mDispatcher;
}

std::wstring EntityRenderer::getOriginalModelName()
{
    if (mModel != NULL)
        return mModel->getName();
    return L"";
}

unsigned int EntityRenderer::getTeamColor(boost::shared_ptr<Entity>&)
{
    return 0x00FFFFFFU;
}

void EntityRenderer::setSolidRender(bool solid)
{
    mSolidRender = solid;
}
