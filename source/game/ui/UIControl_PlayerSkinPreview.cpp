#include "game/ui/UIControl_PlayerSkinPreview.hpp"

#include <cmath>

#include "game/client/render/EntityRenderer.hpp"
#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/PlayerRenderer.hpp"
#include "game/client/render/ModelPart.hpp"
#include "game/client/render/PlayerModel.hpp"
#include "game/core/HumanoidArm.hpp"
#include "game/core/Options.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/ScreenSizeCalculator.hpp"
#include "game/ui/fuiRect.hpp"
#include "game/world/generation/Player.hpp"

UIControl_PlayerSkinPreview::UIControl_PlayerSkinPreview()
{
    m_controlType = eUIControlType_PlayerSkinPreview;
    mUnknown54 = 1.0f;
    mUnknown58 = 1.0f;
    mUnknown50 = 0;
    Minecraft* minecraft = Minecraft::GetInstance();
    ScreenSizeCalculator calculator(minecraft->mOptions, minecraft->mFullWidth, minecraft->mFullHeight, -1);
    mWidth = (float)minecraft->mFullWidth;
    mWidthScale = (float)calculator.mWidthScale;
    mHeight = (float)minecraft->mFullHeight;
    mHeightScale = (float)calculator.mHeightScale;
    mTexturePath = L"default";
    mTextureName = TN_MOB_STEVE;
    mCapePath = L"";

    mRotation = 0;
    mPitch = 0;

    mAnimationAngle = 0.0f;
    mUnknown110 = 0.0f;
    mLimbSwingAmountOld = 0.0f;
    mLimbSwingAmount = 0.0f;
    mLimbSwing = 0.0f;

    mCapeVisible = false;
    mFacingLeft = false;
    mRotating = false;
    mRotateBack = false;
    mRotateForward = false;
    mRotateBackward = false;

    mAnimation = 0;

    mFacingRotation = 0;
    mInitialRotation = 0;
    mAnimationTicks = 0;
    mAnimating = false;
    mAdditionalModelParts = NULL;
#ifdef MINECRAFT_PC
    mPcTickTime = 0;
    mPcTickCarry = 0;
    mPcRenderTime = 0;
    mPcRenderCarry = 0;
#endif
}

#ifdef MINECRAFT_PC
static const int kPcPreviewStepsPerSecond = 60;
static const int kPcPreviewMaxSteps = 6;

static int PcPreviewSteps(long long& lastTime, int& carry)
{
    long long now = Minecraft::currentTimeMillis();
    if (lastTime == 0 || now < lastTime)
    {
        lastTime = now;
        carry = 0;
        return 1;
    }
    long long elapsed = (now - lastTime) * kPcPreviewStepsPerSecond + carry;
    lastTime = now;
    int steps = (int)(elapsed / 1000);
    carry = (int)(elapsed % 1000);
    if (steps > kPcPreviewMaxSteps)
    {
        steps = kPcPreviewMaxSteps;
        carry = 0;
    }
    return steps;
}

void UIControl_PlayerSkinPreview::tick()
{
    for (int steps = PcPreviewSteps(mPcTickTime, mPcTickCarry); steps > 0; --steps)
        PcTickStep();
}

void UIControl_PlayerSkinPreview::PcTickStep()
#else
void UIControl_PlayerSkinPreview::tick()
#endif
{
    if (mAnimating)
    {
        ++mAnimationTicks;
        mRotation = mInitialRotation + mAnimationTicks * ((mFacingRotation - mInitialRotation) / 15);
        return;
    }

    if (mRotating)
    {
        mPitch += 2;
        if (mPitch > 22)
            mPitch = 22;
    }
    if (mRotateBack)
    {
        mPitch -= 2;
        if (mPitch < -22)
            mPitch = -22;
    }
    if (mRotateForward)
    {
        mRotation += 4;
        if (mRotation >= 180)
            mRotation = -180;
    }
    if (mRotateBackward)
    {
        mRotation -= 4;
        if (mRotation <= -180)
            mRotation = 180;
    }

    if (!mCapeVisible)
        return;

    ++mCapeSwingTicks;
    if ((mCapeSwingTicks & 3) != 0)
        return;

    if (mRotation >= 45)
        mFacingLeft = false;
    else if (mRotation <= -45)
        mFacingLeft = true;

    if (mFacingLeft)
    {
        mRotation += 4;
        if (mRotation >= 180)
            mRotation = -180;
    }
    else
    {
        mRotation -= 4;
        if (mRotation <= -180)
            mRotation = 180;
    }
}

