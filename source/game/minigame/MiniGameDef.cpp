#include "game/minigame/MiniGameDef.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/EntityType.hpp"

#include <algorithm>
#include <cwchar>
#include <deque>
#include <sstream>
#include "game/core/L10N.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/ConditionConnective.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/File.hpp"
#include "game/core/FileInputStream.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GameType.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/StringFormat.hpp"
#include "game/core/StringUtils.hpp"
#include "game/core/WeighedTreasure.hpp"
#include "game/core/XmlConstruct.hpp"
#include "game/core/XmlChild.hpp"
#include "game/core/XmlObject.hpp"
#include "game/core/XmlValue.hpp"
#include "game/core/XmlParser.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/DyeItem.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/FireworksItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ItemPotionFix.hpp"
#include "game/item/Item.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Items.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/entity/mob/MobEffect.hpp"
#include "game/entity/mob/MobEffectInstance.hpp"
#include "game/network/ClientboundMGPlayerSettingsUpdatePacket.hpp"
#include "game/network/Packet.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/ui/LaunchMoreOptionsMenuInitData.hpp"
#include "game/util/GameModeUtilities.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelRules.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/item/ItemId.hpp"

class Block;

static bool sCrcTableBuilt = false;

static const unsigned int sHostOptionCategoryStringIds[4] = {IDS_MG_OPTION_SPECTATE_MODE_INVISIBLE,
                                                             IDS_MG_OPTION_SPECTATE_MODE_BAT, IDS_PARROT, IDS_VEX};

static MiniGamePlayerSettings sPlayerSettings;
static OSFastMutex sPlayerSettingsMutex;
static MiniGameDef* sMiniGameDefs[eMiniGameId_Count];
static MiniGameDef* sDefaultMiniGameDef;
static unsigned int sCrcTable[256];
static const unsigned int sValidCRCs[eMiniGameId_Count] = {0xe34a6dd7U, 0x0940817cU, 0x62401305U, 0xf44e4784U,
                                                           0xb98849e2U};

static const MiniGameDef::FoodBalance sDefaultFoodBalance;
static const MiniGameDef::ModeVariationLootGroup sEmptyLootGroup;
static const MiniGameDef::LootSetDef sEmptyLootSet;

static const std::wstring sLootSetNameAttribute(L"name");
static const std::wstring sBowAndArrowLootSet(L"BowAndArrow");
static const std::wstring sLevitatePotionsLootSet(L"LevitatePotions");
static const std::wstring sVoidTNTLootSet(L"VoidTNT");
static const std::wstring sSnowballLootSet(L"Snowball");
const std::wstring MiniGameDef::sPlayerInventoryLootGroup(L"PlayerInventory");
static const std::wstring sMaxLayerCountAttribute(L"maxLayerCount");

static Block* sBlackList[50] = {NULL};
static Block* sWhiteList[12] = {NULL};
static Block* sPlacementRestrictedList[18] = {NULL};

static bool sBlackListInitialised;
static bool sWhiteListInitialised;
static bool sPlacementRestrictedListInitialised;

template <class T, T MiniGamePlayerSettings::* Setting>
T GetSetting()
{
    EnterCriticalSection(&sPlayerSettingsMutex);
    T value = sPlayerSettings.*Setting;
    LeaveCriticalSection(&sPlayerSettingsMutex);
    return value;
}

template <class T, T MiniGamePlayerSettings::* Setting>
void SetSetting(T value)
{
    EnterCriticalSection(&sPlayerSettingsMutex);
    sPlayerSettings.*Setting = value;
    LeaveCriticalSection(&sPlayerSettingsMutex);
}

maybe<int> parseDifficulty(std::wstring value)
{
    if (value == L"Peaceful")
        return maybe<int>(0);
    if (value == L"Easy")
        return maybe<int>(1);
    if (value == L"Normal")
        return maybe<int>(2);
    if (value == L"Hard")
        return maybe<int>(3);
    return maybe<int>();
}

maybe<int> parseGameType(std::wstring value)
{
    if (value == L"NotSet")
        return maybe<int>(GameType::NOT_SET->getId());
    if (value == L"Survival")
        return maybe<int>(GameType::SURVIVAL->getId());
    if (value == L"Creative")
        return maybe<int>(GameType::CREATIVE->getId());
    if (value == L"Adventure")
        return maybe<int>(GameType::ADVENTURE->getId());
    if (value == L"Spectator")
        return maybe<int>(GameType::SPECTATOR->getId());
    if (value == L"Lobby")
        return maybe<int>(GameType::LOBBY->getId());
    return maybe<int>();
}

maybe<ESpectateMode> parseSpectateMode(std::wstring value)
{
    if (value == L"$SpectateMode")
        return maybe<ESpectateMode>(eSpectateMode_HostOption);
    if (value == L"Invisible")
        return maybe<ESpectateMode>(eSpectateMode_Invisible);
    if (value == L"Bat")
        return maybe<ESpectateMode>(eSpectateMode_Bat);
    if (value == L"Parrot")
        return maybe<ESpectateMode>(eSpectateMode_Parrot);
    if (value == L"Vex")
        return maybe<ESpectateMode>(eSpectateMode_Vex);
    return maybe<ESpectateMode>();
}

maybe<int> parseBuildMode(std::wstring value)
{
    if (value == L"NoRestrictions")
        return maybe<int>(EBuildMode_NoRestrictions);
    if (value == L"WhiteList")
        return maybe<int>(EBuildMode_WhiteList);
    if (value == L"BlackList")
        return maybe<int>(EBuildMode_BlackList);
    return maybe<int>();
}

void getVersionNumber(const XmlObject& object, unsigned short* major, unsigned short* minor)
{
    const std::wstring version = object[L"version"].XmlConstruct::asString(L"1.0");
    const std::wstring::size_type separator = version.find(L'.');
    *major = _fromString<unsigned short>(version.substr(0, separator));
    *minor = _fromString<unsigned short>(version.substr(separator + 1));
}

Block** getBlackList()
{
    if (!sBlackListInitialised)
    {
        sBlackList[0] = Blocks::beacon;
        sBlackList[1] = Blocks::ladder;
        sBlackList[2] = Blocks::wooden_button;
        sBlackList[3] = Blocks::stone_button;
        sBlackList[4] = Blocks::chest;
        sBlackList[5] = Blocks::trapped_chest;
        sBlackList[6] = Blocks::ender_chest;
        sBlackList[7] = Blocks::hopper;
        sBlackList[8] = Blocks::tripwire_hook;
        sBlackList[9] = Blocks::dispenser;
        sBlackList[10] = Blocks::dropper;
        sBlackList[11] = Blocks::furnace;
        sBlackList[12] = Blocks::enchanting_table;
        sBlackList[13] = Blocks::anvil;
        sBlackList[14] = Blocks::vine;
        sBlackList[15] = Blocks::sponge;
        sBlackList[16] = Blocks::cactus;
        sBlackList[17] = Blocks::sapling;
        sBlackList[18] = Blocks::stained_glass_pane;
        sBlackList[19] = Blocks::glass_pane;
        sBlackList[20] = Blocks::leaves;
        sBlackList[21] = Blocks::leaves2;
        sBlackList[22] = Blocks::redstone_block;
        sBlackList[23] = Blocks::wooden_pressure_plate;
        sBlackList[24] = Blocks::stone_pressure_plate;
        sBlackList[25] = Blocks::heavy_weighted_pressure_plate;
        sBlackList[26] = Blocks::light_weighted_pressure_plate;
        sBlackList[27] = Blocks::iron_trapdoor;
        sBlackList[28] = Blocks::trapdoor;
        sBlackList[29] = Blocks::spruce_trapdoor;
        sBlackList[30] = Blocks::birch_trapdoor;
        sBlackList[31] = Blocks::jungle_trapdoor;
        sBlackList[32] = Blocks::acacia_trapdoor;
        sBlackList[33] = Blocks::dark_oak_trapdoor;
        sBlackList[34] = Blocks::daylight_detector;
        sBlackList[35] = Blocks::jukebox;
        sBlackList[36] = Blocks::noteblock;
        sBlackList[37] = Blocks::daylight_detector_inverted;
        sBlackList[38] = Blocks::mob_spawner;
        sBlackList[39] = Blocks::birch_pressure_plate;
        sBlackList[40] = Blocks::spruce_pressure_plate;
        sBlackList[41] = Blocks::acacia_pressure_plate;
        sBlackList[42] = Blocks::jungle_pressure_plate;
        sBlackList[43] = Blocks::dark_oak_pressure_plate;
        sBlackList[44] = Blocks::acacia_button;
        sBlackList[45] = Blocks::spruce_button;
        sBlackList[46] = Blocks::birch_button;
        sBlackList[47] = Blocks::jungle_button;
        sBlackList[48] = Blocks::dark_oak_button;
        sBlackListInitialised = true;
    }
    return sBlackList;
}

Block** getWhiteList()
{
    if (!sWhiteListInitialised)
    {
        sWhiteList[0] = Blocks::air;
        sWhiteList[1] = Blocks::water;
        sWhiteList[2] = Blocks::lava;
        sWhiteList[3] = Blocks::snow_layer;
        sWhiteList[4] = Blocks::carpet;
        sWhiteList[5] = Blocks::tnt;
        sWhiteList[6] = Blocks::glowstone;
        sWhiteList[7] = Blocks::sea_lantern;
        sWhiteList[8] = Blocks::glass;
        sWhiteList[9] = Blocks::stained_glass;
        sWhiteList[10] = Blocks::ice;
        sWhiteListInitialised = true;
    }
    return sWhiteList;
}

Block** getPlacementRestrictedList()
{
    if (!sPlacementRestrictedListInitialised)
    {
        sPlacementRestrictedList[0] = Blocks::torch;
        sPlacementRestrictedList[1] = Blocks::redstone_torch;
        sPlacementRestrictedList[2] = Blocks::wooden_pressure_plate;
        sPlacementRestrictedList[3] = Blocks::acacia_pressure_plate;
        sPlacementRestrictedList[4] = Blocks::birch_pressure_plate;
        sPlacementRestrictedList[5] = Blocks::dark_oak_pressure_plate;
        sPlacementRestrictedList[6] = Blocks::jungle_pressure_plate;
        sPlacementRestrictedList[7] = Blocks::spruce_pressure_plate;
        sPlacementRestrictedList[8] = Blocks::stone_pressure_plate;
        sPlacementRestrictedList[9] = Blocks::heavy_weighted_pressure_plate;
        sPlacementRestrictedList[10] = Blocks::light_weighted_pressure_plate;
        sPlacementRestrictedList[11] = Blocks::rail;
        sPlacementRestrictedList[12] = Blocks::golden_rail;
        sPlacementRestrictedList[13] = Blocks::detector_rail;
        sPlacementRestrictedList[14] = Blocks::activator_rail;
        sPlacementRestrictedList[15] = Blocks::snow_layer;
        sPlacementRestrictedList[16] = Blocks::carpet;
        sPlacementRestrictedListInitialised = true;
    }
    return sPlacementRestrictedList;
}

MiniGameDef::BlockData::BlockData()
    : mTickOverride(),
      mTickDefault(),
      mTick(),
      mCollisionOverride(),
      mCollisionDefault(),
      mCollision()
{
}

MiniGameDef::ChestRefill::ChestRefill()
    : mCentralRefillChance(),
      mOuterRefillChance(),
      mHVRefillChance(),
      mNumberOfChestsToRefill(),
      mMinPlayerDistanceToEffectChance(),
      mMaxPlayerDistanceToEffectChance(),
      mCentralLootGroup(),
      mOuterLootGroup(),
      mHVLootGroup()
{
}

MiniGameDef::FoodBalance::FoodBalance()
    : mMaxFood(),
      mMaxSaturation(),
      mStartSaturation(),
      mSaturationFloor(),
      mExhaustionDrop(),
      mHealthTickCount(),
      mHealthTickCountSaturated(),
      mHealLevel(),
      mStarveLevel(),
      mSprintLevel(),
      mExhaustionHeal(),
      mExhaustionJump(),
      mExhaustionSprintJump(),
      mExhaustionMine(),
      mExhaustionAttack(),
      mExhaustionDamage(),
      mExhaustionSneak(),
      mExhaustionWalk(),
      mExhaustionSprint(),
      mExhaustionSwim(),
      mExhaustionIdle()
{
}

MiniGameDef::FoodBalance::FoodBalance(const FoodBalance& other)
    : mMaxFood(other.mMaxFood),
      mMaxSaturation(other.mMaxSaturation),
      mStartSaturation(other.mStartSaturation),
      mSaturationFloor(other.mSaturationFloor),
      mExhaustionDrop(other.mExhaustionDrop),
      mHealthTickCount(other.mHealthTickCount),
      mHealthTickCountSaturated(other.mHealthTickCountSaturated),
      mHealLevel(other.mHealLevel),
      mStarveLevel(other.mStarveLevel),
      mSprintLevel(other.mSprintLevel),
      mExhaustionHeal(other.mExhaustionHeal),
      mExhaustionJump(other.mExhaustionJump),
      mExhaustionSprintJump(other.mExhaustionSprintJump),
      mExhaustionMine(other.mExhaustionMine),
      mExhaustionAttack(other.mExhaustionAttack),
      mExhaustionDamage(other.mExhaustionDamage),
      mExhaustionSneak(other.mExhaustionSneak),
      mExhaustionWalk(other.mExhaustionWalk),
      mExhaustionSprint(other.mExhaustionSprint),
      mExhaustionSwim(other.mExhaustionSwim),
      mExhaustionIdle(other.mExhaustionIdle)
{
}

MiniGameDef::SoundSpec::SoundSpec() : mForLast(), mVolume()
{
}

MiniGameDef::SoundSpec::SoundSpec(const XmlObject& object) : mForLast(), mVolume()
{
    mForLast = object[L"forLast"].maybeInt();
    mVolume = object[L"volume"].maybeFloat();
}

unsigned int MiniGameDef::SoundSpec::ForLast() const
{
    return mForLast.isSet() ? (unsigned int)mForLast.get() : 0x7fffffffU;
}
float MiniGameDef::SoundSpec::Volume() const
{
    return mVolume.isSet() ? mVolume.get() : 1.0f;
}

