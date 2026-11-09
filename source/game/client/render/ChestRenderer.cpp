#include "game/client/render/ChestRenderer.hpp"

#include <cstring>

#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/ChestBlockEntity.hpp"
#ifdef MINECRAFT_PC
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/Minecraft.hpp"
#include "platform/pc/PcGoldenAge.hpp"
#include "platform/pc/PcSettings.hpp"
#endif

static ResourceLocation LARGE_TRAPPED_CHEST_LOCATION(TN_ITEM_TRAPPED_DOUBLE);
static ResourceLocation LARGE_CHEST_LOCATION(TN_ITEM_LARGECHEST);
static ResourceLocation TRAPPED_CHEST_LOCATION(TN_ITEM_TRAPPED);
static ResourceLocation CHEST_LOCATION(TN_ITEM_CHEST);

#ifdef MINECRAFT_PC
namespace
{
const int DIR_X[6] = {0, 0, 0, 0, -1, 1};
const int DIR_Z[6] = {0, 0, -1, 1, 0, 0};

void DrawFace(BufferBuilder* builder, float x, float z, int direction, int row, int column)
{
    const float u0 = static_cast<float>(column) / 16.0f;
    const float u1 = static_cast<float>(column + 1) / 16.0f;
    const float v0 = static_cast<float>(row) / 16.0f;
    const float v1 = static_cast<float>(row + 1) / 16.0f;
    float corners[4][3];
    switch (direction)
    {
    case 0:
    {
        const float face[4][3] = {{x, 0, z}, {x + 1, 0, z}, {x + 1, 0, z + 1}, {x, 0, z + 1}};
        memcpy(corners, face, sizeof(corners));
        builder->normal(0.0f, -1.0f, 0.0f);
        break;
    }
    case 1:
    {
        const float face[4][3] = {{x, 1, z + 1}, {x + 1, 1, z + 1}, {x + 1, 1, z}, {x, 1, z}};
        memcpy(corners, face, sizeof(corners));
        builder->normal(0.0f, 1.0f, 0.0f);
        break;
    }
    case 2:
    {
        const float face[4][3] = {{x + 1, 0, z}, {x, 0, z}, {x, 1, z}, {x + 1, 1, z}};
        memcpy(corners, face, sizeof(corners));
        builder->normal(0.0f, 0.0f, -1.0f);
        break;
    }
    case 3:
    {
        const float face[4][3] = {{x, 0, z + 1}, {x + 1, 0, z + 1}, {x + 1, 1, z + 1}, {x, 1, z + 1}};
        memcpy(corners, face, sizeof(corners));
        builder->normal(0.0f, 0.0f, 1.0f);
        break;
    }
    case 4:
    {
        const float face[4][3] = {{x, 0, z}, {x, 0, z + 1}, {x, 1, z + 1}, {x, 1, z}};
        memcpy(corners, face, sizeof(corners));
        builder->normal(-1.0f, 0.0f, 0.0f);
        break;
    }
    default:
    {
        const float face[4][3] = {{x + 1, 0, z + 1}, {x + 1, 0, z}, {x + 1, 1, z}, {x + 1, 1, z + 1}};
        memcpy(corners, face, sizeof(corners));
        builder->normal(1.0f, 0.0f, 0.0f);
        break;
    }
    }
    builder->vertexUV(corners[0][0], corners[0][1], corners[0][2], u0, v1);
    builder->vertexUV(corners[1][0], corners[1][1], corners[1][2], u1, v1);
    builder->vertexUV(corners[2][0], corners[2][1], corners[2][2], u1, v0);
    builder->vertexUV(corners[3][0], corners[3][1], corners[3][2], u0, v0);
}

bool IsOnLeft(int offsetX, int offsetZ, int lookX, int lookZ)
{
    return offsetX * lookZ - offsetZ * lookX > 0;
}

void DrawChestBlock(BufferBuilder* builder, int x, int z, int facing, bool large, int otherX, int otherZ)
{
    const int opposite = facing ^ 1;
    for (int direction = 0; direction < 6; ++direction)
    {
        if (large && x + DIR_X[direction] == otherX && z + DIR_Z[direction] == otherZ)
            continue;
        int row = 1;
        int column = 10;
        if (direction < 2)
        {
            column = 9;
        }
        else if (direction == facing)
        {
            if (large)
            {
                row = 2;
                column = IsOnLeft(x - otherX, z - otherZ, -DIR_X[facing], -DIR_Z[facing]) ? 9 : 10;
            }
            else
            {
                column = 11;
            }
        }
        else if (direction == opposite && large)
        {
            row = 3;
            column = IsOnLeft(x - otherX, z - otherZ, DIR_X[facing], DIR_Z[facing]) ? 9 : 10;
        }
        DrawFace(builder, static_cast<float>(x), static_cast<float>(z), direction, row, column);
    }
}

bool RenderOldChest(boost::shared_ptr<ChestBlockEntity>& chest, double x, double y, double z, int data,
                        bool translucent, float alpha)
{
    const int texture = PcGoldenAgeTerrainTexture();
    if (texture < 0)
        return false;
    const int facing = data >= 2 && data <= 5 ? data : 3;
    int otherX = 0;
    int otherZ = 0;
    bool large = false;
    if (chest->mEastNeighbour.lock())
    {
        otherX = 1;
        large = true;
    }
    else if (chest->mSouthNeighbour.lock())
    {
        otherZ = 1;
        large = true;
    }
    Minecraft::GetInstance()->mTextures->bind(texture);
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, translucent ? alpha : 1.0F);
    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x, (float)y, (float)z);
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->begin();
    DrawChestBlock(builder, 0, 0, facing, large, otherX, otherZ);
    if (large)
        DrawChestBlock(builder, otherX, otherZ, facing, true, 0, 0);
    builder->end();
    GlStateManager::popMatrix();
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    return true;
}
}
#endif

