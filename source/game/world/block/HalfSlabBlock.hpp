#ifndef GAME_WORLD_BLOCK_HALFSLABBLOCK_HPP
#define GAME_WORLD_BLOCK_HALFSLABBLOCK_HPP

#include <string>
#include <boost/shared_ptr.hpp>
#include "game/core/StringRepresentable.hpp"
#include "game/world/block/Block.hpp"

class HalfSlabBlock : public Block
{
public:
    class Half : public StringRepresentable
    {
    public:
        enum
        {
            HALF_COUNT = 2
        };
        Half(const std::wstring& name);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;
        std::wstring mName;
    };
    HalfSlabBlock(Material* material);
    HalfSlabBlock(Material* material, const MaterialColor* materialColor);
    virtual bool isTopSolidBlocking(const BlockState* state);
    virtual ~HalfSlabBlock() {}
    virtual void DerivedInit();
    virtual bool isCubeShaped(const BlockState* state);
    virtual bool isPathfindable(LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSlabBlock();
    virtual bool liquidCanFlowIntoFromDirection(LevelSource* levelSource, const BlockPos& pos,
                                                const Direction* direction) const;
    virtual const AABB* getShape(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool shouldRenderFace(const BlockState* state, LevelSource* levelSource, const BlockPos& pos,
                                  const Direction* direction);
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual int getResourceCount(Random* random);
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual bool isSilkTouchable();
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos,
                                                            const BlockState* state) = 0;
    virtual int getAuxName(int auxValue) = 0;
    virtual bool isFullSize() const = 0;
    virtual Property* getTypeProperty() = 0;
    virtual Boxed* getTypeForItem(not_null_ptr<ItemInstance> itemInstance) = 0;
    static void blockStaticCtor();
    static bool isHalfSlab(Block* block);
    static bool isHalfSlab(const BlockState* state);
    static bool isFullSlab(Block* block);
    static bool isHalfOrFullSlab(Block* block);
    static Block* GetDoubleSlabBlockFromSlab(Block* block);
    static bool isBottomSlab(int data);
    void _init();
    static AbstractProperty<Half*>* TYPE;
    static Half* TOP;
    static Half* BOTTOM;
    static Half* HALVES[Half::HALF_COUNT];
};

#endif
