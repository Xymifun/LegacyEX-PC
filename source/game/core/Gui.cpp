#include "game/core/Gui.hpp"
#include "game/core/StringIds.hpp"

#include <cmath>
#include <vector>

#include "game/client/render/EntityRenderDispatcher.hpp"
#include "game/client/render/Font.hpp"
#include "game/client/render/GameRenderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/client/render/Textures.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/BossHealthOverlay.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/Direction.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/client/render/ItemRenderer.hpp"
#include "game/core/Lighting.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/ScreenSizeCalculator.hpp"
#ifdef MINECRAFT_PC
#include "platform/pc/PcChat.hpp"
#include "platform/pc/PcDebugOverlay.hpp"
#include "platform/pc/PcDisplay.hpp"
#include "platform/pc/PcScripts.hpp"
#include "platform/pc/PcNativeMods.hpp"
#include "platform/pc/PcTextKeyboard.hpp"
#endif
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/MobEffects.hpp"
#include "game/item/Item.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/ThermalAreaRuleDefinition.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/util/AABB.hpp"
#include "game/core/Mth.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/core/XLanguage.hpp"
#include "game/util/Random.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/MemSect.hpp"

float gGuiScreenScale = 1.0f;

namespace
{
ResourceLocation guiPumpkinOverlayTexture(TN_BLUR_MISC_PUMPKINBLUR);
ResourceLocation guiThermalOverlayTexture(TN_ENVIRONMENT_THERMAL_1);
ResourceLocation guiThermalLiftOverlayTexture(TN_ENVIRONMENT_THERMAL_2);

#ifdef MINECRAFT_PC
const int CHAT_LEFT = 4;
const int CHAT_WIDTH = 280;
const int CHAT_LINE_HEIGHT = 9;
const unsigned int CHAT_LINES_CLOSED = 10;
const unsigned int CHAT_LINES_OPEN = 20;

bool IsChatSpace(wchar_t character)
{
    return character == L' ' || character == L'\t' || character == L'\r' || character == L'\n'
           || character == 0x00A0 || character == 0x3000;
}

void WrapChatLine(Font* font, const std::wstring& text, int width, std::vector<std::wstring>* lines)
{
    std::wstring::size_type start = 0;
    while (start < text.length())
    {
        int used = 0;
        std::wstring::size_type end = start;
        std::wstring::size_type lastSpace = std::wstring::npos;
        bool newline = false;
        while (end < text.length())
        {
            if (text[end] == L'\u00A7' && end + 1 < text.length())
            {
                end += 2;
                continue;
            }
            if (text[end] == L'\n' || text[end] == L'\r')
            {
                newline = true;
                break;
            }
            const int characterWidth = font->width(text.substr(end, 1));
            if (used + characterWidth > width && end > start)
                break;
            if (IsChatSpace(text[end]))
                lastSpace = end;
            used += characterWidth;
            ++end;
        }
        if (!newline && end < text.length() && lastSpace != std::wstring::npos && lastSpace > start)
            end = lastSpace;
        while (end > start && IsChatSpace(text[end - 1]))
            --end;
        lines->push_back(text.substr(start, end - start));
        start = end;
        while (start < text.length() && IsChatSpace(text[start]))
            ++start;
    }
    if (text.empty())
        lines->push_back(text);
}
#endif
}

Gui::Gui(Minecraft* minecraft) : mUnknownD4(0), mUnknownE8(0), mUnknownEC(7)
{
    mRandom = new Random();
    mTickCount = 0;
    for (int i = 0; i < 4; ++i)
    {
        mSingleMessageTime[i] = 0;
        mSingleMessages[i] = L"";
        mNowPlaying[i] = L"";
    }
    mUnknown19C = 2.55f;
    mVignetteBrightness = 1.0f;
    mUnknown1D0 = 0.0f;
    mSingleMessageVisible = false;
    for (int i = 0; i < 4; ++i)
    {
        mBossOverlays[i] = new BossHealthOverlay(minecraft);
    }
    mMinecraft = minecraft;
    mItemRenderer = minecraft->getItemRenderer();
    for (int i = 0; i < 4; ++i)
    {
        mThermalOverlayRules[i] = 0;
        mThermalOverlayFade[i] = 0;
        mCharacterDisplayTime[i] = 0;
    }
    mJukeboxTime = 0.0f;
}

BossHealthOverlay* Gui::getBossOverlay(int index)
{
    return mBossOverlays[index];
}

void Gui::renderVignette(float partialTick, int width, int height)
{
    float target = 1.0f - partialTick;
    if (target < 0.0f)
    {
        target = 0.0f;
    }
    else if (target > 1.0f)
    {
        target = 1.0f;
    }
    mVignetteBrightness += (target - mVignetteBrightness) * 0.01f;
}

