#ifndef GAME_CLIENT_RENDER_BLOCKRENDERDISPATCHER_HPP
#define GAME_CLIENT_RENDER_BLOCKRENDERDISPATCHER_HPP

class Block;
class BlockColors;
class BlockRenderer;
class BlockState;
class Level;

class BlockRenderDispatcher
{
public:
    BlockRenderDispatcher(BlockColors* blockColors);

    void renderSingleBlock(const BlockState* state, float x, float y, bool color);
    bool canRenderGeneric(Block* block);
    void setColor(bool color);
    BlockRenderer* getBlockRenderer();
    void prepare(Level* level);

private:
    BlockRenderer* mBlockRenderer;
};

#endif
