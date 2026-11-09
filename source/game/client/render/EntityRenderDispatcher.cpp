#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/AreaEffectCloudRenderer.hpp"
#include "game/client/render/ArmorStandRenderer.hpp"
#include "game/client/render/ArrowRenderer.hpp"
#include "game/client/render/BatRenderer.hpp"
#include "game/client/render/BlazeRenderer.hpp"
#include "game/client/render/BoatRenderer.hpp"
#include "game/client/render/CaveSpiderRenderer.hpp"
#include "game/client/render/ChestedHorseRenderer.hpp"
#include "game/client/render/ChickenModel.hpp"
#include "game/client/render/ChickenRenderer.hpp"
#include "game/client/render/CowModel.hpp"
#include "game/client/render/CowRenderer.hpp"
#include "game/client/render/CreeperRenderer.hpp"
#include "game/client/render/DefaultRenderer.hpp"
#include "game/client/render/DolphinRenderer.hpp"
#include "game/client/render/DragonFireballRenderer.hpp"
#include "game/client/render/DrownedRenderer.hpp"
#include "game/client/render/ElderGuardianRenderer.hpp"
#include "game/client/render/EnderCrystalRenderer.hpp"
#include "game/client/render/EnderDragonRenderer.hpp"
#include "game/client/render/EndermanRenderer.hpp"
#include "game/client/render/EndermiteRenderer.hpp"
#include "game/client/render/EvocationFangsRenderer.hpp"
#include "game/client/render/EvocationIllagerRenderer.hpp"
#include "game/client/render/ExperienceOrbRenderer.hpp"
#include "game/client/render/FallingBlockRenderer.hpp"
#include "game/client/render/FireballRenderer.hpp"
#include "game/client/render/FishRenderer.hpp"
#include "game/client/render/FishingHookRenderer.hpp"
#include "game/client/render/GhastRenderer.hpp"
#include "game/client/render/GiantMobRenderer.hpp"
#include "game/client/render/GuardianRenderer.hpp"
#include "game/client/render/HorseRenderer.hpp"
#include "game/client/render/HuskRenderer.hpp"
#include "game/client/render/IllusionIllagerRenderer.hpp"
#include "game/client/render/ItemEntityRenderer.hpp"
#include "game/client/render/ItemFrameRenderer.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/client/render/LavaSlimeRenderer.hpp"
#include "game/client/render/LeashKnotRenderer.hpp"
#include "game/client/render/LevelRenderer.hpp"
#include "game/client/render/LightningBoltRenderer.hpp"
#include "game/client/render/LlamaRenderer.hpp"
#include "game/client/render/LlamaSpitRenderer.hpp"
#include "game/client/render/MinecartRenderer.hpp"
#include "game/client/render/MinecartSpawnerRenderer.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/MushroomCowRenderer.hpp"
#include "game/client/render/OcelotRenderer.hpp"
#include "game/client/render/PaintingRenderer.hpp"
#include "game/client/render/ParrotRenderer.hpp"
#include "game/client/render/PhantomRenderer.hpp"
#include "game/client/render/PigRenderer.hpp"
#include "game/client/render/PigZombieRenderer.hpp"
#include "game/client/render/PlayerRenderer.hpp"
#include "game/client/render/PolarBearRenderer.hpp"
#include "game/client/render/PufferfishRenderer.hpp"
#include "game/client/render/RabbitRenderer.hpp"
#include "game/client/render/SheepFurModel.hpp"
#include "game/client/render/SheepModel.hpp"
#include "game/client/render/SheepRenderer.hpp"
#include "game/client/render/ShulkerBulletRenderer.hpp"
#include "game/client/render/ShulkerRenderer.hpp"
#include "game/client/render/SilverfishRenderer.hpp"
#include "game/client/render/SkeletonRenderer.hpp"
#include "game/client/render/SlimeRenderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/SnowManRenderer.hpp"
#include "game/client/render/SpectralArrowRenderer.hpp"
#include "game/client/render/SpiderRenderer.hpp"
#include "game/client/render/SquidRenderer.hpp"
#include "game/client/render/StrayRenderer.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/Textures.hpp"
#include "game/client/render/ThrownItemRenderer.hpp"
#include "game/client/render/ThrownPotionRenderer.hpp"
#include "game/client/render/ThrownTridentRenderer.hpp"
#include "game/client/render/TippableArrowRenderer.hpp"
#include "game/client/render/TntMinecartRenderer.hpp"
#include "game/client/render/TntRenderer.hpp"
#include "game/client/render/TropicalFishRenderer.hpp"
#include "game/client/render/TurtleRenderer.hpp"
#include "game/client/render/UndeadHorseRenderer.hpp"
#include "game/client/render/VexRenderer.hpp"
#include "game/client/render/VillagerGolemRenderer.hpp"
#include "game/client/render/VillagerRenderer.hpp"
#include "game/client/render/VindicationIllagerRenderer.hpp"
#include "game/client/render/WitchRenderer.hpp"
#include "game/client/render/WitherBossRenderer.hpp"
#include "game/client/render/WitherSkeletonRenderer.hpp"
#include "game/client/render/WitherSkullRenderer.hpp"
#include "game/client/render/WolfModel.hpp"
#include "game/client/render/WolfRenderer.hpp"
#include "game/client/render/ZombieRenderer.hpp"
#include "game/client/render/ZombieVillagerRenderer.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Culler.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"

