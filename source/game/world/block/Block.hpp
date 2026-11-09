#ifndef GAME_WORLD_BLOCK_BLOCK_HPP
#define GAME_WORLD_BLOCK_BLOCK_HPP

#include "game/item/not_null_ptr.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/DefaultedMappedRegistry.hpp"
#include "game/world/block/IdMapper.hpp"

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include "game/core/InteractionHand.hpp"

class AABB;
class BlockEntity;
class BlockItem;
class BlockPos;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class Entity;
class Explosion;
class HitResult;
class HtmlString;
class IconRegister;
class Item;
class ItemInstance;
class Level;
class LevelSource;
class LivingEntity;
class Material;
class MaterialColor;
class Mirror;
class Player;
class Property;
class Random;
class ResourceLocation;
class Rotation;
class SoundType;
class TextureAtlasSprite;
class Vec3;

enum EBlockType
{
    EBLOCK_TYPE_NONE = 0,
    EBLOCK_TYPE_DOOR = 1,
    EBLOCK_TYPE_RAIL = 2,
    EBLOCK_TYPE_WALL = 3,
    EBLOCK_TYPE_FENCE_GATE = 4,
    EBLOCK_TYPE_FENCE = 5,
    EBLOCK_TYPE_LIQUID = 6,
    EBLOCK_TYPE_SLAB = 8,
    EBLOCK_TYPE_HOPPER = 9,
    EBLOCK_TYPE_FALLING = 11
};

enum EBlockRenderShape
{
    BLOCK_RENDER_INVISIBLE = -1,
    BLOCK_RENDER_MODEL = 0,
    BLOCK_RENDER_CROSS = 1,
    BLOCK_RENDER_TORCH = 2,
    BLOCK_RENDER_FIRE = 3,
    BLOCK_RENDER_WATER = 4,
    BLOCK_RENDER_DUST = 5,
    BLOCK_RENDER_ROW = 6,
    BLOCK_RENDER_DOOR = 7,
    BLOCK_RENDER_LADDER = 8,
    BLOCK_RENDER_RAIL = 9,
    BLOCK_RENDER_STAIRS = 10,
    BLOCK_RENDER_FENCE = 11,
    BLOCK_RENDER_LEVER = 12,
    BLOCK_RENDER_CACTUS = 13,
    BLOCK_RENDER_BED = 14,
    BLOCK_RENDER_REPEATER = 15,
    BLOCK_RENDER_PISTON_BASE = 16,
    BLOCK_RENDER_PISTON_EXTENSION = 17,
    BLOCK_RENDER_THIN_FENCE = 18,
    BLOCK_RENDER_STEM = 19,
    BLOCK_RENDER_VINE = 20,
    BLOCK_RENDER_FENCE_GATE = 21,
    BLOCK_RENDER_ENTITY = 22,
    BLOCK_RENDER_LILYPAD = 23,
    BLOCK_RENDER_CAULDRON = 24,
    BLOCK_RENDER_BREWING_STAND = 25,
    BLOCK_RENDER_AIR_PORTAL_FRAME = 26,
    BLOCK_RENDER_EGG = 27,
    BLOCK_RENDER_COCOA = 28,
    BLOCK_RENDER_TRIPWIRE_SOURCE = 29,
    BLOCK_RENDER_TRIPWIRE = 30,
    BLOCK_RENDER_TREE = 31,
    BLOCK_RENDER_WALL = 32,
    BLOCK_RENDER_FLOWER_POT = 33,
    BLOCK_RENDER_BEACON = 34,
    BLOCK_RENDER_ANVIL = 35,
    BLOCK_RENDER_DIODE = 36,
    BLOCK_RENDER_COMPARATOR = 37,
    BLOCK_RENDER_HOPPER = 38,
    BLOCK_RENDER_QUARTZ = 39,
    BLOCK_RENDER_GLASS_PANE = 40,
    BLOCK_RENDER_DOUBLE_PLANT = 41,
    BLOCK_RENDER_SLIME_BLOCK = 42,
    BLOCK_RENDER_TRAPDOOR = 43,
    BLOCK_RENDER_END_ROD = 44,
    BLOCK_RENDER_CHORUS_FLOWER = 45,
    BLOCK_RENDER_CHORUS_PLANT = 46,
    BLOCK_RENDER_HORIZONTAL_DIRECTIONAL = 47,
    BLOCK_RENDER_OBSERVER = 48,
    BLOCK_RENDER_KELP = 49,
    BLOCK_RENDER_TURTLE_EGG = 50,
    BLOCK_RENDER_CORAL_FAN = 51,
    BLOCK_RENDER_SEAGRASS = 52,
    BLOCK_RENDER_SEA_PICKLE = 53,
    BLOCK_RENDER_CONDUIT = 54,
    BLOCK_RENDER_COMMAND_BLOCK = 55
};

