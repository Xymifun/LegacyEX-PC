#include "game/client/render/SignRenderer.hpp"

#include "game/client/render/Font.hpp"
#include "game/client/render/ModelDefinition.hpp"
#include "game/client/render/ModelPartSkin.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/SignBlockEntity.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/XLanguage.hpp"


static ResourceLocation SIGN_LOCATION(TN_ITEM_SIGN);

SignRenderer::SignRenderer()
{
    mModel = new SignModel();
    mTextDisplayLists = gRenderer->CBuffCreate(200);
    for (int i = 0; i < 200; ++i)
        mTextCache[i].uniqueId = -1;
    mTextCacheCount = 0;
    mCensorText = false;
    mLanguage = XGetLanguage();
}

SignRenderer::~SignRenderer()
{
}

void SignRenderer::CensorString(std::wstring& text)
{
    if (text.compare(L" ") != 0)
    {
        unsigned int language = XGetLanguage();
        if (language == XC_LANGUAGE_JAPANESE || language == XC_LANGUAGE_KOREAN || language == XC_LANGUAGE_TCHINESE
            || language == XC_LANGUAGE_ZH_HANS)
            text = L"Censored";
        else
            text = L10N::GetString(IDS_STRINGVERIFY_CENSORED);
    }
}

void SignRenderer::render(boost::shared_ptr<BlockEntity> blockEntity, double x, double y, double z, float partialTicks,
                          int breakProgress, bool translucent, float alpha, bool useDisplayList)
{
    boost::shared_ptr<SignBlockEntity> sign = boost::static_pointer_cast<SignBlockEntity>(blockEntity);
    Block* block = sign->getBlock();

    GlStateManager::pushMatrix();
    if (block == Blocks::standing_sign)
    {
        GlStateManager::translatef((float)x + 0.5f, (float)y + 0.5f, (float)z + 0.5f);
        GlStateManager::rotatef(-((float)(sign->getData() * 360) * 0.0625f), 0.0f, 1.0f, 0.0f);
        mModel->mSignStick->mShow = true;
    }
    else
    {
        int data = sign->getData();
        float rotation = 0.0f;
        if (data == 2)
            rotation = 180.0f;
        else if (data == 4)
            rotation = 90.0f;
        else if (data == 5)
            rotation = -90.0f;
        GlStateManager::translatef((float)x + 0.5f, (float)y + 0.5f, (float)z + 0.5f);
        GlStateManager::rotatef(-rotation, 0.0f, 1.0f, 0.0f);
        GlStateManager::translatef(0.0f, -0.3125f, -0.4375f);
        mModel->mSignStick->mShow = false;
    }

    if (breakProgress >= 0)
    {
        bindTexture(&BREAKING_LOCATIONS[breakProgress]);
        GlStateManager::matrixMode(2);
        GlStateManager::pushMatrix();
        GlStateManager::scalef(4.0f, 2.0f, 1.0f);
        GlStateManager::translatef(0.0625f, 0.0625f, 0.0625f);
        GlStateManager::matrixMode(0);
    }
    else
    {
        bindTexture(&SIGN_LOCATION);
    }

    GlStateManager::pushMatrix();
    GlStateManager::scalef(0.6666667f, -0.6666667f, -0.6666667f);
    mModel->render(true);
    GlStateManager::popMatrix();

    Font* font = getFont();
    GlStateManager::translatef(0.0f, 0.33333334f, 0.046666667f);
    GlStateManager::scalef(0.010416667f, -0.010416667f, 0.010416667f);
    GlStateManager::normal3f(0.0f, 0.0f, -0.010416667f);
    GlStateManager::depthMask(false);

    unsigned int colour = Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Sign_Text);
    colour |= 0xff000000;
    std::wstring text = L"";
    int cacheIndex = -1;

    if (breakProgress < 0)
    {
        GlStateManager::disableLighting();
        int uniqueId = sign->getUniqueId();
        cacheIndex = sign->getTextCacheIndex();
        if (mTextCache[cacheIndex].uniqueId != uniqueId)
        {
            cacheIndex = -1;
            if (sign->getTextCacheEnabled() && mTextCacheCount < 200)
            {
                for (int i = 0; i < 200; ++i)
                {
                    if (mTextCache[i].uniqueId == -1)
                    {
                        cacheIndex = i;
                        break;
                    }
                }
                if (cacheIndex != -1)
                    gRenderer->CBuffStart(mTextDisplayLists + cacheIndex, false);
            }

            BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
            builder->begin();
            builder->color((int)colour);

            bool localCreator = false;
            if (sign->getCreatedPlayerUUID() != L"")
            {
                for (int i = 0; i < 4; ++i)
                {
                    boost::shared_ptr<MultiplayerLocalPlayer> player = Minecraft::GetInstance()->mLocalPlayers[i];
                    if (player && player->getUUID() == sign->getCreatedPlayerUUID())
                    {
                        localCreator = true;
                        break;
                    }
                }
            }

            bool rendered = true;
            for (int line = 0; line < 4; ++line)
            {
                text = std::wstring(sign->mMessages[line]);
#ifndef MINECRAFT_PC
                if ((mCensorText || sign->wasCreatedByRestrictedPlayer()) && !localCreator
                    && !(CGameNetworkManager::sInstance.IsHost() && sign->wasCreatedByHostPlayer()))
                {
                    CensorString(text);
                }
                else if (sign->IsVerified())
                {
                    if (sign->IsCensored())
                        CensorString(text);
                }
                else if (mLanguage == XC_LANGUAGE_JAPANESE || mLanguage == XC_LANGUAGE_KOREAN
                         || mLanguage == XC_LANGUAGE_TCHINESE || mLanguage == XC_LANGUAGE_ZH_HANS)
                {
                    text = L"Awaiting Approval";
                }
                else
                {
                    text = L10N::GetString(IDS_STRINGVERIFY_AWAITING_APPROVAL);
                }
#endif

                if (line == sign->mSelectedLine)
                    text = L"> " + text + L" <";

                rendered &= font->drawNoBeginEndBuilder(text, -font->width(text) / 2, line * 10 - 20, (int)colour);
            }
            builder->end();

            if (cacheIndex != -1)
            {
                gRenderer->CBuffEnd();
                if (rendered)
                {
                    mTextCache[cacheIndex].uniqueId = sign->getUniqueId();
                    ++mTextCacheCount;
                    sign->setTextCacheIndex(cacheIndex);
                }
                else
                {
                    sign->disableTextCache();
                }
            }
        }
        else
        {
            font->bindTexture();
        }

        GlStateManager::color3f(1.0f, 1.0f, 1.0f);
        if (cacheIndex != -1)
        {
            gRenderer->CBuffCall(mTextDisplayLists + cacheIndex, true, 0xff);
            mTextCache[cacheIndex].age = 0;
        }
        GlStateManager::enableLighting();
    }

    GlStateManager::depthMask(true);
    GlStateManager::color4f(1.0f, 1.0f, 1.0f, 1.0f);
    GlStateManager::popMatrix();
    if (breakProgress >= 0)
    {
        GlStateManager::matrixMode(2);
        GlStateManager::popMatrix();
        GlStateManager::matrixMode(0);
    }
}

void SignRenderer::tick()
{
    bool censorText = gApp.IsConsoleChatCensored();
    int language = XGetLanguage();
    if (censorText != mCensorText || mLanguage != language)
    {
        mLanguage = language;
        mCensorText = censorText;
        for (int i = 0; i < 200; ++i)
            mTextCache[i].uniqueId = -1;
        mTextCacheCount = 0;
    }
    else
    {
        for (int i = 0; i < 200; ++i)
        {
            if (mTextCache[i].uniqueId != -1 && ++mTextCache[i].age > 20)
            {
                mTextCache[i].uniqueId = -1;
                --mTextCacheCount;
            }
        }
    }
}
