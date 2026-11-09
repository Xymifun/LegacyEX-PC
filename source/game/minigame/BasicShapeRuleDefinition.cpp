#include "game/minigame/BasicShapeRuleDefinition.hpp"

#include "game/core/CircleShape.hpp"
#include "game/core/SquareShape.hpp"
#include "game/core/TemplateShape.hpp"
#include "game/core/IGameRuleAttribute.hpp"

namespace
{
const ConsoleGameRules::EGameRuleType BASIC_SHAPE_ACTION_TYPE = ConsoleGameRules::eGameRuleType_BasicShape;
}

BasicShapeRuleDefinition::BasicShapeRuleDefinition() : mShape(L"")
{
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_type, &mShape);
}

ConsoleGameRules::EGameRuleType BasicShapeRuleDefinition::getActionType()
{
    return BASIC_SHAPE_ACTION_TYPE;
}

LayerShape* BasicShapeRuleDefinition::Generate() const
{
    if (mShape.compare(L"circle") == 0)
    {
        return new CircleShape();
    }

    if (mShape.compare(L"square") == 0)
    {
        return new SquareShape();
    }

    if (mShape.compare(L"template") == 0)
    {
        return new TemplateShape();
    }

    return NULL;
}