ChestRenderer::ChestRenderer() : BlockEntityRenderer()
{
    mChestModel = new ChestModel();
    mLargeChestModel = new LargeChestModel();
}

ChestRenderer::~ChestRenderer()
{
    delete mChestModel;
    delete mLargeChestModel;
}

void ChestRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a,
                           int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList)
{
    boost::shared_ptr<ChestBlockEntity> chest = boost::static_pointer_cast<ChestBlockEntity>(blockEntity);

    GlStateManager::enableDepthTest();
    GlStateManager::depthFunc(3);
    GlStateManager::depthMask(true);

    int data;
    if (chest->hasLevel())
    {
        data = chest->getData();
        chest->checkNeighbors();
    }
    else
    {
        data = 0;
    }

    if (chest->mNorthNeighbour.lock() || chest->mWestNeighbour.lock())
    {
        return;
    }

#ifdef MINECRAFT_PC
    if (chest->getType() == 0 && PcGoldenAgeOn(kPcGoldenAgeModels))
    {
        if (breakProgress >= 0 || RenderOldChest(chest, x, y, z, data, bTranslucent, alpha))
            return;
    }
#endif

    ChestModel* model;
    if (chest->mEastNeighbour.lock() || chest->mSouthNeighbour.lock())
    {
        model = mLargeChestModel;
        if (breakProgress >= 0)
        {
            bindTexture(&BREAKING_LOCATIONS[breakProgress]);

            GlStateManager::matrixMode(2);
            GlStateManager::pushMatrix();
            GlStateManager::scalef(8.0F, 4.0F, 1.0F);
            GlStateManager::translatef(0.0625F, 0.0625F, 0.0625F);
            GlStateManager::matrixMode(0);
            GlStateManager::pushMatrix();
            GlStateManager::enableRescaleNormal();
        }
        else
        {
            if (chest->getType() == 1)
            {
                bindTexture(&LARGE_TRAPPED_CHEST_LOCATION);
            }
            else
            {
                bindTexture(&LARGE_CHEST_LOCATION);
            }

            GlStateManager::pushMatrix();
            GlStateManager::enableRescaleNormal();
        }
    }
    else
    {
        model = mChestModel;
        if (breakProgress >= 0)
        {
            bindTexture(&BREAKING_LOCATIONS[breakProgress]);

            GlStateManager::matrixMode(2);
            GlStateManager::pushMatrix();
            GlStateManager::scalef(4.0F, 4.0F, 1.0F);
            GlStateManager::translatef(0.0625F, 0.0625F, 0.0625F);
            GlStateManager::matrixMode(0);
            GlStateManager::pushMatrix();
            GlStateManager::enableRescaleNormal();
        }
        else
        {
            if (chest->getType() == 1)
            {
                bindTexture(&TRAPPED_CHEST_LOCATION);
            }
            else
            {
                bindTexture(&CHEST_LOCATION);
            }

            GlStateManager::pushMatrix();
            GlStateManager::enableRescaleNormal();
        }
    }

    if (bTranslucent)
    {
        GlStateManager::color4f(1.0F, 1.0F, 1.0F, alpha);
    }

    GlStateManager::translatef((float)x, (float)y + 1.0F, (float)z + 1.0F);
    GlStateManager::scalef(1.0F, -1.0F, -1.0F);
    GlStateManager::translatef(0.5F, 0.5F, 0.5F);

    float yRot = 0.0F;
    if (data == 2)
    {
        yRot = 180.0F;
    }
    if (data == 3)
    {
        yRot = 0.0F;
    }
    if (data == 4)
    {
        yRot = 90.0F;
    }
    if (data == 5)
    {
        yRot = -90.0F;
    }

    if (data == 2 && chest->mEastNeighbour.lock())
    {
        GlStateManager::translatef(1.0F, 0.0F, 0.0F);
    }

    if (data == 5 && chest->mSouthNeighbour.lock())
    {
        GlStateManager::translatef(0.0F, 0.0F, -1.0F);
    }

    GlStateManager::rotatef(yRot, 0.0F, 1.0F, 0.0F);
    GlStateManager::translatef(-0.5F, -0.5F, -0.5F);

    float openness = chest->mOpenOld + (chest->mOpen - chest->mOpenOld) * a;
    if (chest->mNorthNeighbour.lock())
    {
        float neighbourOpenness = chest->mNorthNeighbour.lock()->mOpenOld
                                  + (chest->mNorthNeighbour.lock()->mOpen - chest->mNorthNeighbour.lock()->mOpenOld)
                                        * a;
        if (neighbourOpenness > openness)
        {
            openness = neighbourOpenness;
        }
    }

    if (chest->mWestNeighbour.lock())
    {
        float neighbourOpenness = chest->mWestNeighbour.lock()->mOpenOld
                                  + (chest->mWestNeighbour.lock()->mOpen - chest->mWestNeighbour.lock()->mOpenOld) * a;
        if (neighbourOpenness > openness)
        {
            openness = neighbourOpenness;
        }
    }

    openness = 1.0F - openness;
    openness = 1.0F - openness * openness * openness;

    model->mLid->mXRot = -(openness * 1.5707964F);
    model->render(bUseDisplayList);

    GlStateManager::disableRescaleNormal();
    GlStateManager::popMatrix();

    if (bTranslucent)
    {
        GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    }

    if (breakProgress >= 0)
    {
        GlStateManager::matrixMode(2);
        GlStateManager::popMatrix();
        GlStateManager::matrixMode(0);
    }
}
