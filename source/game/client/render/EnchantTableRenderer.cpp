#include "game/client/render/EnchantTableRenderer.hpp"

#include <cmath>

#include "game/client/render/BookModel.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/EnchantmentTableEntity.hpp"
#include "game/util/ResourceLocation.hpp"

static ResourceLocation BOOK_TEXTURE(TN_ITEM_BOOK);

EnchantTableRenderer::EnchantTableRenderer()
{
    mBookModel = new BookModel();
}

EnchantTableRenderer::~EnchantTableRenderer()
{
    delete mBookModel;
}

void EnchantTableRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float a,
                                  int breakProgress, bool bTranslucent, float alpha, bool bUseDisplayList)
{
    boost::shared_ptr<EnchantmentTableEntity> enchantmentTable = boost::static_pointer_cast<EnchantmentTableEntity>(
        blockEntity);

    GlStateManager::pushMatrix();
    GlStateManager::translatef((float)x + 0.5F, (float)y + 0.75F, (float)z + 0.5F);

    float time = (float)enchantmentTable->mTime + a;
    GlStateManager::translatef(0.0F, 0.1F + sin(time * 0.1F) * 0.01F, 0.0F);

    float rotation = enchantmentTable->mRot - enchantmentTable->mRotOld;
    while (rotation >= 3.1415927F)
    {
        rotation -= 6.2831855F;
    }
    while (rotation < -3.1415927F)
    {
        rotation += 6.2831855F;
    }

    float bookRotation = enchantmentTable->mRotOld + rotation * a;
    GlStateManager::rotatef(-bookRotation * Mth::RAD_TO_DEG, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef(80.0F, 0.0F, 0.0F, 1.0F);
    bindTexture(&BOOK_TEXTURE);

    float page1 = enchantmentTable->mFlipOld + (enchantmentTable->mFlip - enchantmentTable->mFlipOld) * a + 0.25F;
    float page2 = enchantmentTable->mFlipOld + (enchantmentTable->mFlip - enchantmentTable->mFlipOld) * a + 0.75F;
    page1 = (page1 - (float)Mth::fastFloor((double)page1)) * 1.6F - 0.3F;
    page2 = (page2 - (float)Mth::fastFloor((double)page2)) * 1.6F - 0.3F;
    if (page1 < 0.0F)
    {
        page1 = 0.0F;
    }
    if (page2 < 0.0F)
    {
        page2 = 0.0F;
    }
    if (page1 > 1.0F)
    {
        page1 = 1.0F;
    }
    if (page2 > 1.0F)
    {
        page2 = 1.0F;
    }

    float open = enchantmentTable->mOpenOld + (enchantmentTable->mOpen - enchantmentTable->mOpenOld) * a;
    GlStateManager::enableCull();
    boost::shared_ptr<Entity> entity;
    mBookModel->render(entity, time, page1, page2, open, 0.0F, 0.0625F, true);

    GlStateManager::popMatrix();
}
