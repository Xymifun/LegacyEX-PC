#include "game/world/block/WallBlock.hpp"
#include "game/core/StringIds.hpp"

#include <typeinfo>
#include <boost/unordered_set.hpp>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/FenceGateBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Predicates.hpp"
#include "game/world/block/TypedBoxed.hpp"

static AbstractProperty<bool>* UP = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"up"));
static AbstractProperty<bool>* NORTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"north"));
static AbstractProperty<bool>* EAST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"east"));
static AbstractProperty<bool>* SOUTH = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"south"));
static AbstractProperty<bool>* WEST = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"west"));

static float POST_MIN = 0.25f;
static float POST_MAX = 0.75f;
static float POST_MIN_Z = 0.25f;
static float POST_MAX_Z = 0.75f;
static float SIDE_MIN = 0.3125f;
static float SIDE_MAX = 0.6875f;

static AABB* AABB_BY_INDEX[16] = {
    AABB::newPermanent(0.25, 0.0, 0.25, 0.75, 1.0, 0.75),
    AABB::newPermanent(POST_MIN, 0.0, POST_MIN_Z, POST_MAX, 1.0, 1.0),
    AABB::newPermanent(0.0, 0.0, POST_MIN_Z, POST_MAX, 1.0, POST_MAX_Z),
    AABB::newPermanent(0.0, 0.0, POST_MIN_Z, POST_MAX, 1.0, 1.0),
    AABB::newPermanent(POST_MIN, 0.0, 0.0, POST_MAX, 1.0, POST_MAX_Z),
    AABB::newPermanent(SIDE_MIN, 0.0, 0.0, SIDE_MAX, 0.875, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, POST_MAX, 1.0, POST_MAX_Z),
    AABB::newPermanent(0.0, 0.0, 0.0, POST_MAX, 1.0, 1.0),
    AABB::newPermanent(POST_MIN, 0.0, POST_MIN_Z, 1.0, 1.0, POST_MAX_Z),
    AABB::newPermanent(POST_MIN, 0.0, POST_MIN_Z, 1.0, 1.0, 1.0),
    AABB::newPermanent(0.0, 0.0, SIDE_MIN, 1.0, 0.875, SIDE_MAX),
    AABB::newPermanent(0.0, 0.0, POST_MIN_Z, 1.0, 1.0, 1.0),
    AABB::newPermanent(POST_MIN, 0.0, 0.0, 1.0, 1.0, POST_MAX_Z),
    AABB::newPermanent(POST_MIN, 0.0, 0.0, 1.0, 1.0, 1.0),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, POST_MAX_Z),
    AABB::newPermanent(0.0, 0.0, 0.0, 1.0, 1.0, 1.0),
};

static AABB* CLIP_AABB_BY_INDEX[16] = {
    AABB_BY_INDEX[0]->setMaxY(1.5),  AABB_BY_INDEX[1]->setMaxY(1.5),  AABB_BY_INDEX[2]->setMaxY(1.5),
    AABB_BY_INDEX[3]->setMaxY(1.5),  AABB_BY_INDEX[4]->setMaxY(1.5),  AABB_BY_INDEX[5]->setMaxY(1.5),
    AABB_BY_INDEX[6]->setMaxY(1.5),  AABB_BY_INDEX[7]->setMaxY(1.5),  AABB_BY_INDEX[8]->setMaxY(1.5),
    AABB_BY_INDEX[9]->setMaxY(1.5),  AABB_BY_INDEX[10]->setMaxY(1.5), AABB_BY_INDEX[11]->setMaxY(1.5),
    AABB_BY_INDEX[12]->setMaxY(1.5), AABB_BY_INDEX[13]->setMaxY(1.5), AABB_BY_INDEX[14]->setMaxY(1.5),
    AABB_BY_INDEX[15]->setMaxY(1.5),
};

WallBlock::Variant* WallBlock::Variant::COBBLESTONE = new WallBlock::Variant(0, L"cobblestone",
                                                                             IDS_TILE_COBBLESTONE_WALL);
WallBlock::Variant* WallBlock::Variant::MOSSY = new WallBlock::Variant(1, L"mossy_cobblestone",
                                                                       IDS_TILE_COBBLESTONE_WALL_MOSSY);
WallBlock::Variant* WallBlock::Variant::VALUES[2] = {WallBlock::Variant::COBBLESTONE, WallBlock::Variant::MOSSY};
EnumProperty<WallBlock::Variant*>* WallBlock::VARIANT = NULL;
const double WallBlock::POST_HEIGHT = 1.0;
const float WallBlock::ARM_HALF_WIDTH = 0.1875f;
const float WallBlock::ARM_HEIGHT = 0.875f;
const float WallBlock::POST_HALF_WIDTH = 0.25f;

WallBlock::Variant::Variant(int data, const std::wstring& name, int translationKey)
    : mData(data),
      mName(name),
      mTranslationKey(translationKey)
{
}

int WallBlock::Variant::getData()
{
    return mData;
}

WallBlock::Variant* WallBlock::Variant::byData(int data)
{
    return VALUES[(unsigned int)data < 2 ? data : 0];
}

std::wstring WallBlock::Variant::toString() const
{
    return mName;
}
std::wstring WallBlock::Variant::getSerializedName() const
{
    return mName;
}
int WallBlock::Variant::getTranslationKey()
{
    return mTranslationKey;
}

