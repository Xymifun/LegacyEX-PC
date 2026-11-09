#include "game/client/render/BoatRenderer.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>

#include "game/client/render/BoatModel.hpp"
#include "game/client/render/Model.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/Boat.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Culler.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcGoldenAge.hpp"
#include "platform/pc/PcSettings.hpp"
#endif

static ResourceLocation BOAT_TEXTURES[6] = {
    ResourceLocation(TN_ITEM_BOAT_BOAT_OAK),    ResourceLocation(TN_ITEM_BOAT_BOAT_SPRUCE),
    ResourceLocation(TN_ITEM_BOAT_BOAT_BIRCH),  ResourceLocation(TN_ITEM_BOAT_BOAT_JUNGLE),
    ResourceLocation(TN_ITEM_BOAT_BOAT_ACACIA), ResourceLocation(TN_ITEM_BOAT_BOAT_DARKOAK)};

BoatRenderer::BoatRenderer(EntityRenderDispatcher* dispatcher) : EntityRenderer(dispatcher)
{
    mShadowRadius = 0.5F;
    mModel = new BoatModel();
}

BoatRenderer::~BoatRenderer()
{
}

void BoatRenderer::setupTranslation(double x, double y, double z)
{
    GlStateManager::translatef((float)x, (float)y + 0.375F, (float)z);
}

void BoatRenderer::setupRotation(boost::shared_ptr<Entity> entity, float yaw, float partialTick)
{
    boost::shared_ptr<Boat> boat = boost::static_pointer_cast<Boat>(entity);
    GlStateManager::rotatef(180.0F - yaw, 0.0F, 1.0F, 0.0F);
    float hurtTime = (float)boat->getHurtTime() - partialTick;
    float damage = boat->getDamage() - partialTick;
    if (damage < 0.0F)
        damage = 0.0F;
    if (hurtTime > 0.0F)
    {
        GlStateManager::rotatef(std::sin(hurtTime) * hurtTime * damage / 10.0F * boat->getHurtDir(), 1.0F, 0.0F, 0.0F);
    }
    float bubbleAngle = boat->getBubbleAngle(partialTick);
    if (!Mth::equal(bubbleAngle, 0.0F))
        GlStateManager::rotatef(boat->getBubbleAngle(partialTick), 1.0F, 0.0F, 1.0F);
    GlStateManager::scalef(-1.0F, -1.0F, 1.0F);
}

void BoatRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw, float partialTick)
{
    boost::shared_ptr<Boat> boat = boost::static_pointer_cast<Boat>(entity);
    GlStateManager::pushMatrix();
    setupTranslation(x, y, z);
    setupRotation(entity, yaw, partialTick);
    bindTexture(entity);
    if (mSolidRender)
    {
        GlStateManager::enableColorMaterial();
        GlStateManager::setupSolidRenderingTextureCombine((int)getTeamColor(entity));
    }
    mModel->render(entity, partialTick, 0.0F, -0.1F, 0.0F, 0.0F, 0.0625F, true);
    if (mSolidRender)
    {
        GlStateManager::tearDownSolidRenderingTextureCombine();
        GlStateManager::disableColorMaterial();
    }
    GlStateManager::popMatrix();
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
    if (!mSolidRender)
    {
        const bool isBoat = eTYPE_DERIVED_FROM(EntityType::BOAT, entity->GetType());
        if (isBoat)
        {
            boost::shared_ptr<Boat> leashedBoat = boost::static_pointer_cast<Boat>(entity);
            renderLeash(leashedBoat, x, y, z, yaw, partialTick);
        }
    }
}

const ResourceLocation* BoatRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<Boat> boat = boost::static_pointer_cast<Boat>(entity);
#ifdef MINECRAFT_PC
    if (PcGoldenAgeOn(kPcGoldenAgeModels))
        return &BOAT_TEXTURES[0];
#endif
    return &BOAT_TEXTURES[boat->getType()];
}

bool BoatRenderer::hasSecondPass()
{
    return true;
}

void BoatRenderer::renderSecondPass(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                    float partialTick)
{
    GlStateManager::pushMatrix();
    setupTranslation(x, y, z);
    setupRotation(entity, yaw, partialTick);
    bindTexture(entity);
    static_cast<BoatModel*>(mModel)->renderSecondPass(entity, partialTick, 0.0F, -0.1F, 0.0F, 0.0F, 0.0625F, true);
    GlStateManager::popMatrix();
}

double BoatRenderer::lerp(double oldValue, double newValue, double partialTick)
{
    return oldValue + (newValue - oldValue) * partialTick;
}

