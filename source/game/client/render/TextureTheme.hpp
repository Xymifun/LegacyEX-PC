#ifndef GAME_CLIENT_RENDER_TEXTURETHEME_HPP
#define GAME_CLIENT_RENDER_TEXTURETHEME_HPP

#include <string>
#include <vector>
#include <utility>
#include <boost/unordered_map.hpp>

#include "game/core/AssetTheme.hpp"
#include "game/world/generation/LayerTheme.hpp"

class BlockState;
class BlockTransaction;
class BufferedImage;
class Random;

class TextureTheme : public LayerTheme, public AssetTheme<BufferedImage*>
{
public:
    class BlockStack
    {
    public:
        BlockStack();
        BlockStack(const BlockState* state, unsigned int count, int offset);
        BlockStack(std::vector<const BlockState*>& states, int offset);
        const BlockState* at(int index) const;

    private:
        int mOffset;
        std::vector<const BlockState*> mStates;
    };

    TextureTheme();
    ~TextureTheme();
    const BlockStack& getStack(BufferedImage* image, int x, int y) const;
    void addStack(const std::wstring& key, const BlockStack& stack);
    virtual void Apply(Random& random, BlockTransaction* transaction, int& width, int& depth) const;

private:
    boost::unordered_map<int, BlockStack> mStacks;
};

#endif
