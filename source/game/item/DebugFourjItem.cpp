#include "game/item/DebugFourjItem.hpp"

#include "game/core/BuiltInLootTables.hpp"
#include "game/core/EntityType.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/Level.hpp"

DebugFourjItem::DebugFourjItem()
{
    mLootTables[0] = BuiltInLootTables::ABANDONED_MINESHAFT;
    mLootTables[1] = BuiltInLootTables::BURIEDTREASURE;
    mLootTables[2] = BuiltInLootTables::DESERT_PYRAMID;
    mLootTables[3] = BuiltInLootTables::DISPENSER_TRAP;
    mLootTables[4] = BuiltInLootTables::END_CITY_TREASURE;
    mLootTables[5] = BuiltInLootTables::IGLOO_CHEST;
    mLootTables[6] = BuiltInLootTables::JUNGLE_TEMPLE;
    mLootTables[7] = BuiltInLootTables::JUNGLE_TEMPLE_DISPENSER;
    mLootTables[8] = BuiltInLootTables::NETHER_BRIDGE;
    mLootTables[9] = BuiltInLootTables::SHIPWRECK;
    mLootTables[10] = BuiltInLootTables::SHIPWRECKSUPPLY;
    mLootTables[11] = BuiltInLootTables::SHIPWRECKTREASURE;
    mLootTables[12] = BuiltInLootTables::SIMPLE_DUNGEON;
    mLootTables[13] = BuiltInLootTables::SPAWN_BONUS_CHEST;
    mLootTables[14] = BuiltInLootTables::STRONGHOLD_CORRIDOR;
    mLootTables[15] = BuiltInLootTables::STRONGHOLD_CROSSING;
    mLootTables[16] = BuiltInLootTables::STRONGHOLD_LIBRARY;
    mLootTables[17] = BuiltInLootTables::UNDERWATER_RUIN_BIG;
    mLootTables[18] = BuiltInLootTables::UNDERWATER_RUIN_SMALL;
    mLootTables[19] = BuiltInLootTables::VILLAGE_BLACKSMITH;
    mLootTables[20] = BuiltInLootTables::VILLAGE_TWO_ROOM_HOUSE;
    mLootTables[21] = BuiltInLootTables::WOODLAND_MANSION;
}

bool DebugFourjItem::interactEnemy(not_null_ptr<ItemInstance>, boost::shared_ptr<Player> player,
                                   boost::shared_ptr<LivingEntity> target, InteractionHand::EInteractionHand)
{
    if (!player->mLevel->mIsClientSide)
    {
        if (eTYPE_DERIVED_FROM(EntityType::MOB, target->GetType()))
        {
            boost::shared_ptr<Mob> mob = boost::dynamic_pointer_cast<Mob>(target);
            bool persistent = mob->isPersistenceRequired();
            mob->setPersistenceRequired(!persistent);
            return true;
        }
    }

    return false;
}

InteractionResult::EInteractionResult DebugFourjItem::useOn(boost::shared_ptr<Player>, Level*, const BlockPos&,
                                                            InteractionHand::EInteractionHand, const Direction*, float,
                                                            float, float, bool)
{
    return InteractionResult::eInteractionResult_Pass;
}
