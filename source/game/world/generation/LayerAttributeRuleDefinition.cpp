#include "game/world/generation/LayerAttributeRuleDefinition.hpp"

#include "game/core/LayerSize.hpp"
#include "game/world/generation/FilterRuleDefinition.hpp"
#include "game/world/generation/FunctionLayerFill.hpp"
#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"
#include "game/world/generation/LayerShape.hpp"
#include "game/world/generation/LayerTheme.hpp"

template <class T>
LayerAttributeRuleDefinition<T>::LayerAttributeRuleDefinition(ConsoleGameRules::EGameRuleType actionType,
                                                              CreateChildFunc createChild)
    : mActionType(actionType),
      mWholeLevel(false),
      mCreateChild(createChild)
{
}

template <class T>
ConsoleGameRules::EGameRuleType LayerAttributeRuleDefinition<T>::getActionType()
{
    return mActionType;
}

template <class T>
void LayerAttributeRuleDefinition<T>::writeAttributes(GameRuleSaveInterface* saveInterface,
                                                      unsigned int extraAttributeCount)
{
    GameRuleDefinition::writeAttributes(saveInterface, extraAttributeCount + 1);

    saveInterface->saveAttribute(ConsoleGameRules::eGameRuleAttr_name, mWholeLevel ? L"WholeLevel" : L"PerLayer");
}

template <class T>
void LayerAttributeRuleDefinition<T>::getChildren(std::vector<GameRuleDefinition*>* children)
{
    children->insert(children->begin(), mChildren.begin(), mChildren.end());
}

template <class T>
GameRuleDefinition* LayerAttributeRuleDefinition<T>::addChild(ConsoleGameRules::EGameRuleType type)
{
    if (type == ConsoleGameRules::eGameRuleType_FilterTheme)
    {
        FilterRuleDefinition<T>* filter = new FilterRuleDefinition<T>(mCreateChild);
        mChildren.push_back(filter);
        return filter;
    }

    LayerAttributeChildRuleDefinition<T>* child = mCreateChild(type);
    if (child != NULL)
    {
        mChildren.push_back(child);
        return child;
    }

    return GameRuleDefinition::addChild(type);
}

template <class T>
void LayerAttributeRuleDefinition<T>::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name.compare(L"for") == 0)
    {
        if (value.compare(L"WholeLevel") == 0)
        {
            mWholeLevel = true;
        }
        else if (value.compare(L"PerLayer") == 0)
        {
            mWholeLevel = false;
        }
    }
    else
    {
        GameRuleDefinition::addAttribute(name, value);
    }
}

template class LayerAttributeRuleDefinition<LayerFill*>;
template class LayerAttributeRuleDefinition<LayerShape*>;
template class LayerAttributeRuleDefinition<LayerSize*>;
template class LayerAttributeRuleDefinition<LayerTheme*>;
