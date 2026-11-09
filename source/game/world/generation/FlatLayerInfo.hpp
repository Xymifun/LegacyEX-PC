#ifndef GAME_WORLD_GENERATION_FLATLAYERINFO_HPP
#define GAME_WORLD_GENERATION_FLATLAYERINFO_HPP
#include <string>
class Block;
class BlockState;
class FlatLayerInfo
{
public:
    FlatLayerInfo();
    FlatLayerInfo(int height, Block* block);
    FlatLayerInfo(int height, int blockId, int data);
    FlatLayerInfo(int version, int height, Block* block);
    FlatLayerInfo(int version, int height, Block* block, int data);
    unsigned int getStart() const;
    const BlockState* getBlockState() const;
    unsigned int getHeight() const;
    int getEnd() const;
    void _init(int height, Block* block);
    void setHeight(int height);
    void setBlockState(const BlockState* state);
    Block* getBlock() const;
    int getId() const;
    int getData() const;
    void setStart(int start);
    std::wstring toString();
    std::wstring getHeightText() const;
    std::wstring getNameText() const;
    std::wstring getDescriptionText() const;

private:
    int mVersion;
    const BlockState* mBlockState;
    int mHeight;
    int mStart;
};
#endif
