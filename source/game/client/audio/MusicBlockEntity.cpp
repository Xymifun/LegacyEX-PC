#include "game/client/audio/MusicBlockEntity.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Mth.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Material.hpp"

MusicBlockEntity::MusicBlockEntity() : BlockEntity(), mNote(0), mPowered(false)
{
}

BlockEntity* MusicBlockEntity::create()
{
    return new MusicBlockEntity();
}

void MusicBlockEntity::playNote(Level* level, BlockPos pos)
{
    if (level->getBlockState(pos.above())->getBlock()->mMaterial != Material::AIR)
    {
        return;
    }

    const BlockState* state = level->getBlockState(pos.below());
    Material* material = state->getMaterial();
    int instrument = 0;

    if (material == Material::STONE)
    {
        instrument = 1;
    }
    else if (material == Material::SAND)
    {
        instrument = 2;
    }
    else if (material == Material::GLASS)
    {
        instrument = 3;
    }
    else if (material == Material::WOOD)
    {
        instrument = 4;
    }

    Block* block = state->getBlock();

    if (block == Blocks::clay)
    {
        instrument = 5;
    }
    else if (block == Blocks::gold_block)
    {
        instrument = 6;
    }
    else if (block == Blocks::wool)
    {
        instrument = 7;
    }
    else if (block == Blocks::packed_ice)
    {
        instrument = 8;
    }
    else if (block == Blocks::bone_block)
    {
        instrument = 9;
    }

    level->blockEvent(pos, Blocks::noteblock, instrument, mNote);
}

void MusicBlockEntity::tune()
{
    mNote = (unsigned char)((mNote + 1) % 25);
    setChanged();
}

CompoundTag* MusicBlockEntity::save(CompoundTag* tag)
{
    BlockEntity::save(tag);
    tag->putByte(L"note", mNote);
    tag->putBoolean(L"powered", mPowered);
    return tag;
}

void MusicBlockEntity::load(CompoundTag* tag)
{
    BlockEntity::load(tag);
    mNote = tag->getByte(L"note");
    mNote = (unsigned char)Mth::clamp(mNote, 0, 24);
    mPowered = tag->getBoolean(L"powered");
}

boost::shared_ptr<BlockEntity> MusicBlockEntity::clone()
{
    boost::shared_ptr<MusicBlockEntity> result(new MusicBlockEntity());
    BlockEntity::clone(result);
    result->mNote = mNote;
    return result;
}

unsigned long long MusicBlockEntity::GetType()
{
    return BlockEntityType::MUSIC;
}
