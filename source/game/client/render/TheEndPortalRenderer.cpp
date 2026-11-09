#include "game/client/render/TheEndPortalRenderer.hpp"

#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Direction.hpp"
#include "game/core/FloatBuffer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/System.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/TheEndPortalBlockEntity.hpp"

#include <boost/shared_ptr.hpp>

static ResourceLocation END_SKY_LOCATION(TN_MISC_TUNNEL);
static ResourceLocation END_PORTAL_LOCATION(TN_MISC_PARTICLEFIELD);

static int RANDOM_SEED = 31100;
static Random RANDOM(RANDOM_SEED);

static FloatBuffer* MODELVIEW = MemoryTracker::createFloatBuffer(16);
static FloatBuffer* PROJECTION = MemoryTracker::createFloatBuffer(16);

TheEndPortalRenderer::TheEndPortalRenderer()
{
    mBuffer = MemoryTracker::createFloatBuffer(16);
}

FloatBuffer* TheEndPortalRenderer::getBuffer(float x, float y, float z, float w)
{
    mBuffer->clear();
    mBuffer->put(x)->put(y)->put(z)->put(w);
    mBuffer->flip();

    return mBuffer;
}

void TheEndPortalRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a,
                                  int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList)
{
    boost::shared_ptr<TheEndPortalBlockEntity> portal = boost::static_pointer_cast<TheEndPortalBlockEntity>(
        blockEntity);

    GlStateManager::disableLighting();

    RANDOM.setSeed(RANDOM_SEED);

    GlStateManager::getMatrix(GL_MODELVIEW_MATRIX, MODELVIEW);
    GlStateManager::getMatrix(GL_PROJECTION_MATRIX, PROJECTION);

    int passes = getPasses(x * x + y * y + z * z);
    double offset = getOffset();

    bool fogColorReset = false;

    for (int i = 0; i < passes; ++i)
    {
        GlStateManager::pushMatrix();

        float brightness = 2.0F / (float)(18 - i);

        if (i == 0)
        {
            bindTexture(&END_SKY_LOCATION);
            brightness = 0.15F;
            GlStateManager::enableBlend();
            GlStateManager::blendFunc(4, 5);
        }

        else if (i > 0)
        {
            bindTexture(&END_PORTAL_LOCATION);
            fogColorReset = true;
            Minecraft::GetInstance()->mGameRenderer->resetFogColor(true);

            if (i == 1)
            {
                GlStateManager::enableBlend();
                GlStateManager::blendFunc(1, 1);
            }
        }

        GlStateManager::texGenMode(GlStateManager::S, GL_EYE_LINEAR);
        GlStateManager::texGenMode(GlStateManager::T, GL_EYE_LINEAR);
        GlStateManager::texGenMode(GlStateManager::R, GL_EYE_LINEAR);

        GlStateManager::texGenParam(GlStateManager::S, GL_EYE_PLANE, getBuffer(1.0F, 0.0F, 0.0F, 0.0F));
        GlStateManager::texGenParam(GlStateManager::T, GL_EYE_PLANE, getBuffer(0.0F, 1.0F, 0.0F, 0.0F));
        GlStateManager::texGenParam(GlStateManager::R, GL_EYE_PLANE, getBuffer(0.0F, 0.0F, 1.0F, 0.0F));
        GlStateManager::texGenParam(GlStateManager::Q, GL_OBJECT_PLANE, getBuffer(0.0F, 0.0F, 0.0F, 1.0F));

        GlStateManager::enableTexGen(GlStateManager::S);
        GlStateManager::enableTexGen(GlStateManager::T);
        GlStateManager::enableTexGen(GlStateManager::R);

        GlStateManager::popMatrix();

        GlStateManager::matrixMode(GL_TEXTURE);
        GlStateManager::pushMatrix();
        GlStateManager::loadIdentity();
        GlStateManager::translatef(0.5F, 0.5F, 0.0F);
        GlStateManager::scalef(0.5F, 0.5F, 1.0F);

        float layer = (float)(i + 1);

        double timeFraction = (float)(System::processTimeInMilliSecs() % 800000LL) / 800000.0F;

        GlStateManager::translatef(17.0F / layer, (2.0F + layer / 1.5F) * timeFraction, 0.0F);
        GlStateManager::rotatef((layer * layer * 4321.0F + layer * 9.0F) * 2.0F, 0.0F, 0.0F, 1.0F);

        GlStateManager::scalef(4.5F - layer / 4.0F, 4.5F - layer / 4.0F, 1.0F);
        GlStateManager::multMatrix(PROJECTION);
        GlStateManager::multMatrix(MODELVIEW);

        BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

        builder->useProjectedTexture(true);
        builder->begin();

        float red = RANDOM.nextFloat() * 0.5F + 0.1F;
        float green = RANDOM.nextFloat() * 0.5F + 0.4F;
        float blue = RANDOM.nextFloat() * 0.5F + 0.5F;

        if (i == 0)
        {
            red = 1.0F;
            green = 1.0F;
            blue = 1.0F;
        }

        builder->color(red * brightness, green * brightness, blue * brightness, 1.0F);

        if (portal->shouldRenderFace(Direction::SOUTH))
        {
            builder->vertexDouble(x, y, z + 1.0);
            builder->vertexDouble(x + 1.0, y, z + 1.0);
            builder->vertexDouble(x + 1.0, y + 1.0, z + 1.0);
            builder->vertexDouble(x, y + 1.0, z + 1.0);
        }

        if (portal->shouldRenderFace(Direction::NORTH))
        {
            builder->vertexDouble(x, y + 1.0, z);
            builder->vertexDouble(x + 1.0, y + 1.0, z);
            builder->vertexDouble(x + 1.0, y, z);
            builder->vertexDouble(x, y, z);
        }

        if (portal->shouldRenderFace(Direction::EAST))
        {
            builder->vertexDouble(x + 1.0, y + 1.0, z);
            builder->vertexDouble(x + 1.0, y + 1.0, z + 1.0);
            builder->vertexDouble(x + 1.0, y, z + 1.0);
            builder->vertexDouble(x + 1.0, y, z);
        }

        if (portal->shouldRenderFace(Direction::WEST))
        {
            builder->vertexDouble(x, y, z);
            builder->vertexDouble(x, y, z + 1.0);
            builder->vertexDouble(x, y + 1.0, z + 1.0);
            builder->vertexDouble(x, y + 1.0, z);
        }

        if (portal->shouldRenderFace(Direction::DOWN))
        {
            builder->vertexDouble(x, y, z);
            builder->vertexDouble(x + 1.0, y, z);
            builder->vertexDouble(x + 1.0, y, z + 1.0);
            builder->vertexDouble(x, y, z + 1.0);
        }

        if (portal->shouldRenderFace(Direction::UP))
        {
            builder->vertexDouble(x, y + offset, z + 1.0);
            builder->vertexDouble(x + 1.0, y + offset, z + 1.0);
            builder->vertexDouble(x + 1.0, y + offset, z);
            builder->vertexDouble(x, y + offset, z);
        }

        builder->end();
        builder->useProjectedTexture(false);

        GlStateManager::popMatrix();
        GlStateManager::matrixMode(GL_MODELVIEW);
    }

    GlStateManager::disableBlend();

    GlStateManager::disableTexGen(GlStateManager::S);
    GlStateManager::disableTexGen(GlStateManager::T);
    GlStateManager::disableTexGen(GlStateManager::R);

    GlStateManager::enableLighting();

    if (fogColorReset)
    {
        Minecraft::GetInstance()->mGameRenderer->resetFogColor(false);
    }
}

int TheEndPortalRenderer::getPasses(double distanceSqr)
{
    if (distanceSqr > 36864.0)
    {
        return 1;
    }

    if (distanceSqr > 25600.0)
    {
        return 3;
    }

    if (distanceSqr > 16384.0)
    {
        return 5;
    }

    if (distanceSqr > 9216.0)
    {
        return 7;
    }

    if (distanceSqr > 4096.0)
    {
        return 9;
    }

    if (distanceSqr > 1024.0)
    {
        return 11;
    }

    if (distanceSqr > 576.0)
    {
        return 13;
    }

    if (distanceSqr > 256.0)
    {
        return 14;
    }

    return 15;
}

double TheEndPortalRenderer::getOffset()
{
    float offset = 0.75F;
    return offset;
}
