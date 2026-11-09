#ifndef GAME_CORE_DIRECTIONALDEGRADE_HPP
#define GAME_CORE_DIRECTIONALDEGRADE_HPP

#include <string>
#include <vector>

#include "game/minigame/DegradationRuleDefinition.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockDegradeRoutine.hpp"
#include "game/world/block/BlockPos.hpp"

class DirectionalDegrade : public DegradationRuleDefinition
{
public:
    enum EDirection
    {
        eDirection_North = 1,
        eDirection_South,
        eDirection_East,
        eDirection_West,
        eDirection_ToEdges,
        eDirection_ToCentre
    };

    DirectionalDegrade();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount);
    virtual void addAttribute(const std::wstring& name, const std::wstring& value);
    virtual void ClearCaches();
    virtual void NextBlocks(BlockDegradeRoutine::LayerCache* layerCache, arrayWithLength<BlockPos>& blocks);

private:
    EDirection mDirection;
    std::vector<BlockPos> mBlocks;
};

#endif
