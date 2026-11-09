#include "game/client/render/LightningBoltRenderer.hpp"

#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/LightningBolt.hpp"
#include "game/util/Random.hpp"

#include <boost/shared_ptr.hpp>
#include <cstring>

LightningBoltRenderer::LightningBoltRenderer(EntityRenderDispatcher* dispatcher) : EntityRenderer(dispatcher)
{
}

LightningBoltRenderer::~LightningBoltRenderer()
{
}

void LightningBoltRenderer::render(boost::shared_ptr<Entity>& entity, double x, double y, double z, float yaw,
                                   float partialTick)
{
    boost::shared_ptr<LightningBolt> lightningBolt = boost::static_pointer_cast<LightningBolt>(entity);
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();

    GlStateManager::disableTexture();
    GlStateManager::disableLighting();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 1);

    double xOffsets[8];
    double zOffsets[8];
    double xOffset = 0.0;
    double zOffset = 0.0;
    Random random(lightningBolt->mSeed);
    for (int index = 7; index >= 0; --index)
    {
        xOffsets[index] = xOffset;
        zOffsets[index] = zOffset;
        xOffset += (double)(random.nextInt(11) - 5);
        zOffset += (double)(random.nextInt(11) - 5);
    }

    for (int layer = 0; layer < 4; ++layer)
    {
        Random branchRandom(lightningBolt->mSeed);
        for (int branch = 0; branch < 3; ++branch)
        {
            int top = 7;
            int bottom = 0;
            if (branch > 0)
            {
                top = 7 - branch;
                bottom = 5 - branch;
            }

            double currentX = xOffsets[top] - xOffset;
            double currentZ = zOffsets[top] - zOffset;
            for (int segment = top; segment >= bottom; --segment)
            {
                double previousX = currentX;
                double previousZ = currentZ;
                if (branch == 0)
                {
                    currentX += (double)(branchRandom.nextInt(11) - 5);
                    currentZ += (double)(branchRandom.nextInt(11) - 5);
                }
                else
                {
                    currentX += (double)(branchRandom.nextInt(31) - 15);
                    currentZ += (double)(branchRandom.nextInt(31) - 15);
                }

                builder->begin(1);
                builder->color(0.45F, 0.45F, 0.45F, 0.3F);

                double upperWidth = 0.1 + (double)layer * 0.2;
                if (branch == 0)
                    upperWidth *= (double)segment * 0.1 + 1.0;

                double lowerWidth = 0.1 + (double)layer * 0.2;
                if (branch == 0)
                    lowerWidth *= (double)(segment - 1) * 0.1 + 1.0;

                for (int corner = 0; corner < 5; ++corner)
                {
                    double upperX = x + 0.5 - upperWidth;
                    double upperZ = z + 0.5 - upperWidth;
                    if (corner == 1 || corner == 2)
                        upperX += upperWidth * 2.0;
                    if (corner == 2 || corner == 3)
                        upperZ += upperWidth * 2.0;

                    double lowerX = x + 0.5 - lowerWidth;
                    double lowerZ = z + 0.5 - lowerWidth;
                    if (corner == 1 || corner == 2)
                        lowerX += lowerWidth * 2.0;
                    if (corner == 2 || corner == 3)
                        lowerZ += lowerWidth * 2.0;

                    builder->vertex((float)(lowerX + currentX), (float)(y + (double)(segment * 16)),
                                    (float)(lowerZ + currentZ));
                    builder->vertex((float)(upperX + previousX), (float)(y + (double)((segment + 1) * 16)),
                                    (float)(upperZ + previousZ));
                }

                builder->end();
            }
        }
    }

    GlStateManager::disableBlend();
    GlStateManager::enableLighting();
    GlStateManager::enableTexture();
}
