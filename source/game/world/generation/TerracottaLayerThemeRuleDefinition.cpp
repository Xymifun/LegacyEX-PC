#include "game/world/generation/TerracottaLayerThemeRuleDefinition.hpp"
#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"

TerracottaLayerThemeFunction getColourFunc(const std::wstring& function);

TerracottaLayerTheme::Pattern getPattern(const std::wstring& pattern)
{
    Random random;

    if (pattern == L"RANDOM:1")
    {
        TerracottaLayerTheme::Pattern result = {1};
        const Direction* directions[4] = {Direction::NORTH, Direction::SOUTH, Direction::EAST, Direction::WEST};
        const Direction* selected = directions[random.nextInt(4)];
        result.values[0] = selected;
        return result;
    }
    else if (pattern == L"RANDOM:2")
    {
        const Direction* directions[4] = {Direction::NORTH, Direction::SOUTH, Direction::EAST, Direction::WEST};
        const Direction* selected = directions[random.nextInt(4)];
        TerracottaLayerTheme::Pattern result = {2};
        result.values[0] = selected->getClockWise();
        result.values[1] = selected->getOpposite();
        result.values[4] = selected;
        result.values[5] = selected->getCounterClockWise();
        return result;
    }
    else if (pattern == L"RANDOM:3")
    {
        const Direction* directions[4] = {Direction::NORTH, Direction::SOUTH, Direction::EAST, Direction::WEST};
        const Direction* selected = directions[random.nextInt(4)];
        const Direction* opposite = selected->getOpposite();
        TerracottaLayerTheme::Pattern result = {3};
        result.values[0] = selected;
        result.values[1] = opposite;
        result.values[2] = selected;
        result.values[4] = opposite;
        result.values[5] = selected;
        result.values[6] = opposite;
        result.values[8] = selected;
        result.values[9] = opposite;
        result.values[10] = selected;
        return result;
    }
    else
    {
        const Direction* directions[4] = {Direction::NORTH, Direction::SOUTH, Direction::EAST, Direction::WEST};
        const Direction* selected = directions[random.nextInt(4)];
        const Direction* clockwise = selected->getClockWise();
        const Direction* counterClockwise = selected->getCounterClockWise();
        const Direction* opposite = selected->getOpposite();
        TerracottaLayerTheme::Pattern result = {4};
        result.values[0] = clockwise;
        result.values[1] = selected;
        result.values[2] = clockwise;
        result.values[3] = opposite;
        result.values[4] = opposite;
        result.values[5] = counterClockwise;
        result.values[6] = selected;
        result.values[7] = counterClockwise;
        result.values[8] = clockwise;
        result.values[9] = opposite;
        result.values[10] = clockwise;
        result.values[11] = selected;
        result.values[12] = selected;
        result.values[13] = counterClockwise;
        result.values[14] = opposite;
        result.values[15] = counterClockwise;
        return result;
    }
}

TerracottaLayerThemeRuleDefinition::TerracottaLayerThemeRuleDefinition()
    : mPattern(L"RANDOM:2"),
      mColourFunction(L"UNIFORM")
{
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_colour, &mColourFunction);
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_pattern, &mPattern);
}

ConsoleGameRules::EGameRuleType TerracottaLayerThemeRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_TerracottaTheme;
}

LayerTheme* TerracottaLayerThemeRuleDefinition::Generate() const
{
    TerracottaLayerTheme::Pattern pattern = getPattern(mPattern);
    TerracottaLayerThemeFunction function = getColourFunc(mColourFunction);
    return new TerracottaLayerTheme(pattern, function);
}
