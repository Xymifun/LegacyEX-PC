#ifndef GAME_WORLD_BLOCK_GENERICMULTIBLOCKTHEMERULEDEFINITION_HPP
#define GAME_WORLD_BLOCK_GENERICMULTIBLOCKTHEMERULEDEFINITION_HPP

#include <vector>

#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"

class BlockState;
class LayerTheme;

class GenericMultiBlockThemeRuleDefinition : public LayerAttributeChildRuleDefinition<LayerTheme*>
{
public:
    class BlockRuleDefinition : public GameRuleDefinition
    {
    public:
        BlockRuleDefinition();

        virtual ~BlockRuleDefinition() {}

        virtual ConsoleGameRules::EGameRuleType getActionType();

        const BlockState* getBlockState() const;
        int getBlockId() const;
        int getBlockAux() const;

    private:
        const BlockState* mBlockState;
        int mBlockAux;
    };

    virtual ~GenericMultiBlockThemeRuleDefinition() {}

    virtual GameRuleDefinition* addChild(ConsoleGameRules::EGameRuleType type);

protected:
    void fillStack(std::vector<const BlockState*>& stack) const;

    std::vector<BlockRuleDefinition*> mBlocks;
};

#endif
