#ifndef GAME_WORLD_BLOCK_BLOCKPATTERNBUILDER_HPP
#define GAME_WORLD_BLOCK_BLOCKPATTERNBUILDER_HPP

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/Predicate.hpp"
#include "game/world/chunk/array3DWithLength.hpp"

class BlockInWorld;
class BlockPattern;
class Direction;

class BlockPatternBuilder
{
public:
    static BlockPatternBuilder* start();

    BlockPatternBuilder();

    template <unsigned int N>
    BlockPatternBuilder* aisle(std::wstring (&aisle)[N]);

    BlockPatternBuilder* where(wchar_t symbol, Predicate<BlockInWorld*>* predicate);

    BlockPattern* build(const Direction* forward, const Direction* up);

private:
    void ensureAllCharactersMatched();
    array3DWithLength<Predicate<BlockInWorld*>*> createPattern();

    std::vector<arrayWithLength<std::wstring> > mPattern;
    boost::unordered_map<wchar_t, Predicate<BlockInWorld*>*> mLookup;
    int mHeight;
    int mWidth;
};

template <unsigned int N>
BlockPatternBuilder* BlockPatternBuilder::aisle(std::wstring (&aisle)[N])
{
    if (mPattern.empty())
    {
        mHeight = N;
        mWidth = aisle[0].size();
    }

    for (unsigned int i = 0; i < N; i++)
    {
        for (std::wstring::iterator it = aisle[i].begin(); it != aisle[i].end(); ++it)
        {
            if (mLookup.find(*it) == mLookup.end())
            {
                mLookup[*it] = NULL;
            }
        }
    }

    arrayWithLength<std::wstring> row;
    row.data = aisle;
    row.length = N;

    mPattern.push_back(row);

    return this;
}

#endif