unsigned int UIControl_PlayerSkinPreview::bindTexture(const std::wstring& name, int texture)
{
    Textures* textures = Minecraft::GetInstance()->mTextures;
    int loaded = textures->loadMemTexture(name, texture);
    if (loaded >= 0)
    {
        textures->bind(loaded);
        return 1;
    }
    return 0;
}

unsigned int UIControl_PlayerSkinPreview::bindTexture(const std::wstring& name, const std::wstring& path)
{
    Textures* textures = Minecraft::GetInstance()->mTextures;
    int loaded = textures->loadMemTexture(name, path);
    if (loaded >= 0)
    {
        textures->bind(loaded);
        return 1;
    }
    return 0;
}

void UIControl_PlayerSkinPreview::render(EntityRenderer* renderer, double x, double y, double z, float yaw,
                                         float partialTick)
{
    GlStateManager::pushMatrix();
    GlStateManager::disableCull();

    PlayerModel* model = static_cast<PlayerModel*>(renderer->getModel());
    model->mCrouching = false;
    model->mAttackTime = 0.0f;
    model->mSwimming = false;
    model->setArmPose(HumanoidArm::RIGHT, HumanoidModel::eArmPose_Empty);
    model->setArmPose(HumanoidArm::LEFT, HumanoidModel::eArmPose_Empty);
    model->mUseItemProgress = 0.0f;
    model->mUseItemRemainingTicks = 0.0f;
    model->mSitting = false;
    model->mEating = false;
    model->mYoung = false;
    model->mRiding = false;
    model->mSwimAmount = 0.0f;
    model->mSpearThrowProgress = 0.0f;
    model->mHead->mXRot = 0.0f;
    model->mSkinAdjustments = mSkinAdjustments;

    model->mHead->mYRot = 0.0f;
#ifdef MINECRAFT_PC
    const int pcSteps = PcPreviewSteps(mPcRenderTime, mPcRenderCarry);
    partialTick = (float)mPcRenderCarry / 1000.0f;
#endif
    if (!mAnimating)
    {
        switch (mAnimation)
        {
        case 1:
            model->mCrouching = true;
            model->mHead->mYRot = 0.1f;
            break;
        case 2:
            model->setArmPose(HumanoidArm::RIGHT, HumanoidModel::eArmPose_Item);
#ifdef MINECRAFT_PC
            mAnimationAngle += (float)pcSteps;
            while (mAnimationAngle >= 18.0f)
                mAnimationAngle -= 18.0f;
#else
            mAnimationAngle += 1.0f;
            if (mAnimationAngle >= 18.0f)
                mAnimationAngle = 0.0f;
#endif
            model->mAttackTime = mAnimationAngle / 18.0f;
            break;
        }
    }

    float bodyRotation = (float)mRotation;
    float headRotation = bodyRotation;
    GlStateManager::translatef((float)x, (float)y, (float)z);
    GlStateManager::rotatef(180.0f - bodyRotation, 0.0f, 1.0f, 0.0f);
    GlStateManager::enableRescaleNormal();
    GlStateManager::scalef(-1.0f, -1.0f, 1.0f);
    GlStateManager::scalef(0.9375f, 0.9375f, 0.9375f);
    GlStateManager::translatef(0.0f, -1.5078125f, 0.0f);

#ifdef MINECRAFT_PC
    for (int step = 0; step < pcSteps; ++step)
    {
        mLimbSwingAmountOld = mLimbSwingAmount;
        mLimbSwingAmount += (0.1f - mLimbSwingAmount) * 0.4f;
        mLimbSwing += mLimbSwingAmount;
    }
#else
    mLimbSwingAmountOld = mLimbSwingAmount;
    mLimbSwingAmount += (0.1f - mLimbSwingAmount) * 0.4f;
    mLimbSwing += mLimbSwingAmount;
#endif
    float limbSwingAmount = mLimbSwingAmountOld + (mLimbSwingAmount - mLimbSwingAmountOld) * partialTick;
    if (limbSwingAmount > 1.0f)
        limbSwingAmount = 1.0f;
    float limbSwing = mLimbSwing - mLimbSwingAmount * (1.0f - partialTick);

    MemSect(0x1f);
    bindTexture(mTexturePath, mTextureName);
    MemSect(0);
    GlStateManager::enableAlphaTest();
    model->setAllVisible(true);
    boost::shared_ptr<Entity> noEntity;
    model->render(noEntity, limbSwing, limbSwingAmount, 0.0f, headRotation - bodyRotation, 0.0f, 0.0625f, true);

    if (bindTexture(mCapePath, std::wstring(L"")) != 0)
    {
        GlStateManager::pushMatrix();
        GlStateManager::translatef(0.0f, 0.0f, 0.125f);
        double motionX = 0.0;
        double motionZ = 0.0;
        float sinYaw = sinf(0.017453292f);
        float cosYaw = cosf(0.017453292f);
        float capeSwing = (float)(motionX * sinYaw + motionZ * -cosYaw) * 100.0f;
        if (capeSwing < 0.0f)
            capeSwing = 0.0f;
        float capeSway = (float)(motionX * -cosYaw - motionZ * sinYaw) * 100.0f;
        float capeLift = 1.0f;
        if (model->mCrouching)
            capeLift = 26.0f;
        GlStateManager::rotatef(6.0f + capeSwing * 0.5f + capeLift, 1.0f, 0.0f, 0.0f);
        GlStateManager::rotatef(capeSway * 0.5f, 0.0f, 0.0f, 1.0f);
        GlStateManager::rotatef(-(capeSway * 0.5f), 0.0f, 1.0f, 0.0f);
        GlStateManager::rotatef(180.0f, 0.0f, 1.0f, 0.0f);
        model->renderCloak(0.0625f);
        GlStateManager::popMatrix();
    }

    GlStateManager::disableRescaleNormal();
    GlStateManager::enableCull();
    GlStateManager::popMatrix();
    MemSect(0x1f);
    MemSect(0);

    model->mCrouching = false;
    model->mAttackTime = 0.0f;
    model->setArmPose(HumanoidArm::RIGHT, HumanoidModel::eArmPose_Empty);
    model->setArmPose(HumanoidArm::LEFT, HumanoidModel::eArmPose_Empty);
    model->clearAllOffsets();
}

