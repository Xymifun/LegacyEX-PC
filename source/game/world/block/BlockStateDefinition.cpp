#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Property.hpp"
#include "game/core/Boxed.hpp"
#include "game/core/Cartesian__Product.hpp"
#include "game/core/Mth.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/world/LevelSource.hpp"
#include <algorithm>
#include <cstring>

BlockStateDefinition::BlockStateDefinition(Block* block)
{
    mBlock = block;

    mBitOffsets = new unsigned char[1];
    mBitMasks = new unsigned short[1];

    std::memset(mBitOffsets, 0, 1);
    std::memset(mBitMasks, 0, 2);

    BlockStateImpl* state = new BlockStateImpl(this, 0);

    mStates.data = new BlockStateImpl*[1];
    mStates.data[0] = state;
    mStates.length = 1;
}

void BlockStateDefinition::_init(Block* block, const Property** properties, unsigned int count)
{
    mBlock = block;

    mBitOffsets = new unsigned char[count];
    mBitMasks = new unsigned short[count];

    std::memset(mBitOffsets, 0, count);
    std::memset(mBitMasks, 0, count * 2);

    unsigned int stateCount = 1;
    unsigned char bitOffset = 0;

    for (unsigned int i = 0; i < count; i++)
    {
        mProperties.push_back(properties[i]);

        int valueCount = Mth::smallestEncompassingPowerOfTwo(properties[i]->getValueCount());

        mBitOffsets[i] = bitOffset;
        bitOffset = (unsigned char)(bitOffset + Mth::countTrailingZeros(valueCount));
        mBitMasks[i] = (unsigned short)(valueCount - 1);

        stateCount *= valueCount;
    }

    BlockStateImpl** states = new BlockStateImpl*[stateCount];

    for (unsigned int i = 0; i < stateCount; i++)
    {
        states[i] = NULL;
    }

    mStates.data = states;
    mStates.length = stateCount;

    boost::unordered_map<std::map<const Property*, Boxed*>, BlockStateImpl*> statesByValues(11);

    std::vector<const std::vector<Boxed*>*>* propertyValues = getPropertyValues();
    Cartesian::Product* product = new Cartesian::Product(propertyValues);

    std::map<const Property*, Boxed*> values;

    Cartesian::Product::ProductIterator* iterator = product->iterator();

    while (iterator->hasNext())
    {
        arrayWithLength<Boxed*> combination = iterator->next();

        values.clear();

        unsigned short data = 0;

        for (unsigned int i = 0; i < count; i++)
        {
            values[properties[i]] = combination.data[i];

            unsigned int index = properties[i]->getIndexForValue(combination.data[i]);

            data |= index << mBitOffsets[i];
        }

        BlockStateImpl* state = new BlockStateImpl(this, data);

        statesByValues[values] = state;
        mStates.data[data] = state;
    }

    delete iterator;
    delete product;
    delete propertyValues;
}

BlockStateDefinition::BlockStateImpl* BlockStateDefinition::any()
{
    return mStates.data[0];
}

const arrayWithLength<BlockStateDefinition::BlockStateImpl*>* BlockStateDefinition::getPossibleBlockStates() const
{
    return &mStates;
}

Block* BlockStateDefinition::getBlock() const
{
    return mBlock;
}

const std::vector<const Property*>& BlockStateDefinition::getProperties() const
{
    return mProperties;
}

std::vector<const std::vector<Boxed*>*>* BlockStateDefinition::getPropertyValues()
{
    std::vector<const std::vector<Boxed*>*>* values = new std::vector<const std::vector<Boxed*>*>();

    for (unsigned int i = 0; i < mProperties.size(); i++)
    {
        values->push_back(&mProperties.at(i)->getPossibleValues());
    }

    return values;
}

std::wstring BlockStateDefinition::PROPERTY_TO_STRING_FUNCTION(const Property* property)
{
    if (property == NULL)
    {
        return std::wstring(L"<NULL>");
    }

    return property->getName();
}

bool BlockStateDefinition::hasProperty(const Property* property) const
{
    return std::find(mProperties.begin(), mProperties.end(), property) != mProperties.end();
}

Boxed* BlockStateDefinition::getPropertyValue(const Property* property, unsigned short data) const
{
    unsigned int i = 0;

    for (std::vector<const Property*>::const_iterator it = mProperties.begin(); it != mProperties.end(); ++it, ++i)
    {
        if (*it == property)
        {
            return property->getValueAtIndex((data >> mBitOffsets[i]) & mBitMasks[i]);
        }
    }

    return NULL;
}

BlockStateDefinition::BlockStateImpl* BlockStateDefinition::getNeighbour(const Property* property, Boxed* value,
                                                                         unsigned short data) const
{
    unsigned int i = 0;
    unsigned short index = 0;

    for (std::vector<const Property*>::const_iterator it = mProperties.begin(); it != mProperties.end(); ++it, ++i)
    {
        if (*it == property)
        {
            index = property->getIndexForValue(value);
            break;
        }
    }

    return mStates.data[(data & ~(mBitMasks[i] << mBitOffsets[i])) | ((index & mBitMasks[i]) << mBitOffsets[i])];
}

