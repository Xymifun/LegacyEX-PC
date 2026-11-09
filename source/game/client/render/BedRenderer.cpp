#include "game/client/render/BedRenderer.hpp"

#include "game/client/render/BedModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/BedBlockEntity.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcGoldenAge.hpp"
#include "platform/pc/PcSettings.hpp"
#endif

static ResourceLocation BED_TEXTURE(TN_ITEM_BED);

BedRenderer::BedRenderer()
{
    mModel = new BedModel();
    mModelVersion = mModel->modelVersion();
    mDefaultModel = NULL;
}

BedRenderer::~BedRenderer()
{
}

void BedRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float partialTicks,
                         int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList)
{
    if (mModelVersion != mModel->modelVersion())
    {
        delete mModel;
        mModel = new BedModel();
        mModelVersion = mModel->modelVersion();
    }

    boost::shared_ptr<BedBlockEntity> bed = boost::static_pointer_cast<BedBlockEntity>(blockEntity);
    bool hasLevel = bed->getLevel() != NULL;
    bool isHeadPiece = hasLevel ? bed->isHeadPiece() : true;
    const DyeColor* colour = bed != NULL ? bed->getColor() : DyeColor::RED;
#ifdef MINECRAFT_PC
    if (PcGoldenAgeOn(kPcGoldenAgeBlocks))
        colour = DyeColor::RED;
#endif
    unsigned int direction = 0;
    if (hasLevel)
    {
        direction = bed->getData() & 3;
        if (!isHeadPiece)
            return;
    }

    bindTexture(&BED_TEXTURE);
    GlStateManager::pushMatrix();

    float xOffset = 0.0F;
    float zOffset = 0.0F;
    float rotation = 0.0F;
    const float ninetyDegrees = 90.0F;
    const float unit = 1.0F;

    if (mModel->hasOverriddenParts())
    {
        if (direction == Direction::NORTH->get2DDataValue())
        {
            zOffset = 0.5F;
            rotation = 0.0F;
        }
        else if (direction == Direction::SOUTH->get2DDataValue())
        {
            xOffset = 1.0F;
            zOffset = 0.5F;
            rotation = 180.0F;
        }
        else if (direction == Direction::WEST->get2DDataValue())
        {
            xOffset = 0.5F;
            zOffset = 1.0F;
            rotation = -90.0F;
        }
        else if (direction == Direction::EAST->get2DDataValue())
        {
            xOffset = 0.5F;
            rotation = 90.0F;
        }
    }
    else
    {
        if (direction == Direction::NORTH->get2DDataValue())
        {
            rotation = 0.0F;
        }
        else if (direction == Direction::SOUTH->get2DDataValue())
        {
            xOffset = 1.0F;
            zOffset = 1.0F;
            rotation = 180.0F;
        }
        else if (direction == Direction::WEST->get2DDataValue())
        {
            zOffset = 1.0F;
            rotation = -90.0F;
        }
        else if (direction == Direction::EAST->get2DDataValue())
        {
            xOffset = 1.0F;
            rotation = 90.0F;
        }
    }

    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x + xOffset, (float)y + 0.5625F, (float)z + zOffset);
    GlStateManager::rotatef(ninetyDegrees, unit, 0.0F, 0.0F);
    GlStateManager::rotatef(rotation, 0.0F, 0.0F, unit);
    GlStateManager::enableRescaleNormal();
    GlStateManager::pushMatrix();

    ColourTable* colourTable = Minecraft::GetInstance()->getColourTable();
    int colourValue = colourTable->getColour(colour->getBedColor());
    GlStateManager::color4f((float)((colourValue >> 16) & 0xff) / 255.0F, (float)((colourValue >> 8) & 0xff) / 255.0F,
                            (float)(colourValue & 0xff) / 255.0F, alpha);
    mModel->render(bUseDisplayList);
    GlStateManager::popMatrix();

    GlStateManager::color4f(1.0F, 1.0F, 1.0F, alpha);
    GlStateManager::matrixMode(2);
    GlStateManager::loadIdentity();
    GlStateManager::translatef(0.0F, 0.5F, 0.0F);
    GlStateManager::matrixMode(0);
    mModel->render(bUseDisplayList);
    GlStateManager::matrixMode(2);
    GlStateManager::loadIdentity();
    GlStateManager::translatef(0.0F, 0.0F, 0.0F);
    GlStateManager::matrixMode(0);
    GlStateManager::popMatrix();
    GlStateManager::color4f(1.0F, 1.0F, 1.0F, alpha);
    GlStateManager::popMatrix();
}

void BedRenderer::setupForTexturePack(TexturePack* pTexturePack)
{
    if (mDefaultModel == NULL)
    {
        mDefaultModel = mModel;
    }
    if (mModel != mDefaultModel)
    {
        delete mModel;
        mModel = NULL;
    }

    if (pTexturePack->HasCustomModel(mDefaultModel->getName()))
    {
        mModel = new BedModel(*pTexturePack->GetCustomModel(mDefaultModel->getName()));
    }
    else
    {
        mModel = mDefaultModel;
    }
}
