#include "game/world/block/BlockPatternBuilder.hpp"

#include "game/world/block/BlockPattern.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/chunk/array2DWithLength.hpp"

BlockPatternBuilder::BlockPatternBuilder() : mPattern(), mLookup(11)
{
    mLookup[L' '] = Predicates<BlockInWorld*>::alwaysTrue->copy();
}

BlockPatternBuilder* BlockPatternBuilder::start()
{
    return new BlockPatternBuilder();
}

BlockPatternBuilder* BlockPatternBuilder::where(wchar_t symbol, Predicate<BlockInWorld*>* predicate)
{
    mLookup[symbol] = predicate;

    return this;
}

void BlockPatternBuilder::ensureAllCharactersMatched()
{
    std::vector<wchar_t> unmatchedCharacters;

    boost::unordered_map<wchar_t, Predicate<BlockInWorld*>*>::iterator it;
    for (it = mLookup.begin(); it != mLookup.end(); ++it)
    {
        if (it->second == NULL)
        {
            unmatchedCharacters.push_back(it->first);
        }
    }
}

array3DWithLength<Predicate<BlockInWorld*>*> BlockPatternBuilder::createPattern()
{
    ensureAllCharactersMatched();

    array3DWithLength<Predicate<BlockInWorld*>*> pattern(mPattern.size(), mHeight, mWidth);

    for (unsigned int i = 0; i < mPattern.size(); i++)
    {
        for (int j = 0; j < mHeight; j++)
        {
            for (int k = 0; k < mWidth; k++)
            {
                pattern.data[i]->data[j]->data[k] = mLookup[mPattern.at(i).data[j].at(k)];
            }
        }
    }

    return pattern;
}

BlockPattern* BlockPatternBuilder::build(const Direction* forward, const Direction* up)
{
    return new BlockPattern(createPattern(), forward, up);
}
