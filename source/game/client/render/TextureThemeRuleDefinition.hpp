#ifndef GAME_CLIENT_RENDER_TEXTURETHEMERULEDEFINITION_HPP
#define GAME_CLIENT_RENDER_TEXTURETHEMERULEDEFINITION_HPP

#include <string>
#include <vector>

#include "game/minigame/GameRuleDefinition.hpp"
#include "game/client/render/TextureTheme.hpp"
#include "game/world/block/GenericMultiBlockThemeRuleDefinition.hpp"
#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"
#include "game/world/generation/LayerTheme.hpp"
#include "game/world/generation/ShaftsLayerThemeRuleDefinition.hpp"

class TextureThemeRuleDefinition : public LayerAttributeChildRuleDefinition<LayerTheme*>
{
public:
    class BlockStackGetter
    {
    public:
        virtual void getStack(TextureTheme::BlockStack& stack) = 0;
        virtual ~BlockStackGetter() {}
        std::vector<std::wstring> getColours() const;

    protected:
        std::wstring mColours;
    };

    class IndexedBlockDef : public GenericMultiBlockThemeRuleDefinition::BlockRuleDefinition, public BlockStackGetter
    {
    public:
        IndexedBlockDef() { addStringAttribute(ConsoleGameRules::eGameRuleAttr_colour, &mColours); }
        virtual void getStack(TextureTheme::BlockStack& stack);
    };

    class LayerTexture : public GameRuleDefinition
    {
    public:
        LayerTexture();
        virtual ConsoleGameRules::EGameRuleType getActionType();
        std::wstring mFilename;
        int mMinimumWidth;
    };

    class MaskedShaftsThemeRuleDefinition : public ShaftsLayerThemeRuleDefinition, public BlockStackGetter
    {
    public:
        MaskedShaftsThemeRuleDefinition() { addStringAttribute(ConsoleGameRules::eGameRuleAttr_colour, &mColours); }
        virtual void getStack(TextureTheme::BlockStack& stack);
    };

    TextureThemeRuleDefinition();
    virtual ~TextureThemeRuleDefinition();
    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual LayerTheme* Generate() const;
    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);

private:
    std::wstring mValue;
    std::vector<LayerTexture*> mLayerTextures;
    std::vector<BlockStackGetter*> mBlockStackGetters;
};

#endif