void Gui::renderPumpkin(int width, int height)
{
    GlStateManager::disableDepthTest();
    GlStateManager::depthMask(false);
    GlStateManager::blendFunc(4, 5);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::disableAlphaTest();
    MemSect(0x1f);
    mMinecraft->mTextures->bindTexture(&guiPumpkinOverlayTexture);
    MemSect(0);
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    builder->begin();
    builder->vertexUV(0.0f, static_cast<float>(height), -90.0f, 0.0f, 1.0f);
    builder->vertexUV(static_cast<float>(width), static_cast<float>(height), -90.0f, 1.0f, 1.0f);
    builder->vertexUV(static_cast<float>(width), 0.0f, -90.0f, 1.0f, 0.0f);
    builder->vertexUV(0.0f, 0.0f, -90.0f, 0.0f, 0.0f);
    builder->end();
    GlStateManager::depthMask(true);
    GlStateManager::enableDepthTest();
    GlStateManager::enableAlphaTest();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void Gui::renderPortalOverlay(float portalTime, int width, int height)
{
    if (portalTime < 1.0f)
    {
        portalTime *= portalTime;
        portalTime *= portalTime;
        portalTime = portalTime * 0.8f + 0.2f;
    }
    GlStateManager::disableAlphaTest();
    GlStateManager::disableDepthTest();
    GlStateManager::depthMask(false);
    GlStateManager::blendFunc(4, 5);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, portalTime);
    MemSect(0x1f);
    mMinecraft->mTextures->bindTexture(&BLOCKS_LOCATION);
    MemSect(0);
    TextureAtlasSprite* sprite = Blocks::portal->getTexture(Direction::UP);
    float minU = sprite->getU0(false);
    float minV = sprite->getV0(false);
    float maxU = sprite->getU1(false);
    float maxV = sprite->getV1(false);
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    builder->begin();
    builder->vertexUV(0.0f, static_cast<float>(height), -90.0f, minU, maxV);
    builder->vertexUV(static_cast<float>(width), static_cast<float>(height), -90.0f, maxU, maxV);
    builder->vertexUV(static_cast<float>(width), 0.0f, -90.0f, maxU, minV);
    builder->vertexUV(0.0f, 0.0f, -90.0f, minU, minV);
    builder->end();
    GlStateManager::depthMask(true);
    GlStateManager::enableDepthTest();
    GlStateManager::enableAlphaTest();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void Gui::renderThermalOverlay(ThermalAreaRuleDefinition* rule, float partialTick, int width, int height, float alpha)
{
    if (rule->getElytraSpeedBoost() > 0.0)
    {
        return;
    }
    GlStateManager::disableAlphaTest();
    GlStateManager::disableDepthTest();
    GlStateManager::depthMask(false);
    GlStateManager::blendFunc(4, 5);
    float textureAlpha = 1.0f;
    MemSect(0x1f);
    if (rule->getLiftForceModifier() > 1.0)
    {
        mMinecraft->mTextures->bindTexture(&guiThermalOverlayTexture);
        textureAlpha = 0.5f;
    }
    else if (rule->getMaxHeight() > 0.0 || rule->getLiftSpeed() != 0.0)
    {
        mMinecraft->mTextures->bindTexture(&guiThermalLiftOverlayTexture);
    }
    else
    {
        mMinecraft->mTextures->bindTexture(&guiThermalOverlayTexture);
    }
    MemSect(0);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, textureAlpha);
    const float top = static_cast<float>(height) * 0.25f;
    const float time = (static_cast<float>(Minecraft::GetInstance()->mGameRenderer->getTickCount()) + partialTick)
                       * -0.011111111f;
    const float bottomV = time + 1.0f;
    const float topV = time + (bottomV - time) * 0.25f;
    Tesselator* tesselator = Tesselator::getInstance();
    BufferBuilder* builder = tesselator->getBuilder();
    builder->begin();
    builder->color(1.0f, 1.0f, 1.0f, 0.5f * alpha);
    builder->vertexUV(0.0f, static_cast<float>(height), -90.0f, 0.0f, bottomV);
    builder->vertexUV(static_cast<float>(width), static_cast<float>(height), -90.0f, 6.0f, bottomV);
    builder->color(1.0f, 1.0f, 1.0f, 0.0f);
    builder->vertexUV(static_cast<float>(width), top, -90.0f, 6.0f, topV);
    builder->vertexUV(0.0f, top, -90.0f, 0.0f, topV);
    builder->end();
    GlStateManager::depthMask(true);
    GlStateManager::enableDepthTest();
    GlStateManager::enableAlphaTest();
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
}

