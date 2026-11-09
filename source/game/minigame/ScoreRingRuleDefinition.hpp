#ifndef GAME_MINIGAME_SCORERINGRULEDEFINITION_HPP
#define GAME_MINIGAME_SCORERINGRULEDEFINITION_HPP

#include <string>

#include "game/minigame/TargetAreaRuleDefinition.hpp"
#include "game/world/generation/GlideRingGenerator.hpp"

class ScoreRingRuleDefinition : public TargetAreaRuleDefinition
{
public:
    ScoreRingRuleDefinition();
    ScoreRingRuleDefinition(int x, int y, int z, GlideRingGenerator::eGlideRingOrientation orientation,
                            GlideRingGenerator::eGlideRingSize size);

    virtual ~ScoreRingRuleDefinition();

    virtual ConsoleGameRules::EGameRuleType getActionType();
    virtual void addAttribute(const std::wstring& name, const std::wstring& value);
    virtual void onAttributesAdded();
    virtual int getPointValue();
    virtual int getSize();

protected:
    int mX;
    int mY;
    int mZ;
    GlideRingGenerator::eGlideRingOrientation mOrientation;
    GlideRingGenerator::eGlideRingSize mSize;
};

#endif