namespace
{
}
EntityRenderDispatcher::EntityRenderDispatcher(Textures* textures, ItemRenderer* itemRenderer)
    : boost::unordered_map<unsigned long long, EntityRenderer*>(),
      mPlayerRenderers(),
      mX(0.0),
      mY(0.0),
      mZ(0.0),
      mTextures(textures),
      mCamera(),
      mPlayer(),
      mRenderShadow(true),
      mRenderHitBoxes(false)
{
    GlStateManager::enableLighting();
    (*this)[EntityType::SPIDER] = new SpiderRenderer(this);
    (*this)[EntityType::CAVE_SPIDER] = new CaveSpiderRenderer(this);
    (*this)[EntityType::PIG] = new PigRenderer(this);
    (*this)[EntityType::SHEEP] = new SheepRenderer(this, new SheepModel(), new SheepFurModel(), 0.7F);
    (*this)[EntityType::COW] = new CowRenderer(this, new CowModel(), 0.7F);
    (*this)[EntityType::MUSHROOM_COW] = new MushroomCowRenderer(this);
    (*this)[EntityType::WOLF] = new WolfRenderer(this, new WolfModel(), 0.5F);
    (*this)[EntityType::CHICKEN] = new ChickenRenderer(this, new ChickenModel(), 0.3F);
    (*this)[EntityType::OCELOT] = new OcelotRenderer(this);
    (*this)[EntityType::RABBIT] = new RabbitRenderer(this);
    (*this)[EntityType::PARROT] = new ParrotRenderer(this);
    (*this)[EntityType::SILVERFISH] = new SilverfishRenderer(this);
    (*this)[EntityType::ENDERMITE] = new EndermiteRenderer(this);
    (*this)[EntityType::CREEPER] = new CreeperRenderer(this);
    (*this)[EntityType::ENDER_MAN] = new EndermanRenderer(this);
    (*this)[EntityType::SNOW_MAN] = new SnowManRenderer(this);
    (*this)[EntityType::SKELETON] = new SkeletonRenderer(this);
    (*this)[EntityType::WITHER_SKELETON] = new WitherSkeletonRenderer(this);
    (*this)[EntityType::STRAY] = new StrayRenderer(this);
    (*this)[EntityType::WITCH] = new WitchRenderer(this);
    (*this)[EntityType::BLAZE] = new BlazeRenderer(this);
    (*this)[EntityType::PIG_ZOMBIE] = new PigZombieRenderer(this);
    (*this)[EntityType::ZOMBIE] = new ZombieRenderer(this);
    (*this)[EntityType::ZOMBIE_VILLAGER] = new ZombieVillagerRenderer(this);
    (*this)[EntityType::HUSK] = new HuskRenderer(this);
    (*this)[EntityType::DROWNED] = new DrownedRenderer(this);
    (*this)[EntityType::SLIME] = new SlimeRenderer(this);
    (*this)[EntityType::MAGMA_CUBE] = new LavaSlimeRenderer(this);
    (*this)[EntityType::GIANT] = new GiantMobRenderer(this, 6.0F);
    (*this)[EntityType::GHAST] = new GhastRenderer(this);
    (*this)[EntityType::SQUID] = new SquidRenderer(this);
    (*this)[EntityType::TROPICAL_FISH] = new TropicalFishRenderer(this);
    (*this)[EntityType::FISH] = new FishRenderer(this, FishRenderer::FISH_TYPE_COD, 0.2F);
    (*this)[EntityType::PUFFERFISH] = new PufferfishRenderer(this);
    (*this)[EntityType::SALMON] = new FishRenderer(this, FishRenderer::FISH_TYPE_SALMON, 0.25F);
    (*this)[EntityType::TURTLE] = new TurtleRenderer(this);
    (*this)[EntityType::DOLPHIN] = new DolphinRenderer(this);
    (*this)[EntityType::VILLAGER] = new VillagerRenderer(this);
    (*this)[EntityType::VILLAGER_GOLEM] = new VillagerGolemRenderer(this);
    (*this)[EntityType::BAT] = new BatRenderer(this);
    (*this)[EntityType::GUARDIAN] = new GuardianRenderer(this);
    (*this)[EntityType::ELDER_GUARDIAN] = new ElderGuardianRenderer(this);
    (*this)[EntityType::SHULKER] = new ShulkerRenderer(this);
    (*this)[EntityType::POLAR_BEAR] = new PolarBearRenderer(this);
    (*this)[EntityType::EVOCATION_ILLAGER] = new EvocationIllagerRenderer(this);
    (*this)[EntityType::VINDICATION_ILLAGER] = new VindicationIllagerRenderer(this);
    (*this)[EntityType::VEX] = new VexRenderer(this);
    (*this)[EntityType::PHANTOM] = new PhantomRenderer(this);
    (*this)[EntityType::ILLUSION_ILLAGER] = new IllusionIllagerRenderer(this);
    (*this)[EntityType::ENDER_DRAGON] = new EnderDragonRenderer(this);
    (*this)[EntityType::ENDER_CRYSTAL] = new EnderCrystalRenderer(this);
    (*this)[EntityType::WITHER_BOSS] = new WitherBossRenderer(this);
    (*this)[EntityType::ENTITY] = new DefaultRenderer(this);
    (*this)[EntityType::PAINTING] = new PaintingRenderer(this);
    (*this)[EntityType::ITEM_FRAME] = new ItemFrameRenderer(this, itemRenderer);
    (*this)[EntityType::LEASH_FENCE_KNOT_ENTITY] = new LeashKnotRenderer(this);
    (*this)[EntityType::ARROW] = new ArrowRenderer(this);
    (*this)[EntityType::TIPPABLE_ARROW] = new TippableArrowRenderer(this);
    (*this)[EntityType::SPECTRAL_ARROW] = new SpectralArrowRenderer(this);
    (*this)[EntityType::SNOWBALL] = new ThrownItemRenderer(this, Items::snowball);
    (*this)[EntityType::THROWN_ENDERPEARL] = new ThrownItemRenderer(this, Items::enderPearl);
    (*this)[EntityType::EYE_OF_ENDER_SIGNAL] = new ThrownItemRenderer(this, Items::enderEye);
    (*this)[EntityType::THROWN_EGG] = new ThrownItemRenderer(this, Items::egg);
    (*this)[EntityType::THROWN_POTION] = new ThrownPotionRenderer(this);
    (*this)[EntityType::THROWN_EXP_BOTTLE] = new ThrownItemRenderer(this, Items::experienceBottle);
    (*this)[EntityType::FIREWORKS_ROCKET_ENTITY] = new ThrownItemRenderer(this, Items::fireworks);
    (*this)[EntityType::LARGE_FIREBALL] = new FireballRenderer(this, 2.0F);
    (*this)[EntityType::SMALL_FIREBALL] = new FireballRenderer(this, 0.5F);
    (*this)[EntityType::DRAGON_FIREBALL] = new DragonFireballRenderer(this);
    (*this)[EntityType::WITHER_SKULL] = new WitherSkullRenderer(this);
    (*this)[EntityType::SHULKER_BULLET] = new ShulkerBulletRenderer(this);
    (*this)[EntityType::ITEM_ENTITY] = new ItemEntityRenderer(this, itemRenderer);
    (*this)[EntityType::EXPERIENCE_ORB] = new ExperienceOrbRenderer(this);
    (*this)[EntityType::PRIMED_TNT] = new TntRenderer(this);
    (*this)[EntityType::FALLING_BLOCK_ENTITY] = new FallingBlockRenderer(this);
    (*this)[EntityType::ARMOR_STAND] = new ArmorStandRenderer(this);
    (*this)[EntityType::EVOCATION_FANGS] = new EvocationFangsRenderer(this);
    (*this)[EntityType::MINECART_TNT] = new TntMinecartRenderer(this);
    (*this)[EntityType::MINECART_SPAWNER] = new MinecartSpawnerRenderer(this);
    (*this)[EntityType::MINECART_RIDEABLE] = new MinecartRenderer(this);
    (*this)[EntityType::MINECART_FURNACE] = new MinecartRenderer(this);
    (*this)[EntityType::MINECART_CHEST] = new MinecartRenderer(this);
    (*this)[EntityType::MINECART_HOPPER] = new MinecartRenderer(this);
    (*this)[EntityType::BOAT] = new BoatRenderer(this);
    (*this)[EntityType::FISHING_HOOK] = new FishingHookRenderer(this);
    (*this)[EntityType::AREA_EFFECT_CLOUD] = new AreaEffectCloudRenderer(this);
    (*this)[EntityType::HORSE] = new HorseRenderer(this);
    (*this)[EntityType::SKELETON_HORSE] = new UndeadHorseRenderer(this);
    (*this)[EntityType::ZOMBIE_HORSE] = new UndeadHorseRenderer(this);
    (*this)[EntityType::MULE] = new ChestedHorseRenderer(this, 0.92F);
    (*this)[EntityType::DONKEY] = new ChestedHorseRenderer(this, 0.87F);
    (*this)[EntityType::LLAMA] = new LlamaRenderer(this);
    (*this)[EntityType::LLAMA_SPIT] = new LlamaSpitRenderer(this);
    (*this)[EntityType::LIGHTNING_BOLT] = new LightningBoltRenderer(this);
    (*this)[EntityType::THROWN_TRIDENT] = new ThrownTridentRenderer(this);
    mPlayerRenderer = new PlayerRenderer(this, false, false);
    const EPlayerModelType defaultModel = EPLAYER_MODEL_DEFAULT;
    mPlayerRenderers[defaultModel] = mPlayerRenderer;
    mPlayerRenderers[EPLAYER_MODEL_SLIM] = new PlayerRenderer(this, false, true);
    mPlayerRenderers[EPLAYER_MODEL_SLIM_64] = new PlayerRenderer(this, true, true);
    GlStateManager::disableLighting();
    mRenderingGuiPlayer = false;
    mRenderLeash = false;
    mSolidRendering = false;
}