void BoatRenderer::renderLeash(boost::shared_ptr<Boat>& boat, double x, double y, double z, float yaw,
                               float partialTick)
{
    if (!boat)
        return;
    boost::shared_ptr<Entity> holder = boat->getLeashHolder();
    if (!holder)
        return;
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    y -= boat->mBbHeight;
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    double holderYaw = lerp(holder->mYRotO, holder->mYRot, partialTick * 0.5F) * Mth::DEG_TO_RAD;
    double holderPitch = lerp(holder->mXRotO, holder->mXRot, partialTick * 0.5F) * Mth::DEG_TO_RAD;
    double cosYaw = std::cos(holderYaw);
    double sinYaw = std::sin(holderYaw);
    double sinPitch = std::sin(holderPitch);
    if (eTYPE_DERIVED_FROM(EntityType::HANGING_ENTITY, holder->GetType()))
    {
        sinYaw = 0.0;
        sinPitch = -1.0;
        cosYaw = 0.0;
    }
    double cosPitch = std::cos(holderPitch);
    double holderX = lerp(holder->mXo, holder->mX, partialTick);
    double holderY = lerp(holder->mYo + holder->getEyeHeight() * 0.7, holder->mY + holder->getEyeHeight() * 0.7,
                          partialTick);
    double holderZ = lerp(holder->mZo, holder->mZ, partialTick);
    double boatX = lerp(boat->mXo, boat->mX, partialTick);
    double boatY = lerp(boat->mYo, boat->mY, partialTick);
    double boatZ = lerp(boat->mZo, boat->mZ, partialTick);
    double dx = (float)((holderX - cosYaw * 0.7 - sinYaw * 0.5 * cosPitch) - (boatX + cosYaw));
    double dy = (float)((holderY - sinPitch * 0.5 - 0.25) - boatY);
    double dz = (float)((holderZ - sinYaw * 0.7 + cosYaw * 0.5 * cosPitch) - (boatZ + sinYaw));
    GlStateManager::disableTexture();
    GlStateManager::disableLighting();
    GlStateManager::disableCull();
    unsigned int lightColour = Minecraft::GetInstance()->getColourTable()->getColour(
        eMinecraftColour_Leash_Light_Colour);
    const float lightRed = (float)((lightColour >> 16) & 255) / 255.0F;
    const float lightGreen = (float)((lightColour >> 8) & 255) / 255.0F;
    const float lightBlue = (float)(lightColour & 255) / 255.0F;
    unsigned int darkColour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Leash_Dark_Colour);
    const float darkRed = (float)((darkColour >> 16) & 255) / 255.0F;
    const float darkGreen = (float)((darkColour >> 8) & 255) / 255.0F;
    const float darkBlue = (float)(darkColour & 255) / 255.0F;
    builder->begin(1);
    for (int i = 0; i < 25; ++i)
    {
        double red = lightRed;
        double green = lightGreen;
        double blue = lightBlue;
        if ((i & 1) == 0)
        {
            red = darkRed;
            green = darkGreen;
            blue = darkBlue;
        }
        builder->color(red, green, blue, 1.0F);
        double t = (float)i / 24.0F;
        double px = x + dx * t;
        double py = dy * (float)(t * t + t) * 0.5 + y + (double)((24.0F - (float)i) / 18.0F + 0.125F);
        double pz = z + dz * t;
        builder->vertex((float)px, (float)py, (float)pz);
        builder->vertex((float)(px + 0.025), (float)(py + 0.025), (float)pz);
    }
    builder->end();
    builder->begin(1);
    for (int i = 0; i < 25; ++i)
    {
        double red = lightRed;
        double green = lightGreen;
        double blue = lightBlue;
        if ((i & 1) == 0)
        {
            red = darkRed;
            green = darkGreen;
            blue = darkBlue;
        }
        builder->color(red, green, blue, 1.0F);
        double t = (float)i / 24.0F;
        double px = x + dx * t;
        double py = dy * (float)(t * t + t) * 0.5 + y + (double)((24.0F - (float)i) / 18.0F + 0.125F);
        double pz = z + dz * t;
        builder->vertex((float)px, (float)(py + 0.025), (float)pz);
        builder->vertex((float)(px + 0.025), (float)py, (float)(pz + 0.025));
    }
    builder->end();
    GlStateManager::enableLighting();
    GlStateManager::enableTexture();
    GlStateManager::enableCull();
}

bool BoatRenderer::shouldRender(boost::shared_ptr<Entity>& entity, Culler* culler, double x, double y, double z)
{
    if (EntityRenderer::shouldRender(entity, culler, x, y, z))
        return true;
    boost::shared_ptr<Boat> boat = boost::static_pointer_cast<Boat>(entity);
    if (boat && boat->isLeashed() && boat->getLeashHolder())
    {
        boost::shared_ptr<Entity> holder = boat->getLeashHolder();
        return culler->isVisible(holder->getBoundingBoxForCulling());
    }
    return false;
}

Model* BoatRenderer::createModel(const ModelDefinition& definition)
{
    return new BoatModel(definition);
}