template <class T>
T MiniGameDef::atMaybe(const boost::unordered_map<int, T>& values, int key, T defaultValue)
{
    typename boost::unordered_map<int, T>::const_iterator it = values.find(key);
    return it == values.end() ? defaultValue : it->second;
}

MiniGameDef::TimerSpec::TimerSpec()
    : mDuration(),
      mAnnounceEvery(),
      mChangeAnnounceAfter(),
      mChangeAnnounceWhenRemaining(),
      mChangedAnnounce()
{
}

MiniGameDef::TimerSpec::TimerSpec(const XmlObject& object)
    : mDuration(),
      mAnnounceEvery(),
      mChangeAnnounceAfter(),
      mChangeAnnounceWhenRemaining(),
      mChangedAnnounce()
{
    mDuration = object[L"duration"].maybeInt();
    mAnnounceEvery = object[L"announceEvery"].maybeInt();
    mChangeAnnounceAfter = object[L"changeAnnounceAfter"].maybeInt();
    mChangeAnnounceWhenRemaining = object[L"changeAnnounceWhenRemaining"].maybeInt();
    mChangedAnnounce = object[L"changedAnnounce"].maybeInt();
}

void MiniGameDef::TimerSpec::SetDurationS(int value)
{
    mDuration = maybe<int>(value);
}
unsigned int MiniGameDef::TimerSpec::TotalDurationS() const
{
    return mDuration.isSet() ? (unsigned int)mDuration.get() : 0xffffffffU;
}
int MiniGameDef::TimerSpec::TotalDurationMs() const
{
    int seconds = (int)TotalDurationS();
    return seconds < 0 ? -1 : seconds * 1000;
}
unsigned int MiniGameDef::TimerSpec::AnnounceEveryS(bool isFinal) const
{
    if (isFinal)
    {
        if (mChangedAnnounce.isSet())
            return mChangedAnnounce.get();
    }
    else if (mAnnounceEvery.isSet())
    {
        return mAnnounceEvery.get();
    }
    return 0xffffffffU;
}
int MiniGameDef::TimerSpec::AnnounceEveryMs(bool isFinal) const
{
    int seconds = (int)AnnounceEveryS(isFinal);
    return seconds < 0 ? -1 : seconds * 1000;
}
int MiniGameDef::TimerSpec::ChangeAnnounceAfterS() const
{
    if (mChangeAnnounceAfter.isSet())
        return mChangeAnnounceAfter.get();
    if (mChangeAnnounceWhenRemaining.isSet())
        return (int)TotalDurationS() - mChangeAnnounceWhenRemaining.get();
    return -1;
}
int MiniGameDef::TimerSpec::ChangeAnnounceAfterMs() const
{
    int seconds = ChangeAnnounceAfterS();
    return seconds < 0 ? -1 : seconds * 1000;
}

MiniGameDef::LootSetDef::LootSetDef() : mAttributes(), mGroups()
{
}
const MiniGameDef::ModeVariationLootGroup* MiniGameDef::LootSetDef::getLootGroup(const std::wstring& name) const
{
    boost::unordered_map<std::wstring, ModeVariationLootGroup>::const_iterator it = mGroups.find(name);
    if (it != mGroups.end())
        return &it->second;
    return &sEmptyLootGroup;
}

std::wstring MiniGameDef::LootSetDef::getAttribute(const std::wstring& group, std::wstring defaultValue) const
{
    boost::unordered_map<std::wstring, std::wstring>::const_iterator it = mAttributes.find(group);
    return it == mAttributes.end() ? defaultValue : it->second;
}

MiniGameDef::ModeVariationLootGroup::ModeVariationLootGroup()
    : std::vector<ChestItemsInfo>(),
      mKits(),
      mPlayerEquipment(),
      mChestKpScore(0),
      mDistributionMethod(eLootDistributionMethod_Spread)
{
}

MiniGameDef::ModeVariationLootGroup::ModeVariationLootGroup(const ModeVariationLootGroup& other)
    : std::vector<ChestItemsInfo>(other),
      mKits(other.mKits),
      mPlayerEquipment(other.mPlayerEquipment),
      mChestKpScore(other.mChestKpScore),
      mDistributionMethod(other.mDistributionMethod)
{
}

std::vector<not_null_ptr<ItemInstance> >* MiniGameDef::ModeVariationLootGroup::GetChestItems() const
{
    unsigned int playerCount = gMinecraftServer->mMasterGameMode->GetPlayerCount(true, false);
    std::vector<not_null_ptr<ItemInstance> >* items = new std::vector<not_null_ptr<ItemInstance> >();
    Random random;
    for (const_iterator it = begin(); it != end(); ++it)
    {
        unsigned int copies = it->mMinCount;
        unsigned int playersForMin = it->mPlayersForMin;
        if (playersForMin < playerCount)
        {
            float scale = (float)(playerCount - playersForMin) / (float)(8 - playersForMin);
            copies += (unsigned int)((float)(unsigned int)(it->mMaxCount - it->mMinCount) * scale);
        }
        for (unsigned int i = 0; i < copies; ++i)
        {
            not_null_ptr<ItemInstance> item = it->mItem->copy();
            int count = it->mMinStackCount;
            if (it->mMaxStackCount - it->mMinStackCount > 0)
                count += random.nextInt(it->mMaxStackCount - it->mMinStackCount);
            item->setCount(std::min(count, item->getMaxStackSize()));
            items->push_back(item);
        }
    }
    return items;
}

const std::vector<MiniGameDef::ChestItemsInfo>& MiniGameDef::ModeVariationLootGroup::GetAllChestItems() const
{
    return *this;
}
const std::vector<MiniGameDef::ChestKitInfo>& MiniGameDef::ModeVariationLootGroup::GetAllKits() const
{
    return mKits;
}
unsigned int MiniGameDef::ModeVariationLootGroup::GetChestKPScore() const
{
    return mChestKpScore;
}

arrayWithLength<WeighedTreasure*> MiniGameDef::ModeVariationLootGroup::GetWeightedChestItems() const
{
    arrayWithLength<WeighedTreasure*> treasures(size(), true);
    int index = 0;
    for (const_iterator it = begin(); it != end(); ++it)
    {
        treasures.data[index] = new WeighedTreasure(it->mItem->copy(), it->mMinCount, it->mMaxCount, it->mWeight);
        ++index;
    }
    return treasures;
}

const MiniGameDef::PlayerEquipmentInfo& MiniGameDef::ModeVariationLootGroup::GetPlayerEquipment() const
{
    return mPlayerEquipment;
}

MiniGameDef::MiniGameDef(const XmlObject& object)
    : mHostOptionVersion(0),
      mOptionDefaults(),
      mOptionLists(),
      mMatchmakingFilters(),
      mMatchmakingFilterDefaults(),
      mPlayerSettings(),
      mTimers(),
      mSounds(),
      mId(eMiniGameId_Lobby),
      mStartConditions(0),
      mEndConditions(0),
      mPostGameConditions(0),
      mShowdownConditions(0),
      mVersionMajor(0),
      mVersionMinor(0),
      mCrc(0),
      mCanAutoStart(true),
      mLoadOnlyActiveAreaChunks(false),
      mEditActionsDisabled(false),
      mDifficulty(),
      mPlayerGameType(),
      mSpectateMode(0),
      mSpectateEntity(0),
      mAllowCrafting(false),
      mShowInvulnerableEffect(false),
      mLockedInventory(false),
      mShowdownAnnouncements(false),
      mAllowPvP(false),
      mTutorialsEnabled(false),
      mDamageIndicatorDelay(0.0f),
      mCanLavaConvertWater(),
      mPlaceMode(EBuildMode_NoRestrictions),
      mDestroyMode(EBuildMode_NoRestrictions),
      mUseMode(EBuildMode_NoRestrictions),
      mUseBlockMode(EBuildMode_NoRestrictions),
      mPlacePermissions(),
      mDestroyPermissions(),
      mUsePermissions(),
      mBlockUsePermissions(),
      mAllowsUGC(false),
      mMaxPlayersForSmallMaps(0),
      mFoodBalances(),
      mChestRefill(),
      mBlockData(),
      mItemBalance(),
      mLootSets()
{
    getVersionNumber(object, &mVersionMajor, &mVersionMinor);
    mId = (EMiniGameId)object[L"id"].asUInt(0);

    mDifficulty = object.child(L"DifficultyOverride")->maybeString().fmap(parseDifficulty);
    mPlayerGameType = object.child(L"PlayerGameType")->maybeString().fmap(parseGameType);

    Abilities abilities;
    if (mPlayerGameType.isSet())
        GameType::byId(mPlayerGameType.get())->updatePlayerAbilities(&abilities);
    mPlayerSettings.mPlayerAbilities = abilities;

    const XmlObject* playerAbilities = object.child(L"PlayerAbilities");
    if (!equalsIgnoreCase(playerAbilities->mName, L""))
    {
        abilities.mInvulnerable = playerAbilities->child(L"Invulnerable")->asBool(abilities.mInvulnerable);
        abilities.mMayfly = playerAbilities->child(L"MayFly")->asBool(abilities.mMayfly);
        abilities.mInstabuild = playerAbilities->child(L"Instabuild")->asBool(abilities.mInstabuild);
        abilities.mMayBuild = playerAbilities->child(L"MayBuild")->asBool(abilities.mMayBuild);
        mPlayerSettings.mPlayerAbilities = abilities;
    }

    mPlayerSettings.mLivesPerRound = object.child(L"LivesPerRound")->asUInt(!HasFeatureSupportForInfiniteLives());
    mPlayerSettings.mTeamCount = object.child(L"TeamCount")->asUInt(0);
    mPlayerSettings.mTeamBasedSpawn = object.child(L"TeamBasedSpawn")->asBool(false);
    mPlayerSettings.mTeamMaxSize = object.child(L"TeamMaxSize")->asUInt(1);
    mPlayerSettings.mCheckpointBasedSpawn = object.child(L"CheckpointBasedSpawn")->asBool(false);
    mMaxPlayersForSmallMaps = object.child(L"MaxPlayersForSmallMaps")->asUInt(6);

    const std::vector<XmlChild>& timers = object.child(L"Timers")->getChildrenRaw();
    for (std::vector<XmlChild>::const_iterator timer = timers.begin(); timer != timers.end(); ++timer)
    {
        if (timer->m_type == 1)
            mTimers[parseTimer(timer->xml()->mName).get()] = TimerSpec(*timer->xml());
    }

    const std::vector<XmlChild>& sounds = object.child(L"Sounds")->getChildrenRaw();
    for (std::vector<XmlChild>::const_iterator sound = sounds.begin(); sound != sounds.end(); ++sound)
    {
        if (sound->m_type == 1)
            mSounds[parseSound(sound->xml()->mName).get()] = SoundSpec(*sound->xml());
    }

    mSpectateMode =
        object.child(L"SpectateMode")->maybeString().fmap(parseSpectateMode).otherwise(eSpectateMode_Invisible);

    boost::unordered_map<std::wstring, unsigned long long> spectateEntities;
    spectateEntities[L""] = 0;
    spectateEntities[L"Bat"] = EntityType::BAT;
    mSpectateEntity = spectateEntities[object.child(L"SpectateEntity")->asString(L"")];

    mShowInvulnerableEffect = object.child(L"ShowInvulnerableEffect")->asBool(false);
    mLockedInventory = object.child(L"LockedInventory")->asBool(false);
    mShowdownAnnouncements = object.child(L"ShowdownAnnouncements")->asBool(false);
    mAllowPvP = object.child(L"AllowPvP")->asBool(true);
    mAllowCrafting = object.child(L"AllowCrafting")->asBool(false);
    mTutorialsEnabled = object.child(L"EnableTutorials")->asBool(true);
    mPlayerSettings.mShowNametags = object.child(L"ShowNametags")->asBool(false);
    mDamageIndicatorDelay = object.child(L"IndicatorDelay")->asFloat(1.0f);
    mLoadOnlyActiveAreaChunks = object.child(L"LoadOnlyActiveAreaChunks")->asBool(false);
    mCanLavaConvertWater = object.child(L"CanLavaConvertWater")->maybeBool();
    mPlayerSettings.mCanTntDestroyBlocks = object.child(L"CanTntDestroyBlocks")->maybeBool();
    mPlayerSettings.mNaturalRegeneration = object.child(L"NaturalRegeneration")->asBool(true);
    mPlayerSettings.mRoundCount = object.child(L"RoundCount")->asInt(1);

    const XmlObject* foodBalance = object.child(L"FoodBalance");
    if (!equalsIgnoreCase(foodBalance->mName, L""))
        parseFoodBalance(*foodBalance, NULL);

    parsePermissions(*object.child(L"DestroyPermissions"));
    parsePermissions(*object.child(L"PlacePermissions"));
    parsePermissions(*object.child(L"UsePermissions"));
    parsePermissions(*object.child(L"BlockUsePermissions"));

    mStartConditions = ConditionConnective::parseCondition(*object.child(L"StartConditions")->child(L""));
    mEndConditions = ConditionConnective::parseCondition(*object.child(L"EndConditions")->child(L""));
    mPostGameConditions = ConditionConnective::parseCondition(*object.child(L"PostGameConditions")->child(L""));
    mShowdownConditions = ConditionConnective::parseCondition(*object.child(L"ShowdownConditions")->child(L""));

    parseChestRefill(*object.child(L"ChestRefill"));

    mLootSets = std::vector<LootSetDef>();
    const std::vector<XmlChild>& lootSets = object.child(L"LootSets")->getChildrenRaw();
    for (std::vector<XmlChild>::const_iterator lootSet = lootSets.begin(); lootSet != lootSets.end(); ++lootSet)
    {
        mLootSets.push_back(LootSetDef());
        LootSetDef& definition = mLootSets.back();

        std::vector<std::pair<std::wstring, std::wstring> >& attributes = lootSet->xml()->getAttrs();
        for (std::vector<std::pair<std::wstring, std::wstring> >::const_iterator attribute = attributes.begin();
             attribute != attributes.end(); ++attribute)
        {
            definition.mAttributes.insert(*attribute);
        }

        const std::vector<XmlChild>& groups = lootSet->xml()->getChildrenRaw();
        for (std::vector<XmlChild>::const_iterator group = groups.begin(); group != groups.end(); ++group)
        {
            ModeVariationLootGroup& lootGroup = definition.mGroups[(*group->xml())[L"name"].asString()];
            lootGroup.mChestKpScore = (*group->xml())[L"kpScore"].asInt();
            lootGroup.mDistributionMethod =
                (*group->xml())[L"distributionMethod"].maybeString().fmap(parseDistributionMethod).get();

            const std::vector<XmlChild>& entries = group->xml()->getChildrenRaw();
            for (std::vector<XmlChild>::const_iterator entry = entries.begin(); entry != entries.end(); ++entry)
            {
                if (equalsIgnoreCase(entry->xml()->mName, L"Kit"))
                    parseKit(&lootGroup.mKits, *entry->xml());
                else if (equalsIgnoreCase(entry->xml()->mName, L"Equipment"))
                    parseEquipment(lootGroup.mPlayerEquipment, *entry->xml());
                else
                    parseChestItem(&lootGroup, *entry->xml());
            }
        }
    }

    const XmlObject* blockTicking = object.child(L"BlockTicking");
    mBlockData.mTickOverride = (*blockTicking)[L"override"].maybeBool();
    mBlockData.mTickDefault = (*blockTicking)[L"default"].maybeBool();
    const std::vector<XmlChild>& tickingBlocks = blockTicking->getChildrenRaw();
    for (std::vector<XmlChild>::const_iterator block = tickingBlocks.begin(); block != tickingBlocks.end(); ++block)
        mBlockData.mTick[(*block->xml())[L"id"].asUInt()] = block->asBool();

    const XmlObject* blockCollision = object.child(L"BlockCollision");
    mBlockData.mCollisionOverride = (*blockCollision)[L"override"].maybeBool();
    mBlockData.mCollisionDefault = (*blockCollision)[L"default"].maybeBool();
    const std::vector<XmlChild>& collisionBlocks = blockCollision->getChildrenRaw();
    for (std::vector<XmlChild>::const_iterator block = collisionBlocks.begin(); block != collisionBlocks.end(); ++block)
        mBlockData.mCollision[(*block->xml())[L"id"].asUInt()] = block->asBool();

    const std::vector<XmlChild>& itemBalances = object.child(L"ItemBalance")->getChildrenRaw();
    for (std::vector<XmlChild>::const_iterator item = itemBalances.begin(); item != itemBalances.end(); ++item)
    {
        unsigned int itemId = (*item->xml())[L"id"].asUInt();
        const std::vector<XmlChild>& overrides = item->xml()->getChildrenRaw();
        for (std::vector<XmlChild>::const_iterator entry = overrides.begin(); entry != overrides.end(); ++entry)
        {
            if (entry->m_type == 1)
                parseItemOverrideMap(itemId, *entry->xml());
        }
    }

    maybe<unsigned int> hostOptionVersion = (*object.child(L"HostOptions"))[L"version"].maybeUInt();
    mHostOptionVersion = hostOptionVersion.isSet() ? (unsigned char)hostOptionVersion.get() : 0;
    parseHostOptionSettings(*object.child(L"HostOptions"));
    mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_All, GetOptionsDefault(eCustomGameModeHostOption_All));

    mAllowsUGC = object.child(L"AllowsUGC")->asBool(false);
}