void Gui::render(float partialTick, bool hideGui, int width, int height)
{
#ifdef MINECRAFT_PC
    gRenderer->PcCheckLeftovers("the HUD");
#endif
    const int playerIndex = mMinecraft->mPlayer->mUserIndex;
    int guiScale;
    if (mMinecraft->mPlayer->mViewport == C4JRender::eViewportType_FullScreen)
    {
        guiScale = GameSettings::GetGameSettings(playerIndex, eGameSetting_HUDSize) + 2;
    }
    else
    {
        guiScale = GameSettings::GetGameSettings(playerIndex, eGameSetting_HUDSizeSplitscreen) + 2;
    }
    ScreenSizeCalculator size(mMinecraft->mOptions, guiScale);
    const int screenWidth = size.getWidth();
    const int screenHeight = size.getHeight();
    int x = 0;
    int y = 0;
    float xScale = 1.0f;
    float yScale = 1.0f;
    bool joinedLayout = false;
    gGuiScreenScale = static_cast<float>(guiScale);

    int splitYOffset;
    switch (guiScale)
    {
    case 3:
        splitYOffset = 0;
        break;
    case 4:
        splitYOffset = -5;
        break;
    default:
        splitYOffset = 10;
        break;
    }

    switch (mMinecraft->mPlayer->mViewport)
    {
    case C4JRender::eViewportType_FullScreen:
        x = screenWidth / 20;
        y = screenHeight / 20;
        break;
    case C4JRender::eViewportType_TopHalf:
        gGuiScreenScale *= 0.5f;
        x = screenWidth / 10;
        y = screenHeight / 10;
        xScale = 0.5f;
        joinedLayout = true;
        break;
    case C4JRender::eViewportType_BottomHalf:
        gGuiScreenScale *= 0.5f;
        x = screenWidth / 10;
        y = 0;
        xScale = 0.5f;
        joinedLayout = true;
        break;
    case C4JRender::eViewportType_LeftHalf:
        gGuiScreenScale *= 0.5f;
        x = screenWidth / 10;
        y = screenHeight / 10;
        yScale = 0.5f;
        joinedLayout = true;
        break;
    case C4JRender::eViewportType_RightHalf:
        gGuiScreenScale *= 0.5f;
        x = 0;
        y = splitYOffset + screenHeight / 10;
        yScale = 0.5f;
        joinedLayout = true;
        break;
    case C4JRender::eViewportType_TopLeftQuarter:
        gGuiScreenScale *= 0.5f;
        x = screenWidth / 10;
        y = screenHeight / 10;
        break;
    case C4JRender::eViewportType_TopRightQuarter:
        gGuiScreenScale *= 0.5f;
        x = 0;
        y = screenHeight / 10;
        break;
    case C4JRender::eViewportType_BottomLeftQuarter:
        gGuiScreenScale *= 0.5f;
        x = screenWidth / 10;
        y = 0;
        break;
    case C4JRender::eViewportType_BottomRightQuarter:
        gGuiScreenScale *= 0.5f;
        x = 0;
        y = 0;
        break;
    default:
        break;
    }

    bool displayGui = !gConsoleUIController.GetMenuDisplayed(playerIndex)
                      && gApp.GetXuiAction(playerIndex) != eXuiAction_AutoSave;
    int tooltipsYOffset = 0;
    if (GameSettings::GetGameSettings(playerIndex, eGameSetting_Tooltips) == 0 && displayGui)
    {
        switch (mMinecraft->mPlayer->mViewport)
        {
        case C4JRender::eViewportType_FullScreen:
            tooltipsYOffset = screenHeight / 10;
            break;
        default:
            switch (guiScale)
            {
            case 3:
                tooltipsYOffset = 28;
                break;
            case 4:
                tooltipsYOffset = 28;
                break;
            default:
                tooltipsYOffset = 14;
                break;
            }
            break;
        }
    }
    if (GameSettings::GetGameSettings(playerIndex, eGameSetting_DisplayHUD) == 0)
    {
        displayGui = false;
    }

    mMinecraft->mGameRenderer->setupGuiScreen(guiScale);
    mMinecraft->mGameRenderer->renderItemActivationAnimation(screenWidth, screenHeight, partialTick);
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
#ifdef MINECRAFT_PC
    PcSetWideWorldPass(true);
#endif

    if (Minecraft::useFancyGraphics())
    {
        renderVignette(mMinecraft->mPlayer->getBrightness(), screenWidth, screenHeight);
    }

    not_null_ptr<ItemInstance> helmet = mMinecraft->mPlayer->mInventory->getArmor(3);
    if (!mMinecraft->mPlayer->GetThirdPersonView(false) && helmet->getItem() == Item::byBlock(Blocks::carved_pumpkin)
        && !mMinecraft->mPlayer->isSpectator())
    {
        renderPumpkin(screenWidth, screenHeight);
    }

    if (!mMinecraft->mPlayer->hasEffect(MobEffects::CONFUSION))
    {
        const float portalIntensity = mMinecraft->mPlayer->mPortalEffectIntensityO
                                      + (mMinecraft->mPlayer->mPortalEffectIntensity
                                         - mMinecraft->mPlayer->mPortalEffectIntensityO)
                                            * partialTick;
        if (portalIntensity > 0.0f)
        {
            renderPortalOverlay(portalIntensity, screenWidth, screenHeight);
        }
    }

    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (Minecraft::InMiniGame(eMiniGameId_Glide, true) && options && options->getRequiredGameRules())
    {
        std::vector<ThermalAreaRuleDefinition*> areas;
        options->getRequiredGameRules()->getThermalAreas(&areas);
        for (std::vector<ThermalAreaRuleDefinition*>::iterator it = areas.begin(); it != areas.end(); ++it)
        {
            if ((*it)->isEnabled() && mMinecraft->mPlayer->getFallFlyingTicks() > 4
                && (*it)->getConditionsMet(mMinecraft->mPlayer)
                && (*it)->getArea()->intersects(mMinecraft->mPlayer->getSweptVolume()))
            {
                mThermalOverlayRules[playerIndex] = *it;
                mThermalOverlayFade[playerIndex] = 30;
                break;
            }
        }
        if (mThermalOverlayRules[playerIndex] != NULL && mThermalOverlayFade[playerIndex] >= 0)
        {
            renderThermalOverlay(mThermalOverlayRules[playerIndex], partialTick, screenWidth, screenHeight,
                                 static_cast<float>(mThermalOverlayFade[playerIndex]) / 30.0f);
            --mThermalOverlayFade[playerIndex];
        }
        else
        {
            mThermalOverlayRules[playerIndex] = NULL;
            mThermalOverlayFade[playerIndex] = 0;
        }
    }
#ifdef MINECRAFT_PC
    PcSetWideWorldPass(false);
#endif

    if (!mMinecraft->mGameMode->isCutScene())
    {
        if (displayGui && joinedLayout)
        {
            GlStateManager::pushMatrix();
            GlStateManager::scalef(xScale, yScale, xScale);
        }
        if (GameSettings::GetGameSettings(playerIndex, eGameSetting_AnimatedCharacter) != 0 && displayGui)
        {
            if (mMinecraft->mPlayer->isSneaking() || mMinecraft->mPlayer->isSprinting())
            {
                mCharacterDisplayTime[playerIndex] = 30;
            }
            else if (mMinecraft->mPlayer->mAbilities.mFlying || mMinecraft->mPlayer->isFallFlying()
                     || mMinecraft->mPlayer->isFallFlying())
            {
                mCharacterDisplayTime[playerIndex] = 5;
            }
            else if (mCharacterDisplayTime[playerIndex] > 0)
            {
                --mCharacterDisplayTime[playerIndex];
            }

            if (mCharacterDisplayTime[playerIndex] > 0)
            {
                Minecraft::GetInstance()->getEntityRenderDispatcher()->prepare(
                    mMinecraft->mLevel, mMinecraft->mFont, mMinecraft->getCameraEntity(), mMinecraft->mSpectateEntity,
                    mMinecraft->mOptions, partialTick);

                GlStateManager::enableRescaleNormal();
                GlStateManager::enableColorMaterial();
                GlStateManager::pushMatrix();
                GlStateManager::translatef(static_cast<float>(x + 10), static_cast<float>(y + 17), 50.0f);
                GlStateManager::scalef(-12.0f, 12.0f, 12.0f);
                GlStateManager::rotatef(180.0f, 0.0f, 0.0f, 1.0f);
                const float savedYRot = mMinecraft->mPlayer->mYRot;
                const float savedYRotO = mMinecraft->mPlayer->mYRotO;
                const float savedXRot = mMinecraft->mPlayer->mXRot;
                const int savedFireTicks = mMinecraft->mPlayer->mRemainingFireTicks;
                const bool savedOnFire = mMinecraft->mPlayer->getSharedFlag(Entity::FLAG_ONFIRE);
                GlStateManager::rotatef(135.0f, 0.0f, 1.0f, 0.0f);
                Lighting::turnOn();
                GlStateManager::rotatef(-135.0f, 0.0f, 1.0f, 0.0f);
                GlStateManager::rotatef(-(std::atan(0.25f) * 20.0f), 1.0f, 0.0f, 0.0f);
                const float bodyYaw = mMinecraft->mPlayer->mYBodyRotO
                                      + (mMinecraft->mPlayer->mYBodyRot - mMinecraft->mPlayer->mYBodyRotO);
                GlStateManager::rotatef(bodyYaw - -15.707964f, 0.0f, 1.0f, 0.0f);
                GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
                if (!mMinecraft->mPlayer->isFallFlying())
                {
                    mMinecraft->mPlayer->mYRot = bodyYaw;
                    mMinecraft->mPlayer->mYRotO = mMinecraft->mPlayer->mYRot;
                    mMinecraft->mPlayer->mXRot = -(std::atan(0.25f) * 20.0f);
                }
                mMinecraft->mPlayer->clearFire();
                mMinecraft->mPlayer->setSharedFlag(Entity::FLAG_ONFIRE, false);
                if (mMinecraft->mPlayer->isFallFlying())
                {
                    GlStateManager::translatef(
                        0.0f, 1.62f * (Mth::clamp(mMinecraft->mPlayer->mXRot, -90.0f, 90.0f) / 90.0f), 0.0f);
                }
                else
                {
                    GlStateManager::translatef(0.0f, -1.62f, 0.0f);
                }
                Minecraft::GetInstance()->getEntityRenderDispatcher()->mCameraYRot = 180.0f;
                Minecraft::GetInstance()->getEntityRenderDispatcher()->mRenderingGuiPlayer = true;
                boost::shared_ptr<Entity> entity = mMinecraft->mPlayer;
                Minecraft::GetInstance()->getEntityRenderDispatcher()->render(entity, 0.0, 0.0, 0.0, 0.0f, 1.0f, false,
                                                                              false, true, false);
                Minecraft::GetInstance()->getEntityRenderDispatcher()->mRenderingGuiPlayer = false;
                mMinecraft->mPlayer->mYRot = savedYRot;
                mMinecraft->mPlayer->mYRotO = savedYRotO;
                mMinecraft->mPlayer->mXRot = savedXRot;
                mMinecraft->mPlayer->mRemainingFireTicks = savedFireTicks;
                mMinecraft->mPlayer->setSharedFlag(Entity::FLAG_ONFIRE, savedOnFire);
                GlStateManager::popMatrix();
                Lighting::turnOff();
                GlStateManager::disableRescaleNormal();
            }
        }
    }

    GlStateManager::disableBlend();
    GlStateManager::blendFunc(4, 5);
    if (mMinecraft->mPlayer->getSleepTimer() > 0)
    {
        GlStateManager::disableDepthTest();
        GlStateManager::disableAlphaTest();
        const int sleepTimer = mMinecraft->mPlayer->getSleepTimer();
        float alpha = static_cast<float>(sleepTimer) / 100.0f;
        if (alpha > 1.0f)
        {
            alpha = 1.0f - static_cast<float>(sleepTimer - 100) / 10.0f;
        }
        const int colour = (static_cast<int>(220.0f * alpha) << 24) | 0x101020;
        fill(0, 0, static_cast<int>(static_cast<float>(screenWidth) / xScale),
             static_cast<int>(static_cast<float>(screenHeight) / yScale), colour);
        GlStateManager::enableAlphaTest();
        GlStateManager::enableDepthTest();
    }
    if (!mMinecraft->mPlayer->isAlive())
    {
        GlStateManager::disableDepthTest();
        GlStateManager::disableAlphaTest();
        const float alpha = static_cast<float>(mMinecraft->mPlayer->getDeathFadeTimer()) / 21.0f;
        const int colour = (static_cast<int>(220.0f * alpha) << 24) | 0x200000;
        fill(0, 0, static_cast<int>(static_cast<float>(screenWidth) / xScale),
             static_cast<int>(static_cast<float>(screenHeight) / yScale), colour);
        GlStateManager::enableAlphaTest();
        GlStateManager::enableDepthTest();
    }

    mJukeboxTime = partialTick;
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::disableAlphaTest();
    if (displayGui && joinedLayout)
    {
        GlStateManager::popMatrix();
    }
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::disableBlend();
    GlStateManager::enableAlphaTest();

    gRenderer->TextureBindVertex(-1);
    GlStateManager::disableDepthTest();
    GlStateManager::disableLighting();
    GlStateManager::enableBlend();
    GlStateManager::blendFunc(4, 5);
    GlStateManager::pushMatrix();
    GlStateManager::scalef(0.5f, 0.5f, 1.0f);
    mMinecraft->mFont->drawShadow(L" ", 4, 4, 0xAAFFFFFF);
    GlStateManager::popMatrix();
#ifdef MINECRAFT_PC
    {
        std::vector<std::wstring> debugLeft;
        std::vector<std::wstring> debugRight;
        if (PcDebugOverlayLines(mMinecraft, &debugLeft, &debugRight))
        {
            const C4JRender::eViewportType viewport = (C4JRender::eViewportType)mMinecraft->mPlayer->mViewport;
            const bool halfWidth = viewport == C4JRender::eViewportType_LeftHalf
                                   || viewport == C4JRender::eViewportType_RightHalf
                                   || viewport >= C4JRender::eViewportType_TopLeftQuarter;
            const bool halfHeight = viewport == C4JRender::eViewportType_TopHalf
                                    || viewport == C4JRender::eViewportType_BottomHalf
                                    || viewport >= C4JRender::eViewportType_TopLeftQuarter;
            const float viewWidth = halfWidth ? 640.0f : 1280.0f;
            const float viewHeight = halfHeight ? 360.0f : 720.0f;
            const float debugScaleX = 2.0f * static_cast<float>(size.mWidthScale) / viewWidth;
            const float debugScaleY = 2.0f * static_cast<float>(size.mHeightScale) / viewHeight;
            const int debugWidth = static_cast<int>(viewWidth / 2.0f);
            const int debugTop = 10;
            GlStateManager::pushMatrix();
            GlStateManager::scalef(debugScaleX, debugScaleY, 1.0f);
            for (unsigned int line = 0; line < debugLeft.size(); ++line)
            {
                if (debugLeft[line].empty())
                    continue;
                const int lineY = debugTop + static_cast<int>(line) * 9;
                const int lineWidth = mMinecraft->mFont->width(debugLeft[line]);
                fill(1, lineY - 1, 2 + lineWidth + 1, lineY + 8, 0x90505050);
                mMinecraft->mFont->draw(debugLeft[line], 2, lineY, 0xFFE0E0E0);
            }
            for (unsigned int line = 0; line < debugRight.size(); ++line)
            {
                if (debugRight[line].empty())
                    continue;
                const int lineY = debugTop + static_cast<int>(line) * 9;
                const int lineWidth = mMinecraft->mFont->width(debugRight[line]);
                const int lineX = debugWidth - 2 - lineWidth;
                fill(lineX - 1, lineY - 1, lineX + lineWidth + 1, lineY + 8, 0x90505050);
                mMinecraft->mFont->draw(debugRight[line], lineX, lineY, 0xFFE0E0E0);
            }
            GlStateManager::popMatrix();
        }
    }
    if (mMinecraft->mPlayer->mViewport == C4JRender::eViewportType_FullScreen && displayGui
        && (PcScriptsClientHasHud() || PcNativeModsWantHud()))
        renderScriptHud(2.0f * static_cast<float>(size.mWidthScale) / 1280.0f,
                          2.0f * static_cast<float>(size.mHeightScale) / 720.0f);
    if (mMinecraft->mPlayer->mViewport == C4JRender::eViewportType_FullScreen && (displayGui || PcChatActive()))
        renderChat(playerIndex, screenHeight);
#endif
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::disableBlend();
    GlStateManager::enableDepthTest();
}

