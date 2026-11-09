#include "game/client/render/TextureTheme.hpp"

#include "game/core/BufferedImage.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockTransaction.hpp"
#include "game/world/generation/RandomPatchesLayerTheme.hpp"

template <class T>
T _fromHEXString(const std::wstring& text);

TextureTheme::BlockStack::BlockStack() : mOffset(0)
{
}

TextureTheme::BlockStack::BlockStack(const BlockState* state, unsigned int count, int offset) : mOffset(offset)
{
    for (unsigned int i = 0; i < count; ++i)
        mStates.push_back(state);
    static const BlockState* air = Blocks::air->defaultBlockState();
    mStates.push_back(air);
}

TextureTheme::BlockStack::BlockStack(std::vector<const BlockState*>& states, int offset)
    : mOffset(offset),
      mStates(states)
{
}

const BlockState* TextureTheme::BlockStack::at(int index) const
{
    index += mOffset;
    if (mStates.empty())
        return NULL;
    if (index < 0)
        return Blocks::air->defaultBlockState();
    int last = (int)mStates.size() - 1;
    if (index < last)
        last = index;
    return mStates.at(last);
}

TextureTheme::TextureTheme()
{
}

const TextureTheme::BlockStack& TextureTheme::getStack(BufferedImage* image, int x, int y) const
{
    if (x > 0 && y > 0 && x < (int)image->getWidth() && y < (int)image->getHeight())
    {
        int key = image->getData()[(x * image->getWidth() + y)] & 0xFFFFFF;
        boost::unordered_map<int, BlockStack>::const_iterator it = mStacks.find(key);
        if (it != mStacks.end())
            return mStacks.at(key);
        static BlockStack defaultStack;
        return defaultStack;
    }
    static BlockStack invalidStack(Blocks::air->defaultBlockState(), 1, 0);
    return invalidStack;
}

void TextureTheme::addStack(const std::wstring& key, const BlockStack& stack)
{
    int value = _fromHEXString<int>(key);
    mStacks.insert(std::make_pair(value, stack));
}

void TextureTheme::Apply(Random&, BlockTransaction* transaction, int&, int&) const
{
    const int width = transaction->mSize.x;
    const int depth = transaction->mSize.z;
    BufferedImage* image = getAssetForSize(width < depth ? depth : width);
    int xOffset = ((int)image->getWidth() - width) / 2;
    int zOffset = ((int)image->getHeight() - depth) / 2;
    for (int x = 0; x < width; ++x)
    {
        for (int z = 0; z < depth; ++z)
        {
            const BlockStack& stack = getStack(image, x + xOffset, z + zOffset);
            int top = RandomPatchesLayerTheme::HighestSolidBlock(transaction, x, z);
            if (top == -1)
                continue;
            for (int y = transaction->mSize.y - 1; y > 0; --y)
            {
                const BlockState* state = stack.at(top - y);
                if (state != NULL)
                    transaction->setBlock(BlockPos(x, y, z), state);
            }
        }
    }
}