void WallBlock::blockStaticCtor()
{
    std::wstring name(L"variant");
    EnumProperty<Variant*>* variant;
    const Predicate<Variant*>* filter = Predicates<Variant*>::alwaysTrue;
    {
        boost::unordered_set<Variant*> values;
        for (int i = 0; i < 2; ++i)
        {
            values.insert(Variant::VALUES[i]);
        }
        variant = EnumProperty<Variant*>::create(name, typeid(Variant), values, filter);
    }
    VARIANT = variant;
}

WallBlock::WallBlock(Block* block) : Block(block->mMaterial)
{
    mIsWall = true;
    mBlockType = EBLOCK_TYPE_WALL;
    DerivedInit();
    registerDefaultState(mStateDefinition->any()
                             ->setValue(UP, false)
                             ->setValue(NORTH, false)
                             ->setValue(EAST, false)
                             ->setValue(SOUTH, false)
                             ->setValue(WEST, false)
                             ->setValue(VARIANT, Variant::COBBLESTONE));
    setDestroyTime(block->mDestroyTime);
    setExplodeable(block->mExplosionResistance / 3.0f);
    setSoundType(block->mSoundType);
}

unsigned int WallBlock::getAABBIndex(const BlockState* state)
{
    unsigned int index = 0;
    if (state->getValue(NORTH))
        index = 1 << Direction::NORTH->get2DDataValue();
    if (state->getValue(EAST))
        index |= 1 << Direction::EAST->get2DDataValue();
    if (state->getValue(SOUTH))
        index |= 1 << Direction::SOUTH->get2DDataValue();
    if (state->getValue(WEST))
        index |= 1 << Direction::WEST->get2DDataValue();
    return index;
}

const AABB* WallBlock::getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    state = fillVirtualBlockStateProperties(state, levelSource, pos);
    return AABB_BY_INDEX[getAABBIndex(state)];
}

void WallBlock::addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                  std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity>, bool isActualState)
{
    if (!isActualState)
        state = fillVirtualBlockStateProperties(state, level, pos);
    addCollisionAABB(pos, entityBox, collidingBoxes, CLIP_AABB_BY_INDEX[getAABBIndex(state)]);
}

const AABB* WallBlock::getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos)
{
    state = fillVirtualBlockStateProperties(state, levelSource, pos);
    return CLIP_AABB_BY_INDEX[getAABBIndex(state)];
}

std::wstring WallBlock::getName()
{
    return L10N::GetString(IDS_TILE_COBBLESTONE_WALL);
}
bool WallBlock::isCubeShaped(const BlockState*)
{
    return false;
}
bool WallBlock::isPathfindable(LevelSource*, const BlockPos&)
{
    return false;
}
bool WallBlock::isSolidRender(const BlockState*) const
{
    return false;
}
bool WallBlock::canContainLiquid(const BlockState*) const
{
    return true;
}

bool WallBlock::isExceptionBlockForAttachingPlusPiston(Block* block)
{
    return Block::isExceptionBlockForAttachingPlusPiston(block) || block == Blocks::barrier
           || block == Blocks::melon_block || block == Blocks::pumpkin || block == Blocks::carved_pumpkin
           || block == Blocks::lit_pumpkin;
}

bool WallBlock::connectsTo(LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    const BlockState* state = levelSource->getBlockState(pos);
    Block* block = state->getBlock();
    EBlockFaceShape faceShape = state->getBlockFaceShape(levelSource, pos, direction);
    bool wallFace = faceShape == BLOCK_FACE_MIDDLE_POLE_THICK
                    || (faceShape == BLOCK_FACE_MIDDLE_POLE && block != NULL
                        && dynamic_cast<FenceGateBlock*>(block) != NULL);
    return (!isExceptionBlockForAttachingPlusPiston(block) && faceShape == BLOCK_FACE_SOLID) || wallFace;
}

int WallBlock::getSpawnResourcesAuxValue(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

bool WallBlock::shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                 const Direction* direction)
{
    if (direction == Direction::DOWN)
        return Block::shouldRenderFace(state, levelSource, pos, direction);
    return true;
}

const BlockState* WallBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(VARIANT, Variant::byData(data));
}

int WallBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(VARIANT)->getData();
}

const BlockState* WallBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                             const BlockPos& pos)
{
    bool north = connectsTo(levelSource, pos.north(), Direction::SOUTH);
    bool east = connectsTo(levelSource, pos.east(), Direction::WEST);
    bool south = connectsTo(levelSource, pos.south(), Direction::NORTH);
    bool west = connectsTo(levelSource, pos.west(), Direction::EAST);
    bool straight = (north && !east && south && !west) || (!north && east && !south && west);
    return state->setValue(UP, !straight || !levelSource->isEmptyBlock(pos.above()))
        ->setValue(NORTH, north)
        ->setValue(EAST, east)
        ->setValue(SOUTH, south)
        ->setValue(WEST, west);
}

BlockStateDefinition* WallBlock::createBlockStateDefinition()
{
    static const Property* properties[] = {UP, NORTH, EAST, SOUTH, WEST, VARIANT};
    return new BlockStateDefinition(this, properties, 6);
}

EBlockFaceShape WallBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&,
                                             const Direction* direction)
{
    if (direction != Direction::UP && direction != Direction::DOWN)
        return BLOCK_FACE_MIDDLE_POLE_THICK;
    return BLOCK_FACE_CENTER_BIG;
}

TextureAtlasSprite* WallBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (state->getValue(VARIANT) == Variant::MOSSY)
        return Blocks::mossy_cobblestone->getTexture(direction);
    return Blocks::cobblestone->getTexture(direction);
}

void WallBlock::registerIcons(IconRegister*)
{
}

EBlockRenderShape WallBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_WALL;
}