void UIControl_PlayerSkinPreview::render(fuiRect* rect)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    GlStateManager::enableRescaleNormal();
    GlStateManager::enableColorMaterial();
    GlStateManager::pushMatrix();

    float rectWidth = rect->xMax - rect->xMin;
    float rectHeight = rect->yMax - rect->yMin;
    float centreX = rectWidth / 2;
    if ((mSkinAdjustments.mAnimFlags & 0x80000000) == 0)
        GlStateManager::translatef(centreX, rectHeight - 3.5f, 50.0f);
    else
        GlStateManager::translatef(centreX, rectHeight + -253.5f, 50.0f);
    float scale = rectWidth / (mWidth / mHeight);
    GlStateManager::scalef(-scale, scale, scale);
    if ((mSkinAdjustments.mAnimFlags & 0x80000000) == 0)
        GlStateManager::rotatef(180.0f, 0.0f, 0.0f, 1.0f);

    Lighting::turnOn();
    GlStateManager::rotatef(-(float)mPitch, 1.0f, 0.0f, 0.0f);

    bool hideGui = minecraft->mOptions->mHideGui;
    minecraft->mOptions->mHideGui = true;

    EPlayerModelType modelType;
    if (!mTexturePath.empty())
    {
        unsigned int skinId = CMinecraftApp::getSkinIdFromPath(mTexturePath);
        _SkinAdjustments adjustments = gApp.GetSkinAdjustments(skinId);
        modelType = static_cast<EPlayerModelType>(Player::GetModelTypeFromAnimBitmask(adjustments.mAnimFlags));
    }
    else
    {
        modelType = static_cast<EPlayerModelType>(Player::GetModelTypeFromTextureId(mTextureName));
    }
    PlayerRenderer* renderer = Minecraft::GetInstance()->getEntityRenderDispatcher()->getPlayerRenderer(modelType);

    if (renderer != NULL)
    {
        if (mAdditionalModelParts != NULL && !mAdditionalModelParts->empty())
        {
            for (std::vector<ModelPart*>::iterator it = mAdditionalModelParts->begin();
                 it != mAdditionalModelParts->end(); ++it)
            {
                (*it)->mShow = true;
            }
        }
        render(renderer, 0.0, 0.0, 0.0, 0.0f, 1.0f);
        if (mAdditionalModelParts != NULL && !mAdditionalModelParts->empty())
        {
            for (std::vector<ModelPart*>::iterator it = mAdditionalModelParts->begin();
                 it != mAdditionalModelParts->end(); ++it)
            {
                (*it)->mShow = false;
            }
        }
    }

    minecraft->mOptions->mHideGui = hideGui;
    GlStateManager::popMatrix();
    Lighting::turnOff();
    GlStateManager::disableRescaleNormal();
}