boost::unordered_map<const Property*, Boxed*>* BlockStateDefinition::getValues(unsigned short data) const
{
    boost::unordered_map<const Property*, Boxed*>* values = new boost::unordered_map<const Property*, Boxed*>();

    unsigned int i = 0;

    for (std::vector<const Property*>::const_iterator it = mProperties.begin(); it != mProperties.end(); ++it, ++i)
    {
        const Property* property = *it;

        values->insert(std::make_pair(property, property->getValueAtIndex((data >> mBitOffsets[i]) & mBitMasks[i])));
    }

    return values;
}

std::wstring BlockStateDefinition::toString()
{
    return mBlock->toString();
}

BlockStateDefinition::BlockStateImpl::BlockStateImpl(BlockStateDefinition* definition, unsigned short data)
{
    mBlock = definition->getBlock();
    mCachedMapColor = NULL;
    mDefinition = definition;
    mData = data;
}

bool BlockStateDefinition::BlockStateImpl::valuesContainsKey(const Property* property) const
{
    return mDefinition->hasProperty(property);
}

const std::vector<const Property*>& BlockStateDefinition::BlockStateImpl::getProperties() const
{
    return mDefinition->getProperties();
}

bool BlockStateDefinition::BlockStateImpl::hasProperty(const Property* property) const
{
    return mDefinition->hasProperty(property);
}

Boxed* BlockStateDefinition::BlockStateImpl::getBoxedValue(const Property* property) const
{
    valuesContainsKey(property);

    return mDefinition->getPropertyValue(property, mData);
}

const BlockState* BlockStateDefinition::BlockStateImpl::setBoxedValue(const Property* property, Boxed* value) const
{
    Boxed* current = mDefinition->getPropertyValue(property, mData);

    if (*current == value)
    {
        return this;
    }

    return mDefinition->getNeighbour(property, value, mData);
}

boost::unordered_map<const Property*, Boxed*>* BlockStateDefinition::BlockStateImpl::getValues() const
{
    return mDefinition->getValues(mData);
}

Block* BlockStateDefinition::BlockStateImpl::getBlock() const
{
    return mBlock;
}

bool BlockStateDefinition::BlockStateImpl::equals(const BlockState* other)
{
    return this == other;
}

int BlockStateDefinition::BlockStateImpl::hashCode() const
{
    return 0;
}

Material* BlockStateDefinition::BlockStateImpl::getMaterial() const
{
    return mBlock->getCraftingMaterial();
}

bool BlockStateDefinition::BlockStateImpl::isSolid() const
{
    return mBlock->mFullBlock;
}

bool BlockStateDefinition::BlockStateImpl::isValidSpawn(boost::shared_ptr<Entity> entity) const
{
    return mBlock->isValidSpawn(this, entity);
}

int BlockStateDefinition::BlockStateImpl::getLightBlock() const
{
    return mBlock->mLightBlock;
}

int BlockStateDefinition::BlockStateImpl::getLightEmission() const
{
    return mBlock->getLightEmission(this);
}

bool BlockStateDefinition::BlockStateImpl::isTranslucent() const
{
    return mBlock->mTranslucent;
}

bool BlockStateDefinition::BlockStateImpl::isTransparent() const
{
    return mBlock->isTransparent(this);
}

bool BlockStateDefinition::BlockStateImpl::doesPropagate() const
{
    return mBlock->mUseNeighbourBrightness;
}

const MaterialColor* BlockStateDefinition::BlockStateImpl::getMapColor(LevelSource* levelSource, const BlockPos& pos)
{
    if (mCachedMapColor == NULL)
    {
        mCachedMapColor = mDefinition->mBlock->getMapColor(this, levelSource, pos);
    }

    return mCachedMapColor;
}

const BlockState* BlockStateDefinition::BlockStateImpl::rotate(Rotation* rotation) const
{
    return mBlock->rotate(this, rotation);
}

const BlockState* BlockStateDefinition::BlockStateImpl::mirror(Mirror* mirror) const
{
    return mBlock->mirror(this, mirror);
}

bool BlockStateDefinition::BlockStateImpl::isCubeShaped() const
{
    return mBlock->isCubeShaped(this);
}

bool BlockStateDefinition::BlockStateImpl::hasCustomBreakingProgress() const
{
    return mBlock->hasCustomBreakingProgress(this);
}

int BlockStateDefinition::BlockStateImpl::getRenderShape() const
{
    return mBlock->getRenderShape(this);
}

int BlockStateDefinition::BlockStateImpl::getLightColor(LevelSource* levelSource, const BlockPos& pos) const
{
    return mBlock->getLightColor(this, levelSource, pos, -1);
}

float BlockStateDefinition::BlockStateImpl::getShadeBrightness() const
{
    return mBlock->getShadeBrightness(this);
}

bool BlockStateDefinition::BlockStateImpl::isSolidBlockingCube() const
{
    return mBlock->isSolidBlockingCube(this);
}

