#include "game/item/FrostWalkerEnchantment.hpp"
#include "game/core/StringIds.hpp"

#include "game/item/EnchantmentCategory.hpp"
#include "game/item/Enchantments.hpp"
#include "game/core/Direction.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Iterator.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/util/Random.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/Mth.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"

FrostWalkerEnchantment::FrostWalkerEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::ARMOR_FEET, slots)
{
    setDescriptionId((int)IDS_ENCHANTMENT_FROST_WALKER);
}

int FrostWalkerEnchantment::getMinCost(int level)
{
    return level * 10;
}

int FrostWalkerEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + 15;
}

bool FrostWalkerEnchantment::isTreasureOnly()
{
    return true;
}

int FrostWalkerEnchantment::getMaxLevel()
{
    return 2;
}

void FrostWalkerEnchantment::onEntityMoved(const boost::shared_ptr<LivingEntity>& entity, Level* level,
                                           const BlockPos& pos, int levelValue)
{
    boost::shared_ptr<Player> player = boost::dynamic_pointer_cast<Player>(entity);
    if (!entity->mOnGround || (player && player->mAbilities.mFlying))
    {
        return;
    }

    bool placed = false;
    const float radius = levelValue + 2 < 16 ? (float)(levelValue + 2) : 16.0f;
    MutableBlockPos abovePos(0, 0, 0);
    const BlockPos from = pos.offset(-radius, -1.0, -radius);
    const BlockPos to = pos.offset(radius, -1.0, radius);
    Iterator<MutableBlockPos*>* positions = BlockPos::mutablesBetween(from, to);
    while (positions->hasNext())
    {
        const BlockPos current = *positions->next();
        if (current.distSqrToCenter(entity->mX, entity->mY, entity->mZ) <= radius * radius)
        {
            abovePos.set(current.x, current.y + 1, current.z);
            if (level->getBlockState(abovePos)->getBlock() == Blocks::air)
            {
                const BlockState* state = level->getBlockState(current);
                if (state->getMaterial() == Material::WATER && state->getValue(LiquidBlock::LEVEL) == 0)
                {
                    bool canPlace = level->mayPlace(Blocks::frosted_ice, current, false, Direction::DOWN,
                                                    boost::shared_ptr<Entity>());
                    if (canPlace)
                    {
                        level->setBlockAndUpdate(current, Blocks::frosted_ice->defaultBlockState());
                        level->addToTickNextTick(current, Blocks::frosted_ice,
                                                 Mth::nextInt(entity->getRandom(), 60, 120));
                        placed = true;
                        if (level->getBiome(current) != Biome::DEEP_OCEAN
                            && level->getBiome(current) != Biome::DEEP_COLD_OCEAN
                            && level->getBiome(current) != Biome::DEEP_FROZEN_OCEAN
                            && level->getBiome(current) != Biome::DEEP_LUKEWARM_OCEAN)
                        {
                            placed = false;
                        }
                    }
                }
            }
        }
    }

    delete positions;

    if (player && level->mIsClientSide && placed)
    {
        player->awardStat(GenericStats::letItGo(), GenericStats::param_letItGo());
    }
}

bool FrostWalkerEnchantment::checkCompatibility(const Enchantment* other) const
{
    return Enchantment::checkCompatibility(other) && other != Enchantments::DEPTH_STRIDER;
}

FrostWalkerEnchantment::~FrostWalkerEnchantment()
{
}
