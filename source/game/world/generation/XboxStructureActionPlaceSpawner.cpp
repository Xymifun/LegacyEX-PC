#include "game/world/generation/XboxStructureActionPlaceSpawner.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/BoundingBox.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MobSpawnerBlockEntity.hpp"
#include "game/world/generation/StructurePiece.hpp"

XboxStructureActionPlaceSpawner::XboxStructureActionPlaceSpawner()
{
    mBlock = Blocks::mob_spawner;
    mEntityId.assign(L"Pig");
    addStringAttribute(ConsoleGameRules::eGameRuleAttr_entity, &mEntityId);
}

XboxStructureActionPlaceSpawner::~XboxStructureActionPlaceSpawner()
{
}

uint32_t XboxStructureActionPlaceSpawner::placeSpawnerInLevel(StructurePiece* structurePiece, Level* level,
                                                              BoundingBox* box)
{
    BlockPos pos(structurePiece->getWorldX(mEndX, mEndZ), structurePiece->getWorldY(mEndY),
                 structurePiece->getWorldZ(mEndX, mEndZ));
    if (box->isInside(pos))
    {
        if (level->getBlockEntity(pos))
        {
            level->removeBlockEntity(pos);
            level->setBlock(pos, Blocks::air->defaultBlockState(), Block::UPDATE_ALL, false);
        }

        level->setBlock(pos, mBlock->defaultBlockState(), Block::UPDATE_ALL, false);
        boost::shared_ptr<MobSpawnerBlockEntity> spawner = boost::dynamic_pointer_cast<MobSpawnerBlockEntity>(
            level->getBlockEntity(pos));
        if (spawner)
        {
            spawner->setEntityId(mEntityId);
        }

        return 1;
    }

    return 0;
}

ConsoleGameRules::EGameRuleType XboxStructureActionPlaceSpawner::getActionType()
{
    return ConsoleGameRules::eGameRuleType_PlaceSpawner;
}
