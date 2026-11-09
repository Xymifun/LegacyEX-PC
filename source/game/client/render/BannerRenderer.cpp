#include "game/client/render/BannerRenderer.hpp"

#include <cmath>
#include <string>

#include "game/client/render/BannerModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/BannerPattern.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BannerBlockEntity.hpp"
#include "game/world/block/Blocks.hpp"

static ResourceLocation* BANNER_LOCATION = new ResourceLocation(TN_ITEM_BANNER_BANNER_BASE);
static int BANNER_PATTERN_TEXTURES[2] = {146, 147};
static ResourceLocation* BANNER_PATTERN_LOCATION = new ResourceLocation(
    arrayWithLength<int>(BANNER_PATTERN_TEXTURES, 2));

BannerRenderer::BannerRenderer() : BlockEntityRenderer()
{
    mModel = new BannerModel();
}

BannerRenderer::~BannerRenderer()
{
}

bool BannerRenderer::hasTexture(boost::shared_ptr<BlockEntity> blockEntity)
{
    boost::shared_ptr<BannerBlockEntity> banner = boost::static_pointer_cast<BannerBlockEntity>(blockEntity);
    std::wstring textureHashName = banner->getTextureHashName();
    if (textureHashName.empty())
    {
        return false;
    }
    return true;
}

void BannerRenderer::getUV(int atlasIndex, bool layered, float& u, float& v)
{
    int columns;
    float cellWidth;
    float cellHeight;
    float textureHeight;
    if (layered)
    {
        columns = 9;
        cellWidth = 13.0F;
        cellHeight = 23.0F;
        textureHeight = 128.0F;
    }
    else
    {
        columns = 6;
        cellWidth = 42.0F;
        cellHeight = 41.0F;
        textureHeight = 288.0F;
    }
    float column = (float)(atlasIndex % columns);
    float row = (float)(atlasIndex / columns);
    u = column * cellWidth;
    v = row * cellHeight / textureHeight;
}

void BannerRenderer::setupLayeredParams(const std::vector<const BannerPattern*>* patterns,
                                        const std::vector<const DyeColor*>* colors, bool layered)
{
    const int first = (int)eMinecraftColour_Banner_White;
    for (int i = 0; i < 16; ++i)
    {
        unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour((eMinecraftColour)(first + i));
        gRenderer->SetLayeredColour(i, (float)((colour >> 16) & 0xff) / 255.0F, (float)((colour >> 8) & 0xff) / 255.0F,
                                    (float)(colour & 0xff) / 255.0F);
    }

    if (layered)
        gRenderer->SetLayeredTextureSize(128, 128);
    else
        gRenderer->SetLayeredTextureSize(256, 288);

    std::vector<std::wstring> textureNames;
    unsigned int layer = 0;
    std::vector<const BannerPattern*>::const_iterator pattern = patterns->begin();
    std::vector<const DyeColor*>::const_iterator colour = colors->begin();
    while (pattern != patterns->end() && colour != colors->end())
    {
        float u;
        float v;
        getUV((int)(*pattern)->getAtlasIndex(), layered, u, v);
        gRenderer->SetLayeredParam((int)layer, (*colour)->getBlockData(), u, v);
        ++layer;
        ++pattern;
        ++colour;
    }

    while (layer < 7)
    {
        float u;
        float v;
        getUV(0x27, layered, u, v);
        gRenderer->SetLayeredParam((int)layer, 15, u, v);
        ++layer;
    }
}

void BannerRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z,
                            float partialTicks, int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList)
{
    bool hasLevel = blockEntity->getLevel() != NULL;
    bool standing = !hasLevel || blockEntity->getBlock() == Blocks::standing_banner;
    int direction = hasLevel ? blockEntity->getData() : 0;
    long long gameTime = hasLevel ? blockEntity->getLevel()->getGameTime() : 0;

    GlStateManager::pushMatrix();
    float scale = 0.6666667F;
    if (standing)
    {
        GlStateManager::translatef((float)x + 0.5F, (float)y + 0.5F, (float)z + 0.5F);
        GlStateManager::rotatef(-(float)(direction * 360) / 16.0F, 0.0F, 1.0F, 0.0F);
        mModel->mPole->mShow = true;
    }
    else
    {
        float rotation = 0.0F;
        if (direction == 2)
            rotation = 180.0F;
        if (direction == 4)
            rotation = 90.0F;
        if (direction == 5)
            rotation = -90.0F;
        GlStateManager::translatef((float)x + 0.5F, (float)y - 0.16666667F, (float)z + 0.5F);
        GlStateManager::rotatef(-rotation, 0.0F, 1.0F, 0.0F);
        GlStateManager::translatef(0.0F, -0.3125F, -0.4375F);
        mModel->mPole->mShow = false;
    }

    if (bTranslucent)
    {
        GlStateManager::color4f(1.0F, 1.0F, 1.0F, alpha);
    }

    BlockPos position = blockEntity->getBlockPos();
    float phase = (float)(position.x * 7 + position.y * 9 + position.z * 13) + (float)gameTime + partialTicks;
    mModel->mFlag->mXRot = (-0.0125F + 0.01F * cosf(phase * 3.1415927F * 0.02F)) * 3.1415927F;
    GlStateManager::enableRescaleNormal();

    if (hasTexture(blockEntity))
    {
        bindTexture(BANNER_LOCATION);
        GlStateManager::pushMatrix();
        GlStateManager::scalef(scale, -scale, -scale);
        mModel->mFlag->mShow = false;
        mModel->render();
        mModel->mFlag->mShow = true;
        bindTextureLayers(BANNER_PATTERN_LOCATION);
        gRenderer->TextureBindMask(1);
        boost::shared_ptr<BannerBlockEntity> banner = boost::static_pointer_cast<BannerBlockEntity>(blockEntity);
        setupLayeredParams(banner->getPatterns(), banner->getColors(), false);
        mModel->mPole->mShow = false;
        mModel->mBar->mShow = false;
        mModel->render();
        mModel->mPole->mShow = true;
        mModel->mBar->mShow = true;
        GlStateManager::popMatrix();
        gRenderer->TextureBind(1, -1);
    }

    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);
    GlStateManager::popMatrix();
}