double EntityRenderDispatcher::distanceToSqr(double x, double y, double z)
{
    x -= mCameraX;
    y -= mCameraY;
    z -= mCameraZ;
    return x * x + y * y + z * z;
}

unsigned char EntityRenderDispatcher::shouldRenderShadow()
{
    return mRenderShadow;
}

Font* EntityRenderDispatcher::getFont()
{
    return mFont;
}

void EntityRenderDispatcher::setPosition(double x, double y, double z)
{
    mX = x;
    mY = y;
    mZ = z;
}

EntityRenderer* EntityRenderDispatcher::getRenderer(unsigned long long type)
{
    bool isPlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, type);
    if (isPlayer)
        return mPlayerRenderer;
    iterator it = find(type);
    return it->second;
}

PlayerRenderer* EntityRenderDispatcher::getPlayerRenderer(EPlayerModelType modelType)
{
    boost::unordered_map<EPlayerModelType, PlayerRenderer*>::iterator it = mPlayerRenderers.find(modelType);
    if (it != mPlayerRenderers.end())
        return it->second;
    return mPlayerRenderer;
}

EntityRenderer* EntityRenderDispatcher::getRenderer(boost::shared_ptr<Entity> entity)
{
    bool isPlayer = eTYPE_DERIVED_FROM(EntityType::PLAYER, entity->GetType());
    if (isPlayer)
    {
        boost::shared_ptr<Player> player = boost::static_pointer_cast<Player>(entity);
        if (player->isSpectator() && player->SpectateAsOtherEntity())
            return getRenderer(player->GetSpectateEntity());
        return getPlayerRenderer(static_cast<EPlayerModelType>(player->getModelName()));
    }

    bool isFireworks = eTYPE_DERIVED_FROM(EntityType::FIREWORKS_ROCKET_ENTITY, entity->GetType());
    if (isFireworks)
        return getRenderer(EntityType::ARROW);

    return getRenderer(entity->GetType());
}

