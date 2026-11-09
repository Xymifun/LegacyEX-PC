#ifndef GAME_MINIGAME_DEGRADATIONRULEDEFINITION_HPP
#define GAME_MINIGAME_DEGRADATIONRULEDEFINITION_HPP

#include <string>

#include "game/minigame/GameRuleDefinition.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockDegradeRoutine.hpp"

class BlockPos;

class DegradationRuleDefinition : public GameRuleDefinition
{
public:
    DegradationRuleDefinition();

    virtual ~DegradationRuleDefinition();

    virtual void writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount);
    virtual void addAttribute(const std::wstring& name, const std::wstring& value);
    virtual void ClearCaches();
    virtual void NextBlocks(BlockDegradeRoutine::LayerCache* layerCache, arrayWithLength<BlockPos>& blocks) = 0;

    const std::wstring& getTargetLayer() const;

protected:
    std::wstring mTargetLayer;
    int mSpeed;
};

#endif
