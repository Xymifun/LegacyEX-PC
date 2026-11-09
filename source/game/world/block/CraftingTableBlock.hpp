#ifndef GAME_WORLD_BLOCK_CRAFTINGTABLEBLOCK_HPP
#define GAME_WORLD_BLOCK_CRAFTINGTABLEBLOCK_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/world/block/Block.hpp"
#include "game/core/InteractionObject.hpp"
#include "game/world/block/BlockPos.hpp"

class CraftingMenu;
class Level;

class CraftingTableBlock : public Block
{
public:
    class CraftingTable : public InteractionObject
    {
    public:
        CraftingTable(Level* level, const BlockPos& pos);

        virtual AbstractContainerMenu* createMenu(boost::shared_ptr<Inventory> inventory,
                                                  boost::shared_ptr<Player> player);
        virtual int getTypeId(bool large);
        virtual std::wstring getName();
        virtual std::wstring getCustomName();
        virtual bool hasCustomName();
        virtual std::wstring getDisplayName();

    private:
        Level* mLevel;
        BlockPos mBlockPos;
    };

    CraftingTableBlock();
    virtual bool TestUse();
    virtual bool use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                     InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY,
                     float hitZ, bool clientSide);
    virtual TextureAtlasSprite* getTexture(const Direction* direction, const BlockState* state);
    virtual void registerIcons(IconRegister* iconRegister);
    virtual unsigned int GetInteractTooltip(const BlockTooltipDataHolder& tooltipData);

private:
    TextureAtlasSprite* mTopTexture;
    TextureAtlasSprite* mFrontTexture;
};

#endif