MiniGameDef::MiniGameDef(EMiniGameId id)
    : mHostOptionVersion(0),
      mOptionDefaults(),
      mOptionLists(),
      mMatchmakingFilters(),
      mMatchmakingFilterDefaults(),
      mPlayerSettings(),
      mTimers(),
      mSounds(),
      mId(id),
      mStartConditions(0),
      mEndConditions(0),
      mPostGameConditions(0),
      mShowdownConditions(0),
      mVersionMajor(0),
      mVersionMinor(0),
      mCrc(0),
      mCanAutoStart(false),
      mLoadOnlyActiveAreaChunks(false),
      mEditActionsDisabled(false),
      mDifficulty(),
      mPlayerGameType(),
      mSpectateMode(0),
      mSpectateEntity(EntityType::BAT),
      mAllowCrafting(true),
      mShowInvulnerableEffect(false),
      mLockedInventory(false),
      mShowdownAnnouncements(false),
      mAllowPvP(true),
      mTutorialsEnabled(true),
      mDamageIndicatorDelay(0.0f),
      mCanLavaConvertWater(),
      mPlaceMode(EBuildMode_NoRestrictions),
      mDestroyMode(EBuildMode_NoRestrictions),
      mUseMode(EBuildMode_NoRestrictions),
      mUseBlockMode(EBuildMode_NoRestrictions),
      mPlacePermissions(),
      mDestroyPermissions(),
      mUsePermissions(),
      mBlockUsePermissions(),
      mAllowsUGC(true),
      mMaxPlayersForSmallMaps(0),
      mFoodBalances(),
      mChestRefill(),
      mBlockData(),
      mItemBalance(),
      mLootSets()
{
}

maybe<int> MiniGameDef::parseTimer(std::wstring name)
{
    if (name == L"GameInitialise")
        return maybe<int>(0);
    if (name == L"RoundPreparing")
        return maybe<int>(1);
    if (name == L"RoundStartDelay")
        return maybe<int>(2);
    if (name == L"GracePeriod")
        return maybe<int>(3);
    if (name == L"RoundEndDelay")
        return maybe<int>(4);
    if (name == L"LobbyInterval")
        return maybe<int>(6);
    if (name == L"RoundLimit")
        return maybe<int>(7);
    if (name == L"Showdown")
        return maybe<int>(8);
    if (name == L"PostGame")
        return maybe<int>(9);
    if (name == L"TimeToReachEnd")
        return maybe<int>(11);
    if (name == L"CancelTeam")
        return maybe<int>();
    return maybe<int>();
}

maybe<EGameModeSound> MiniGameDef::parseSound(std::wstring name)
{
    if (name == L"OnShowdown")
        return maybe<EGameModeSound>(EGameModeSound_OnShowdown);
    if (name == L"OnRoundStart")
        return maybe<EGameModeSound>(EGameModeSound_OnRoundStart);
    if (name == L"OnRoundPlay")
        return maybe<EGameModeSound>(EGameModeSound_OnRoundPlay);
    if (name == L"OnRoundEnding")
        return maybe<EGameModeSound>(EGameModeSound_OnRoundEnding);
    if (name == L"OnChestRefill")
        return maybe<EGameModeSound>(EGameModeSound_OnChestRefill);
    if (name == L"OnDeath")
        return maybe<EGameModeSound>(EGameModeSound_OnDeath);
    if (name == L"LobbyTimer")
        return maybe<EGameModeSound>(EGameModeSound_LobbyTimer);
    if (name == L"RoundStartTimer")
        return maybe<EGameModeSound>(EGameModeSound_RoundStartTimer);
    if (name == L"GraceTimer")
        return maybe<EGameModeSound>(EGameModeSound_GraceTimer);
    if (name == L"RoundLimitTimer")
        return maybe<EGameModeSound>(EGameModeSound_RoundLimitTimer);
    if (name == L"OnFalling")
        return maybe<EGameModeSound>(EGameModeSound_OnFalling);
    return maybe<EGameModeSound>();
}

maybe<MiniGameDef::ELootDistributionMethod> MiniGameDef::parseDistributionMethod(std::wstring name)
{
    if (name == L"Spread")
        return maybe<ELootDistributionMethod>(eLootDistributionMethod_Spread);
    if (name == L"KitKPRandom")
        return maybe<ELootDistributionMethod>(eLootDistributionMethod_KitKPRandom);
    if (name == L"KitKPGreedy")
        return maybe<ELootDistributionMethod>(eLootDistributionMethod_KitKPGreedy);
    if (name == L"Fixed")
        return maybe<ELootDistributionMethod>(eLootDistributionMethod_Fixed);
    if (name == L"WeightedRandom")
        return maybe<ELootDistributionMethod>(eLootDistributionMethod_WeightedRandom);
    return maybe<ELootDistributionMethod>();
}

void MiniGameDef::parseFoodBalance(const XmlObject& object, FoodBalance* parent)
{
    FoodBalance* balance = parent == NULL ? new FoodBalance() : new FoodBalance(*parent);

    balance->mMaxFood = object.child(L"MaxFood")->maybeInt().otherwise(balance->mMaxFood);
    balance->mMaxSaturation = object.child(L"MaxSaturation")->maybeFloat().otherwise(balance->mMaxSaturation);
    balance->mStartSaturation = object.child(L"StartSaturation")->maybeFloat().otherwise(balance->mStartSaturation);
    balance->mSaturationFloor = object.child(L"SaturationFloor")->maybeFloat().otherwise(balance->mSaturationFloor);
    balance->mExhaustionDrop = object.child(L"ExhaustionDrop")->maybeFloat().otherwise(balance->mExhaustionDrop);
    balance->mHealthTickCount = object.child(L"HealthTickCount")->maybeInt().otherwise(balance->mHealthTickCount);
    balance->mHealthTickCountSaturated =
        object.child(L"HealthTickCountSaturated")->maybeInt().otherwise(balance->mHealthTickCountSaturated);
    balance->mHealLevel = object.child(L"HealLevel")->maybeInt().otherwise(balance->mHealLevel);
    balance->mStarveLevel = object.child(L"StarveLevel")->maybeInt().otherwise(balance->mStarveLevel);
    balance->mSprintLevel = object.child(L"SprintLevel")->maybeInt().otherwise(balance->mSprintLevel);
    balance->mExhaustionHeal = object.child(L"ExhaustionHeal")->maybeFloat().otherwise(balance->mExhaustionHeal);
    balance->mExhaustionJump = object.child(L"ExhaustionJump")->maybeFloat().otherwise(balance->mExhaustionJump);
    balance->mExhaustionSprintJump =
        object.child(L"ExhaustionSprintJump")->maybeFloat().otherwise(balance->mExhaustionSprintJump);
    balance->mExhaustionMine = object.child(L"ExhaustionMine")->maybeFloat().otherwise(balance->mExhaustionMine);
    balance->mExhaustionAttack = object.child(L"ExhaustionAttack")->maybeFloat().otherwise(balance->mExhaustionAttack);
    balance->mExhaustionDamage = object.child(L"ExhaustionDamage")->maybeFloat().otherwise(balance->mExhaustionDamage);
    balance->mExhaustionSneak = object.child(L"ExhaustionSneak")->maybeFloat().otherwise(balance->mExhaustionSneak);
    balance->mExhaustionWalk = object.child(L"ExhaustionWalk")->maybeFloat().otherwise(balance->mExhaustionWalk);
    balance->mExhaustionSprint = object.child(L"ExhaustionSprint")->maybeFloat().otherwise(balance->mExhaustionSprint);
    balance->mExhaustionSwim = object.child(L"ExhaustionSwim")->maybeFloat().otherwise(balance->mExhaustionSwim);
    balance->mExhaustionIdle = object.child(L"ExhaustionIdle")->maybeFloat().otherwise(balance->mExhaustionIdle);

    mFoodBalances[object[L"id"].asInt()] = balance;

    const std::vector<XmlChild>& variants = object.child(L"Variants")->getChildrenRaw();
    for (std::vector<XmlChild>::const_iterator variant = variants.begin(); variant != variants.end(); ++variant)
    {
        if (variant->m_type == 1 && equalsIgnoreCase(variant->xml()->mName, L"Variant"))
            parseFoodBalance(*variant->xml(), balance);
    }
}

void MiniGameDef::parsePermissions(const XmlObject& object)
{
    if (equalsIgnoreCase(object.mName, L""))
        return;

    EBuildMode* mode;
    std::vector<int>* exceptions;
    if (equalsIgnoreCase(object.mName, L"DestroyPermissions"))
    {
        mode = &mDestroyMode;
        exceptions = &mDestroyPermissions;
    }
    else if (equalsIgnoreCase(object.mName, L"PlacePermissions"))
    {
        mode = &mPlaceMode;
        exceptions = &mPlacePermissions;
    }
    else if (equalsIgnoreCase(object.mName, L"UsePermissions"))
    {
        mode = &mUseMode;
        exceptions = &mUsePermissions;
    }
    else if (equalsIgnoreCase(object.mName, L"BlockUsePermissions"))
    {
        mode = &mUseBlockMode;
        exceptions = &mBlockUsePermissions;
    }
    else
    {
        return;
    }

    EBuildMode buildMode =
        (EBuildMode)object[L"mode"].maybeString().fmap(parseBuildMode).otherwise(EBuildMode_NoRestrictions);
    *mode = buildMode;

    if (buildMode == EBuildMode_NoRestrictions)
        return;

    for (std::vector<XmlChild>::const_iterator child = object.getChildrenRaw().begin();
         child != object.getChildrenRaw().end(); ++child)
    {
        if (child->m_type == 1 && equalsIgnoreCase(child->xml()->mName, L"Exception"))
            exceptions->push_back((*child->xml())[L"id"].asInt());
    }
}

void MiniGameDef::parseChestRefill(const XmlObject& object)
{
    mChestRefill.mCentralRefillChance = object.child(L"CentralRefillChance")->maybeInt();
    mChestRefill.mOuterRefillChance = object.child(L"OuterRefillChance")->maybeInt();
    mChestRefill.mHVRefillChance = object.child(L"HVRefillChance")->maybeInt();
    mChestRefill.mNumberOfChestsToRefill = object.child(L"NumberOfChestsToRefill")->maybeInt();
    mChestRefill.mMinPlayerDistanceToEffectChance = object.child(L"MinPlayerDistanceToEffectChance")->maybeInt();
    mChestRefill.mMaxPlayerDistanceToEffectChance = object.child(L"MaxPlayerDistanceToEffectChance")->maybeInt();
    mChestRefill.mCentralLootGroup = object.child(L"CentralLootGroup")->maybeString();
    mChestRefill.mOuterLootGroup = object.child(L"OuterLootGroup")->maybeString();
    mChestRefill.mHVLootGroup = object.child(L"HVLootGroup")->maybeString();
}

