#include "game/client/render/BeaconRenderer.hpp"

#include "game/client/render/BlockEntityRenderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BeaconBlockEntity.hpp"

#include <boost/shared_ptr.hpp>
#include <vector>
#include <cmath>

static ResourceLocation BEAM_LOCATION(TN_MISC_BEACON_BEAM);

BeaconRenderer::~BeaconRenderer()
{
}

void BeaconRenderer::renderBeaconBeam(double x, double y, double z, double partialTicks, double beamHeight,
                                      double gameTime, int startY, int endY, arrayWithLength<const float> colors)
{
    renderBeaconBeam(x, y, z, partialTicks, beamHeight, gameTime, startY, endY, colors, 0.2, 0.25);
}

void BeaconRenderer::renderBeaconBeam(double x, double y, double z, double partialTicks, double beamHeight,
                                      double gameTime, int startY, int endY, arrayWithLength<const float> colors,
                                      double innerRadius, double outerRadius)
{
    int topY = startY + endY;
    GlStateManager::disableLighting();
    GlStateManager::colorMask(true, true, true, false);
    GlStateManager::disableCull();
    GlStateManager::disableBlend();
    GlStateManager::depthMask(true);
    GlStateManager::blendFunc(4, 1);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    double time = gameTime + partialTicks;
    double signedTime = endY < 0 ? time : -time;
    double offset = Mth::frac(signedTime * 0.2 - (double)Mth::floor(signedTime * 0.1));
    float red = colors.data[0];
    float green = colors.data[1];
    float blue = colors.data[2];
    double angle = time * 0.025 * -1.5;
    double x0 = 0.5 + std::cos(angle + 2.35619455575943) * 0.2;
    double z0 = 0.5 + std::sin(angle + 2.35619455575943) * 0.2;
    double x1 = 0.5 + std::cos(angle + 0.7853981852531433) * 0.2;
    double z1 = 0.5 + std::sin(angle + 0.7853981852531433) * 0.2;
    double x2 = 0.5 + std::cos(angle + 3.9269909262657166) * 0.2;
    double z2 = 0.5 + std::sin(angle + 3.9269909262657166) * 0.2;
    double x3 = 0.5 + std::cos(angle + 5.497787296772003) * 0.2;
    double z3 = 0.5 + std::sin(angle + 5.497787296772003) * 0.2;
    double v0 = -1.0 + offset;
    double v1 = (double)endY * beamHeight * 2.5 + v0;

    builder->begin();
    builder->color(red, green, blue, 0.125F);
    builder->vertexUV((float)(x + x0), (float)(y + (double)topY), (float)(z + z0), 1.0F, (float)v1);
    builder->vertexUV((float)(x + x0), (float)(y + (double)startY), (float)(z + z0), 1.0F, (float)v0);
    builder->vertexUV((float)(x + x1), (float)(y + (double)startY), (float)(z + z1), 0.0F, (float)v0);
    builder->vertexUV((float)(x + x1), (float)(y + (double)topY), (float)(z + z1), 0.0F, (float)v1);
    builder->vertexUV((float)(x + x3), (float)(y + (double)topY), (float)(z + z3), 1.0F, (float)v1);
    builder->vertexUV((float)(x + x3), (float)(y + (double)startY), (float)(z + z3), 1.0F, (float)v0);
    builder->vertexUV((float)(x + x2), (float)(y + (double)startY), (float)(z + z2), 0.0F, (float)v0);
    builder->vertexUV((float)(x + x2), (float)(y + (double)topY), (float)(z + z2), 0.0F, (float)v1);
    builder->vertexUV((float)(x + x1), (float)(y + (double)topY), (float)(z + z1), 1.0F, (float)v1);
    builder->vertexUV((float)(x + x1), (float)(y + (double)startY), (float)(z + z1), 1.0F, (float)v0);
    builder->vertexUV((float)(x + x3), (float)(y + (double)startY), (float)(z + z3), 0.0F, (float)v0);
    builder->vertexUV((float)(x + x3), (float)(y + (double)topY), (float)(z + z3), 0.0F, (float)v1);
    builder->vertexUV((float)(x + x2), (float)(y + (double)topY), (float)(z + z2), 1.0F, (float)v1);
    builder->vertexUV((float)(x + x2), (float)(y + (double)startY), (float)(z + z2), 1.0F, (float)v0);
    builder->vertexUV((float)(x + x0), (float)(y + (double)startY), (float)(z + z0), 0.0F, (float)v0);
    builder->vertexUV((float)(x + x0), (float)(y + (double)topY), (float)(z + z0), 0.0F, (float)v1);
    builder->end();

    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::depthMask(false);
    double glowMin = 0.2;
    double glowMax = 0.8;
    double glowV0 = -1.0 + offset;
    double glowV1 = (double)endY * beamHeight + glowV0;

    builder->begin();
    builder->color(red, green, blue, 0.125F);
    builder->vertexUV((float)(x + glowMin), (float)(y + (double)topY), (float)(z + glowMin), 1.0F, (float)glowV1);
    builder->vertexUV((float)(x + glowMin), (float)(y + (double)startY), (float)(z + glowMin), 1.0F, (float)glowV0);
    builder->vertexUV((float)(x + glowMax), (float)(y + (double)startY), (float)(z + glowMin), 0.0F, (float)glowV0);
    builder->vertexUV((float)(x + glowMax), (float)(y + (double)topY), (float)(z + glowMin), 0.0F, (float)glowV1);
    builder->vertexUV((float)(x + glowMax), (float)(y + (double)topY), (float)(z + glowMax), 1.0F, (float)glowV1);
    builder->vertexUV((float)(x + glowMax), (float)(y + (double)startY), (float)(z + glowMax), 1.0F, (float)glowV0);
    builder->vertexUV((float)(x + glowMin), (float)(y + (double)startY), (float)(z + glowMax), 0.0F, (float)glowV0);
    builder->vertexUV((float)(x + glowMin), (float)(y + (double)topY), (float)(z + glowMax), 0.0F, (float)glowV1);
    builder->vertexUV((float)(x + glowMax), (float)(y + (double)topY), (float)(z + glowMin), 1.0F, (float)glowV1);
    builder->vertexUV((float)(x + glowMax), (float)(y + (double)startY), (float)(z + glowMin), 1.0F, (float)glowV0);
    builder->vertexUV((float)(x + glowMax), (float)(y + (double)startY), (float)(z + glowMax), 0.0F, (float)glowV0);
    builder->vertexUV((float)(x + glowMax), (float)(y + (double)topY), (float)(z + glowMax), 0.0F, (float)glowV1);
    builder->vertexUV((float)(x + glowMin), (float)(y + (double)topY), (float)(z + glowMax), 1.0F, (float)glowV1);
    builder->vertexUV((float)(x + glowMin), (float)(y + (double)startY), (float)(z + glowMax), 1.0F, (float)glowV0);
    builder->vertexUV((float)(x + glowMin), (float)(y + (double)startY), (float)(z + glowMin), 0.0F, (float)glowV0);
    builder->vertexUV((float)(x + glowMin), (float)(y + (double)topY), (float)(z + glowMin), 0.0F, (float)glowV1);
    builder->end();
    GlStateManager::enableLighting();
    GlStateManager::enableTexture();
    GlStateManager::depthMask(true);
    GlStateManager::colorMask(true, true, true, true);
}

