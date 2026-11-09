#include "game/world/generation/FlatLayerInfo.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/core/L10N.hpp"
#include "game/core/StringConversion.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/DefaultedMappedRegistry.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/core/StringFormat.hpp"
FlatLayerInfo::FlatLayerInfo() : mVersion(3), mBlockState(NULL), mHeight(0), mStart(0)
{
}
FlatLayerInfo::FlatLayerInfo(int height, Block* block)
{
    _init(height, block);
}
FlatLayerInfo::FlatLayerInfo(int height, int blockId, int data)
{
    Block* block = Block::byId(blockId);
    _init(height, block);
    mBlockState = block->getBlockState(data);
}
FlatLayerInfo::FlatLayerInfo(int version, int height, Block* block)
{
    _init(height, block);
    mVersion = version;
}
FlatLayerInfo::FlatLayerInfo(int version, int height, Block* block, int data)
{
    _init(height, block);
    mVersion = version;
    mBlockState = block->getBlockState(data);
}
unsigned int FlatLayerInfo::getStart() const
{
    return (unsigned int)mStart;
}
const BlockState* FlatLayerInfo::getBlockState() const
{
    return mBlockState;
}
unsigned int FlatLayerInfo::getHeight() const
{
    return (unsigned int)mHeight;
}
int FlatLayerInfo::getEnd() const
{
    return getStart() + getHeight() - 1;
}
void FlatLayerInfo::_init(int height, Block* block)
{
    mHeight = height;
    mVersion = 3;
    mBlockState = block->defaultBlockState();
    mStart = 0;
}
void FlatLayerInfo::setHeight(int height)
{
    mHeight = height;
}
void FlatLayerInfo::setBlockState(const BlockState* state)
{
    mBlockState = state;
}
Block* FlatLayerInfo::getBlock() const
{
    return mBlockState->getBlock();
}
int FlatLayerInfo::getId() const
{
    return Block::getId(getBlock());
}
int FlatLayerInfo::getData() const
{
    return mBlockState->getBlock()->convertBlockStateToLegacyData(mBlockState);
}
void FlatLayerInfo::setStart(int start)
{
    mStart = start;
}
std::wstring FlatLayerInfo::toString()
{
    std::wstring result;
    if (mVersion >= 3)
    {
        ResourceLocation key = Block::blockRegistry->getKey(getBlock());
        result = key.toString();
        if (mHeight > 1)
            result = _toString<int>(mHeight) + L"*" + result;
    }
    else
    {
        result = _toString<int>(Block::getId(getBlock()));
        if (mHeight > 1)
            result = _toString<int>(mHeight) + L"x" + result;
    }
    int data = getData();
    if (data > 0)
        result += L":" + _toString<int>(data);
    return result;
}

std::wstring FlatLayerInfo::getHeightText() const
{
    return formatwstr(L"%ix", getHeight());
}

std::wstring FlatLayerInfo::getNameText() const
{
    if (getBlock() == Blocks::air)
        return std::wstring(L10N::GetString(IDS_SUPERFLAT_AIR_LAYER));
    Block* block = getBlock();
    if (block)
    {
        not_null_ptr<ItemInstance> item(new ItemInstance(block));
        item->setAuxValue(getData());
        return item->getHoverName();
    }
    return std::wstring(L"");
}

std::wstring FlatLayerInfo::getDescriptionText() const
{
    if (getBlock() == Blocks::air)
        return std::wstring(L10N::GetString(IDS_SUPERFLAT_DESC_AIR_LAYER));
    Block* block = getBlock();
    if (block != NULL)
        return std::wstring(L10N::GetString(block->getUseDescriptionId(-1)));
    return std::wstring(L"");
}