std::deque<not_null_ptr<ItemInstance> > MiniGameDef::parseItemInstance(const XmlObject& object, int* slot)
{
    int id = object[L"id"].asInt();
    int count = object[L"qty"].asInt(1);
    int aux = object[L"aux"].asInt(0);
    if (slot != NULL)
        *slot = object[L"slot"].asInt(-1);

    not_null_ptr<ItemInstance> item;
    if (id == 0)
    {
        item = ItemInstance::EMPTY;
    }
    else
    {
        Item* baseItem = Item::byId(id);
        item = not_null_ptr<ItemInstance>(new ItemInstance(baseItem, count, aux));

        std::vector<MobEffectInstance*> effects;
        std::wstring potionName;
        for (std::vector<XmlChild>::const_iterator child = object.getChildrenRaw().begin();
             child != object.getChildrenRaw().end(); ++child)
        {
            if (child->m_type != 1)
                continue;

            if (equalsIgnoreCase(child->xml()->mName, L"Enchantment"))
            {
                bool overrideEnchantable = (*child->xml())[L"override"].asBool(false);
                if (item->isEnchantable() || overrideEnchantable)
                {
                    int enchantmentId = (*child->xml())[L"id"].asInt();
                    int level = (*child->xml())[L"level"].asInt();
                    Enchantment* enchantment = Enchantment::byId(enchantmentId);
                    if (enchantment != NULL && overrideEnchantable)
                        item->enchant(enchantment, level);
                    else if (enchantment != NULL && enchantment->canEnchant(item))
                        item->enchant(enchantment, std::min(level, enchantment->getMaxLevel()));
                }
            }
            else if (equalsIgnoreCase(child->xml()->mName, L"Dye"))
            {
                ArmorItem* armor = dynamic_cast<ArmorItem*>(baseItem);
                int colour = _fromHEXString<int>((*child->xml())[L"rgb"].maybeString().get());
                if (armor != NULL)
                    armor->setColor(item, colour);
            }
            else if (equalsIgnoreCase(child->xml()->mName, L"FireworkData"))
            {
                ListTag* explosions = new ListTag();
                CompoundTag* explosion = new CompoundTag();
                std::vector<int> colours;
                colours.push_back(DyeItem::DYE_COLORS[(*child->xml())[L"DyeColorAux"].asInt() & 0xf]);
                arrayWithLength<int> colourArray(colours.size(), true);
                for (unsigned int i = 0; i < colourArray.length; ++i)
                    colourArray[i] = colours.at(i);
                explosion->putIntArray(FireworksItem::TAG_E_COLORS, colourArray);
                explosion->putByte(FireworksItem::TAG_E_TYPE, (unsigned char)(*child->xml())[L"Type"].asInt());
                explosions->add(explosion);
                CompoundTag* fireworks = new CompoundTag();
                fireworks->put(FireworksItem::TAG_EXPLOSIONS, explosions);
                CompoundTag* tag = new CompoundTag();
                tag->put(FireworksItem::TAG_FIREWORKS, fireworks);
                item->setTag(tag);
            }
            else if (equalsIgnoreCase(child->xml()->mName, L"CustomMobEffect"))
            {
                int effectId = (*child->xml())[L"Id"].asInt(0);
                MobEffect* effect = MobEffect::byId(effectId);
                int amplifier = (*child->xml())[L"Amplifier"].asInt(1);
                int duration = (*child->xml())[L"Duration"].asInt(200);
                bool ambient = (*child->xml())[L"Ambient"].asBool(false);
                potionName = (*child->xml())[L"Name"].XmlConstruct::asString(L"");
                if (amplifier < 0)
                    amplifier = 0;
                effects.push_back(new MobEffectInstance(effect, duration, amplifier, ambient, true));
            }
        }

        ItemPotionFix::fix(item);
        if (!effects.empty())
        {
            PotionUtils::setPotionName(item, potionName);
            PotionUtils::setCustomEffects(item, &effects);
            for (std::vector<MobEffectInstance*>::iterator it = effects.begin(); it != effects.end(); ++it)
                delete *it;
            effects.clear();
        }
    }

    std::deque<not_null_ptr<ItemInstance> > items;
    if (!item->isStackable() && item->getCount() > 1)
    {
        int total = item->getCount();
        for (int i = 0; i < total; ++i)
        {
            not_null_ptr<ItemInstance> single = item->copy();
            single->setCount(1);
            items.push_back(single);
        }
    }
    else
    {
        items.push_back(item);
    }
    return items;
}

void MiniGameDef::parseKit(std::vector<ChestKitInfo>* kits, const XmlObject& object)
{
    ChestKitInfo kit;
    kit.mWeight = object[L"weight"].asInt();
    kit.mKpScore = object[L"kpScore"].asInt();

    const std::vector<XmlChild>& children = object.getChildrenRaw();
    for (std::vector<XmlChild>::const_iterator child = children.begin(); child != children.end(); ++child)
    {
        if (child->m_type != 1)
            continue;

        std::deque<not_null_ptr<ItemInstance> > items = parseItemInstance(*child->xml(), NULL);
        while (!items.empty())
        {
            kit.mItems.push_back(items.front());
            items.pop_front();
        }
    }
    kits->push_back(kit);
}

void MiniGameDef::parseEquipment(PlayerEquipmentInfo& equipment, const XmlObject& object)
{
    const std::vector<XmlChild>& children = object.getChildrenRaw();
    for (std::vector<XmlChild>::const_iterator child = children.begin(); child != children.end(); ++child)
    {
        if (child->m_type != 1)
            continue;

        int slot = -1;
        std::deque<not_null_ptr<ItemInstance> > items = parseItemInstance(*child->xml(), &slot);
        if ((unsigned int)slot < 6)
        {
            equipment.mSlots[slot] = maybe<not_null_ptr<ItemInstance> >(items.front());
        }
    }
}

void MiniGameDef::parseChestItem(std::vector<ChestItemsInfo>* items, const XmlObject& object)
{
    ChestItemsInfo info;
    info.mMinStackCount = 1;
    info.mMaxStackCount = 1;
    maybe<int> minCount = object[L"minCount"].maybeInt();
    info.mMinCount = minCount.isSet() ? minCount.get() : 1;
    maybe<int> maxCount = object[L"maxCount"].maybeInt();
    info.mMaxCount = maxCount.isSet() ? maxCount.get() : 1;
    maybe<unsigned int> playersForMin = object[L"playersForMin"].maybeUInt();
    info.mPlayersForMin = playersForMin.isSet() ? playersForMin.get() : 0;
    maybe<unsigned int> kpScore = object[L"kpScore"].maybeUInt();
    info.mKpScore = kpScore.isSet() ? kpScore.get() : 1;
    maybe<unsigned int> weight = object[L"weight"].maybeUInt();
    info.mWeight = weight.isSet() ? weight.get() : 1;

    const std::vector<XmlChild>& children = object.getChildrenRaw();
    for (std::vector<XmlChild>::const_iterator child = children.begin(); child != children.end(); ++child)
    {
        if (child->m_type != 1)
            continue;

        std::deque<not_null_ptr<ItemInstance> > parsed = parseItemInstance(*child->xml(), NULL);
        while (!parsed.empty())
        {
            info.mItem = parsed.front();
            items->push_back(info);
            parsed.pop_front();
        }
    }
}

void MiniGameDef::parseItemOverrideMap(int itemId, const XmlObject& object)
{
    if (equalsIgnoreCase(object.mName, L"BaseDamage"))
    {
        mItemBalance.mBaseDamage[itemId] = object.asDouble();
    }
    else if (equalsIgnoreCase(object.mName, L"Protection"))
    {
        mItemBalance.mProtection[itemId] = object.asInt();
    }
    else if (equalsIgnoreCase(object.mName, L"Degradation"))
    {
        mItemBalance.mDegradation[itemId] = object.asDouble();
    }
    else if (equalsIgnoreCase(object.mName, L"Nutrition"))
    {
        mItemBalance.mNutrition[itemId] = object.asInt();
    }
    else if (equalsIgnoreCase(object.mName, L"SaturationFactor"))
    {
        mItemBalance.mSaturationFactor[itemId] = object.asFloat();
    }
    else if (equalsIgnoreCase(object.mName, L"PotionDuration")
             && (itemId == ItemId::POTION || itemId == ItemId::SPLASH_POTION))
    {
        int effectId = object[L"effectId"].asInt();
        mItemBalance.mPotionDuration[effectId] = object.asInt();
    }
    else if (equalsIgnoreCase(object.mName, L"LingeringDuration") && itemId == ItemId::LINGERING_POTION)
    {
        int effectId = object[L"effectId"].asInt();
        mItemBalance.mLingeringDuration[effectId] = object.asInt();
    }
}

void MiniGameDef::parseMatchmakingFilter(const XmlObject& object)
{
    mMatchmakingFilterDefaults.push_back(object[L"default"].asInt());

    boost::unordered_map<int, boost::unordered_map<int, int> > filter;
    const std::vector<XmlChild>& variants = object.getChildrenRaw();
    int variantIndex = 0;
    for (std::vector<XmlChild>::const_iterator variant = variants.begin(); variant != variants.end();
         ++variant, ++variantIndex)
    {
        const std::vector<XmlChild>& options = variant->xml()->getChildrenRaw();
        for (std::vector<XmlChild>::const_iterator option = options.begin(); option != options.end(); ++option)
        {
            const XmlObject* optionObject = option->xml();
            maybe<eCustomGameModeHostOption> parsed = GameModeOptions::parse(optionObject->mName);
            if (parsed.isSet())
                filter[variantIndex][parsed.get()] = (*optionObject)[L"value"].asInt();
        }
    }

    mMatchmakingFilters.push_back(filter);
}

void MiniGameDef::parseHostOptionSettings(const XmlObject& object)
{
    mMatchmakingFilters.clear();
    mMatchmakingFilterDefaults.clear();

    for (int option = eCustomGameModeHostOption_NoSmallSkins; option < eCustomGameModeHostOption_All; ++option)
    {
        mOptionDefaults[option] = 0;
        mOptionLists[option].clear();
    }

    const std::vector<XmlChild>& children = object.getChildrenRaw();
    for (std::vector<XmlChild>::const_iterator child = children.begin(); child != children.end(); ++child)
    {
        const XmlObject* childObject = child->xml();

        if (childObject->mName == L"MatchmakingFilter")
        {
            parseMatchmakingFilter(*childObject);
            continue;
        }

        maybe<eCustomGameModeHostOption> parsed = GameModeOptions::parse(childObject->mName);
        if (!parsed.isSet())
            continue;

        eCustomGameModeHostOption option = parsed.get();
        unsigned int type = GetOptionType(option);
        unsigned int defaultValue = (*childObject)[L"default"].asInt();
        mOptionDefaults[option] = -1;
        std::vector<int>& values = mOptionLists[option];

        if (type == 0)
        {
            values.push_back(0);
            values.push_back(1);
            mOptionDefaults[option] = defaultValue;
        }
        else if (type < 3)
        {
            std::vector<std::wstring> splitValues = stringSplit(childObject->asString(), L' ');
            for (unsigned int index = 0; index < splitValues.size(); ++index)
            {
                int value = _fromString<int>(splitValues[index]);
                values.push_back(value);
                if (value == static_cast<int>(defaultValue))
                    mOptionDefaults[option] = index;
            }
        }
    }
}

unsigned int MiniGameDef::GetOptionType(eCustomGameModeHostOption option)
{
    static const unsigned char types[] = {0, 1, 2, 1};
    if (option != eCustomGameModeHostOption_NoSmallSkins)
    {
        if ((unsigned int)option > (unsigned int)eCustomGameModeHostOption_MaxPlayers)
        {
            switch (option)
            {
            case eCustomGameModeHostOption_MapVariant:
            case eCustomGameModeHostOption_ModeVariant:
            case eCustomGameModeHostOption_GameVariant:
                return 1;
            default:
                break;
            }
        }
        else
        {
            return types[option];
        }
    }
    return 0;
}

unsigned int MiniGameDef::GetOptionsDefault(eCustomGameModeHostOption option) const
{
    if (option == eCustomGameModeHostOption_All)
    {
        GameModeOptions options;
        for (int i = 0; i < eCustomGameModeHostOption_All; ++i)
            options.Set((eCustomGameModeHostOption)i, GetOptionsDefault((eCustomGameModeHostOption)i));
        return options.Get(eCustomGameModeHostOption_All);
    }
    return mOptionDefaults.at(option);
}

MiniGameDef* MiniGameDef::parseMiniGameDef(XmlObject* object)
{
    if (object != NULL)
    {
        const XmlObject* mode = object->child(L"");
        unsigned short major, minor;
        getVersionNumber(*mode, &major, &minor);
        if (major == 7)
            return new MiniGameDef(*mode);
    }
    return new MiniGameDef();
}

void MiniGameDef::MakeCRCTable()
{
    for (unsigned int i = 0; i < 256; ++i)
    {
        unsigned int value = i;
        for (int bit = 0; bit < 8; ++bit)
            value = (value & 1) != 0 ? (value >> 1) ^ 0xedb88320U : value >> 1;
        sCrcTable[i] = value;
    }
    sCrcTableBuilt = true;
}

unsigned int MiniGameDef::UpdateCRC(unsigned int crc, unsigned char* data, int length)
{
    if (!sCrcTableBuilt)
        MakeCRCTable();
    for (int i = 0; i < length; ++i)
        crc = sCrcTable[(crc ^ data[i]) & 0xff] ^ (crc >> 8);
    return crc;
}

unsigned int MiniGameDef::CRC(unsigned char* data, int length)
{
    return UpdateCRC(0xffffffffU, data, length) ^ 0xffffffffU;
}

void MiniGameDef::setCRC(unsigned int crc)
{
    mCrc = crc;
}

MiniGameDef* MiniGameDef::parseMiniGameDef(arrayWithLength<unsigned char> data)
{
    MiniGameDef* result = parseMiniGameDef(XmlParser::readXml(data.data, data.length));
    result->setCRC(CRC(data.data, data.length));
    return result;
}

