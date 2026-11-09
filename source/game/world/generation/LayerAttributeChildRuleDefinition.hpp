#ifndef GAME_WORLD_GENERATION_LAYERATTRIBUTECHILDRULEDEFINITION_HPP
#define GAME_WORLD_GENERATION_LAYERATTRIBUTECHILDRULEDEFINITION_HPP

#include "game/minigame/GameRuleDefinition.hpp"

class LayerMetadata;

template <class T>
class LayerAttributeChildRuleDefinition : public GameRuleDefinition
{
public:
    LayerAttributeChildRuleDefinition() : mGeneratedValue(NULL), mWholeLevel(false) {}

    virtual ~LayerAttributeChildRuleDefinition() {}

    virtual T Generate() const = 0;

    virtual bool CanUse(const LayerMetadata& metadata) const { return true; }

    T GetValue()
    {
        if (!mWholeLevel)
        {
            mGeneratedValue = Generate();
        }

        return mGeneratedValue;
    }

private:
    T mGeneratedValue;
    bool mWholeLevel;
};

#endif