void EntityRenderDispatcher::prepare(Level* level, Font* font, boost::shared_ptr<Entity> camera,
                                     boost::shared_ptr<Entity> player, Options* options, float partialTicks)
{
    mLevel = level;
    mOptions = options;
    mCamera = camera;
    mFont = font;
    mPlayer = player;

    bool isLiving = eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, camera->GetType());
    if (isLiving && boost::static_pointer_cast<LivingEntity>(camera)->isSleeping())
    {
        const BlockState* state = level->getBlockState(BlockPos(camera));
        if (state->getBlock() == Blocks::bed)
        {
            mCameraYRot = (float)(state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->get2DDataValue() * 90
                                  + 180);
            mCameraXRot = 0.0F;
        }
    }
    else
    {
        mCameraYRot = Minecraft::GetInstance()->mPlayer->mYRotO + (camera->mYRot - camera->mYRotO) * partialTicks;
        mCameraXRot = Minecraft::GetInstance()->mPlayer->mXRotO + (camera->mXRot - camera->mXRotO) * partialTicks;
    }

    if ((Minecraft::GetInstance()->mPlayer->GetType() & EntityType::PLAYER) == EntityType::PLAYER
        && boost::static_pointer_cast<Player>(camera)->GetThirdPersonView(false) == 2)
    {
        mCameraYRot += 180.0F;
        mCameraXRot = -((camera->mXRot - camera->mXRotO) * partialTicks + camera->mXRotO);
    }

    mCameraX = (camera->mX - camera->mXOld) * partialTicks + camera->mXOld;
    mCameraY = (camera->mY - camera->mYOld) * partialTicks + camera->mYOld;
    mCameraZ = (camera->mZ - camera->mZOld) * partialTicks + camera->mZOld;
}