MiniGameDef* MiniGameDef::parseMiniGameDef(std::wstring fileName)
{
    fileName = std::wstring(L"Common\\res\\TitleUpdate\\GameRules\\") + fileName;
    File file(fileName);
    FileInputStream stream(file);
    unsigned int size = stream.filesize();
    arrayWithLength<unsigned char> data(size, true);
    bool valid = file.ext() == L".xml" && file.exists();
    if (valid)
    {
        stream.read(data);
        return parseMiniGameDef(data);
    }
    return new MiniGameDef();
}

void MiniGameDef::SetPlayerSettings(const MiniGamePlayerSettings& settings)
{
    EnterCriticalSection(&sPlayerSettingsMutex);
    sPlayerSettings = settings;
    LeaveCriticalSection(&sPlayerSettingsMutex);
}
void MiniGameDef::GetPlayerSettings(MiniGamePlayerSettings& settings)
{
    EnterCriticalSection(&sPlayerSettingsMutex);
    settings = sPlayerSettings;
    LeaveCriticalSection(&sPlayerSettingsMutex);
}

MiniGameDef* MiniGameDef::GetCustomGameModeReferenceById(EMiniGameId id)
{
    if ((unsigned int)id < eMiniGameId_Count)
        return sMiniGameDefs[id];
    return sDefaultMiniGameDef;
}

EMiniGameId MiniGameDef::GetId() const
{
    return mId;
}

void MiniGameDef::cleanUp()
{
    delete mStartConditions;
    delete mEndConditions;
    delete mPostGameConditions;
    delete mShowdownConditions;
}

void MiniGameDef::StaticCtor()
{
    sDefaultMiniGameDef = new MiniGameDef(eMiniGameId_Any);
    sMiniGameDefs[eMiniGameId_Lobby] = parseMiniGameDef(std::wstring(L"lobby.xml"));
    sMiniGameDefs[eMiniGameId_Battle] = parseMiniGameDef(std::wstring(L"MG01.xml"));
    sMiniGameDefs[eMiniGameId_Tumble] = parseMiniGameDef(std::wstring(L"MG02.xml"));
    sMiniGameDefs[eMiniGameId_Glide] = parseMiniGameDef(std::wstring(L"MG03.xml"));
    InitializeCriticalSection(&sPlayerSettingsMutex);
}

MiniGameDef* MiniGameDef::GetCustomGameModeById(EMiniGameId id, bool)
{
    return GetCustomGameModeReferenceById(id);
}

unsigned int MiniGameDef::GetHostOption(eCustomGameModeHostOption option) const
{
    GameModeOptions options = GetSetting<GameModeOptions, &MiniGamePlayerSettings::mHostOptions>();
    return options.Get(option);
}
unsigned int MiniGameDef::GetRoundCount() const
{
    unsigned int roundCount = 1;
    if (GetHostOption(eCustomGameModeHostOption_TimeTrial) == 0)
        roundCount = GetSetting<int, &MiniGamePlayerSettings::mRoundCount>();
    return roundCount;
}
int MiniGameDef::GetFinalRoundIndex() const
{
    int index = GetRoundCount() - 1;
    if (index < 0)
        index = 0;
    return index;
}
unsigned int MiniGameDef::GetGameVariant() const
{
    GameModeOptions options = GetSetting<GameModeOptions, &MiniGamePlayerSettings::mHostOptions>();
    return options.Get(eCustomGameModeHostOption_GameVariant);
}
bool MiniGameDef::HostOption_Bool(eCustomGameModeHostOption, unsigned int index) const
{
    return index != 0;
}
bool MiniGameDef::HostOption_Bool(eCustomGameModeHostOption option) const
{
    return HostOption_Bool(option, GetHostOption(option));
}
bool MiniGameDef::ArePointsActive() const
{
    return HostOption_Bool(eCustomGameModeHostOption_PointsActive);
}
bool MiniGameDef::ArePowerupsActive() const
{
    return HostOption_Bool(eCustomGameModeHostOption_PowerupsActive);
}
bool MiniGameDef::HasFeatureSupportForInfiniteLives() const
{
    return mId != eMiniGameId_Tumble;
}

const MiniGameDef::LootSetDef& MiniGameDef::getCurrentLootSet() const
{
    unsigned int lootSetId = GetSetting<unsigned int, &MiniGamePlayerSettings::mLootSetId>();
    if (!mLootSets.empty() && lootSetId < mLootSets.size())
        return mLootSets.at(lootSetId);
    return sEmptyLootSet;
}

bool MiniGameDef::HasLootGroup(const std::wstring& group) const
{
    return getCurrentLootSet().getLootGroup(group) != NULL;
}

bool MiniGameDef::PopulateContainers(const std::vector<boost::shared_ptr<Container> >* containers,
                                     const std::wstring& group) const
{
    const ModeVariationLootGroup* lootGroup = getCurrentLootSet().getLootGroup(group);
    switch (lootGroup->mDistributionMethod)
    {
    case eLootDistributionMethod_Spread:
    {
        return GameModeUtilities::DistributeItemsSpread(containers, lootGroup->GetChestItems());
    }
    case eLootDistributionMethod_KitKPRandom:
        return GameModeUtilities::DistributeItemsKitsAndKP(
            containers, &lootGroup->GetAllChestItems(), &lootGroup->GetAllKits(), lootGroup->GetChestKPScore(), false);
    case eLootDistributionMethod_KitKPGreedy:
        return GameModeUtilities::DistributeItemsKitsAndKP(
            containers, &lootGroup->GetAllChestItems(), &lootGroup->GetAllKits(), lootGroup->GetChestKPScore(), true);
    case eLootDistributionMethod_Fixed:
        return GameModeUtilities::DistributeItemsKitsFixed(containers, &lootGroup->GetAllKits());
    case eLootDistributionMethod_WeightedRandom:
        return GameModeUtilities::DistributeItemsWeightedRandom(containers, lootGroup->GetWeightedChestItems(),
                                                                lootGroup->GetChestKPScore());
    default:
        return false;
    }
}

unsigned int MiniGameDef::EquipPlayers(std::vector<boost::shared_ptr<ServerPlayer> >* players) const
{
    const PlayerEquipmentInfo* equipment =
        &getCurrentLootSet().getLootGroup(sPlayerInventoryLootGroup)->GetPlayerEquipment();
    if (equipment == NULL)
        return 0;

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players->begin(); it != players->end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> player = *it;
        player->mInventory->m_selected = 0;
        for (int slot = 0; slot < 6; ++slot)
        {
            if (equipment->mSlots[slot].isSet())
                player->setItemSlot(EquipmentSlot::SLOTS[slot], equipment->mSlots[slot].get());
        }
    }
    return 1;
}

bool MiniGameDef::IsCustomGame(EMiniGameId id, const MiniGamePlayerSettings& settings)
{
    unsigned int variant = settings.mHostOptions.Get(eCustomGameModeHostOption_GameVariant);
    return id == eMiniGameId_Battle ? variant == 2 : id == eMiniGameId_Tumble && variant == 3;
}

bool MiniGameDef::IsCustomGame() const
{
    EnterCriticalSection(&sPlayerSettingsMutex);
    bool customGame = IsCustomGame(mId, sPlayerSettings);
    LeaveCriticalSection(&sPlayerSettingsMutex);
    return customGame;
}

int MiniGameDef::GetOptionsCount(eCustomGameModeHostOption option) const
{
    if (option < eCustomGameModeHostOption_All)
        return mOptionLists.at(option).size();

    int index = option - eCustomGameModeHostOption_MapVariant;
    if ((unsigned int)index <= mMatchmakingFilters.size() && index >= 0)
        return mMatchmakingFilters[index].size();
    return 0;
}

unsigned int MiniGameDef::GetGameVariantCount() const
{
    return GetOptionsCount(eCustomGameModeHostOption_ModeVariant);
}
int MiniGameDef::GetLootSetCount() const
{
    return mLootSets.size();
}
int MiniGameDef::GetLootSetId() const
{
    return GetSetting<unsigned int, &MiniGamePlayerSettings::mLootSetId>();
}
void MiniGameDef::SetLootSet(int id) const
{
    SetSetting<unsigned int, &MiniGamePlayerSettings::mLootSetId>(id);
}

std::wstring MiniGameDef::GetLootSetText(int id) const
{
    unsigned int mode = GetId();
    if (mode == 1)
    {
        switch (id)
        {
        case 0:
            return L10N::GetString(IDS_MG01_ITEMSET_0);
        case 1:
            return L10N::GetString(IDS_MG01_ITEMSET_1);
        case 2:
            return L10N::GetString(IDS_MG01_ITEMSET_2);
        case 3:
            return L10N::GetString(IDS_MG01_ITEMSET_3);
        case 4:
            return L10N::GetString(IDS_MG01_ITEMSET_4);
        case 5:
            return L10N::GetString(IDS_MG01_ITEMSET_RANDOM);
        default:
            return L"";
        }
    }
    if (mode == 2)
    {
        switch (id)
        {
        case 0:
            return L10N::GetString(IDS_MG02_VARIANT_1);
        case 1:
            return L10N::GetString(IDS_MG02_VARIANT_2);
        case 2:
            return L10N::GetString(IDS_MG02_VARIANT_FIREWORKS);
        case 3:
            return L10N::GetString(IDS_MG02_VARIANT_LEVITATE);
        default:
            return L"";
        }
    }
    return L"";
}

std::wstring MiniGameDef::GetLootSetText() const
{
    return GetLootSetText(GetLootSetId());
}

std::wstring MiniGameDef::GetLootSetDescription(int id) const
{
    unsigned int mode = GetId();
    if (mode == 1)
    {
        switch (id)
        {
        case 0:
            return L10N::GetString(IDS_MG01_ITEMSET_0_DESC);
        case 1:
            return L10N::GetString(IDS_MG01_ITEMSET_1_DESC);
        case 2:
            return L10N::GetString(IDS_MG01_ITEMSET_2_DESC);
        case 3:
            return L10N::GetString(IDS_MG01_ITEMSET_3_DESC);
        case 4:
            return L10N::GetString(IDS_MG01_ITEMSET_4_DESC);
        case 5:
            return L10N::GetString(IDS_MG01_ITEMSET_RANDOM_DESC);
        default:
            return L"";
        }
    }
    if (mode == 2)
    {
        switch (id)
        {
        case 0:
            return L10N::GetString(IDS_MG02_VARIANT_1_DESC);
        case 1:
            return L10N::GetString(IDS_MG02_VARIANT_2_DESC);
        case 2:
            return L10N::GetString(IDS_MG02_VARIANT_FIREWORKS_DESC);
        case 3:
            return L10N::GetString(IDS_MG02_VARIANT_LEVITATE_DESC);
        default:
            return L"";
        }
    }
    return L"";
}

MiniGamePlayerSettings MiniGameDef::GetDefaultPlayerSettings() const
{
    return mPlayerSettings;
}

boost::shared_ptr<Packet> MiniGameDef::GetPlayerSettingsPacket() const
{
    return boost::shared_ptr<Packet>(new ClientboundMGPlayerSettingsUpdatePacket(sPlayerSettings));
}

std::wstring MiniGameDef::getVersionString() const
{
    return _toString(mVersionMajor) + L"." + _toString(mVersionMinor);
}

bool MiniGameDef::IsCRCValid() const
{
    return mCrc == sValidCRCs[GetId()];
}

bool MiniGameDef::HasPermission(EBuildMode mode, const std::vector<int>& permissions, int value) const
{
    if (mode == EBuildMode_NoRestrictions || mode > EBuildMode_BlackList)
        return true;
    bool listed = std::find(permissions.begin(), permissions.end(), value) != permissions.end();
    return (mode == EBuildMode_WhiteList) == listed;
}

bool MiniGameDef::MayPlace(int id) const
{
    return !mEditActionsDisabled && HasPermission(mPlaceMode, mPlacePermissions, id);
}
bool MiniGameDef::MayDestroy(int id) const
{
    return !mEditActionsDisabled && HasPermission(mDestroyMode, mDestroyPermissions, id);
}
bool MiniGameDef::MayUse(int id) const
{
    return !mEditActionsDisabled && HasPermission(mUseMode, mUsePermissions, id);
}
unsigned int MiniGameDef::MayUseBlock(int id) const
{
    return !mEditActionsDisabled && HasPermission(mUseBlockMode, mBlockUsePermissions, id);
}

bool MiniGameDef::CanAutoStart() const
{
    return mCanAutoStart;
}
bool MiniGameDef::ShowInvulnerableEffect() const
{
    return mShowInvulnerableEffect;
}
bool MiniGameDef::LockedInventory() const
{
    return mLockedInventory;
}
bool MiniGameDef::ShowdownAnnouncements() const
{
    return mShowdownAnnouncements;
}
bool MiniGameDef::AllowPvP() const
{
    return mAllowPvP;
}
bool MiniGameDef::GetTutorialsEnabled() const
{
    return mTutorialsEnabled;
}
bool MiniGameDef::GetShowNametags() const
{
    return GetSetting<bool, &MiniGamePlayerSettings::mShowNametags>();
}
double MiniGameDef::GetDamageIndicatorDelay() const
{
    return mDamageIndicatorDelay;
}
bool MiniGameDef::LoadOnlyActiveAreaChunks() const
{
    return mLoadOnlyActiveAreaChunks;
}
unsigned int MiniGameDef::GetLivesPerRound() const
{
    return GetSetting<unsigned int, &MiniGamePlayerSettings::mLivesPerRound>();
}
unsigned int MiniGameDef::GetTeamCount() const
{
    return GetSetting<unsigned int, &MiniGamePlayerSettings::mTeamCount>();
}
bool MiniGameDef::IsTeamBasedSpawn() const
{
    return GetSetting<bool, &MiniGamePlayerSettings::mTeamBasedSpawn>();
}
bool MiniGameDef::IsCheckpointBasedSpawn() const
{
    return GetSetting<bool, &MiniGamePlayerSettings::mCheckpointBasedSpawn>();
}
bool MiniGameDef::HasSpectatorParticipation() const
{
    return mId == eMiniGameId_Tumble && GetSetting<bool, &MiniGamePlayerSettings::mSpectatorParticipation>();
}
bool MiniGameDef::IsSmallInventoryEnabled() const
{
    return GetSetting<bool, &MiniGamePlayerSettings::mSmallInventoryEnabled>();
}
unsigned int MiniGameDef::GetMaxPlayersForSmallMaps() const
{
    return mMaxPlayersForSmallMaps;
}
bool MiniGameDef::IsTakeEverythingEnabled() const
{
    return GetSetting<bool, &MiniGamePlayerSettings::mTakeEverythingEnabled>();
}
bool MiniGameDef::IsChestRefillEnabled() const
{
    return GetSetting<bool, &MiniGamePlayerSettings::mChestRefillEnabled>();
}
bool MiniGameDef::ForceSpectateModeFollow() const
{
    return GetId() == eMiniGameId_Glide;
}
bool MiniGameDef::SupressGameModeMessages() const
{
    return true;
}