#ifdef MINECRAFT_PC
void Gui::renderChat(int playerIndex, int screenHeight)
{
    Font* font = mMinecraft->mFont;
    const bool open = PcChatActive();
    const float chatScale = 2.0f / gGuiScreenScale;
    const int spaceHeight = static_cast<int>(static_cast<float>(screenHeight) * gGuiScreenScale / 2.0f);
    const int inputHeight = 12;
    const int bottom = spaceHeight * 64 / 100;

    std::vector<std::wstring> lineText;
    std::vector<float> lineOpacity;
    const unsigned int maxLines = open ? CHAT_LINES_OPEN : CHAT_LINES_CLOSED;
    for (unsigned int i = 0; i < mMessages[playerIndex].size() && lineText.size() < maxLines; ++i)
    {
        const float opacity = open ? 1.0f : getOpacity(playerIndex, i);
        if (opacity <= 0.0f)
            break;
        std::vector<std::wstring> wrapped;
        WrapChatLine(font, mMessages[playerIndex][i].message, CHAT_WIDTH, &wrapped);
        for (std::vector<std::wstring>::size_type k = wrapped.size(); k > 0 && lineText.size() < maxLines; --k)
        {
            lineText.push_back(wrapped[k - 1]);
            lineOpacity.push_back(opacity);
        }
    }
    if (lineText.empty() && !open)
        return;

    const float widthScale = PcGetUiWidthScale();
    PcSetWideUiPass(true);
    gRenderer->TextureBindVertex(-1);
    GlStateManager::pushMatrix();
    GlStateManager::scalef(1.0f / widthScale, 1.0f, 1.0f);
    GlStateManager::scalef(chatScale, chatScale, 1.0f);
    for (std::vector<std::wstring>::size_type i = 0; i < lineText.size(); ++i)
    {
        const int alpha = static_cast<int>(lineOpacity[i] * 255.0f);
        if (alpha <= 3)
            continue;
        const int top = bottom - static_cast<int>(i + 1) * CHAT_LINE_HEIGHT;
        fill(CHAT_LEFT - 2, top, CHAT_LEFT + CHAT_WIDTH + 2, top + CHAT_LINE_HEIGHT, (alpha / 2) << 24);
        font->drawShadow(lineText[i], CHAT_LEFT, top + 1, 0xFFFFFF | (alpha << 24));
    }
    if (open)
    {
        const int spaceWidth = static_cast<int>(640.0f * widthScale);
        const int top = spaceHeight - inputHeight - 2;
        fill(2, top, spaceWidth - 2, top + inputHeight, 0x80000000);
        PcTextEntryDrawLine(font, this, CHAT_LEFT, top + 2, spaceWidth - 12, static_cast<int>(0xFFE0E0E0u),
                            (mTickCount / 6) % 2 == 0);
    }
    GlStateManager::popMatrix();
    PcSetWideUiPass(false);
}

