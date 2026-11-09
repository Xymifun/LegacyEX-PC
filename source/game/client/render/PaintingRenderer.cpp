#include "game/client/render/PaintingRenderer.hpp"

#include <boost/shared_ptr.hpp>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/core/Painting.hpp"
#include "game/util/Random.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/Level.hpp"

static ResourceLocation PAINTING_LOCATION(TN_ART_KZ);

PaintingRenderer::PaintingRenderer(EntityRenderDispatcher* dispatcher)
    : EntityRenderer(dispatcher),
      mRandom(new Random())
{
}

PaintingRenderer::~PaintingRenderer()
{
}

void PaintingRenderer::setBrightness(boost::shared_ptr<Painting> painting, float x, float z)
{
    int lightX = Mth::floor(painting->mX);
    int lightY = Mth::floor(painting->mY + (double)(z * 0.0625F));
    int lightZ = Mth::floor(painting->mZ);

    const Direction* direction = painting->mDirection;
    if (direction == Direction::NORTH)
        lightX = Mth::floor(painting->mX + (double)(x * 0.0625F));
    if (direction == Direction::WEST)
        lightZ = Mth::floor(painting->mZ - (double)(x * 0.0625F));
    if (direction == Direction::SOUTH)
        lightX = Mth::floor(painting->mX - (double)(x * 0.0625F));
    if (direction == Direction::EAST)
        lightZ = Mth::floor(painting->mZ + (double)(x * 0.0625F));

    int light = mDispatcher->mLevel->getLightColor(BlockPos(lightX, lightY, lightZ), 0, -1);
    GlStateManager::multiTexCoord2f((float)(light % 65536), (float)(light / 65536));
    GlStateManager::color3f(1.0F, 1.0F, 1.0F);
}

void PaintingRenderer::renderPainting(boost::shared_ptr<Painting> painting, int width, int height, int offsetX,
                                      int offsetY)
{
    float xStart = -width * 0.5F;
    float yStart = -height * 0.5F;
    float depth = 0.5F;
    float backU0 = 0.75F;
    float backU1 = 0.8125F;
    float backV0 = 0.0F;
    float backV1 = 0.0625F;
    float edgeU0 = 0.75F;
    float edgeU1 = 0.8125F;
    float edgeV0 = 0.001953125F;
    float edgeV1 = 0.001953125F;
    float sideU0 = 0.751953125F;
    float sideU1 = 0.751953125F;
    float sideV0 = 0.0F;
    float sideV1 = 0.0625F;

    for (int xs = 0; xs < width / 16; ++xs)
    {
        for (int ys = 0; ys < height / 16; ++ys)
        {
            float x0 = xStart + (float)((xs + 1) * 16);
            float x1 = xStart + (float)(xs * 16);
            float y0 = yStart + (float)((ys + 1) * 16);
            float y1 = yStart + (float)(ys * 16);
            setBrightness(painting, (x0 + x1) * 0.5F, (y0 + y1) * 0.5F);
            float u0 = (float)(offsetX + width - xs * 16) * 0.00390625F;
            float u1 = (float)(offsetX + width - (xs + 1) * 16) * 0.00390625F;
            float v0 = (float)(offsetY + height - ys * 16) * 0.00390625F;
            float v1 = (float)(offsetY + height - (ys + 1) * 16) * 0.00390625F;
            BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
            builder->begin();
            builder->normal(0.0F, 0.0F, -1.0F);
            builder->vertexUV(x0, y1, -depth, u1, v0);
            builder->vertexUV(x1, y1, -depth, u0, v0);
            builder->vertexUV(x1, y0, -depth, u0, v1);
            builder->vertexUV(x0, y0, -depth, u1, v1);
            builder->normal(0.0F, 0.0F, 1.0F);
            builder->vertexUV(x0, y0, depth, backU0, backV0);
            builder->vertexUV(x1, y0, depth, backU1, backV0);
            builder->vertexUV(x1, y1, depth, backU1, backV1);
            builder->vertexUV(x0, y1, depth, backU0, backV1);
            builder->normal(0.0F, 1.0F, 0.0F);
            builder->vertexUV(x0, y0, -depth, edgeU0, edgeV0);
            builder->vertexUV(x1, y0, -depth, edgeU1, edgeV0);
            builder->vertexUV(x1, y0, depth, edgeU1, edgeV1);
            builder->vertexUV(x0, y0, depth, edgeU0, edgeV1);
            builder->normal(0.0F, -1.0F, 0.0F);
            builder->vertexUV(x0, y1, depth, edgeU0, edgeV0);
            builder->vertexUV(x1, y1, depth, edgeU1, edgeV0);
            builder->vertexUV(x1, y1, -depth, edgeU1, edgeV1);
            builder->vertexUV(x0, y1, -depth, edgeU0, edgeV1);
            builder->normal(-1.0F, 0.0F, 0.0F);
            builder->vertexUV(x0, y0, depth, sideU1, sideV0);
            builder->vertexUV(x0, y1, depth, sideU1, sideV1);
            builder->vertexUV(x0, y1, -depth, sideU0, sideV1);
            builder->vertexUV(x0, y0, -depth, sideU0, sideV0);
            builder->normal(1.0F, 0.0F, 0.0F);
            builder->vertexUV(x1, y0, -depth, sideU1, sideV0);
            builder->vertexUV(x1, y1, -depth, sideU1, sideV1);
            builder->vertexUV(x1, y1, depth, sideU0, sideV1);
            builder->vertexUV(x1, y0, depth, sideU0, sideV0);
            builder->end();
        }
    }
}

void PaintingRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                              float partialTick)
{
    ++gHangingEntityRenderCount;
    boost::shared_ptr<Painting> painting = boost::static_pointer_cast<Painting>(entity);
    mRandom->setSeed(187LL);
    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y, (float)z);
    GlStateManager::rotatef(180.0F - yaw, 0.0F, 1.0F, 0.0F);
    GlStateManager::enableRescaleNormal();
    bindTexture(entity);
    Painting::Motive* motive = painting->motive;
    GlStateManager::scalef(0.0625F, 0.0625F, 0.0625F);
    if (mSolidRender)
    {
        GlStateManager::enableColorMaterial();
        GlStateManager::setupSolidRenderingTextureCombine((int)getTeamColor(entity));
    }
    renderPainting(painting, motive->width, motive->height, motive->textureX, motive->textureY);
    if (mSolidRender)
    {
        GlStateManager::tearDownSolidRenderingTextureCombine();
        GlStateManager::disableColorMaterial();
    }
    GlStateManager::disableRescaleNormal();
    GlStateManager::popMatrix();
    EntityRenderer::render(entity, x, y, z, yaw, partialTick);
}

const ResourceLocation* PaintingRenderer::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    return &PAINTING_LOCATION;
}