const MiniGameDef::FoodBalance* MiniGameDef::GetFoodBalance() const
{
    int foodBalance = GetSetting<int, &MiniGamePlayerSettings::mFoodBalance>();
    if (mFoodBalances.find(foodBalance) != mFoodBalances.end())
        return mFoodBalances.at(foodBalance);
    return &sDefaultFoodBalance;
}

unsigned int MiniGameDef::getMaxFood(int value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mMaxFood.otherwise(value);
}
float MiniGameDef::getMaxSaturation(float value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mMaxSaturation.otherwise(value);
}
float MiniGameDef::getStartSaturation(float value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mStartSaturation.otherwise(value);
}
float MiniGameDef::getSaturationFloor(float value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mSaturationFloor.otherwise(value);
}
float MiniGameDef::getExhaustionDrop(float value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mExhaustionDrop.otherwise(value);
}
unsigned int MiniGameDef::getHealthTickCount(int value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mHealthTickCount.otherwise(value);
}
unsigned int MiniGameDef::getHealthTickCountSaturated(int value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mHealthTickCountSaturated.otherwise(value);
}
unsigned int MiniGameDef::getHealLevel(int value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mHealLevel.otherwise(value);
}
unsigned int MiniGameDef::getStarveLevel(int value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mStarveLevel.otherwise(value);
}
unsigned int MiniGameDef::getSprintLevel(int value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mSprintLevel.otherwise(value);
}
float MiniGameDef::getExhaustionHeal(float value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mExhaustionHeal.otherwise(value);
}
float MiniGameDef::getExhaustionJump(float value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mExhaustionJump.otherwise(value);
}
float MiniGameDef::getExhaustionSprintJump(float value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mExhaustionSprintJump.otherwise(value);
}
float MiniGameDef::getExhaustionMine(float value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mExhaustionMine.otherwise(value);
}
float MiniGameDef::getExhaustionAttack(float value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mExhaustionAttack.otherwise(value);
}
float MiniGameDef::getExhaustionDamage(float value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mExhaustionDamage.otherwise(value);
}
float MiniGameDef::getExhaustionWalk(float value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mExhaustionWalk.otherwise(value);
}
float MiniGameDef::getExhaustionSprint(float value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mExhaustionSprint.otherwise(value);
}
float MiniGameDef::getExhaustionSneak(float value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mExhaustionSneak.otherwise(value);
}
float MiniGameDef::getExhaustionSwim(float value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mExhaustionSwim.otherwise(value);
}
float MiniGameDef::getExhaustionIdle(float value) const
{
    const FoodBalance* f = GetFoodBalance();
    return f->mExhaustionIdle.otherwise(value);
}

int MiniGameDef::getCentralRefillChance() const
{
    return mChestRefill.mCentralRefillChance.isSet() ? mChestRefill.mCentralRefillChance.get() : 0;
}
int MiniGameDef::getOuterRefillChance() const
{
    return mChestRefill.mOuterRefillChance.isSet() ? mChestRefill.mOuterRefillChance.get() : 0;
}
int MiniGameDef::getHVRefillChance() const
{
    return mChestRefill.mHVRefillChance.isSet() ? mChestRefill.mHVRefillChance.get() : 0;
}
int MiniGameDef::getNumberOfChestsToRefill() const
{
    return mChestRefill.mNumberOfChestsToRefill.isSet() ? mChestRefill.mNumberOfChestsToRefill.get() : 0;
}
int MiniGameDef::getMinPlayerDistanceToEffectChance() const
{
    return mChestRefill.mMinPlayerDistanceToEffectChance.isSet() ? mChestRefill.mMinPlayerDistanceToEffectChance.get()
                                                                 : 0;
}
int MiniGameDef::getMaxPlayerDistanceToEffectChance() const
{
    return mChestRefill.mMaxPlayerDistanceToEffectChance.isSet() ? mChestRefill.mMaxPlayerDistanceToEffectChance.get()
                                                                 : 0;
}
std::wstring MiniGameDef::getCentralLootGroup() const
{
    return mChestRefill.mCentralLootGroup.isSet() ? mChestRefill.mCentralLootGroup.get() : std::wstring(L"");
}
std::wstring MiniGameDef::getOuterLootGroup() const
{
    return mChestRefill.mOuterLootGroup.isSet() ? mChestRefill.mOuterLootGroup.get() : std::wstring(L"");
}
std::wstring MiniGameDef::getHVLootGroup() const
{
    return mChestRefill.mHVLootGroup.isSet() ? mChestRefill.mHVLootGroup.get() : std::wstring(L"");
}

not_null_ptr<ItemInstance> MiniGameDef::getEquipmentForSlot(EquipmentSlot::EIndex slot) const
{
    not_null_ptr<ItemInstance> empty = ItemInstance::EMPTY;
    const maybe<not_null_ptr<ItemInstance> >& equipment =
        getCurrentLootSet().getLootGroup(sPlayerInventoryLootGroup)->mPlayerEquipment.mSlots[slot];
    return equipment.isSet() ? equipment.get() : empty;
}

GameCondition* MiniGameDef::GetStartConditions() const
{
    return mStartConditions;
}
GameCondition* MiniGameDef::GetEndConditions() const
{
    return mEndConditions;
}
GameCondition* MiniGameDef::GetPostGameConditions() const
{
    return mPostGameConditions;
}
GameCondition* MiniGameDef::GetShowdownConditions() const
{
    return mShowdownConditions;
}

const std::vector<int>& MiniGameDef::GetOptionsList(eCustomGameModeHostOption option) const
{
    return mOptionLists.at(option);
}

unsigned int MiniGameDef::HostOption_Category(eCustomGameModeHostOption option, unsigned int index) const
{
    return GetOptionsList(option)[index];
}

unsigned int MiniGameDef::HostOption_Category(eCustomGameModeHostOption option) const
{
    return HostOption_Category(option, GetHostOption(option));
}
int MiniGameDef::GetSpectateMode() const
{
    return mSpectateMode != eSpectateMode_HostOption ? mSpectateMode
                                                     : HostOption_Category(eCustomGameModeHostOption_SpectateMode);
}
unsigned long long MiniGameDef::GetSpectateEntity() const
{
    const int spectateMode = GetSpectateMode();
    if (spectateMode == eSpectateMode_Bat)
        return EntityType::BAT;
    if (spectateMode == eSpectateMode_Parrot)
        return EntityType::PARROT;
    if (spectateMode == eSpectateMode_Vex)
        return EntityType::VEX;
    return EntityType::PLAYER;
}
maybe<int> MiniGameDef::getDifficulty() const
{
    int difficulty = gApp.GetGameHostOption(eGameHostOption_Difficulty);
    if (mDifficulty.isSet())
        difficulty = mDifficulty.get();
    return maybe<int>(difficulty);
}
const GameType* MiniGameDef::getPlayerGameType() const
{
    int gameType = GameType::SURVIVAL->getId();
    if (mPlayerGameType.isSet())
        gameType = mPlayerGameType.get();
    return GameType::byId(gameType);
}
Abilities MiniGameDef::getPlayerAbilities() const
{
    return GetSetting<Abilities, &MiniGamePlayerSettings::mPlayerAbilities>();
}
MiniGameDef::TimerSpec MiniGameDef::getTimerSpec(int timer) const
{
    return atMaybe(mTimers, timer, TimerSpec());
}
MiniGameDef::SoundSpec MiniGameDef::getSoundSpec(EGameModeSound sound) const
{
    return atMaybe(mSounds, sound, SoundSpec());
}
double MiniGameDef::getBaseDamage(int itemId, double value) const
{
    return atMaybe(mItemBalance.mBaseDamage, itemId, value);
}
int MiniGameDef::getNutrition(int itemId, int value) const
{
    return atMaybe(mItemBalance.mNutrition, itemId, value);
}
float MiniGameDef::getSaturationModifier(int itemId, float value) const
{
    return atMaybe(mItemBalance.mSaturationFactor, itemId, value);
}
int MiniGameDef::getPotionDuration(int effectId, int value) const
{
    return atMaybe(mItemBalance.mPotionDuration, effectId, value);
}
int MiniGameDef::getLingeringDuration(int effectId, int value) const
{
    return atMaybe(mItemBalance.mLingeringDuration, effectId, value);
}
double MiniGameDef::getThrowPower(unsigned long long entityType, double value) const
{
    if (entityType == EntityType::THROWN_POTION)
        return getBaseDamage(Item::getId(Items::splashPotion), value);
    if (entityType == EntityType::THROWN_EXP_BOTTLE)
        return getBaseDamage(Item::getId(Items::experienceBottle), value);
    if (entityType == EntityType::THROWN_ENDERPEARL)
        return getBaseDamage(Item::getId(Items::enderPearl), value);
    if (entityType == EntityType::THROWN_EGG)
        return getBaseDamage(Item::getId(Items::egg), value);
    if (entityType == EntityType::SNOWBALL)
        return getBaseDamage(Item::getId(Items::snowball), value);
    return value;
}

double MiniGameDef::getThrowAngle(unsigned long long entityType, double value) const
{
    if ((entityType == EntityType::THROWN_EXP_BOTTLE || entityType == EntityType::THROWN_POTION
         || entityType == EntityType::THROWN_ENDERPEARL || entityType == EntityType::THROWN_EGG
         || entityType == EntityType::SNOWBALL)
        && getThrowPower(entityType, -1.0) != -1.0)
        return 0.0;
    return value;
}
bool MiniGameDef::shouldTickBlocks() const
{
    return mBlockData.mTickOverride.isSet() ? mBlockData.mTickOverride.get() : true;
}
bool MiniGameDef::shouldTickBlock(int blockId) const
{
    bool defaultValue = mBlockData.mTickDefault.isSet() ? mBlockData.mTickDefault.get() : true;
    return atMaybe(mBlockData.mTick, blockId, defaultValue);
}
bool MiniGameDef::canLavaConvertWater() const
{
    return mCanLavaConvertWater.isSet() ? mCanLavaConvertWater.get() : false;
}
bool MiniGameDef::canTntDestroyBlocks() const
{
    maybe<bool> canTntDestroyBlocks = GetSetting<maybe<bool>, &MiniGamePlayerSettings::mCanTntDestroyBlocks>();
    return canTntDestroyBlocks.isSet() ? canTntDestroyBlocks.get() : false;
}
unsigned char MiniGameDef::CurrentHostOptionVersion() const
{
    return mHostOptionVersion;
}

unsigned int MiniGameDef::HostOption_Time(eCustomGameModeHostOption option, unsigned int index) const
{
    return GetOptionsList(option)[index];
}

unsigned int MiniGameDef::HostOption_Time(eCustomGameModeHostOption option) const
{
    return HostOption_Time(option, GetHostOption(option));
}

