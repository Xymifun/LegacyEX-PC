#ifndef GAME_WORLD_BLOCK_FLOWERPOTBLOCK_HPP
#define GAME_WORLD_BLOCK_FLOWERPOTBLOCK_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/InteractionHand.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/BaseEntityBlock.hpp"

class AABB;
class BlockEntity;
class BlockPos;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class Item;
class Level;
class LevelSource;
class Random;

template <class T> class AbstractProperty;
template <class T> class EnumProperty;

class FlowerPotBlock : public BaseEntityBlock
{
public:
    class Contents : public StringRepresentable
    {
    public:
        enum EContents
        {
            eContentsEmpty = 0,
            eContentsPoppy = 1,
            eContentsBlueOrchid = 2,
            eContentsAllium = 3,
            eContentsHoustonia = 4,
            eContentsRedTulip = 5,
            eContentsOrangeTulip = 6,
            eContentsWhiteTulip = 7,
            eContentsPinkTulip = 8,
            eContentsOxeyeDaisy = 9,
            eContentsDandelion = 10,
            eContentsOakSapling = 11,
            eContentsSpruceSapling = 12,
            eContentsBirchSapling = 13,
            eContentsJungleSapling = 14,
            eContentsAcaciaSapling = 15,
            eContentsDarkOakSapling = 16,
            eContentsMushroomRed = 17,
            eContentsMushroomBrown = 18,
            eContentsDeadBush = 19,
            eContentsFern = 20,
            eContentsCactus = 21,
            eContents_Count = 22
        };

        Contents(EContents contents, const std::wstring& name);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;

        static Contents* EMPTY;
        static Contents* POPPY;
        static Contents* BLUE_ORCHID;
        static Contents* ALLIUM;
        static Contents* HOUSTONIA;
        static Contents* RED_TULIP;
        static Contents* ORANGE_TULIP;
        static Contents* WHITE_TULIP;
        static Contents* PINK_TULIP;
        static Contents* OXEYE_DAISY;
        static Contents* DANDELION;
        static Contents* OAK_SAPLING;
        static Contents* SPRUCE_SAPLING;
        static Contents* BIRCH_SAPLING;
        static Contents* JUNGLE_SAPLING;
        static Contents* ACACIA_SAPLING;
        static Contents* DARK_OAK_SAPLING;
        static Contents* MUSHROOM_RED;
        static Contents* MUSHROOM_BROWN;
        static Contents* DEAD_BUSH;
        static Contents* FERN;
        static Contents* CACTUS;
        static const Contents* VALUES[eContents_Count];

        std::wstring mName;
        EContents mContents;
    };

    FlowerPotBlock();
    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int data);
    static void blockStaticCtor();
    virtual std::wstring getName();
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual int getColor(LevelSource* levelSource, const BlockPos& pos, int tintIndex);
    boost::shared_ptr<class FlowerPotBlockEntity> getFlowerPotEntity(Level* level, const BlockPos& pos);
    bool canBePotted(const not_null_ptr<ItemInstance>& item);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    bool CanSurvive(const BlockState* state);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool playerWillDestroy(Level* level, const BlockPos& pos, const BlockState* state,
                                   boost::shared_ptr<Player> player);
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual const BlockState* fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                              const BlockPos& pos);
    virtual int getRenderLayer();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual bool waterSpreadCausesSpawn();

    static EnumProperty<const Contents*>* CONTENTS;
    static AbstractProperty<int>* LEGACY_DATA;
    static AABB* SHAPE;
    static const double POT_WIDTH;
};

#endif
