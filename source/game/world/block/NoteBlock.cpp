#include "game/world/block/NoteBlock.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <cmath>
#include <vector>

#include "game/client/audio/MusicBlockEntity.hpp"
#include "game/client/audio/SoundEvent.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

static std::vector<const SoundEvent*> noteBlockInstruments;

NoteBlock::NoteBlock() : BaseEntityBlock(Material::WOOD)
{
    DerivedInit();
}

boost::shared_ptr<BlockEntity> NoteBlock::newBlockEntity(Level* level, int)
{
    boost::shared_ptr<BlockEntity> entity = boost::shared_ptr<MusicBlockEntity>(new MusicBlockEntity());
    entity->setLevel(level);
    return entity;
}

void NoteBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    bool powered = level->hasNeighborSignal(pos);
    boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
    if (eTYPE_DERIVED_FROM(BlockEntityType::MUSIC, entity->GetType()))
    {
        boost::shared_ptr<MusicBlockEntity> music = boost::dynamic_pointer_cast<MusicBlockEntity>(entity);
        if (music->mPowered != powered)
        {
            if (powered)
            {
                music->playNote(level, BlockPos(pos));
            }
            music->mPowered = powered;
        }
    }
}

bool NoteBlock::TestUse()
{
    return true;
}

bool NoteBlock::use(Level* level, const BlockPos& pos, const BlockState*, boost::shared_ptr<Player>,
                    InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (clientSide)
    {
        return false;
    }
    if (!level->mIsClientSide)
    {
        boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
        if (eTYPE_DERIVED_FROM(BlockEntityType::MUSIC, entity->GetType()))
        {
            boost::shared_ptr<MusicBlockEntity> music = boost::dynamic_pointer_cast<MusicBlockEntity>(entity);
            music->tune();
            music->playNote(level, BlockPos(pos));
        }
    }
    return true;
}

void NoteBlock::attack(Level* level, const BlockPos& pos, boost::shared_ptr<Player>)
{
    if (!level->mIsClientSide)
    {
        boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
        if (eTYPE_DERIVED_FROM(BlockEntityType::MUSIC, entity->GetType()))
        {
            boost::shared_ptr<MusicBlockEntity> music = boost::dynamic_pointer_cast<MusicBlockEntity>(entity);
            music->playNote(level, BlockPos(pos));
        }
    }
}

unsigned int NoteBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseBlock || tooltipData.m_bUseItemOnBlock) && tooltipData.m_bCanPlace)
    {
        return IDS_TOOLTIPS_CHANGEPITCH;
    }
    return Block::GetInteractTooltip(tooltipData);
}

unsigned int NoteBlock::GetHitTooltip(const BlockTooltipDataHolder& tooltipData)
{
    MultiplayerLocalPlayer* player = tooltipData.m_pPlayer.operator->();
    if (tooltipData.m_bCanDestroy)
    {
        if (player->mAbilities.mInstabuild)
        {
            return IDS_TOOLTIPS_MINE;
        }
        return IDS_TOOLTIPS_PLAY;
    }
    if (Minecraft::InMiniGame(eMiniGameId_Lobby, true))
    {
        return IDS_TOOLTIPS_PLAY;
    }
    return IDS_NONE;
}

const SoundEvent* NoteBlock::getInstrument(int instrument)
{
    if (instrument < 0 || (unsigned int)instrument >= noteBlockInstruments.size())
    {
        instrument = 0;
    }
    return noteBlockInstruments.at(instrument);
}

bool NoteBlock::triggerEvent(const BlockState*, Level* level, const BlockPos& pos, int instrument, int eventParam)
{
    float pitch = (float)std::pow(2.0, (double)(eventParam - 12) / 12.0);
    level->playSound(boost::shared_ptr<Player>(), pos, getInstrument(instrument), SoundSource::eSoundSource_Records,
                     3.0f, pitch, 64.0f);
    level->addParticle(PARTICLE_NOTE, (double)pos.x + 0.5, (double)pos.y + 1.2, (double)pos.z + 0.5,
                       (double)eventParam / 24.0, 0.0, 0.0, arrayWithLength<int>());
    return true;
}

EBlockRenderShape NoteBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_MODEL;
}

void NoteBlock::blockStaticCtor()
{
    const SoundEvent* instruments[] = {SoundEvents::NOTE_HARP,     SoundEvents::NOTE_BASEDRUM, SoundEvents::NOTE_SNARE,
                                       SoundEvents::NOTE_HAT,      SoundEvents::NOTE_BASS,     SoundEvents::NOTE_FLUTE,
                                       SoundEvents::NOTE_BELL,     SoundEvents::NOTE_GUITAR,   SoundEvents::NOTE_CHIME,
                                       SoundEvents::NOTE_XYLOPHONE};
    std::vector<const SoundEvent*> instrumentList(instruments,
                                                  instruments + sizeof(instruments) / sizeof(instruments[0]));
    noteBlockInstruments = instrumentList;
}
