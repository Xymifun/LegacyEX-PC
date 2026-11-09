#include "game/client/render/ConduitRenderer.hpp"
#include "game/core/EntityType.hpp"

#include "game/client/render/ConduitBaseModel.hpp"
#include "game/client/render/ConduitCageModel.hpp"
#include "game/client/render/ConduitWindModel.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/ConduitBlockEntity.hpp"

static ResourceLocation SHELL_TEXTURE(TN_ITEM_CONDUIT_CONDUIT_BASE);
static ResourceLocation ACTIVE_SHELL_TEXTURE(TN_ITEM_CONDUIT_CONDUIT_CAGE);
static ResourceLocation WIND_TEXTURE(TN_ITEM_CONDUIT_CONDUIT_WIND_HORIZONTAL);
static ResourceLocation VERTICAL_WIND_TEXTURE(TN_ITEM_CONDUIT_CONDUIT_WIND_VERTICAL);
static ResourceLocation CLOSED_EYE_TEXTURE(TN_ITEM_CONDUIT_CONDUIT_CLOSED);
static ResourceLocation OPEN_EYE_TEXTURE(TN_ITEM_CONDUIT_CONDUIT_OPEN);

ConduitRenderer::ConduitRenderer()
{
    mBaseModel = new ConduitBaseModel(0, 0, 24, 12);
    mCageModel = new ConduitCageModel(0, 0, 32, 16);
    mWindModel = new ConduitWindModel(0, 0, 64, 704);
}

ConduitRenderer::~ConduitRenderer()
{
}

void ConduitRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z,
                             float partialTicks, int breakProgress, bool bTranslucent, float alpha,
                             bool bUseDisplayList)
{
    if (!eTYPE_DERIVED_FROM(BlockEntityType::CONDUIT, blockEntity->GetType()))
    {
        return;
    }

    boost::shared_ptr<ConduitBlockEntity> conduit = boost::static_pointer_cast<ConduitBlockEntity>(blockEntity);

    float fx = (float)x;
    float fy = (float)y;
    float fz = (float)z;

    float rotation = conduit->mActiveRotation;

    if (!conduit->mActive)
    {
        GlStateManager::pushMatrix();
        GlStateManager::translatef(fx + 0.5F, fy + 0.188125F, fz + 0.5F);
        GlStateManager::scalef(-1.0F, -1.0F, 1.0F);
        GlStateManager::rotatef(rotation, 0.0F, 1.0F, 0.0F);

        if (breakProgress >= 0)
        {
            bindTexture(&BREAKING_LOCATIONS[breakProgress]);

            GlStateManager::matrixMode(2);
            GlStateManager::pushMatrix();
            GlStateManager::scalef(4.0F, 2.0F, 1.0F);
            GlStateManager::matrixMode(0);
        }
        else
        {
            bindTexture(&SHELL_TEXTURE);
        }

        mBaseModel->renderBase(bUseDisplayList);

        if (breakProgress >= 0)
        {
            GlStateManager::matrixMode(2);
            GlStateManager::popMatrix();
            GlStateManager::matrixMode(0);
        }

        GlStateManager::popMatrix();

        return;
    }

    GlStateManager::disableCull();

    conduit->mActiveRotation = Mth::wrapDegrees(((float)conduit->mRotationTick + partialTicks) * -2.0F);
    ++conduit->mRotationTick;

    float bob = Mth::sin(((float)conduit->mTickCount + partialTicks) * 0.1F) * 0.5F + 0.5F;
    bob = bob * bob + bob;

    GlStateManager::pushMatrix();

    GlStateManager::translatef(fx + 0.5F, fy + 0.250625F + bob * 0.2F, fz + 0.5F);
    GlStateManager::scalef(-1.0F, -1.0F, 1.0F);
    GlStateManager::rotatef(rotation, 0.5F, 1.0F, 0.5F);

    if (breakProgress >= 0)
    {
        bindTexture(&BREAKING_LOCATIONS[breakProgress]);

        GlStateManager::matrixMode(2);
        GlStateManager::pushMatrix();
        GlStateManager::scalef(4.0F, 2.0F, 1.0F);
        GlStateManager::matrixMode(0);
    }
    else
    {
        bindTexture(&ACTIVE_SHELL_TEXTURE);
    }

    mCageModel->renderCage(bUseDisplayList);

    if (breakProgress >= 0)
    {
        GlStateManager::matrixMode(2);
        GlStateManager::popMatrix();
        GlStateManager::matrixMode(0);
    }

    GlStateManager::popMatrix();
    GlStateManager::enableCull();

    GlStateManager::pushMatrix();

    if (conduit->mHunting)
    {
        bindTexture(&OPEN_EYE_TEXTURE);
    }
    else
    {
        bindTexture(&CLOSED_EYE_TEXTURE);
    }

    GlStateManager::translatef(fx + 0.5F, fy + 0.1875F + bob * 0.2F, fz + 0.5F);
    GlStateManager::enableRescaleNormal();
    GlStateManager::scalef(0.33333334F, 0.33333334F, 0.33333334F);

    _tessellateHeart();

    GlStateManager::popMatrix();
    GlStateManager::disableCull();

    int windLevel = conduit->mWindLevel;
    float windOffset = conduit->mAnimationValue;

    if ((conduit->mRotationTick & 7) == 0)
    {
        conduit->incrementAnimationValue(0.045454547F);

        if (conduit->mAnimationValue == 0.0F)
        {
            conduit->incrementWindLevel();
        }
    }

    GlStateManager::pushMatrix();
    GlStateManager::translatef(fx + 0.5F, fy + 0.5F, fz + 0.5F);
    GlStateManager::scalef(-1.0F, -1.0F, 1.0F);

    switch (windLevel)
    {
    case 2:
    default:
        bindTexture(&WIND_TEXTURE);

        GlStateManager::matrixMode(2);
        GlStateManager::loadIdentity();
        GlStateManager::translatef(0.0F, windOffset, 0.0F);
        GlStateManager::matrixMode(0);
        GlStateManager::scalef(1.0F, 1.0F, 1.0F);
        GlStateManager::rotatef(90.0F, 0.0F, 0.0F, 1.0F);
        break;
    case 0:
        bindTexture(&WIND_TEXTURE);

        GlStateManager::matrixMode(2);
        GlStateManager::loadIdentity();
        GlStateManager::translatef(0.0F, windOffset, 0.0F);
        GlStateManager::matrixMode(0);
        break;
    case 1:
        bindTexture(&VERTICAL_WIND_TEXTURE);

        GlStateManager::matrixMode(2);
        GlStateManager::loadIdentity();
        GlStateManager::translatef(0.0F, windOffset, 0.0F);
        GlStateManager::matrixMode(0);
        GlStateManager::scalef(1.0F, 1.0F, 1.0F);
        GlStateManager::rotatef(90.0F, 1.0F, 0.0F, 0.0F);
        break;
    }

    mWindModel->renderWind(bUseDisplayList);

    GlStateManager::scalef(0.875F, 0.875F, 0.875F);
    GlStateManager::rotatef(180.0F, 1.0F, 0.0F, 0.0F);
    GlStateManager::rotatef(180.0F, 0.0F, 0.0F, 1.0F);

    mWindModel->renderWind(bUseDisplayList);

    GlStateManager::matrixMode(2);
    GlStateManager::loadIdentity();
    GlStateManager::translatef(0.0F, 0.0F, 0.0F);
    GlStateManager::matrixMode(0);

    GlStateManager::popMatrix();
    GlStateManager::enableCull();
}

void ConduitRenderer::_tessellateHeart() const
{
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();

    float width = 1.0F;
    float halfWidth = 0.5F;
    float quarterWidth = 0.25F;

    GlStateManager::rotatef(180.0F - Minecraft::GetInstance()->getCameraEntity()->mYRot, 0.0F, 1.0F, 0.0F);
    GlStateManager::rotatef(-Minecraft::GetInstance()->getCameraEntity()->mXRot, 1.0F, 0.0F, 0.0F);

    GlStateManager::color4f(1.0F, 1.0F, 1.0F, 1.0F);

    builder->begin();
    builder->normal(0.0F, 1.0F, 0.0F);
    builder->vertexUV(0.0F - halfWidth, 0.0F - quarterWidth, 0.0F, 0.0F, 1.0F);
    builder->vertexUV(width - halfWidth, 0.0F - quarterWidth, 0.0F, 1.0F, 1.0F);
    builder->vertexUV(width - halfWidth, width - quarterWidth, 0.0F, 1.0F, 0.0F);
    builder->vertexUV(0.0F - halfWidth, width - quarterWidth, 0.0F, 0.0F, 0.0F);

    tesselator->end();
}