std::wstring MiniGameDef::GetValueText(eCustomGameModeHostOption option, int value) const
{
    unsigned int type = GetOptionType(option);
    switch (type)
    {
    case 1:
        switch (option)
        {
        case eCustomGameModeHostOption_SpectateMode:
            return L10N::GetString(sHostOptionCategoryStringIds[HostOption_Category(option, value)]);

        case eCustomGameModeHostOption_MapVariant:
            switch (value)
            {
            case 0:
                return L10N::GetString(IDS_MG_ANY);
            case 1:
                return L10N::GetString(IDS_MG_SHORT);
            case 2:
                return L10N::GetString(IDS_MG_MEDIUM);
            case 3:
                return L10N::GetString(IDS_MG_LONG);
            default:
                return L"";
            }

        case eCustomGameModeHostOption_ModeVariant:
            switch (GetId())
            {
            case eMiniGameId_Battle:
                switch (value)
                {
                case 0:
                    return L10N::GetString(IDS_MG_VARIANT_ANY);
                case 1:
                    return L10N::GetString(IDS_MG01_VARIANT_CASUAL);
                case 2:
                    return L10N::GetString(IDS_MG01_VARIANT_COMPETITIVE);
                case 3:
                    return L10N::GetString(IDS_MG_VARIANT_CUSTOM);
                default:
                    return L"";
                }
            case eMiniGameId_Tumble:
                switch (value)
                {
                case 0:
                    return L10N::GetString(IDS_MG_VARIANT_ANY);
                case 1:
                    return L10N::GetString(IDS_MG02_VARIANT_1);
                case 2:
                    return L10N::GetString(IDS_MG02_VARIANT_2);
                case 3:
                    return L10N::GetString(IDS_MG_VARIANT_RANDOM);
                case 4:
                    return L10N::GetString(IDS_MG_VARIANT_CUSTOM);
                default:
                    return L"";
                }
            case eMiniGameId_Glide:
                switch (value)
                {
                case 0:
                    return L10N::GetString(IDS_MG_VARIANT_ANY);
                case 1:
                    return L10N::GetString(IDS_MG03_VARIANT_TIMEATTACK);
                case 2:
                    return L10N::GetString(IDS_MG03_VARIANT_SCOREATTACK);
                case 4:
                    return L10N::GetString(IDS_MG_VARIANT_CUSTOM);
                default:
                    return L"";
                }
            }
            return L"";

        case eCustomGameModeHostOption_GameVariant:
            switch (GetId())
            {
            case eMiniGameId_Battle:
                switch (value)
                {
                case 0:
                    return L10N::GetString(IDS_MG01_VARIANT_CASUAL);
                case 1:
                    return L10N::GetString(IDS_MG01_VARIANT_COMPETITIVE);
                case 2:
                    return L10N::GetString(IDS_MG_VARIANT_CUSTOM);
                default:
                    return L"";
                }
            case eMiniGameId_Tumble:
                switch (value)
                {
                case 0:
                    return L10N::GetString(IDS_MG02_VARIANT_1);
                case 1:
                    return L10N::GetString(IDS_MG02_VARIANT_2);
                case 2:
                    return L10N::GetString(IDS_MG_VARIANT_RANDOM);
                case 3:
                    return L10N::GetString(IDS_MG_VARIANT_CUSTOM);
                default:
                    return L"";
                }
            case eMiniGameId_Glide:
                switch (value)
                {
                case 0:
                    return L10N::GetString(IDS_MG03_VARIANT_TIMEATTACK);
                case 1:
                    return L10N::GetString(IDS_MG03_VARIANT_SCOREATTACK);
                default:
                    return L"";
                }
            }
            return L"";
        }
        return L"";

    case 2:
        if (option == eCustomGameModeHostOption_RoundLength)
        {
            switch (value)
            {
            case 0:
                return L10N::GetString(IDS_MG_SHORT);
            case 1:
                return L10N::GetString(IDS_MG_MEDIUM);
            case 2:
                return L10N::GetString(IDS_MG_LONG);
            default:
                return L"";
            }
        }
        else
        {
            int seconds = HostOption_Time(option, value);
            if (seconds == -2)
                return L10N::GetString(IDS_MG_OPTION_NONE);
            if (seconds == -1)
                return L10N::GetString(IDS_MG_OPTION_UNLIMITED);
            if (seconds >= 60)
            {
                std::wstring text = L10N::GetString(seconds == 60 ? IDS_MG_OPTION_MIN : IDS_MG_OPTION_MINS);
                return replaceAll(text, L"{*MINUTES*}", _toString(seconds / 60));
            }
            if (seconds >= 0)
            {
                std::wstring text = L10N::GetString(seconds == 1 ? IDS_MG_OPTION_SEC : IDS_MG_OPTION_SECS);
                return replaceAll(text, L"{*SECONDS*}", _toString(seconds));
            }
            return L"";
        }

    case 0:
        if (option == eCustomGameModeHostOption_NoSmallSkins)
            value = !value;
        return L10N::GetString(value ? IDS_ON : IDS_OFF);
    }
    return L"";
}

std::wstring MiniGameDef::getComparisonImageForGameMode(EMiniGameId id)
{
    switch (id)
    {
    case eMiniGameId_Battle:
        return std::wstring(L"Graphics\\GameModeGraphics\\BattleModeScreen.png");
    case eMiniGameId_Tumble:
        return std::wstring(L"Graphics\\GameModeGraphics\\MG02Screen.png");
    case eMiniGameId_Glide:
        return std::wstring(L"Graphics\\GameModeGraphics\\MG03Screen.png");
    default:
        return std::wstring(L"Graphics\\MinecraftBrokenIcon.png");
    }
}

std::wstring MiniGameDef::GetVariantIconPath(EMiniGameId id, unsigned int variant)
{
    if (id == eMiniGameId_Tumble)
    {
        if (variant > 2)
            variant = 2;
        return formatwstr(L"Graphics\\GameModeGraphics\\MG02_Mode_%02i.png", variant);
    }
    return std::wstring(L"default");
}

void MiniGameDef::SetHostOption(eCustomGameModeHostOption option, unsigned int value) const
{
    EnterCriticalSection(&sPlayerSettingsMutex);
    GameModeOptions options = GetSetting<GameModeOptions, &MiniGamePlayerSettings::mHostOptions>();
    options.Set(option, value);
    SetSetting<GameModeOptions, &MiniGamePlayerSettings::mHostOptions>(options);
    LeaveCriticalSection(&sPlayerSettingsMutex);
}

int MiniGameDef::GetHostOptionIndexFrom_Time(eCustomGameModeHostOption option, int value) const
{
    const std::vector<int> values = GetOptionsList(option);
    int index = 0;
    for (std::vector<int>::const_iterator it = values.begin(); it != values.end(); ++it, ++index)
    {
        if (*it == value)
            return index;
    }
    return 0;
}

int MiniGameDef::GetHostOptionIndexFrom_Category(eCustomGameModeHostOption option, int value) const
{
    const std::vector<int> values = GetOptionsList(option);
    int index = 0;
    for (std::vector<int>::const_iterator it = values.begin(); it != values.end(); ++it, ++index)
    {
        if (*it == value)
            return index;
    }
    return 0;
}

unsigned int MiniGameDef::GetHostOptionIndexFrom_Bool(eCustomGameModeHostOption, bool value) const
{
    return value;
}

unsigned int MiniGameDef::GetHostOptionIndexFromValue(eCustomGameModeHostOption option, int value) const
{
    switch (GetOptionType(option))
    {
    case 0:
        return GetHostOptionIndexFrom_Bool(option, value != 0);
    case 1:
        return GetHostOptionIndexFrom_Category(option, value);
    case 2:
        return GetHostOptionIndexFrom_Time(option, value);
    default:
        return 0;
    }
}

unsigned int MiniGameDef::GetOptionMin(eCustomGameModeHostOption option, bool includeRandom) const
{
    if (includeRandom)
    {
        switch (option)
        {
        case eCustomGameModeHostOption_MapVariant:
        case eCustomGameModeHostOption_ModeVariant:
            return 1;
        default:
            break;
        }
    }
    return 0;
}

int MiniGameDef::GetOptionMax(eCustomGameModeHostOption option) const
{
    int count = GetOptionsCount(option);
    switch (option)
    {
    case eCustomGameModeHostOption_MapVariant:
    case eCustomGameModeHostOption_ModeVariant:
        return count;
    default:
        return count - 1;
    }
}

unsigned int MiniGameDef::AllowSmallSkins() const
{
    return GetId() != eMiniGameId_Any && GetHostOption(eCustomGameModeHostOption_NoSmallSkins) != 0;
}

std::wstring MiniGameDef::GetFoodBalanceIndexText(int index) const
{
    switch (index)
    {
    case 0:
        return L10N::GetString(0x1ea80257);
    case 1:
        return L10N::GetString(0xf8a587ee);
    case 2:
        return L10N::GetString(0xd2a30d85);
    case 3:
        return L10N::GetString(0xaca0931c);
    case 4:
        return L10N::GetString(0x869e18b3);
    case 5:
        return L10N::GetString(0x609b9e4a);
    case 6:
        return L10N::GetString(0x3a9923e1);
    case 7:
        return L10N::GetString(0x1496a978);
    default:
        return std::wstring();
    }
}

std::wstring MiniGameDef::GetFoodBalanceIndexDescription(int index) const
{
    switch (index)
    {
    case 0:
        return L10N::GetString(0x9a084979);
    case 1:
        return L10N::GetString(0x3ce24768);
    case 2:
        return L10N::GetString(0x175deebb);
    case 3:
        return L10N::GetString(0xa7ba696a);
    case 4:
        return L10N::GetString(0x4b98883d);
    case 5:
        return L10N::GetString(0xabd0762c);
    case 6:
        return L10N::GetString(0x6c2ba0ff);
    case 7:
        return L10N::GetString(0xfc881bae);
    default:
        return std::wstring();
    }
}

void MiniGameDef::SetDisabledEditActions(bool disabled) const
{
    const_cast<MiniGameDef*>(this)->mEditActionsDisabled = disabled;
}
bool MiniGameDef::EditActionsDisabled() const
{
    return mEditActionsDisabled;
}
int MiniGameDef::GetItemRespawnTime() const
{
    return 30;
}
unsigned int MiniGameDef::IsShowdownEnabled() const
{
    return 1;
}
bool MiniGameDef::CanCraft() const
{
    return mAllowCrafting;
}
bool MiniGameDef::HasMultipleRounds() const
{
    return GetRoundCount() > 1;
}

void MiniGameDef::addWeightedChestItem(std::vector<ChestItemsInfo>* items, not_null_ptr<ItemInstance> item,
                                       unsigned int minCount, unsigned int maxCount, unsigned int weight)
{
    ChestItemsInfo info;
    info.mItem = item;
    info.mMinStackCount = 1;
    info.mMaxStackCount = 1;
    info.mMinCount = minCount;
    info.mMaxCount = maxCount;
    info.mPlayersForMin = 1;
    info.mWeight = weight;
    items->push_back(info);
}

void MiniGameDef::addChestItem(std::vector<ChestItemsInfo>* items, not_null_ptr<ItemInstance> item,
                               unsigned int minStackCount, unsigned int maxStackCount, unsigned int minCount,
                               unsigned int maxCount, unsigned int playersForMin)
{
    ChestItemsInfo info;
    info.mItem = item;
    info.mMinStackCount = minStackCount;
    info.mMaxStackCount = maxStackCount;
    info.mMinCount = minCount;
    info.mMaxCount = maxCount;
    info.mPlayersForMin = playersForMin;
    info.mWeight = 1;
    items->push_back(info);
}

unsigned int MiniGameDef::GetName() const
{
    static const unsigned int names[] = {0, IDS_MINI_GAME_01, IDS_MINI_GAME_02, IDS_MINI_GAME_03};
    unsigned int id = mId;
    if (id != eMiniGameId_Lobby && id < eMiniGameId_Build)
        return names[id];
    return 0xffffffffU;
}

unsigned int MiniGameDef::GetHostOptionStringId(eCustomGameModeHostOption option)
{
    static const unsigned int names[] = {IDS_MG_OPTION_ALLOW_ALL_SKINS, 0x901c1fe8U, 0x908811f4U};
    switch (option)
    {
    case eCustomGameModeHostOption_NoSmallSkins:
    case eCustomGameModeHostOption_SpectateMode:
    case eCustomGameModeHostOption_RoundLength:
        return names[option];
    case eCustomGameModeHostOption_CentreSpawn:
    case eCustomGameModeHostOption_IsTeams:
        return 0xef2a2f6aU;
    case eCustomGameModeHostOption_GameVariant:
        return 0x5bd476fbU;
    default:
        return 0xffffffffU;
    }
}

void MiniGameDef::AddLabelForOption(std::vector<HtmlString>& labels, eCustomGameModeHostOption option,
                                    unsigned int value) const
{
    if (GetOptionsCount(option) == 0)
        return;
    int stringId = GetHostOptionStringId(option);
    if (stringId == -1)
        return;

    std::wstring valueText = GetValueText(option, GameModeOptions(value).Get(option));
    std::wstring label = formatwstr(L"%ls: %ls", L10N::GetString(stringId), valueText.c_str());
    labels.push_back(HtmlString(label, eMinecraftColour_NOTSET, false, false, 0));
}

void MiniGameDef::GetGameSettingsDescription(std::vector<HtmlString>& labels, unsigned int flags) const
{
    std::wstring header = formatwstr(L"%ls: %ls", L10N::GetString(IDS_MINI_GAME), L10N::GetString(GetName()));
    labels.push_back(HtmlString(header, eMinecraftColour_NOTSET, false, false, 0));

    unsigned int id = GetId();
    if (id == eMiniGameId_Lobby)
        return;
    if (id == eMiniGameId_Battle)
    {
        AddLabelForOption(labels, eCustomGameModeHostOption_GameVariant, flags);
        AddLabelForOption(labels, eCustomGameModeHostOption_NoSmallSkins, flags);
        AddLabelForOption(labels, eCustomGameModeHostOption_SpectateMode, flags);
        AddLabelForOption(labels, eCustomGameModeHostOption_RoundLength, flags);
        AddLabelForOption(labels, eCustomGameModeHostOption_MaxPlayers, flags);
        AddLabelForOption(labels, eCustomGameModeHostOption_CentreSpawn, flags);
        AddLabelForOption(labels, eCustomGameModeHostOption_PointsActive, flags);
        AddLabelForOption(labels, eCustomGameModeHostOption_PowerupsActive, flags);
        AddLabelForOption(labels, eCustomGameModeHostOption_EnableVoting, flags);
        AddLabelForOption(labels, eCustomGameModeHostOption_IsTeams, flags);
        AddLabelForOption(labels, eCustomGameModeHostOption_Nothing, flags);
        return;
    }
    if (id > eMiniGameId_Glide)
        return;
    AddLabelForOption(labels, eCustomGameModeHostOption_GameVariant, flags);
}

void MiniGameDef::SetDefaultMatchmakingSearchData(bool joining, LaunchMoreOptionsMenuInitData* data) const
{
    if (data->mMiniGameId == eMiniGameId_Lobby || data->mMiniGameId > eMiniGameId_Build)
        return;
    if (joining)
    {
        data->mMatchmakingFilters[1] = 0;
        data->mMatchmakingFilters[0] = 0;
        return;
    }
    data->mMatchmakingFilters[0] = mMatchmakingFilterDefaults[0];
    data->mMatchmakingFilters[1] = mMatchmakingFilterDefaults[1];
}

bool MiniGameDef::HasFeatureSeasonalMaps() const
{
    return mId == eMiniGameId_Tumble;
}
bool MiniGameDef::HasFeatureKeepInventory() const
{
    return mId != eMiniGameId_Battle;
}