void BeaconRenderer::renderBeaconBeam(double x, double y, double z, double partialTicks, double beamHeight,
                                      std::vector<BeaconBlockEntity::BeaconBeamSection>* sections, double gameTime)
{
    bindTexture(&BEAM_LOCATION);
    if (beamHeight > 0.0)
    {
        GlStateManager::disableFog();
        int height = 0;
        unsigned int index = 0;
        while (sections->size() > index)
        {
            BeaconBlockEntity::BeaconBeamSection& section = sections->at(index);
            arrayWithLength<const float> colors;
            colors.data = section.getColor();
            colors.length = 3;
            renderBeaconBeam(x, y, z, partialTicks, beamHeight, gameTime, height, section.getHeight(), colors);
            height += section.getHeight();
            ++index;
        }
        GlStateManager::enableFog();
    }
}

void BeaconRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a,
                            int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList)
{
    boost::shared_ptr<BeaconBlockEntity> beacon = boost::static_pointer_cast<BeaconBlockEntity>(blockEntity);
    renderBeaconBeam(x, y, z, a, beacon->getAndUpdateClientSideScale(), beacon->getBeamSections(),
                     (double)beacon->getLevel()->getGameTime());
}

void BeaconRenderer::renderSpecialBeamDirection(boost::shared_ptr<BlockEntity> blockEntity, double x, double y,
                                                double z, double xOffset, double yOffset, double zOffset, float a,
                                                int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList)
{
    boost::shared_ptr<BeaconBlockEntity> beacon = boost::static_pointer_cast<BeaconBlockEntity>(blockEntity);
    BlockPos position = beacon->getBlockPos();
    float angle;
    float axisX = 0.0F;
    float axisY = 0.0F;
    float axisZ = 0.0F;
    float offsetX;
    float offsetY;
    float offsetZ;
    bool rotated = true;
    switch (beacon->getBeamDirection())
    {
    case 3:
        offsetX = (float)(yOffset - position.y) + 0.5F;
        offsetY = (float)(position.x - xOffset) + 1.0F;
        offsetZ = (float)(zOffset - position.z);
        angle = -90.0F;
        axisZ = 1.0F;
        break;
    case 4:
        offsetX = (float)(position.y - yOffset) + 0.5F;
        offsetY = (float)(xOffset - position.x);
        offsetZ = (float)(zOffset - position.z);
        angle = 90.0F;
        axisZ = 1.0F;
        break;
    case 5:
        offsetX = (float)(xOffset - position.x);
        offsetY = (float)(position.z - zOffset) + 1.0F;
        offsetZ = (float)(yOffset - position.y) + 0.5F;
        angle = 90.0F;
        axisX = 1.0F;
        break;
    case 6:
        offsetX = (float)(xOffset - position.x);
        offsetY = (float)(zOffset - position.z);
        offsetZ = (float)(position.y - yOffset) + 0.5F;
        angle = -90.0F;
        axisX = 1.0F;
        break;
    default:
        rotated = false;
        break;
    }
    if (rotated)
    {
        GlStateManager::pushMatrix();
        GlStateManager::translatef((float)x + offsetX, (float)y + offsetY, (float)z + offsetZ);
        GlStateManager::rotatef(angle, axisX, axisY, axisZ);
        renderBeaconBeam(x, y, z, a, beacon->getAndUpdateClientSideScale(), beacon->getBeamSections(),
                         (double)beacon->getLevel()->getGameTime());
        GlStateManager::popMatrix();
    }
    else
    {
        renderBeaconBeam(x, y, z, a, beacon->getAndUpdateClientSideScale(), beacon->getBeamSections(),
                         (double)beacon->getLevel()->getGameTime());
    }
}

bool BeaconRenderer::shouldRenderOffScreen(boost::shared_ptr<BlockEntity> blockEntity)
{
    return true;
}