void Gui::renderScriptHud(float scaleX, float scaleY)
{
    std::vector<PcScriptHudItem> items;
    PcScriptsClientHud(640, 360, &items);
    PcNativeModsHud(&items);
    if (items.empty())
        return;
    Font* font = mMinecraft->mFont;
    gRenderer->TextureBindVertex(-1);
    GlStateManager::pushMatrix();
    GlStateManager::scalef(scaleX, scaleY, 1.0f);
    for (std::vector<PcScriptHudItem>::const_iterator it = items.begin(); it != items.end(); ++it)
    {
        if (!it->text)
            fill(it->x1, it->y1, it->x2, it->y2, static_cast<int>(it->colour));
        else if (it->shadow)
            font->drawShadow(it->string, it->x1, it->y1, static_cast<int>(it->colour));
        else
            font->draw(it->string, it->x1, it->y1, static_cast<int>(it->colour));
    }
    GlStateManager::popMatrix();
}
#endif

void Gui::renderSlot(int playerIndex, int x, int y, float partialTick)
{
    not_null_ptr<ItemInstance> item = mMinecraft->mPlayer->mInventory->m_items[playerIndex];
    if (item->isEmpty())
    {
        return;
    }

    const float popRemaining = static_cast<float>(item->getPopTime()) - partialTick;
    if (popRemaining > 0.0f)
    {
        GlStateManager::pushMatrix();
        const float popScale = popRemaining / 5.0f + 1.0f;
        GlStateManager::translatef(static_cast<float>(x + 8), static_cast<float>(y + 12), 0.0f);
        GlStateManager::scalef(1.0f / popScale, (popScale + 1.0f) * 0.5f, 1.0f);
        GlStateManager::translatef(static_cast<float>(-(x + 8)), static_cast<float>(-(y + 12)), 0.0f);
    }

    mItemRenderer->renderAndDecorateItem(item, x, y);

    if (popRemaining > 0.0f)
    {
        GlStateManager::popMatrix();
    }

    mItemRenderer->renderGuiItemDecorations(mMinecraft->mFont, item, x, y, 1.0f);
}

