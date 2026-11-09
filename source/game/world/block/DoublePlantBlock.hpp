#ifndef GAME_WORLD_BLOCK_DOUBLEPLANTBLOCK_HPP
#define GAME_WORLD_BLOCK_DOUBLEPLANTBLOCK_HPP

#include "game/core/Bush.hpp"
#include "game/core/StringRepresentable.hpp"
#include "game/world/block/BonemealableBlock.hpp"
#include "game/item/not_null_ptr.hpp"

#include <string>

class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class IconRegister;
class Level;
class LevelSource;
class Random;
class TextureAtlasSprite;
class ItemInstance;
class LivingEntity;
class BlockEntity;
class Player;

class DoublePlantBlock : public Bush, public BonemealableBlock
{
public:
    class Half : public StringRepresentable
    {
    public:
        enum EHalf
        {
            UPPER = 0,
            LOWER = 1
        };

        explicit Half(int id);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        int getId() const;

        int mId;
    };

    class Variant : public StringRepresentable
    {
    public:
        enum ETexture
        {
            BOTTOM = 0,
            TOP = 1,
            SIDE = 2,
            INVENTORY = 3
        };

        Variant(int data, const std::wstring& name, int nameId, int descriptionId, std::wstring* textures);
        int getData();
        static Variant* byData(int data);

        static Variant* SUNFLOWER;
        static Variant* SYRINGA;
        static Variant* GRASS;
        static Variant* FERN;
        static Variant* ROSE;
        static Variant* PAEONIA;

        TextureAtlasSprite* getIcon(ETexture texture);
        std::wstring* getTexture(int texture);
        void setIcon(ETexture texture, TextureAtlasSprite* icon);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        int getNameId();
        int getDescriptionId();
        std::wstring getName();

        int mData;
        std::wstring mName;
        int mNameId;
        int mDescriptionId;
        std::wstring* mTextures;
        TextureAtlasSprite* mIcons[4];
    };

    DoublePlantBlock();
    Variant* getType(LevelSource* levelSource, const BlockPos& pos, const BlockState* state);
    virtual bool isValidBonemealTarget(Level* level, const BlockPos& pos, const BlockState* state, bool isClientSide);
    virtual bool isBonemealSuccess(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    virtual void performBonemeal(Level* level, Random* random, const BlockPos& pos, const BlockState* state);
    static void blockStaticCtor();
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual bool mayReplaceWithPlace(LevelSource* levelSource, const BlockPos& pos);
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual void checkAlive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool canSurvive(Level* level, const BlockPos& pos, const BlockState* state);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual int getSpawnResourcesAuxValue(const BlockState* state);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, int tintIndex);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, const BlockState* state);
    void placeAt(Level* level, const BlockPos& pos, Variant* variant, int flags);
    virtual void setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> item);
    int popGrassResources(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player);
    virtual void playerDestroy(Level* level, boost::shared_ptr<Player> player, const BlockPos& pos,
                               const BlockState* state, boost::shared_ptr<BlockEntity> blockEntity,
                               not_null_ptr<ItemInstance> tool);
    virtual bool playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                   boost::shared_ptr<Player> player);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual const BlockState* getBlockState(int data);
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                              const BlockPos& pos);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockOffsetType getOffsetType();
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    TextureAtlasSprite* getTexture(const BlockState* state, Variant::ETexture texture);
    virtual TextureAtlasSprite* getIconTexture(const Direction* direction, const BlockState* state);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);

    static AbstractProperty<Variant*>* VARIANT;
    static AbstractProperty<Half*>* HALF;
    static AbstractProperty<const Direction*>* FACING;
    static Variant* VARIANTS[6];
    static Half* UPPER_HALF;
    static Half* LOWER_HALF;
    static Half* HALVES[2];
};

extern DoublePlantBlock::Variant* gDoublePlantGrass;
extern DoublePlantBlock::Variant* gDoublePlantFern;

#endif
