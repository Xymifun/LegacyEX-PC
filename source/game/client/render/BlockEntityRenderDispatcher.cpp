#include "game/client/render/BlockEntityRenderDispatcher.hpp"

#include <boost/unordered_map.hpp>

#include "game/client/render/BannerRenderer.hpp"
#include "game/client/render/BeaconRenderer.hpp"
#include "game/client/render/BedRenderer.hpp"
#include "game/client/render/ChestRenderer.hpp"
#include "game/client/render/ConduitRenderer.hpp"
#include "game/client/render/EnderChestRenderer.hpp"
#include "game/client/render/EnchantTableRenderer.hpp"
#include "game/client/render/Font.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/MobSpawnerRenderer.hpp"
#include "game/client/render/PistonHeadRenderer.hpp"
#include "game/client/render/ShulkerBoxRenderer.hpp"
#include "game/client/render/ShulkerModel.hpp"
#include "game/client/render/SignRenderer.hpp"
#include "game/client/render/SkullBlockRenderer.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/Textures.hpp"
#include "game/client/render/TheEndGatewayRenderer.hpp"
#include "game/client/render/TheEndPortalRenderer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/HitResult.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockEntity.hpp"

BlockEntityRenderDispatcher* BlockEntityRenderDispatcher::instance = NULL;
double BlockEntityRenderDispatcher::xOff = 0.0;
double BlockEntityRenderDispatcher::yOff = 0.0;
double BlockEntityRenderDispatcher::zOff = 0.0;

BlockEntityRenderDispatcher::BlockEntityRenderDispatcher()
    : boost::unordered_map<unsigned long long, BlockEntityRenderer*>(),
      mCamera()
{
    mFont = NULL;
    mTextures = NULL;
    mLevel = NULL;
    mCamera = boost::shared_ptr<Entity>();
    mCameraYRot = 0.0f;
    mCameraXRot = 0.0f;
    mX = mY = mZ = 0.0;
    GlStateManager::enableLighting();

    (*this)[BlockEntityType::SIGN] = new SignRenderer();
    (*this)[BlockEntityType::MOB_SPAWNER] = new MobSpawnerRenderer();
    (*this)[BlockEntityType::PISTON_MOVING] = new PistonHeadRenderer();
    (*this)[BlockEntityType::CHEST] = new ChestRenderer();
    (*this)[BlockEntityType::ENDER_CHEST] = new EnderChestRenderer();
    (*this)[BlockEntityType::ENCHANTMENT_TABLE_ENTITY] = new EnchantTableRenderer();
    (*this)[BlockEntityType::THE_END_PORTAL] = new TheEndPortalRenderer();
    (*this)[BlockEntityType::THE_END_GATEWAY_ENTITY] = new TheEndGatewayRenderer();
    (*this)[BlockEntityType::SKULL] = new SkullBlockRenderer();
    (*this)[BlockEntityType::FURNACE] = NULL;
    (*this)[BlockEntityType::BEACON] = new BeaconRenderer();
    (*this)[BlockEntityType::BANNER] = new BannerRenderer();
    (*this)[BlockEntityType::SHULKER_BOX] = new ShulkerBoxRenderer(new ShulkerModel());
    (*this)[BlockEntityType::BED] = new BedRenderer();
    (*this)[BlockEntityType::CONDUIT] = new ConduitRenderer();

    GlStateManager::disableLighting();
    for (iterator it = begin(); it != end(); ++it)
    {
        if (it->second != NULL)
        {
            it->second->init(this);
        }
    }
}

Font* BlockEntityRenderDispatcher::getFont()
{
    return mFont;
}

void BlockEntityRenderDispatcher::staticCtor()
{
    instance = new BlockEntityRenderDispatcher();
}

BlockEntityRenderer* BlockEntityRenderDispatcher::getRenderer(unsigned long long type)
{
    iterator it = find(type);
    if (it == end())
    {
        return NULL;
    }
    return it->second;
}

BlockEntityRenderer* BlockEntityRenderDispatcher::getRenderer(boost::shared_ptr<BlockEntity> blockEntity)
{
    if (!blockEntity)
    {
        return NULL;
    }
    return getRenderer(blockEntity->GetType());
}

