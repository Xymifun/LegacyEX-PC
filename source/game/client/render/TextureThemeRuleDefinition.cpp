#include "game/client/render/TextureThemeRuleDefinition.hpp"

#include "game/core/BufferedImage.hpp"
#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/DLCFile.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/GameRules_IntAttribute.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/core/StringUtils.hpp"

static const std::wstring gColourReplacements[16][2] = {
    {L"white", L"0xFFFFFF"}, {L"silver", L"0xC0C0C0"}, {L"gray", L"0x808080"},    {L"black", L"0x000000"},
    {L"red", L"0xFF0000"},   {L"maroon", L"0x800000"}, {L"yellow", L"0xFFFF00"},  {L"olive", L"0x808000"},
    {L"lime", L"0x00FF00"},  {L"green", L"0x008000"},  {L"cyan", L"0x00FFFF"},    {L"teal", L"0x008080"},
    {L"blue", L"0x0000FF"},  {L"navy", L"0x000080"},   {L"magenta", L"0xFF00FF"}, {L"purple", L"0x800080"}};

TextureThemeRuleDefinition::TextureThemeRuleDefinition()
{
}

TextureThemeRuleDefinition::~TextureThemeRuleDefinition()
{
    std::vector<LayerTexture*>::iterator textureEnd = mLayerTextures.end();
    for (std::vector<LayerTexture*>::iterator it = mLayerTextures.begin(); it != textureEnd; ++it)
    {
        delete *it;
    }
    std::vector<BlockStackGetter*>::iterator getterEnd = mBlockStackGetters.end();
    for (std::vector<BlockStackGetter*>::iterator it = mBlockStackGetters.begin(); it != getterEnd; ++it)
    {
        delete *it;
    }
}

ConsoleGameRules::EGameRuleType TextureThemeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_TextureTheme;
}

LayerTheme* TextureThemeRuleDefinition::Generate() const
{
    TextureTheme* theme = new TextureTheme();
    for (std::vector<LayerTexture*>::const_iterator it = mLayerTextures.begin(); it != mLayerTextures.end(); ++it)
    {
        LayerTexture* texture = *it;
        CommonMasterGameMode* gameMode = CommonMasterGameMode::GetMasterOrClient();
        LevelGenerationOptions* options = gameMode->getRoundLevelGenOptions();
        DLCPack* pack = options->mPack;
        DLCFile* file = pack->getFile(DLCManager::eDLCType_Texture, texture->mFilename, true, true);
        unsigned int length;
        BufferedImage* image = new BufferedImage(file->getData(length), length);
        theme->addAsset(static_cast<unsigned int>(texture->mMinimumWidth), image);
    }
    for (std::vector<BlockStackGetter*>::const_iterator it = mBlockStackGetters.begin(); it != mBlockStackGetters.end();
         ++it)
    {
        BlockStackGetter* getter = *it;
        TextureTheme::BlockStack stack;
        getter->getStack(stack);
        std::vector<std::wstring> colours = getter->getColours();
        for (std::vector<std::wstring>::iterator colour = colours.begin(); colour != colours.end(); ++colour)
        {
            theme->addStack(*colour, stack);
        }
    }
    return theme;
}

GameRuleDefinition* TextureThemeRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    switch (type)
    {
    case ConsoleGameRules::eGameRuleType_ShaftsTheme:
    {
        MaskedShaftsThemeRuleDefinition* shafts = new MaskedShaftsThemeRuleDefinition();
        mBlockStackGetters.push_back(shafts);
        return shafts;
    }
    case ConsoleGameRules::eGameRuleType_LayerAsset:
    {
        LayerTexture* texture = new LayerTexture();
        mLayerTextures.push_back(texture);
        return texture;
    }
    case ConsoleGameRules::eGameRuleType_BlockDef:
    {
        IndexedBlockDef* block = new IndexedBlockDef();
        mBlockStackGetters.push_back(block);
        return block;
    }
    }
    return GameRuleDefinition::addChild(type);
}

std::vector<std::wstring> TextureThemeRuleDefinition::BlockStackGetter::getColours() const
{
    std::vector<std::wstring> colours = stringSplit(mColours, L',');
    for (std::vector<std::wstring>::iterator colour = colours.begin(); colour != colours.end(); ++colour)
    {
        for (int i = 0; i < 16; ++i)
        {
            if (colour->compare(gColourReplacements[i][0]) == 0)
            {
                colour->assign(gColourReplacements[i][1]);
            }
        }
    }
    return colours;
}

void TextureThemeRuleDefinition::IndexedBlockDef::getStack(TextureTheme::BlockStack& stack)
{
    stack = TextureTheme::BlockStack(getBlockState(), 1, 0);
}

TextureThemeRuleDefinition::LayerTexture::LayerTexture() : mMinimumWidth(0)
{
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_name, &mFilename);
    addIntAttribute(ConsoleGameRules::eGameRuleAttr_size, &mMinimumWidth)->setMin(0)->setMax(128);
}

ConsoleGameRules::EGameRuleType TextureThemeRuleDefinition::LayerTexture::getActionType()
{
    return ConsoleGameRules::eGameRuleType_LayerAsset;
}

void TextureThemeRuleDefinition::MaskedShaftsThemeRuleDefinition::getStack(TextureTheme::BlockStack& stack)
{
    std::vector<const BlockState*> blocks;
    fillStack(blocks);
    stack = TextureTheme::BlockStack(blocks, mHeightOffset);
}
