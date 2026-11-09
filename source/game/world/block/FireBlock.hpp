#ifndef GAME_WORLD_BLOCK_FIREBLOCK_HPP
#define GAME_WORLD_BLOCK_FIREBLOCK_HPP

#include <boost/unordered_map.hpp>

#include "game/world/block/Block.hpp"

template <class T> class AbstractProperty;

class FireBlock : public Block
{
public:
    static AbstractProperty<int>* AGE;
    static AbstractProperty<bool>* NORTH;
    static AbstractProperty<bool>* EAST;
    static AbstractProperty<bool>* SOUTH;
    static AbstractProperty<bool>* WEST;
    static AbstractProperty<bool>* UP;

    static const std::wstring TEXTURE_FIRST;
    static const std::wstring TEXTURE_SECOND;

    FireBlock();

    int getFlameOdds(Block* block);
    bool canBurn(LevelSource* levelSource, const BlockPos& pos);
    const BlockState* fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                      const BlockPos& pos);
    void setFlammable(Block* block, int flameOdds, int burnOdds);
    static void staticCtor();
    virtual const AABB* getClipAABB(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual bool isSolidRender(const BlockState* state) const;
    virtual bool isCubeShaped(const BlockState* state);
    virtual int getResourceCount(Random* random);
    virtual int getTickDelay(Level* level);
    int getFireOdds(Level* level, const BlockPos& pos);
    bool isNearRain(Level* level, const BlockPos& pos);
    bool isValidFireLocation(Level* level, const BlockPos& pos);
    int getBurnOdd(Block* block);
    void checkBurnOut(Level* level, const BlockPos& pos, int odds, Random* random, int age);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual bool canInstantlyTick();
    virtual bool mayPick();
    virtual bool mayPlace(Level* level, const BlockPos& pos);
    virtual void neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block* block,
                                 const BlockPos& fromPos);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random);
    virtual const MaterialColor* getMapColor(const BlockState* state, LevelSource* levelSource, const BlockPos& pos);
    virtual int getRenderLayer();
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual bool waterSpreadCausesSpawn();
    virtual bool canContainLiquid(const BlockState* state) const;
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual EBlockFaceShape getBlockFaceShape(LevelSource* levelSource, const BlockState* state, const BlockPos& pos,
                                              const Direction* direction);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    TextureAtlasSprite* getTextureLayer(int layer);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);

private:
    boost::unordered_map<Block*, int> mFlameOdds;
    boost::unordered_map<Block*, int> mBurnOdds;
    TextureAtlasSprite* mFireIcons[2];
};

#endif