bool EntityRenderDispatcher::hasSecondPass(boost::shared_ptr<Entity>& entity)
{
    return getRenderer(entity)->hasSecondPass();
}

bool EntityRenderDispatcher::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double x, double y,
                                          double z)
{
    EntityRenderer* renderer = getRenderer(entity);
    if (renderer != NULL && renderer->shouldRender(entity, culler, x, y, z))
        return true;
    return false;
}

bool EntityRenderDispatcher::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double distance)
{
    EntityRenderer* renderer = getRenderer(entity);
    if (renderer != NULL && renderer->shouldRender(entity, culler, distance))
        return true;
    return false;
}

void EntityRenderDispatcher::renderHitbox(boost::shared_ptr<Entity>& entity, double x, double y, double z, float,
                                          float partialTicks, int color)
{
    GlStateManager::depthMask(false);
    GlStateManager::disableTexture();
    GlStateManager::disableLighting();
    GlStateManager::disableCull();
    GlStateManager::disableBlend();
    GlStateManager::pushMatrix();

    float halfWidth = entity->mBbWidth * 0.5F;
    AABB* box = entity->mBoundingBox;
    LevelRenderer::renderLineBox(AABB::newTemp(box->mMinX - entity->mX + x, box->mMinY - entity->mY + y,
                                               box->mMinZ - entity->mZ + z, box->mMaxX - entity->mX + x,
                                               box->mMaxY - entity->mY + y, box->mMaxZ - entity->mZ + z),
                                 color);

    if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entity->GetType()))
    {
        LevelRenderer::renderLineBox(AABB::newTemp(x - halfWidth, y + entity->getEyeHeight() - 0.01F, z - halfWidth,
                                                   x + halfWidth, y + entity->getEyeHeight() + 0.01F, z + halfWidth),
                                     0xFFFFFF);
    }

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    Vec3* look = entity->getViewVector(partialTicks);
    builder->begin(5);
    builder->color(0xFF);
    builder->vertex((float)x, (float)(y + entity->getEyeHeight()), (float)z);
    builder->vertex((float)(x + look->x * 2.0), (float)(y + entity->getEyeHeight() + look->y * 2.0),
                    (float)(z + look->z * 2.0));
    builder->end();

    GlStateManager::popMatrix();
    GlStateManager::enableTexture();
    GlStateManager::enableLighting();
    GlStateManager::enableCull();
    GlStateManager::disableBlend();
    GlStateManager::depthMask(true);
}

