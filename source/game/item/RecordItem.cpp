#include "game/item/RecordItem.hpp"
#include "game/world/LevelEvent.hpp"

#include <boost/unordered_map.hpp>

#include "game/client/audio/SoundEvent.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Rarity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/JukeboxBlock.hpp"
#include "game/world/generation/Player.hpp"

static boost::unordered_map<const SoundEvent*, RecordItem*> sBySound;

RecordItem::RecordItem(const std::wstring& name, int recordId, const SoundEvent* sound)
    : Item(),
      mSound(sound),
      mRecordName(name),
      mRecordId(recordId)
{
    mMaxStackSize = 1;
    sBySound[sound] = this;
}

InteractionResult::EInteractionResult RecordItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                        const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                        const Direction*, float, float, float, bool simulate)
{
    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() != Blocks::jukebox || state->getValue(JukeboxBlock::HAS_RECORD))
    {
        return InteractionResult::eInteractionResult_Pass;
    }

    if (!simulate && !level->mIsClientSide)
    {
        not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
        static_cast<JukeboxBlock*>(Blocks::jukebox)->setRecord(level, pos, state, itemInstance);
        level->levelEvent(boost::shared_ptr<Player>(), LevelEvent::SOUND_PLAY_RECORDING, pos, Item::getId(this));
        player->awardStat(GenericStats::musicToMyEars(),
                          GenericStats::param_musicToMyEars(Item::VALIDgetId(itemInstance->getItem())));
        itemInstance->shrink(1);
    }

    return InteractionResult::eInteractionResult_Success;
}

void RecordItem::appendHoverText(not_null_ptr<ItemInstance> item, boost::shared_ptr<Player>,
                                 std::vector<HtmlString>* lines, bool)
{
    eMinecraftColour colour = getRarity(not_null_ptr<ItemInstance>())->colour;
    lines->push_back(HtmlString(L10N::GetString(mRecordId), colour, false, false, 0));
}

std::wstring RecordItem::getDisplayName()
{
    return L10N::GetString(mRecordId);
}

const Rarity* RecordItem::getRarity(not_null_ptr<ItemInstance>)
{
    return Rarity::RARE;
}

RecordItem* RecordItem::getBySound(const SoundEvent* sound)
{
    boost::unordered_map<const SoundEvent*, RecordItem*>::const_iterator it = sBySound.find(sound);
    if (it != sBySound.end())
        return it->second;
    return 0;
}

const SoundEvent* RecordItem::getSound()
{
    return mSound;
}

TextureAtlasSprite* RecordItem::getIcon(int)
{
    return mIcon;
}

void RecordItem::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"record_" + mRecordName);
}