void Gui::tick()
{
    int i;
    ++mTickCount;
    for (i = 0; i < 4; ++i)
    {
        if (mSingleMessageTime[i] > 0)
        {
            --mSingleMessageTime[i];
        }
        if (!gConsoleUIController.GetMenuDisplayed(i))
        {
            std::vector<GuiMessage>::iterator end = mMessages[i].end();
            for (std::vector<GuiMessage>::iterator it = mMessages[i].begin(); it != end; ++it)
            {
                ++it->age;
            }
        }
    }
}

void Gui::clearMessages(int playerIndex)
{
    if (playerIndex == -1)
    {
        int i;
        for (i = 0; i < 4; ++i)
        {
            if (mMinecraft->mLocalPlayers[i] != NULL)
            {
                mMessages[i].clear();
            }
        }
    }
    else
    {
        mMessages[playerIndex].clear();
    }
}

void Gui::addMessage(const std::wstring& message, int playerIndex, bool overlay, bool filtered, bool append, bool force)
{
    std::wstring text = message;
#ifdef MINECRAFT_PC
    if (!append && !PcScriptsClientChat(&text))
        return;
#endif

    int maxLength;
    switch (mMinecraft->mPlayer->mViewport)
    {
    case C4JRender::eViewportType_FullScreen:
    case C4JRender::eViewportType_TopHalf:
    case C4JRender::eViewportType_BottomHalf:
        maxLength = 0x37;
        if (gRenderer->IsHiDef())
        {
            maxLength = 0x69;
        }
        switch (XGetLanguage())
        {
        case XC_LANGUAGE_JAPANESE:
        case XC_LANGUAGE_KOREAN:
        case XC_LANGUAGE_TCHINESE:
            if (gRenderer->IsHiDef())
            {
                maxLength = 0x46;
            }
            else
            {
                maxLength = 0x23;
            }
            break;
        }
        break;
    default:
        maxLength = 0x37;
        switch (XGetLanguage())
        {
        case XC_LANGUAGE_JAPANESE:
        case XC_LANGUAGE_KOREAN:
        case XC_LANGUAGE_TCHINESE:
            maxLength = 0x23;
            break;
        }
        break;
    }

    bool wrapped = false;
    while (text.length() > maxLength)
    {
        unsigned int searchEnd = 1;
        while (searchEnd < text.length() && static_cast<int>(searchEnd + 1) <= maxLength)
        {
            ++searchEnd;
        }
        unsigned int split = text.find_last_of(L" ", searchEnd);
        switch (XGetLanguage())
        {
        case XC_LANGUAGE_JAPANESE:
        case XC_LANGUAGE_KOREAN:
        case XC_LANGUAGE_TCHINESE:
            split = maxLength;
            break;
        default:
            split = text.find_last_of(L" ", searchEnd);
            break;
        }
        if (split != searchEnd)
        {
            ++split;
        }
        addMessage(text.substr(0, split), playerIndex, overlay, filtered, true, false);
        text = text.substr(split);
        wrapped = true;
    }

    if (playerIndex == -1)
    {
        int i;
        for (i = 0; i < 4; ++i)
        {
            if (mMinecraft->mLocalPlayers[i] == NULL)
            {
                continue;
            }
            const bool display = overlay ? GameSettings::GetGameSettings(i, eGameSetting_DeathMessages) != 0
                                         : GameSettings::GetGameSettings(i, eGameSetting_DisplayGameMessages) != 0;
            if (display || force)
            {
                mMessages[i].insert(mMessages[i].begin(), GuiMessage(text));
                while (mMessages[i].size() > 50)
                {
                    mMessages[i].pop_back();
                }
            }
        }
    }
    else
    {
        const bool display = overlay
                                 ? GameSettings::GetGameSettings(playerIndex, eGameSetting_DeathMessages) != 0
                                 : GameSettings::GetGameSettings(playerIndex, eGameSetting_DisplayGameMessages) != 0;
        if (display || force)
        {
            mMessages[playerIndex].insert(mMessages[playerIndex].begin(), GuiMessage(text));
            while (mMessages[playerIndex].size() > 50)
            {
                mMessages[playerIndex].pop_back();
            }
        }
        if (overlay && filtered)
        {
            if (append)
            {
                mNowPlaying[playerIndex] = text + L"\n";
            }
            else if (wrapped)
            {
                mNowPlaying[playerIndex] += text;
            }
            else
            {
                mNowPlaying[playerIndex] = text;
            }
        }
    }
}

