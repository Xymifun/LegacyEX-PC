#include "game/client/render/MinecartRenderer.hpp"

#include <cmath>

#include "game/client/render/BlockRenderDispatcher.hpp"
#include "game/client/render/MinecartModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Minecart.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/core/Mth.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/Block.hpp"

static ResourceLocation MINECART_LOCATION(TN_ITEM_CART);

MinecartRenderer::MinecartRenderer(EntityRenderDispatcher* dispatcher) : EntityRenderer(dispatcher)
{
    mShadowRadius = 0.5F;
    mModel = new MinecartModel();
}

void MinecartRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                              float partialTick)
{
    boost::shared_ptr<Minecart> minecart = boost::static_pointer_cast<Minecart>(entity);
    GlStateManager::pushMatrix();
    bindTexture(entity);

    long long seed = (int)((unsigned int)minecart->getId() * 493286711U);
    seed = seed * seed * 4392167121LL + seed * 98761LL;
    float offsetX = (((float)(seed >> 16 & 7) + 0.5F) / 8.0F - 0.5F) * 0.004F;
    float offsetY = (((float)(seed >> 20 & 7) + 0.5F) / 8.0F - 0.5F) * 0.004F;
    float offsetZ = (((float)(seed >> 24 & 7) + 0.5F) / 8.0F - 0.5F) * 0.004F;
    GlStateManager::translatef(offsetX, offsetY, offsetZ);

    double renderX = minecart->mXOld + (minecart->mX - minecart->mXOld) * partialTick;
    double renderY = minecart->mYOld + (minecart->mY - minecart->mYOld) * partialTick;
    double renderZ = minecart->mZOld + (minecart->mZ - minecart->mZOld) * partialTick;
    Vec3* trackPosition = minecart->getPos(renderX, renderY, renderZ);
    float pitch = minecart->mXRotO + (minecart->mXRot - minecart->mXRotO) * partialTick;
    if (trackPosition != NULL)
    {
        Vec3* frontPosition = minecart->getPosOffs(renderX, renderY, renderZ, 0.3f);
        Vec3* backPosition = minecart->getPosOffs(renderX, renderY, renderZ, -0.3f);
        if (frontPosition == NULL)
            frontPosition = trackPosition;
        if (backPosition == NULL)
            backPosition = trackPosition;
        x += trackPosition->x - renderX;
        y += (frontPosition->y + backPosition->y) * 0.5 - renderY;
        z += trackPosition->z - renderZ;
        Vec3* direction = backPosition->add(-frontPosition->x, -frontPosition->y, -frontPosition->z);
        if (direction->length() != 0.0)
        {
            direction = direction->normalize();
            yaw = (float)(std::atan2(direction->z, direction->x) * 180.0 / 3.1415927410125732);
            pitch = (float)(std::atan(direction->y) * 73.0);
        }
    }

    GlStateManager::translatef((float)x, (float)y + 0.375F, (float)z);
    GlStateManager::rotatef(180.0F - yaw, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef(-pitch, 0.0F, 0.0F, 1.0F);

    float hurtTime = (float)minecart->getHurtTime() - partialTick;
    float damage = minecart->getDamage() - partialTick;
    if (damage < 0.0F)
        damage = 0.0F;
    if (hurtTime > 0.0F)
    {
        GlStateManager::rotatef(Mth::sin(hurtTime) * hurtTime * damage / 10.0F * minecart->getHurtDir(), 1.0F, 0.0F,
                                0.0F);
    }

    int displayOffset = minecart->getDisplayOffset();
    if (mSolidRender)
    {
        GlStateManager::enableColorMaterial();
        GlStateManager::setupSolidRenderingTextureCombine((int)getTeamColor(entity));
    }

    const BlockState* state = minecart->getDisplayBlockState();
    if (state->getBlock()->defaultBlockState()->getRenderShape() != BLOCK_RENDER_INVISIBLE)
    {
        GlStateManager::pushMatrix();
        bindTexture(&BLOCKS_LOCATION);
        GlStateManager::scalef(0.75F, 0.75F, 0.75F);
        GlStateManager::translatef(0.0F, (float)displayOffset * 0.0625F, 0.0F);
        renderMinecartContents(minecart, partialTick, state);
        GlStateManager::popMatrix();
        GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
        bindTexture(entity);
    }

    GlStateManager::scalef(-1.0F, -1.0F, 1.0F);
    static_cast<MinecartModel*>(mModel)->render(entity, 0.0F, 0.0F, -0.1F, 0.0F, 0.0F, 0.0625F, true);
    GlStateManager::popMatrix();

    if (mSolidRender)
    {
        GlStateManager::tearDownSolidRenderingTextureCombine();
        GlStateManager::disableColorMaterial();
    }

    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* MinecartRenderer::getTextureLocation(boost::shared_ptr<Entity>&)
{
    return &MINECART_LOCATION;
}

void MinecartRenderer::renderMinecartContents(boost::shared_ptr<Minecart>, float, const BlockState* state)
{
    GlStateManager::pushMatrix();
    Minecraft::GetInstance()->getBlockRenderer()->renderSingleBlock(state, 1.0F, 1.0F, true);
    GlStateManager::popMatrix();
}

Model* MinecartRenderer::createModel(const ModelDefinition& definition)
{
    return new MinecartModel(definition);
}