bool BlockEntityRenderDispatcher::hasRenderer(boost::shared_ptr<BlockEntity> blockEntity)
{
    return getRenderer(blockEntity) != NULL;
}

void BlockEntityRenderDispatcher::setLevel(Level* level)
{
    mLevel = level;
    if (level == NULL)
    {
        mCamera = boost::shared_ptr<Entity>();
    }
    for (iterator it = begin(); it != end(); ++it)
    {
        if (it->second != NULL)
        {
            it->second->onNewLevel(level);
        }
    }
}

void BlockEntityRenderDispatcher::prepare(Level* level, Textures* textures, Font* font,
                                          boost::shared_ptr<Entity> camera, const HitResult* cameraHitResult,
                                          float partialTicks)
{
    if (mLevel != level)
    {
        setLevel(level);
    }
    mTextures = textures;
    mCamera = camera;
    mFont = font;
    mCameraHitResult = cameraHitResult;
    mCameraYRot = camera->mYRotO + (camera->mYRot - camera->mYRotO) * partialTicks;
    mCameraXRot = camera->mXRotO + (camera->mXRot - camera->mXRotO) * partialTicks;
    mX = camera->mXOld + (camera->mX - camera->mXOld) * partialTicks;
    mY = camera->mYOld + (camera->mY - camera->mYOld) * partialTicks;
    mZ = camera->mZOld + (camera->mZ - camera->mZOld) * partialTicks;
    static_cast<SignRenderer*>((*this)[BlockEntityType::SIGN])->tick();
}

void BlockEntityRenderDispatcher::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z,
                                         float a, int breakProgress, bool bTranslucent, float alpha,
                                         bool bUseDisplayList)
{
    BlockEntityRenderer* renderer = getRenderer(blockEntity);
    if (renderer != NULL)
    {
        if (Minecraft::GetInstance()->GetMiniGame()->HasFeatureCustomBeacons()
            && eTYPE_DERIVED_FROM(BlockEntityType::BEACON, blockEntity->GetType()))
        {
            static_cast<BeaconRenderer*>(renderer)->renderSpecialBeamDirection(
                blockEntity, x, y, z, mX, mY, mZ, a, breakProgress, bTranslucent, alpha, bUseDisplayList);
        }
        else
        {
            renderer->render(blockEntity, x, y, z, a, breakProgress, bTranslucent, alpha, bUseDisplayList);
        }
    }
}

void BlockEntityRenderDispatcher::render(boost::shared_ptr<BlockEntity> blockEntity, float partialTicks,
                                         int breakProgress, bool bTranslucent)
{
    if (blockEntity->distanceToSqr(mX, mY, mZ) < blockEntity->getViewDistance())
    {
        Lighting::turnOn();
        int lightColor = mLevel->getLightColor(blockEntity->getBlockPos(), 0, -1);
        int u = lightColor % 65536;
        int v = lightColor / 65536;
        GlStateManager::multiTexCoord2f(u / 1.0f, v / 1.0f);
        if (bTranslucent)
        {
            GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
        }
        BlockPos position = blockEntity->getBlockPos();
        render(blockEntity, (double)position.x - xOff, (double)position.y - yOff, (double)position.z - zOff,
               partialTicks, breakProgress, bTranslucent, 1.0f, true);
    }
}

void BlockEntityRenderDispatcher::render(boost::shared_ptr<BlockEntity> blockEntity, float partialTicks)
{
    render(blockEntity, partialTicks, -1, true);
}

void BlockEntityRenderDispatcher::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z,
                                         float a, bool bTranslucent, float alpha, bool bUseDisplayList)
{
    render(blockEntity, x, y, z, a, -1, bTranslucent, alpha, bUseDisplayList);
}

void BlockEntityRenderDispatcher::setupForTexturePack(TexturePack* texturePack)
{
    for (iterator it = begin(); it != end(); ++it)
    {
        if (it->second != NULL)
        {
            it->second->setupForTexturePack(texturePack);
        }
    }
}
