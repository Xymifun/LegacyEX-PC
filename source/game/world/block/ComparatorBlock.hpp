#ifndef GAME_WORLD_BLOCK_COMPARATORBLOCK_HPP
#define GAME_WORLD_BLOCK_COMPARATORBLOCK_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/world/block/DiodeBlock.hpp"
#include "game/world/block/EntityBlock.hpp"
#include "game/world/block/Predicate.hpp"
#include "game/core/Direction.hpp"

class BlockEntity;
class BlockState;
class BlockStateDefinition;
class BlockTooltipDataHolder;
class Direction;
class Entity;
class ItemFrame;
class ItemInstance;
class Item;
class Level;
class LevelSource;
class LivingEntity;
class Mirror;
class Random;
class Rotation;
class Player;
class Property;
class TextureAtlasSprite;

template <typename T> class EnumProperty;
template <typename T> class not_null_ptr;

class ComparatorBlock : public DiodeBlock, public EntityBlock
{
public:
    class ComparatorMode : public StringRepresentable
    {
    public:
        static ComparatorMode* COMPARE;
        static ComparatorMode* SUBTRACT;

        explicit ComparatorMode(const std::wstring& name);
        virtual std::wstring toString() const;
        virtual std::wstring getSerializedName() const;

    private:
        std::wstring mName;
    };

    class EntitySelector : public Predicate<boost::shared_ptr<Entity> >
    {
    public:
        explicit EntitySelector(const Direction* direction);
        virtual ~EntitySelector() {}
        virtual bool apply(boost::shared_ptr<Entity> entity) const;

    private:
        const Direction* mDirection;
    };

    static EnumProperty<ComparatorMode*>* MODE;

    ComparatorBlock(bool on);
    virtual int calculateOutputSignal(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void refreshOutputState(Level* level, const BlockPos& pos, const BlockState* state);
    virtual boost::shared_ptr<BlockEntity> newBlockEntity(Level* level, int type);
    static void blockStaticCtor();
    virtual std::wstring getName();
    virtual Item* getResource(const BlockState* state, Random* random, int lootBonus);
    virtual not_null_ptr<ItemInstance> getCloneItemInstance(Level* level, const BlockPos& pos, const BlockState* state);
    virtual int getTurnOnDelay(const BlockState* state);
    virtual const BlockState* getOnBlock(const BlockState* state);
    virtual const BlockState* getOffBlock(const BlockState* state);
    virtual bool isOn(const BlockState* state);
    virtual int getOutputSignal(LevelSource* levelSource, const BlockPos& pos, const BlockState* state);
    virtual bool shouldTurnOn(Level* level, const BlockPos& pos, const BlockState* state);
    boost::shared_ptr<ItemFrame> getItemFrame(Level* level, const Direction* direction, const BlockPos& pos);
    virtual int getInputSignal(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual void checkTickOnNeighbor(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void tick(Level* level, const BlockPos& pos, const BlockState* state, Random* random);
    virtual void onPlace(Level* level, const BlockPos& pos, const BlockState* state);
    virtual void onRemove(Level* level, const BlockPos& pos, const BlockState* state);
    virtual bool triggerEvent(const BlockState* state, Level* level, const BlockPos& pos, int eventId, int eventParam);
    virtual const BlockState* getBlockState(int data);
    virtual int convertBlockStateToLegacyData(const BlockState* state);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);
    virtual const BlockState* rotate(const BlockState* state, Rotation* rotation);
    virtual const BlockState* mirror(const BlockState* state, Mirror* mirror);
    virtual BlockStateDefinition* createBlockStateDefinition();
    virtual const BlockState* getStateForPlacement(Level* level, const BlockPos& pos, const Direction* direction,
                                                   float hitX, float hitY, float hitZ, int auxValue,
                                                   boost::shared_ptr<LivingEntity> placer);
    virtual EBlockRenderShape getRenderShape(const BlockState* state);
    virtual bool TestUse();
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
};

#endif
