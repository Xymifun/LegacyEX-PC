#include "game/world/block/StoneButtonBlock.hpp"

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Material.hpp"

StoneButtonBlock::StoneButtonBlock() : ButtonBlock(false, Material::STONE)
{
    DerivedInit();
}

void StoneButtonBlock::playOnSound(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos)
{
    level->playSound(player, pos, SoundEvents::STONE_BUTTON_CLICK_ON, SoundSource::eSoundSource_Blocks, 0.3f, 0.6f,
                     16.0f);
}

void StoneButtonBlock::playOffSound(Level* level, const BlockPos& pos)
{
    level->playSound(boost::shared_ptr<Player>(), pos, SoundEvents::STONE_BUTTON_CLICK_OFF,
                     SoundSource::eSoundSource_Blocks, 0.3f, 0.5f, 16.0f);
}
