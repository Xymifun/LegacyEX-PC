#ifndef GAME_WORLD_BLOCK_SAPLINGBLOCK_HPP
#define GAME_WORLD_BLOCK_SAPLINGBLOCK_HPP

#include <string>
#include "game/core/Bush.hpp"
#include "game/world/block/BonemealableBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"

template <class T> class EnumProperty;

class SaplingBlock : public Bush, public BonemealableBlock
{
public:
    SaplingBlock();
    virtual void advanceTree(Level*, const BlockPos&, const BlockState*, Random*);
    virtual bool isValidBonemealTarget(Level*, const BlockPos&, const BlockState*, bool);
    virtual bool isBonemealSuccess(Level*, Random*, const BlockPos&, const BlockState*);
    bool isSapling(Level*, const BlockPos&, PlanksBlock::Variant*);
    bool isTwoByTwoSaplingOfType(Level*, const BlockPos&, int, int, PlanksBlock::Variant*);
    void growTree(Level*, const BlockPos&, const BlockState*, Random*, bool);
    virtual void performBonemeal(Level*, Random*, const BlockPos&, const BlockState*);
    static void blockStaticCtor();
    virtual std::wstring getName();
    virtual const AABB* getShape(const BlockState*, LevelSource*, const BlockPos&);
    virtual void tick(Level*, const BlockPos&, const BlockState*, Random*);
    virtual int getSpawnResourcesAuxValue(const BlockState*);
    virtual const BlockState* getBlockState(int);
    virtual int convertBlockStateToLegacyData(const BlockState*);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder&);
    virtual bool canContainLiquid(const BlockState*) const;
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual TextureAtlasSprite* getTexture(const Direction*, const BlockState*);
    virtual void registerIcons(IconRegister*);
    virtual int getDescriptionId(int);

    static EnumProperty<PlanksBlock::Variant*>* TYPE;

private:
    TextureAtlasSprite* mIcons[6];
};

#endif