void EntityRenderDispatcher::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                    float partialTicks, bool renderOutlines, bool itemFrame, bool renderShadow,
                                    bool renderLeash)
{
    EntityRenderer* renderer = getRenderer(entity);
    if (renderer == NULL || mTextures == NULL)
        return;

    renderer->SetItemFrame(itemFrame);
    renderer->setSolidRender(mSolidRendering);
    mRenderLeash = renderLeash;
    renderer->render(entity, x, y, z, yaw, partialTicks);
    mRenderLeash = false;

    if (!mSolidRendering)
        renderer->postRender(entity, x, y, z, yaw, partialTicks, renderShadow);

    if (mRenderHitBoxes && !entity->isInvisible() && !renderOutlines
        && !Minecraft::GetInstance()->showOnlyReducedInfo())
    {
        renderHitbox(entity, x, y, z, yaw, partialTicks, 0xFFFFFF);
    }
}

void EntityRenderDispatcher::render(boost::shared_ptr<Entity>& entity, float partialTicks, bool renderOutlines)
{
    double x = (entity->mX - entity->mXOld) * partialTicks + entity->mXOld - mX;
    double y = (entity->mY - entity->mYOld) * partialTicks + entity->mYOld - mY;
    double z = (entity->mZ - entity->mZOld) * partialTicks + entity->mZOld - mZ;

    float rot = entity->mYRot - entity->mYRotO;
    if (rot > 180.0F || rot < -180.0F)
    {
        if (entity->mYRotO < entity->mYRot)
            rot = entity->mYRot - 360.0F - entity->mYRotO;
        else
            rot = entity->mYRot - (entity->mYRotO - 360.0F);
    }
    float yaw = rot * partialTicks + entity->mYRotO;

    int light = entity->getLightColor();
    if (entity->isOnFire())
        light = 0x00F000F0;
    GlStateManager::multiTexCoord2f((float)(light % 65536), (float)(light / 65536));
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);

    render(entity, x, y, z, yaw, partialTicks, renderOutlines,
           eTYPE_DERIVED_FROM(EntityType::ITEM_FRAME, entity->GetType()), entity->ShouldRenderShadow(), false);
}

void EntityRenderDispatcher::renderSecondPass(boost::shared_ptr<Entity>& entity, float partialTicks)
{
    if (entity->mTickCount == 0)
    {
        entity->mXOld = entity->mX;
        entity->mYOld = entity->mY;
        entity->mZOld = entity->mZ;
    }

    double x = (entity->mX - entity->mXOld) * partialTicks + entity->mXOld - mX;
    double y = (entity->mY - entity->mYOld) * partialTicks + entity->mYOld - mY;
    double z = (entity->mZ - entity->mZOld) * partialTicks + entity->mZOld - mZ;
    float yaw = (entity->mYRot - entity->mYRotO) * partialTicks + entity->mYRotO;

    int light = entity->getLightColor();
    if (entity->isOnFire())
        light = 0x00F000F0;
    GlStateManager::multiTexCoord2f((float)(light % 65536), (float)(light / 65536));
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);

    EntityRenderer* renderer = getRenderer(entity);
    if (renderer != NULL && mTextures != NULL)
        renderer->renderSecondPass(entity, x, y, z, yaw, partialTicks);
}

void EntityRenderDispatcher::setupForTexturePack(TexturePack* texturePack)
{
    for (iterator it = begin(); it != end(); ++it)
        it->second->setupForTexturePack(texturePack);

    boost::unordered_map<EPlayerModelType, PlayerRenderer*>::iterator player = mPlayerRenderers.begin();
    for (; player != mPlayerRenderers.end(); ++player)
        player->second->setupForTexturePack(texturePack);
}