enum EBlockOffsetType
{
    BLOCK_OFFSET_NONE = 0,
    BLOCK_OFFSET_XZ = 1,
    BLOCK_OFFSET_XYZ = 2
};

class Block
{
public:
    enum EUpdateFlag
    {
        UPDATE_NONE = 0,
        UPDATE_NEIGHBORS = 1,
        UPDATE_CLIENTS = 2,
        UPDATE_ALL = UPDATE_NEIGHBORS | UPDATE_CLIENTS,
        UPDATE_INVISIBLE = 4,
        UPDATE_IMMEDIATE = 8,
        UPDATE_NO_OBSERVERS = 16,
        UPDATE_LIGHT_IF_CHANGED = 32,
        UPDATE_CHUNK_ONLY = 64
    };

    static DefaultedMappedRegistry<ResourceLocation, Block*>* const blockRegistry;
    static IdMapper<const BlockState*>* blockStateRegistry;
    static float INDESTRUCTIBLE;
    static const AABB* FULL_BLOCK_SHAPE;
    static const AABB* const NULL_AABB;
    static const int SIGNAL_MAX;
    static const int SIGNAL_NONE;

    class ThreadStorage
    {
    public:
        ThreadStorage();
        ~ThreadStorage();

        int mCacheIndex;
        bool mFastGraphics;
        arrayWithLength<const AABB*> mShapes;
    };

    Block(Material* material);
    Block(Material* material, const MaterialColor* materialColor);