void Gui::clearSingleMessages()
{
    for (int i = 0; i < 4; i++)
    {
        mSingleMessageTime[i] = 0;
        mSingleMessages[i] = L"";
    }
    mSingleMessageVisible = false;
}

void Gui::clearSingleMessages(int playerIndex)
{
    mSingleMessageTime[playerIndex] = 0;
    mSingleMessages[playerIndex] = L"";
    mSingleMessageVisible = false;
}

void Gui::setSingleMessage(const std::wstring& message, int playerIndex, int duration)
{
    mSingleMessages[playerIndex] = message;
    if (duration < 1)
    {
        duration = 0xb4;
    }
    mSingleMessageTime[playerIndex] = duration;
}

void Gui::setNowPlaying(const std::wstring& message)
{
    for (unsigned int i = 0; i < 4; i++)
    {
        mSingleMessages[i] = L10N::GetString(IDS_NOWPLAYING) + message;
        mSingleMessageTime[i] = 60;
    }
    mSingleMessageVisible = true;
}

void Gui::displayClientMessage(int stringId, int playerIndex)
{
    addMessage(std::wstring(L10N::GetString(stringId)), playerIndex, false, false, false, false);
}

void Gui::onDisconnected()
{
    for (int i = 0; i < 4; i++)
    {
        mBossOverlays[i]->reset();
    }
}

float Gui::getOpacity(int playerIndex, unsigned int messageIndex)
{
    float opacity = 0.0f;
    if (mMessages[playerIndex].size() > messageIndex && mMessages[playerIndex][messageIndex].age < 200)
    {
        float value = static_cast<float>(mMessages[playerIndex][messageIndex].age) / 200.0f;
        value = (1.0f - value) * 10.0f;
        if (value < 0.0f)
        {
            value = 0.0f;
        }
        else if (value > 1.0f)
        {
            value = 1.0f;
        }
        opacity = value * value;
    }
    return opacity;
}

float Gui::getJukeboxOpacity(int playerIndex)
{
    float elapsed = static_cast<float>(mSingleMessageTime[playerIndex]) - mJukeboxTime;
    float value = elapsed * 256.0f / 20.0f;
    if (value > 255.0f)
    {
        value = 255.0f;
    }
    value = value / 255.0f;
    if (value < 0.0f)
    {
        value = 0.0f;
    }
    return value;
}

void Gui::ShowCharacterDisplay(int playerIndex)
{
    mCharacterDisplayTime[playerIndex] = 90;
}
