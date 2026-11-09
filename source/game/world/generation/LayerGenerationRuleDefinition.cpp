#include "game/world/generation/LayerGenerationRuleDefinition.hpp"

#include <vector>

#include "game/minigame/AnyCombinationOfRuleDefinition.hpp"
#include "game/minigame/CombinationDefinitionRuleDefinition.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"
#include "game/core/LayerSize.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/LayerAttributeChildRuleDefinition.hpp"
#include "game/world/generation/LayerGenerator.hpp"
#include "game/world/generation/LayerMetadata.hpp"
#include "game/world/generation/LayerShape.hpp"
#include "game/world/generation/LayerTheme.hpp"
#include "game/world/generation/VoidLayerTheme.hpp"
#include "game/util/AABB.hpp"
#include "game/world/LayerGenerationRuleDefinition.hpp"

LayerGenerationRuleDefinition::LayerGenerationRuleDefinition() : mDefinition(NULL), mLayerHeight(10), mLayerDirection(1)
{
}

void LayerGenerationRuleDefinition::getChildren(std::vector<GameRuleDefinition*>* children)
{
    if (mDefinition != NULL)
    {
        children->push_back(mDefinition);
    }

    children->insert(children->end(), mCombinationDefinitions.begin(), mCombinationDefinitions.end());
}

GameRuleDefinition* LayerGenerationRuleDefinition::addChild(ConsoleGameRules::EGameRuleType type)
{
    mDefinition = new AnyCombinationOfRuleDefinition(this);
    return mDefinition;
}

std::vector<LayerGenerator*>* LayerGenerationRuleDefinition::CreateLayers(Random& random, const AABB* area,
                                                                          unsigned int layerCount, float scale) const
{
    std::vector<LayerGenerator*>* generators = new std::vector<LayerGenerator*>();
    int y;

    switch (mLayerDirection)
    {
    default:
        y = static_cast<int>(area->mMinY + area->mMaxY) / 2 + static_cast<int>(layerCount * mLayerHeight) / 2;
        break;
    case 0:
        y = static_cast<int>(area->mMinY) + mLayerHeight * static_cast<int>(layerCount + 1);
        break;
    case 2:
        y = static_cast<int>(area->mMaxY);
        break;
    }

    Vec3i layerStep(0, -mLayerHeight, 0);
    BlockPos origin(static_cast<int>((area->mMinX + area->mMaxX) * 0.5), y,
                    static_cast<int>((area->mMinZ + area->mMaxZ) * 0.5));
    Random layerRandom(random.nextLong());

    for (unsigned int index = 0; index < layerCount; ++index)
    {
        {
            LayerMetadata metadata;
            metadata.m_uGameVariant = Minecraft::GetInstance()->GetMiniGame()->GetGameVariant();
            metadata.m_iLayerIndex = static_cast<int>(index);
            metadata.m_iBottomLayerIndex = static_cast<int>(layerCount - 1);

            Random wholeLevelRandom(layerRandom);
            AnyCombinationOfRuleDefinition* definition = mDefinition->GetRandomLeafChild(metadata, wholeLevelRandom,
                                                                                         random);
            double blend = 0.0;
            if (layerCount > 1)
            {
                blend = 1.0 - static_cast<double>(index) / static_cast<double>(layerCount - 1);
            }

            std::vector<LayerAttributeChildRuleDefinition<LayerSize*>*> sizes;
            definition->GetPossibleLayerSizes(sizes);
            LayerAttributeChildRuleDefinition<LayerSize*>* sizeDefinition = randomUsableElement(random, sizes,
                                                                                                metadata);
            LayerSize* size = sizeDefinition->GetValue();

            std::vector<LayerAttributeChildRuleDefinition<LayerShape*>*> shapes;
            definition->GetPossibleLayerShapes(shapes);
            LayerAttributeChildRuleDefinition<LayerShape*>* shapeDefinition = randomUsableElement(random, shapes,
                                                                                                  metadata);
            LayerShape* shape = shapeDefinition->GetValue();

            std::vector<LayerAttributeChildRuleDefinition<LayerFill*>*> fills;
            definition->GetPossibleLayerFills(fills);
            LayerAttributeChildRuleDefinition<LayerFill*>* fillDefinition = randomUsableElement(random, fills,
                                                                                                metadata);
            LayerFill* fill = fillDefinition->GetValue();

            std::vector<const LayerTheme*> themes;
            static VoidLayerTheme defaultTheme;
            {
                std::vector<LayerAttributeChildRuleDefinition<LayerTheme*>*> possibleThemes;
                for (int themeIndex = 0; themeIndex < 4; ++themeIndex)
                {
                    definition->GetPossibleLayerThemes(themeIndex, possibleThemes);
                    if (possibleThemes.empty())
                    {
                        themes.push_back(&defaultTheme);
                    }
                    else
                    {
                        LayerAttributeChildRuleDefinition<LayerTheme*>* themeDefinition = randomUsableElement(
                            random, possibleThemes, metadata);
                        themes.push_back(themeDefinition->GetValue());
                        possibleThemes.clear();
                    }
                }
            }

            int halfSize = static_cast<int>(static_cast<float>(size->GetLayerSize(random, blend)) * scale * 0.5f);
            AABB* layerArea = AABB::newPermanent(origin.offset(Vec3i(-halfSize, -mLayerHeight, -halfSize)),
                                                 origin.offset(Vec3i(halfSize, 0, halfSize)));
            generators->push_back(new LayerGenerator(layerArea, shape, fill, themes));
        }
        origin = origin.offset(layerStep);
    }

    return generators;
}

ConsoleGameRules::EGameRuleType LayerGenerationRuleDefinition::getActionType()
{
    return ConsoleGameRules::eGameRuleType_LayerGeneration;
}