    virtual bool isTopSolidBlocking(const BlockState* state);
    virtual bool canProvideSupport(const BlockState* state, const Direction* direction, EBlockSupportType supportType);
    virtual bool isValidSpawn(const BlockState* state, boost::shared_ptr<Entity> entity);
    virtual int getLightEmission(const BlockState* state);
    virtual bool ParticlesSurviveWithin();
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual unsigned int GetHitTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* setPropertyValueFromString(const BlockState* state, const Property* property,
                                                         const std::wstring& value);
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                              const BlockPos& pos);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual ~Block();
    virtual void DerivedInit();
    virtual Block* sendBlockData(unsigned char blockData);
    virtual Block* setSoundType(const SoundType* soundType);
    virtual Block* setLightBlock(int lightBlock);
    virtual Block* setLightEmission(float lightEmission);
    virtual Block* setExplodeable(float explosionResistance);
    virtual bool isSolidBlockingCube(const BlockState* state);
    virtual bool isSolidBlockingCubeAndNotSignalSource(const BlockState* state);
    virtual bool isViewBlocking(const BlockState* state);
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool hasCustomBreakingProgress(const BlockState* state);
    virtual bool isPathfindable(LevelSource* levelSource, const BlockPos& pos);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual bool hasInHandRenderOffset();
    virtual bool mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos);
    virtual bool isTransparent(const BlockState* state);
    virtual Block* setDestroyTime(float destroyTime);
    virtual Block* setIndestructible();
    virtual bool isIndestructible();
    virtual float getDestroySpeed(const BlockState* state, Level* level, const BlockPos& pos);
    virtual Block* setTicking(bool ticking);
    virtual Block* disableMipmap();
    virtual Block* setSemiTransparent();
    virtual bool isTicking();
    virtual bool isSlabBlock();
    virtual bool isWaterBlocking() const;
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual bool isStrippable();
    virtual const BlockState* getStrippedBlock(const BlockState* state);
    virtual bool liquidCanFlowIntoFromDirection(LevelSource* levelSource, const BlockPos& pos,
                                                const Direction* direction) const;
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual int getLightColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, int lightValue);
    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                  const Direction* direction);
    virtual const arrayWithLength<const AABB*>* getShapes(const BlockState* state, LevelSource* levelSource,
                                                          const BlockPos& pos);
    virtual bool isSolidFace(LevelSource* levelSource, const BlockPos& pos, const Direction* direction);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual const AABB* getOutlineAABB(const BlockState* state, Level* level, const BlockPos& pos);
    virtual void addCollisionAABBs(const BlockState* state, Level* level, const BlockPos& pos, const AABB* entityBox,
                                   std::vector<AABB*>* collidingBoxes, boost::shared_ptr<Entity> entity,
                                   bool isActualState);
    virtual void addCollisionAABB(const BlockPos& pos, const AABB* entityBox, std::vector<AABB*>* collidingBoxes,
                                  const AABB* blockBox);
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool mayPick(const BlockState* state, bool includeLiquid);
    virtual bool mayPick();
    virtual void randomTick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    virtual void destroy(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual void addLights(Level* level, const BlockPos& pos);
    virtual int getTickDelay(Level* level);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int getResourceCount(Random* random);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual float getDestroyProgress(const BlockState* state, boost::shared_ptr<Player> player, Level* level,
                                     const BlockPos& pos);
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, int lootBonus);
    virtual void spawnResources(Level* level, const BlockPos& pos, const BlockState* state, float chance,
                                int lootBonus);
    virtual void popExperience(Level* level, const BlockPos& pos, int amount);
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual float getExplosionResistance(boost::shared_ptr<Entity> entity);
    virtual HitResult* clip(const BlockState* state, Level* level, const BlockPos& pos, Vec3* from, Vec3* to);
    virtual HitResult* clip(const BlockPos& pos, Vec3* from, Vec3* to, const AABB* box);
    virtual void wasExploded(Level* level, const BlockPos& pos, Explosion* explosion);
    virtual int getRenderLayer();
    virtual bool mayPlace(Level* level, const BlockPos& pos, const Direction* direction);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual bool mayPlaceOn(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool TestUse();
    virtual bool TestUse(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual void stepOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual void prepareRender(Level* level, const BlockPos& pos);
    virtual void attack(Level* level, const BlockPos& pos, boost::shared_ptr<Player> player);
    virtual Vec3* handleEntityInside(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, Vec3* motion);
    virtual int getColor() const;
    virtual int getColor(const BlockState* state);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, int tintIndex);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state);
    virtual int getSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                          const Direction* direction);
    virtual bool isSignalSource(const BlockState* state);
    virtual void entityInside(Level* level, const BlockPos& pos, const BlockState* state,
                              const boost::shared_ptr<Entity>& entity);
    virtual int getDirectSignal(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                const Direction* direction);
    virtual bool waterSpreadCausesSpawn();
    virtual void playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                               const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                               not_null_ptr<ItemInstance> tool);
    virtual bool isSilkTouchable();
    virtual not_null_ptr<ItemInstance> getSilkTouchItemInstance(const BlockState* state);
    virtual bool canSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int getResourceCountForLootBonus(int lootBonus, Random* random);
    virtual void setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance);
    virtual Block* setNameAndDescriptionId(int descriptionId, int useDescriptionId);
    virtual bool isPossibleToRespawnInThis();
    virtual std::wstring getName();
    virtual int getDescriptionId(int auxValue);
    virtual int getUseDescriptionId(int auxValue);
    virtual bool triggerEvent(const BlockState* state, Level* level, const BlockPos& pos, int eventId, int eventParam);
    virtual bool isCollectStatistics();
    virtual bool shouldBlockTick(Level* level, const BlockPos& pos, const BlockState* state);
    virtual Block* setNotCollectStatistics();
    virtual EPistonPushReaction getPistonPushReaction(const BlockState* state);
    virtual float getShadeBrightness(const BlockState* state);
    virtual void fallOn(Level* level, const BlockPos& pos, boost::shared_ptr<Entity> entity, float fallDistance);
    virtual void updateEntityAfterFallOn(Level* level, boost::shared_ptr<Entity> entity);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                   boost::shared_ptr<Player> player);
    virtual void handleRain(Level* level, const BlockPos& pos);
    virtual bool canInstantlyTick();
    virtual bool dropFromExplosion(Explosion* explosion);
    virtual bool isMatching(Block* block);
    virtual bool hasAnalogOutputSignal(const BlockState* state);
    virtual int getAnalogOutputSignal(const BlockState* state, Level* level, const BlockPos& pos);
    virtual Block* setIconName(const std::wstring& iconName);
    virtual std::wstring getIconName();
    virtual void registerIcons(IconRegister* iconRegister);
    virtual std::wstring getTileItemIconName();
    virtual TextureAtlasSprite* getIconTexture(const Direction* direction, const BlockState* state);
    virtual TextureAtlasSprite* getTexture(LevelSource* levelSource, const BlockPos& pos, const Direction* direction,
                                           const BlockState* state);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual TextureAtlasSprite* getTexture(const Direction* direction);
    virtual const BlockState* getInventoryRenderState(const BlockState* state);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual BlockStateDefinition* getBlockStateDefinition();
    virtual EBlockOffsetType getOffsetType();
    virtual Vec3* getOffset(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual const SoundType* getSoundType();
    virtual std::wstring toString();
    virtual void appendHoverText(not_null_ptr<ItemInstance> itemInstance, boost::shared_ptr<Player> player,
                                 std::vector<HtmlString>* lines, bool advanced);
    virtual bool isLiquidBlock();

    static void staticCtor();
    static void CreateNewThreadStorage();
    static void ReleaseThreadStorage();

    static void registerBlock(int id, const ResourceLocation& location, Block* block);
    static void registerBlock(int id, const std::wstring& name, Block* block);

    static int getId(Block* block);
    static int getElementId(Block* block);
    static int getElementId(const BlockState* state);
    static int getIdWithData(const BlockState* state);
    static Block* byId(int id);
    static bool existsWithId(int id);
    static const BlockState* getStateByElementId(unsigned short elementId);
    static const BlockState* stateByIdWithData(int idWithData);
    static Block* byItem(Item* item);
    static Block* byString(const std::wstring& name);

    static bool isExceptionBlockForAttaching(Block* block);
    static bool isExceptionBlockForAttachingPlusPiston(Block* block);
    static bool isExceptionForPistonAttaching(Block* block);
    static bool isExceptionForTopSolidBlocking(Block* block);
    static bool isMatching(Block* first, Block* second);
    static void popResource(Level* level, const BlockPos& pos, not_null_ptr<ItemInstance> itemInstance);

    Block* setBaseItemTypeAndMaterial(int baseItemType, int baseItemMaterial);
    bool calculateIsWaterBlocking() const;
    void setThickness(float thickness);
    void registerDefaultState(const BlockState* state);
    const BlockState* defaultBlockState();
    void ClearDerivedInit();
    int getBaseItemType();
    bool neighbourVisibleThroughShape(const AABB* shape, LevelSource* levelSource, const BlockPos& pos,
                                      const Direction* direction);
    bool neighbourVisibleThroughShapes(const arrayWithLength<const AABB*>* shapes, LevelSource* levelSource,
                                       const BlockPos& pos, const Direction* direction);
    int getFaceFlagsFast(LevelSource* levelSource, const BlockPos& pos);
    int getFaceFlags(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, bool forceAll);
    bool instanceof(EBlockType blockType);

    Material* getCraftingMaterial() { return mMaterial; }

public:
    void _init(Material* material, const MaterialColor* materialColor);

    bool mIsDoor;
    bool mIsRail;
    bool mIsFence;
    bool mIsWall;
    bool mIsFenceGate;
    std::wstring mIconName;
    int mCachedElementId;
    bool mFullBlock;
    int mLightBlock;
    bool mTranslucent;
    int mLightEmission;
    bool mUseNeighbourBrightness;
    bool mMipmap;
    unsigned char mBlockData;
    bool mSemiTransparent;
    float mDestroyTime;
    float mExplosionResistance;
    bool mCollectStatistics;
    bool mTicking;
    bool mBlockEntity;
    bool mWaterBlocking;
    int mBaseItemMaterial;
    int mBaseItemType;
    float mThickness;
    bool mDerivedInitDone;
    const SoundType* mSoundType;
    float mParticleGravity;
    Material* mMaterial;
    const MaterialColor* mMaterialColor;
    float mFriction;
    BlockStateDefinition* mStateDefinition;
    const BlockState* mDefaultBlockState;
    int mDescriptionId;
    int mUseDescriptionId;
    TextureAtlasSprite* mIcon;
    TextureAtlasSprite* mCarriedIcon;
    EBlockType mBlockType;
};

extern unsigned int THREAD_STORAGE_INDEX;


boost::shared_ptr<ItemInstance> createItem(Block* block);

#endif