void MiniGameDef::SetGameSettingsFromMatchmakingFilter(LaunchMoreOptionsMenuInitData* data) const
{
    EnterCriticalSection(&sPlayerSettingsMutex);
    if (data->mMatchmaking && !data->mTimeTrial)
    {
        if (!HasFeatureSeasonalMaps())
        {
            LevelRules::ELevelRuleType type = LevelRules::GetRuleTypeForMiniGame(data->mMiniGameId);
            gApp.getLevelRules()->getRulesByType(type, &data->mLevelRulesetGroups, true);
        }
        data->mOnlineGame = true;
        data->mInviteOnly = false;
    }

    if (data->mMatchmakingFilters[0] != 0)
    {
        const boost::unordered_map<int, int>& options =
            mMatchmakingFilters.at(0).find(data->mMatchmakingFilters[0] - 1)->second;
        for (boost::unordered_map<int, int>::const_iterator option = options.begin(); option != options.end(); ++option)
        {
            data->mPlayerSettings.mHostOptions.Set(
                (eCustomGameModeHostOption)option->first,
                GetHostOptionIndexFromValue((eCustomGameModeHostOption)option->first, option->second));
        }
    }

    if (data->mMatchmakingFilters[1] != 0)
    {
        const boost::unordered_map<int, int>& options =
            mMatchmakingFilters.at(1).find(data->mMatchmakingFilters[1] - 1)->second;
        for (boost::unordered_map<int, int>::const_iterator option = options.begin(); option != options.end(); ++option)
        {
            data->mPlayerSettings.mHostOptions.Set(
                (eCustomGameModeHostOption)option->first,
                GetHostOptionIndexFromValue((eCustomGameModeHostOption)option->first, option->second));
        }
        data->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_GameVariant, data->mMatchmakingFilters[1] - 1);
    }

    gApp.SetGameHostOption(eGameHostOption_PvP, 1);
    gApp.SetGameHostOption(eGameHostOption_TrustPlayers, 1);
    gApp.SetGameHostOption(eGameHostOption_FireSpreads, 0);
    gApp.SetGameHostOption(eGameHostOption_TNT, 1);
    gApp.SetGameHostOption(eGameHostOption_HostCanFly, 0);
    gApp.SetGameHostOption(eGameHostOption_HostCanChangeHunger, 0);
    gApp.SetGameHostOption(eGameHostOption_HostCanBeInvisible, 0);
    gApp.SetGameHostOption(eGameHostOption_MobGriefing, 0);
    gApp.SetGameHostOption(eGameHostOption_DoMobSpawning, 0);
    gApp.SetGameHostOption(eGameHostOption_DoMobLoot, 1);
    gApp.SetGameHostOption(eGameHostOption_DoTileDrops, 0);
    gApp.SetGameHostOption(eGameHostOption_DoDaylightCycle, 0);
    gApp.SetGameHostOption(eGameHostOption_DoWeatherCycle, 0);

    bool enableVoting = true;
    if (data->mMiniGameId == eMiniGameId_Tumble)
        enableVoting = data->mMatchmakingFilters[1] == 3;
    data->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_EnableVoting, enableVoting);

    gApp.SetGameHostOption(eGameHostOption_NaturalRegeneration, data->mPlayerSettings.mNaturalRegeneration);
    gApp.SetGameHostOption(eGameHostOption_KeepInventory, HasFeatureKeepInventory());

    if (data->mHostMiniGame)
    {
        if (data->mMiniGameId == eMiniGameId_Tumble)
            data->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_NoSmallSkins, data->mMatchmaking);
        if (data->mMiniGameId == eMiniGameId_Glide && data->mHostMiniGame)
        {
            data->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_NoSmallSkins, 0);
            data->mPlayerSettings.mHostOptions.Set(eCustomGameModeHostOption_TimeTrial, data->mTimeTrial);
        }
    }

    SetPlayerSettings(data->mPlayerSettings);
    LeaveCriticalSection(&sPlayerSettingsMutex);
}

unsigned int MiniGameDef::CalculateMatchmakingData(bool joining, LaunchMoreOptionsMenuInitData* data,
                                                   unsigned int** values, unsigned int** ids) const
{
    if (data->mMiniGameId < eMiniGameId_Battle || data->mMiniGameId > eMiniGameId_Build)
        return 0;
    if (!joining)
    {
        *values = new unsigned int[3];
        (*values)[0] = data->mMatchmakingFilters[0];
        (*values)[1] = data->mMatchmakingFilters[1];
        return 3;
    }
    unsigned int count = (data->mMatchmakingFilters[0] != 0) + (data->mMatchmakingFilters[1] != 0);
    *values = new unsigned int[count];
    *ids = new unsigned int[count];
    unsigned int index = 0;
    if (data->mMatchmakingFilters[0] != 0)
    {
        (*values)[index] = data->mMatchmakingFilters[0];
        (*ids)[index++] = 0;
    }
    if (data->mMatchmakingFilters[1] != 0)
    {
        (*values)[index] = data->mMatchmakingFilters[1];
        (*ids)[index] = 1;
    }
    return count;
}

unsigned int MiniGameDef::CalculateJoiningMatchmakingData(LaunchMoreOptionsMenuInitData* data, unsigned int** values,
                                                          unsigned int** ids) const
{
    return CalculateMatchmakingData(true, data, values, ids);
}

unsigned int MiniGameDef::CalculateHostingMatchmakingData(LaunchMoreOptionsMenuInitData* data,
                                                          unsigned int** values) const
{
    if (!data->mMatchmaking || data->mTimeTrial)
        return 0;
    return CalculateMatchmakingData(false, data, values, NULL);
}

bool MiniGameDef::AllowsUGC() const
{
    return mAllowsUGC;
}

bool MiniGameDef::HasMultipleMaps() const
{
    EMiniGameId id = mId;
    if (id == eMiniGameId_Lobby)
    {
        CommonMasterGameMode* mode = CommonMasterGameMode::GetMasterOrClient();
        if (mode != NULL)
            id = mode->GetNextGameMode();
        if (id == eMiniGameId_Lobby)
            return false;
    }
    if (id == eMiniGameId_Tumble)
        return false;

    LevelRules::ELevelRuleType type = LevelRules::GetRuleTypeForMiniGame(id);
    std::vector<LevelRules::LevelRulesetGroup*> groups;
    gApp.getLevelRules()->getRulesByType(type, &groups, false);
    return groups.size() > 1;
}

bool MiniGameDef::HasMultipleVariants() const
{
    EMiniGameId id = mId;
    if (id == eMiniGameId_Lobby)
    {
        if (CommonMasterGameMode::GetMasterOrClient() != NULL)
            id = CommonMasterGameMode::GetMasterOrClient()->GetNextGameMode();
        if (id == eMiniGameId_Lobby)
            return false;
    }
    return GetCustomGameModeById(id, true)->GetLootSetCount() > 1;
}

unsigned int MiniGameDef::VotableType() const
{
    if (mId == eMiniGameId_Tumble)
        return 1;
    if (mId != eMiniGameId_Build)
    {
        if (HasMultipleMaps())
            return 0;
        if (HasMultipleVariants())
            return 1;
    }
    return 2;
}

bool MiniGameDef::HasRoundLengthOption() const
{
    return GetOptionsCount(eCustomGameModeHostOption_MapVariant) > 1;
}
bool MiniGameDef::HasGameVariants() const
{
    return GetOptionsCount(eCustomGameModeHostOption_ModeVariant) > 1;
}
bool MiniGameDef::HasCentralSpawnOption() const
{
    return mId == eMiniGameId_Battle;
}
bool MiniGameDef::HasTeamGameOption() const
{
    return mId == eMiniGameId_Build;
}
bool MiniGameDef::IsHUDDisabled() const
{
    return mId == eMiniGameId_Tumble || mId == eMiniGameId_Glide;
}
bool MiniGameDef::ArePortalsDisabled() const
{
    return true;
}
bool MiniGameDef::IsBatSqueakDisabled() const
{
    return mId == eMiniGameId_Glide;
}
unsigned int MiniGameDef::GetLayerCount() const
{
    unsigned int layerCount = _fromString<unsigned int>(
        getCurrentLootSet().getAttribute(sMaxLayerCountAttribute, L"1"));
    maybe<unsigned int> layerCountSetting = GetSetting<maybe<unsigned int>, &MiniGamePlayerSettings::mLayerCount>();
    if (layerCountSetting.isSet())
        layerCount = layerCountSetting.get();
    return layerCount;
}
double MiniGameDef::GetLayerScale(float value) const
{
    maybe<float> arenaSize = GetSetting<maybe<float>, &MiniGamePlayerSettings::mArenaSizeMultiplier>();
    return arenaSize.isSet() ? arenaSize.get() : value;
}
unsigned int MiniGameDef::GetMapSizeOverride() const
{
    maybe<int> mapSize = GetSetting<maybe<int>, &MiniGamePlayerSettings::mMapSizeOverride>();
    return mapSize.isSet() ? mapSize.get() : 0;
}

std::wstring MiniGameDef::GetMapSizeOverrideText(int value) const
{
    if (value == 0)
        return L10N::GetString(IDS_MG01_MAP_SIZE_AUTO);
    if (value == 1)
        return L10N::GetString(IDS_MG01_MAP_SIZE_SMALL);
    if (value == 2)
        return L10N::GetString(IDS_MG01_MAP_SIZE_LARGE);
    return L"";
}

std::wstring MiniGameDef::getTextureName() const
{
    if (mId != eMiniGameId_Lobby)
    {
        if (mId == eMiniGameId_Battle)
            return L"BattleModeIcon";
        if (mId < eMiniGameId_Glide)
            return L"MG02icon";
        if (mId == eMiniGameId_Glide)
            return L"MG03icon";
    }
    return L"MinecraftBroken";
}
std::wstring MiniGameDef::getIcon() const
{
    return std::wstring(L"Graphics\\") + getTextureName() + L".png";
}

bool MiniGameDef::HasFeatureLookBack() const
{
    return mId == eMiniGameId_Glide;
}
bool MiniGameDef::HasFeatureStartCameraAnimation() const
{
    return mId == eMiniGameId_Tumble;
}
bool MiniGameDef::HasFeatureFinishLineSoundEffect() const
{
    return mId == eMiniGameId_Glide;
}
bool MiniGameDef::HasFeatureCheckpoints() const
{
    return mId == eMiniGameId_Glide;
}
bool MiniGameDef::HasFeatureClouds() const
{
    return mId != eMiniGameId_Glide;
}
bool MiniGameDef::HasFeatureCustomBeacons() const
{
    return mId == eMiniGameId_Glide;
}
bool MiniGameDef::HasFeatureDirectionalFireworks() const
{
    return mId == eMiniGameId_Tumble;
}
bool MiniGameDef::HasFeatureFairExplosions() const
{
    return mId == eMiniGameId_Tumble;
}
bool MiniGameDef::HasFeatureReducedKnockback() const
{
    return mId == eMiniGameId_Tumble
           && getCurrentLootSet().getAttribute(sLootSetNameAttribute, L"") == sBowAndArrowLootSet;
}
bool MiniGameDef::HasFeatureArrowCanKnockback() const
{
    return mId == eMiniGameId_Tumble
           && getCurrentLootSet().getAttribute(sLootSetNameAttribute, L"") == sBowAndArrowLootSet;
}
bool MiniGameDef::HasFeatureSplashLevitatePotions() const
{
    return mId == eMiniGameId_Tumble
           && getCurrentLootSet().getAttribute(sLootSetNameAttribute, L"") == sLevitatePotionsLootSet;
}
bool MiniGameDef::HasFeatureShorterHitbox() const
{
    return mId == eMiniGameId_Tumble;
}
bool MiniGameDef::HasFeatureGhostThroughProjectiles() const
{
    return mId == eMiniGameId_Tumble
           && (getCurrentLootSet().getAttribute(sLootSetNameAttribute, L"") == sLevitatePotionsLootSet
               || getCurrentLootSet().getAttribute(sLootSetNameAttribute, L"") == sVoidTNTLootSet);
}
bool MiniGameDef::HasFeatureProjectilesBreakBlocks() const
{
    return mId == eMiniGameId_Tumble;
}
bool MiniGameDef::HasFeatureThrownProjectilesAreFireImmune() const
{
    return mId == eMiniGameId_Tumble;
}
bool MiniGameDef::HasFeatureAutoElytraFly() const
{
    return mId == eMiniGameId_Tumble
           && getCurrentLootSet().getAttribute(sLootSetNameAttribute, L"") == sBowAndArrowLootSet;
}
bool MiniGameDef::HasFeatureInvertedExplodingGrenades() const
{
    return mId == eMiniGameId_Tumble && getCurrentLootSet().getAttribute(sLootSetNameAttribute, L"") == sVoidTNTLootSet;
}
bool MiniGameDef::HasFeatureLessExplosionParticles() const
{
    return mId == eMiniGameId_Tumble;
}
bool MiniGameDef::HasFeatureElytraFlyingOnSpawn() const
{
    return mId == eMiniGameId_Glide;
}
bool MiniGameDef::HasFeatureOverrideHandSwing() const
{
    return mId != eMiniGameId_Any;
}
bool MiniGameDef::HasFeatureReducedParticles() const
{
    return mId == eMiniGameId_Tumble;
}
bool MiniGameDef::HasFeatureSnowballAwards() const
{
    return mId == eMiniGameId_Tumble
           && getCurrentLootSet().getAttribute(sLootSetNameAttribute, L"") == sSnowballLootSet;
}

int MiniGameDef::getCRC() const
{
    return mCrc;
}
bool MiniGameDef::AllowCrafting() const
{
    return mAllowCrafting;
}
bool MiniGameDef::GetNaturalRegeneration() const
{
    return GetSetting<bool, &MiniGamePlayerSettings::mNaturalRegeneration>();
}
int MiniGameDef::GetTeamMaxSize() const
{
    return GetSetting<unsigned int, &MiniGamePlayerSettings::mTeamMaxSize>();
}
int MiniGameDef::getProtection(int itemId, int value) const
{
    return atMaybe(mItemBalance.mProtection, itemId, value);
}
bool MiniGameDef::CanUseSmallSkins() const
{
    return GetSetting<bool, &MiniGamePlayerSettings::mSmallSkinsEnabled>();
}
void MiniGameDef::SetSmallSkinsEnabled(bool enabled) const
{
    SetSetting<bool, &MiniGamePlayerSettings::mSmallSkinsEnabled>(enabled);
}
int MiniGameDef::GetRoundLength() const
{
    return GetSetting<int, &MiniGamePlayerSettings::mRoundLength>();
}
bool MiniGameDef::GetTeamGame() const
{
    return GetSetting<bool, &MiniGamePlayerSettings::mTeamGame>();
}
void MiniGameDef::TimerSpec::SetChangeAnnounceAfterS(int value)
{
    mChangeAnnounceAfter = maybe<int>(value);
}
