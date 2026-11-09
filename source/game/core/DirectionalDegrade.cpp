#include "game/core/DirectionalDegrade.hpp"

#include <algorithm>

#include "game/core/CachedSortFunction.hpp"
#include "game/core/PerlinNoise.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Iterator.hpp"

static struct
{
    std::wstring name;
    DirectionalDegrade::EDirection direction;
} sDirectionNames[7] = {
    {L"North", DirectionalDegrade::eDirection_North},     {L"South", DirectionalDegrade::eDirection_South},
    {L"East", DirectionalDegrade::eDirection_East},       {L"West", DirectionalDegrade::eDirection_West},
    {L"ToEdges", DirectionalDegrade::eDirection_ToEdges}, {L"ToCentre", DirectionalDegrade::eDirection_ToCentre}};

DirectionalDegrade::DirectionalDegrade() : mDirection(eDirection_ToEdges)
{
}

void DirectionalDegrade::writeAttributes(GameRuleSaveInterface* saveInterface, unsigned int extraAttributeCount)
{
    (void)saveInterface;
    (void)extraAttributeCount;
}

void DirectionalDegrade::addAttribute(const std::wstring& name, const std::wstring& value)
{
    if (name == L"dir")
    {
        for (int i = 0; i < 7; ++i)
        {
            if (value.compare(sDirectionNames[i].name) == 0)
            {
                mDirection = sDirectionNames[i].direction;
            }
        }
        return;
    }

    DegradationRuleDefinition::addAttribute(name, value);
}

void DirectionalDegrade::ClearCaches()
{
    mBlocks.clear();
}

void DirectionalDegrade::NextBlocks(BlockDegradeRoutine::LayerCache* layerCache, arrayWithLength<BlockPos>& blocks)
{
    if (mBlocks.empty())
    {
        Iterator<BlockPos*>* iterator = layerCache->iterator();
        BlockPos* position = iterator->next();
        while (iterator->hasNext())
        {
            mBlocks.push_back(*position);
            iterator->next();
        }
        delete iterator;

        AABB* bounds = layerCache->getCurrentBounds();
        BlockPos min(bounds->mMinX, bounds->mMinY, bounds->mMinZ);
        BlockPos size(bounds->mMaxX - bounds->mMinX, bounds->mMaxY - bounds->mMinY, bounds->mMaxZ - bounds->mMinZ);
        BlockPos paddedSize = size.offset(BlockPos(1, 1, 1));

        Random random;
        PerlinNoise noise(&random, 3);
        arrayWithLength<double> values;
        arrayWithLength<double> noiseValues = noise.getRegion(values, min.x, min.y, min.z, paddedSize.x, paddedSize.y,
                                                              paddedSize.z, 5.0, 5.0, 5.0);

        CachedSortFunction sortFunction(noiseValues, bounds);
        std::sort(mBlocks.begin(), mBlocks.end(), sortFunction);
        delete[] noiseValues.data;
    }

    unsigned int blockCount = 0;
    std::vector<BlockPos>::iterator position = mBlocks.begin();
    while (blockCount < blocks.length && position != mBlocks.end())
    {
        if (layerCache->inLayer(*position))
        {
            blocks.data[blockCount] = *position;
            ++blockCount;
        }
        ++position;
    }
}

ConsoleGameRules::EGameRuleType DirectionalDegrade::getActionType()
{
    return ConsoleGameRules::eGameRuleType_DirectionalDegrade;
}
