#include "game/world/block/CraftingTableBlock.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/ui/CraftingMenu.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/generation/Player.hpp"

CraftingTableBlock::CraftingTableBlock() : Block(Material::WOOD)
{
    DerivedInit();
    mTopTexture = NULL;
    mFrontTexture = NULL;
}

bool CraftingTableBlock::TestUse()
{
    return true;
}

bool CraftingTableBlock::use(Level* level, const BlockPos& pos, const BlockState* state,
                             boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand,
                             const Direction* direction, float hitX, float hitY, float hitZ, bool clientSide)
{
    if (clientSide)
    {
        return false;
    }
    if (level->mIsClientSide)
    {
        return true;
    }

    CraftingTable table(level, pos);
    player->startBlockInteraction(table);
    return true;
}

TextureAtlasSprite* CraftingTableBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction == Direction::UP)
    {
        return mTopTexture;
    }
    if (direction == Direction::DOWN)
    {
        return Blocks::planks->getTexture(direction);
    }
    if (direction == Direction::NORTH || direction == Direction::WEST)
    {
        return mFrontTexture;
    }
    return mIcon;
}

void CraftingTableBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"workbench_side");
    mTopTexture = iconRegister->registerIcon(L"workbench_top");
    mFrontTexture = iconRegister->registerIcon(L"workbench_front");
}

unsigned int CraftingTableBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock) && tooltipData.m_bCanPlace)
    {
        return IDS_TOOLTIPS_USE;
    }
    return Block::GetInteractTooltip(tooltipData);
}

CraftingTableBlock::CraftingTable::CraftingTable(Level* level, const BlockPos& pos) : InteractionObject()
{
    mLevel = level;
    mBlockPos = pos;
}

AbstractContainerMenu* CraftingTableBlock::CraftingTable::createMenu(boost::shared_ptr<Inventory> inventory,
                                                                     boost::shared_ptr<Player> player)
{
    return new CraftingMenu(inventory, mLevel, mBlockPos);
}

int CraftingTableBlock::CraftingTable::getTypeId(bool large)
{
    return ClientboundContainerOpenPacket::WORKBENCH;
}

std::wstring CraftingTableBlock::CraftingTable::getName()
{
    return L"crafting_table";
}

std::wstring CraftingTableBlock::CraftingTable::getCustomName()
{
    return L"";
}

bool CraftingTableBlock::CraftingTable::hasCustomName()
{
    return false;
}

std::wstring CraftingTableBlock::CraftingTable::getDisplayName()
{
    return L"";
}