bool BlockStateDefinition::BlockStateImpl::isSolidBlockingCubeAndNotSignalSource() const
{
    return mBlock->isSolidBlockingCubeAndNotSignalSource(this);
}

bool BlockStateDefinition::BlockStateImpl::isSolidBlockingBlock() const
{
    return getMaterial()->isSolidBlocking() && (isCubeShaped() || isSolidBlockingCube());
}

bool BlockStateDefinition::BlockStateImpl::canProvideSupport(const Direction* direction,
                                                             EBlockSupportType supportType) const
{
    return mBlock->canProvideSupport(this, direction, supportType);
}

bool BlockStateDefinition::BlockStateImpl::isSignalSource() const
{
    return mBlock->isSignalSource(this);
}

int BlockStateDefinition::BlockStateImpl::getSignal(LevelSource* levelSource, const BlockPos& pos,
                                                    const Direction* direction) const
{
    return mBlock->getSignal(this, levelSource, pos, direction);
}

bool BlockStateDefinition::BlockStateImpl::hasAnalogOutputSignal() const
{
    return mBlock->hasAnalogOutputSignal(this);
}

int BlockStateDefinition::BlockStateImpl::getAnalogOutputSignal(Level* level, const BlockPos& pos) const
{
    return mBlock->getAnalogOutputSignal(this, level, pos);
}

float BlockStateDefinition::BlockStateImpl::getDestroySpeed(Level* level, const BlockPos& pos) const
{
    return mBlock->getDestroySpeed(this, level, pos);
}

float BlockStateDefinition::BlockStateImpl::getDestroyProgress(boost::shared_ptr<Player> player, Level* level,
                                                               const BlockPos& pos) const
{
    return mBlock->getDestroyProgress(this, player, level, pos);
}

int BlockStateDefinition::BlockStateImpl::getDirectSignal(LevelSource* levelSource, const BlockPos& pos,
                                                          const Direction* direction) const
{
    return mBlock->getDirectSignal(this, levelSource, pos, direction);
}

EPistonPushReaction BlockStateDefinition::BlockStateImpl::getPistonPushReaction() const
{
    return mBlock->getPistonPushReaction(this);
}

const BlockState* BlockStateDefinition::BlockStateImpl::fillVirtualBlockStateProperties(LevelSource* levelSource,
                                                                                        const BlockPos& pos) const
{
    return mBlock->fillVirtualBlockStateProperties(this, levelSource, pos);
}

const AABB* BlockStateDefinition::BlockStateImpl::getOutlineAABB(Level* level, const BlockPos& pos) const
{
    return mBlock->getOutlineAABB(this, level, pos);
}

bool BlockStateDefinition::BlockStateImpl::shouldRenderFace(LevelSource* levelSource, const BlockPos& pos,
                                                            const Direction* direction) const
{
    return mBlock->shouldRenderFace(this, levelSource, pos, direction);
}

bool BlockStateDefinition::BlockStateImpl::isSolidRender() const
{
    return mBlock->isSolidRender(this);
}

const AABB* BlockStateDefinition::BlockStateImpl::getClipAABB(LevelSource* levelSource, const BlockPos& pos) const
{
    return mBlock->getClipAABB(this, levelSource, pos);
}

void BlockStateDefinition::BlockStateImpl::addCollissionAABBs(Level* level, const BlockPos& pos, const AABB* bounds,
                                                              std::vector<AABB*>* collisions,
                                                              boost::shared_ptr<Entity> entity, bool flag) const
{
    mBlock->addCollisionAABBs(this, level, pos, bounds, collisions, entity, flag);
}

const AABB* BlockStateDefinition::BlockStateImpl::getShape(LevelSource* levelSource, const BlockPos& pos) const
{
    return mBlock->getShape(this, levelSource, pos);
}

HitResult* BlockStateDefinition::BlockStateImpl::clip(Level* level, const BlockPos& pos, Vec3* from, Vec3* to) const
{
    return mBlock->clip(this, level, pos, from, to);
}

bool BlockStateDefinition::BlockStateImpl::isTopSolidBlocking() const
{
    return mBlock->isTopSolidBlocking(this);
}

Vec3* BlockStateDefinition::BlockStateImpl::getOffset(LevelSource* levelSource, const BlockPos& pos) const
{
    return mBlock->getOffset(this, levelSource, pos);
}

bool BlockStateDefinition::BlockStateImpl::isViewBlocking() const
{
    return mBlock->isViewBlocking(this);
}

EBlockFaceShape BlockStateDefinition::BlockStateImpl::getBlockFaceShape(LevelSource* levelSource, const BlockPos& pos,
                                                                        const Direction* direction) const
{
    return mBlock->getBlockFaceShape(levelSource, this, pos, direction);
}

bool BlockStateDefinition::BlockStateImpl::triggerEvent(Level* level, const BlockPos& pos, int event,
                                                        int parameter) const
{
    return mBlock->triggerEvent(this, level, pos, event, parameter);
}

void BlockStateDefinition::BlockStateImpl::neighborChanged(Level* level, const BlockPos& pos, Block* block,
                                                           const BlockPos& from) const
{
    mBlock->neighborChanged(this, level, pos, block, from);
}