void UIControl_PlayerSkinPreview::SetTexture(const std::wstring& texture, _TEXTURE_NAME textureName)
{
    mTexturePath = texture;
    mTextureName = (int)textureName;
    _SkinAdjustments adjustments = Player::getSkinAdjustmentsById(CMinecraftApp::getSkinIdFromPath(mTexturePath));
    if (GameSettings::GetGameSettings(eGameSetting_CustomSkinAnim) == 0)
    {
        mSkinAdjustments = adjustments;
        if ((adjustments.mAnimFlags & 0x1f1f810) == 0)
            mSkinAdjustments.mAnimFlags = 0;
    }
    else
    {
        mSkinAdjustments = adjustments;
    }
    mAdditionalModelParts = gApp.GetAdditionalModelParts(CMinecraftApp::getSkinIdFromPath(mTexturePath));
}

void UIControl_PlayerSkinPreview::SetFacing(ESkinPreviewFacing facing, bool animate)
{
    switch (facing)
    {
    case eFacingFront:
        mFacingRotation = 0;
        mFacingLeft = true;
        break;
    case eFacingRight:
        mFacingRotation = 45;
        mFacingLeft = false;
        break;
    case eFacingLeft:
        mFacingRotation = -45;
        mFacingLeft = true;
        break;
    }
    if (!animate)
    {
        mAnimating = false;
        mRotation = mFacingRotation;
        return;
    }
    mAnimationTicks = 0;
    mInitialRotation = mRotation;
    mAnimating = true;
}

void UIControl_PlayerSkinPreview::CycleNextAnimation()
{
    mAnimation = mAnimation + 1;
    if (mAnimation >= 3)
        mAnimation = 0;
    mAnimationAngle = 0.0f;
}

void UIControl_PlayerSkinPreview::CyclePreviousAnimation()
{
    --mAnimation;
    if (mAnimation < 0)
        mAnimation = 2;
    mAnimationAngle = 0.0f;
}
