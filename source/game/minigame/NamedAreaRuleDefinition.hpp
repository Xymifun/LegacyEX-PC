#ifndef GAME_MINIGAME_NAMEDAREARULEDEFINITION_HPP
#define GAME_MINIGAME_NAMEDAREARULEDEFINITION_HPP

#include <string>

#include "game/minigame/GameRuleDefinition.hpp"

class AABB;
class BlockPos;

class NamedAreaRuleDefinition : public GameRuleDefinition
{
public:
    NamedAreaRuleDefinition();

    virtual ~NamedAreaRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual AABB* getBoundingVolume();
    virtual void onAttributesAdded();
    virtual void WriteAttributesAsXML(std::string& xml);
    virtual bool containsBlock(int blockId, const BlockPos& pos);

    AABB* getArea();

    std::wstring getName();
    void setName(const std::wstring& name);

    void updateWithPoints(int x0, int y0, int z0, int x1, int y1, int z1);

protected:
    std::wstring mName;
    AABB* mArea;
};

#endif
