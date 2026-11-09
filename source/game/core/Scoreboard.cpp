#include "game/core/Scoreboard.hpp"

#include <cstring>

#include <boost/unordered_set.hpp>

#include "game/core/DamageSource.hpp"
#include "game/core/DataInputStream.hpp"
#include "game/core/DataOutputStream.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Potion.hpp"
#include "game/item/PotionItem.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/item/Potions.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/GameMedal.hpp"
#include "game/minigame/MiniGameMedals.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/generation/PlayerTeam.hpp"

Scoreboard::Scoreboard() : mTeamsByName(11), mTeamsByPlayer(11), mCurrentRound(0)
{
}

Scoreboard::~Scoreboard()
{
    removeAllPlayerTeams();
}

PlayerTeam* Scoreboard::getPlayerTeam(const std::wstring& name)
{
    boost::unordered_map<std::wstring, PlayerTeam*>::iterator team = mTeamsByName.find(name);

    if (team != mTeamsByName.end())
    {
        return team->second;
    }

    return NULL;
}

PlayerTeam* Scoreboard::addPlayerTeam(const std::wstring& name)
{
    PlayerTeam* pTeam = getPlayerTeam(name);

    if (pTeam == NULL)
    {
        pTeam = new PlayerTeam(this, name, mTeamsByName.size());
        mTeamsByName[name] = pTeam;
        onTeamAdded(pTeam);
    }

    return pTeam;
}

void Scoreboard::removePlayerTeam(PlayerTeam* pTeam)
{
    boost::unordered_map<std::wstring, PlayerTeam*>::iterator team = mTeamsByName.find(pTeam->getName());

    if (team != mTeamsByName.end())
    {
        mTeamsByName.erase(team);

        typedef boost::unordered_set<PlayerUID, boost::hash<PlayerUID>, std::equal_to<PlayerUID> > PlayerSet;

        PlayerSet::const_iterator player;

        for (player = pTeam->getPlayers()->begin(); player != pTeam->getPlayers()->end(); player++)
        {
            mTeamsByPlayer.erase(*player);
        }

        onTeamRemoved(pTeam);
        delete pTeam;
    }
}

void Scoreboard::removeAllPlayerTeams()
{
    while (!mTeamsByName.empty())
    {
        removePlayerTeam(mTeamsByName.begin()->second);
    }
}

PlayerTeam* Scoreboard::getPlayersTeam(PlayerUID uid)
{
    PlayerTeamByUIDMap::iterator team = mTeamsByPlayer.find(uid);

    if (team != mTeamsByPlayer.end())
    {
        return team->second;
    }

    return NULL;
}

bool Scoreboard::removePlayerFromTeam(PlayerUID uid)
{
    PlayerTeam* pTeam = getPlayersTeam(uid);

    if (pTeam != NULL)
    {
        removePlayerFromTeam(uid, pTeam);
        return true;
    }

    return false;
}

void Scoreboard::addPlayerToTeam(PlayerUID uid, PlayerTeam* pTeam)
{
    if (getPlayersTeam(uid) != NULL)
    {
        removePlayerFromTeam(uid);
    }

    mTeamsByPlayer[uid] = pTeam;
    pTeam->getPlayers()->insert(uid);
}

void Scoreboard::removePlayerFromTeam(PlayerUID uid, PlayerTeam* pTeam)
{
    if (getPlayersTeam(uid) == pTeam)
    {
        mTeamsByPlayer.erase(uid);
        pTeam->getPlayers()->erase(uid);
    }
}

std::vector<PlayerTeam*>* Scoreboard::getPlayerTeams()
{
    std::vector<PlayerTeam*>* pTeams = new std::vector<PlayerTeam*>;
    pTeams->reserve(mTeamsByName.size());

    for (boost::unordered_map<std::wstring, PlayerTeam*>::iterator team = mTeamsByName.begin();
         team != mTeamsByName.end(); ++team)
    {
        pTeams->push_back(team->second);
    }

    return pTeams;
}

PlayerTeam* Scoreboard::getTeamByIndex(int index) const
{
    for (boost::unordered_map<std::wstring, PlayerTeam*>::const_iterator team = mTeamsByName.begin();
         team != mTeamsByName.end(); ++team)
    {
        PlayerTeam* pTeam = team->second;

        if (pTeam->getTeamIndex() == index)
        {
            return pTeam;
        }
    }

    return NULL;
}

boost::shared_ptr<ServerPlayer> Scoreboard::getPlayer(PlayerUID uid)
{
    boost::shared_ptr<ServerPlayer> player = gMinecraftServer->getPlayers()->getPlayer(uid);
    return player;
}

void Scoreboard::onTeamAdded(PlayerTeam* pTeam)
{
}

void Scoreboard::onTeamChanged(PlayerTeam* pTeam)
{
}

void Scoreboard::onTeamRemoved(PlayerTeam* pTeam)
{
}

void Scoreboard::syncTeamChanges(PlayerTeam* pTeam)
{
}

Scoreboard::EDamageSource Scoreboard::GetDamageSource(const DamageSource* pSource)
{
    EDamageSource source;

    if (pSource->isExplosion())
        source = DAMAGE_SOURCE_EXPLOSION;
    else if (pSource->equals(DamageSource::IN_FIRE) || pSource->equals(DamageSource::ON_FIRE))
        source = DAMAGE_SOURCE_FIRE;
    else if (pSource->equals(DamageSource::LAVA))
        source = DAMAGE_SOURCE_LAVA;
    else if (pSource->equals(DamageSource::DROWN))
        source = DAMAGE_SOURCE_DROWN;
    else if (pSource->equals(DamageSource::STARVE))
        source = DAMAGE_SOURCE_STARVE;
    else if (pSource->equals(DamageSource::CACTUS))
        source = DAMAGE_SOURCE_CACTUS;
    else if (pSource->equals(DamageSource::FALL) || pSource->equals(DamageSource::OUT_OF_WORLD)
             || pSource->equals(DamageSource::CREATIVE_PLAYER))
        source = DAMAGE_SOURCE_FALL;
    else if (pSource->equals(DamageSource::THORNS))
        source = DAMAGE_SOURCE_THORNS;
    else if (pSource == DamageSource::POISON)
        source = DAMAGE_SOURCE_POISON;
    else if (pSource == DamageSource::INSTANT_DAMAGE)
        source = DAMAGE_SOURCE_HARM;
    else if (pSource->equals(DamageSource::MAGIC))
        source = DAMAGE_SOURCE_MAGIC;
    else if (pSource->equals(DamageSource::ARROW))
        source = DAMAGE_SOURCE_ARROW;
    else if (pSource->equals(DamageSource::FLY_INTO_WALL))
        source = DAMAGE_SOURCE_FLY_INTO_WALL;
    else if (pSource->getEntity())
        source = DAMAGE_SOURCE_MELEE;
    else
        source = DAMAGE_SOURCE_UNKNOWN;

    return source;
}

Potion* Scoreboard::GetPotionByDamageSource(EDamageSource damageSource)
{
    Potion* pPotion = NULL;

    switch (damageSource)
    {
    case DAMAGE_SOURCE_HARM:
        pPotion = Potions::HARMING;
        break;

    case DAMAGE_SOURCE_POISON:
        pPotion = Potions::POISON;
        break;
    }

    return pPotion;
}

Scoreboard::Weapon::Weapon()
{
    mItemId = -1;
    mPotionId = -1;
}

Scoreboard::Weapon::Weapon(int itemId)
{
    mItemId = itemId;
    mPotionId = -1;
}

Scoreboard::Weapon::Weapon(int itemId, Potion* pPotion)
{
    mItemId = itemId;
    mPotionId = Potion::getId(pPotion);
}

uint32_t Scoreboard::Weapon::GetKey() const
{
    return (mPotionId << 16) | mItemId;
}

bool Scoreboard::Weapon::HasItem() const
{
    return mItemId != -1;
}

Item* Scoreboard::Weapon::GetItem() const
{
    return Item::byId(mItemId);
}

not_null_ptr<ItemInstance> Scoreboard::Weapon::GetItemInstance() const
{
    Item* pItem = Item::byId(mItemId);
    Potion* pPotion;

    if (pItem != NULL && dynamic_cast<PotionItem*>(pItem) != NULL && mPotionId >= 0
        && (pPotion = Potion::byId(mPotionId)) != NULL)
    {
        return PotionUtils::setPotion(not_null_ptr<ItemInstance>(new ItemInstance(pItem)), pPotion);
    }

    return not_null_ptr<ItemInstance>(new ItemInstance(pItem, 1));
}

Scoreboard::PlayerStats::PlayerStats()
{
    static_cast<StatsUID&>(*this) = StatsUID::EMPTY_SUID();
    mName = L"";
    mMedal = NULL;
    mRoundStats = RoundStatsMap();
}

StatsUID Scoreboard::PlayerStats::getRecordSUID()
{
    return static_cast<StatsUID&>(*this);
}

void Scoreboard::PlayerStats::setRecordSUID(const StatsUID& suid)
{
    StatsUID::operator=(suid);
}

std::wstring Scoreboard::PlayerStats::getRecord(eStringArgIndex index, int round, int life)
{
    _RoundStats& roundStats = mRoundStats[round];

    switch (index)
    {
    case eStringArg_PlayerName:
        return mName;

    case eStringArg_KillerName:
        if ((unsigned int)life < roundStats.mPerLife.size())
        {
            return roundStats.mPerLife[life].mKillerName;
        }

        return L"";
    }

    return L"";
}

bool Scoreboard::PlayerStats::getRecord(eBoolArgIndex index, int round)
{
    _RoundStats& roundStats = mRoundStats[round];

    switch (index)
    {
    case eBoolArg_Active:
        return roundStats.mActive;

    case eBoolArg_Finished:
        return roundStats.mFinished;

    case eBoolArg_Winner:
        return roundStats.mWinner;
    }

    return false;
}

float Scoreboard::PlayerStats::getRecord(eFloatArgIndex index, int round)
{
    _RoundStats& roundStats = mRoundStats[round];

    switch (index)
    {
    case eFloatArg_DistanceTravelled:
        return (float)roundStats.mDistanceTravelled;

    case eFloatArg_LongestShot:
        return (float)roundStats.mLongestShot;

    case eFloatArg_FallDistance:
        return (float)roundStats.mFallDistance;

    case eFloatArg_BestTime:
        return (float)roundStats.mBestTime;
    }

    return 0.0f;
}

int Scoreboard::PlayerStats::getRecord(eIntArgIndex index, int round)
{
    _RoundStats& roundStats = mRoundStats[round];

    switch (index)
    {
    case eIntArg_MedalId:
        if (mMedal != NULL)
        {
            return mMedal->GetId();
        }
        return -1;

    case eIntArg_FavouriteWeaponItem:
        return mFavouriteWeapon.mItemId;

    case eIntArg_FavouriteWeaponPotion:
        return mFavouriteWeapon.mPotionId;

    case eIntArg_Score:
        return roundStats.mScore;

    case eIntArg_Placement:
        return roundStats.mPlacement;

    case eIntArg_KillerWeaponItem:
        return roundStats.mKillerWeapon.mItemId;

    case eIntArg_KillerWeaponPotion:
        return roundStats.mKillerWeapon.mPotionId;

    case eIntArg_Kills:
        return roundStats.mKills;

    case eIntArg_Deaths:
        return roundStats.mDeaths;

    case eIntArg_Lives:
        return roundStats.mLives;

    case eIntArg_DamageDealt:
        return roundStats.mDamageDealt;

    case eIntArg_DamageTaken:
        return roundStats.mDamageTaken;

    case eIntArg_ArrowsFired:
        return roundStats.mArrowsFired;

    case eIntArg_ArrowsHit:
        return roundStats.mArrowsHit;

    case eIntArg_BlocksPlaced:
        return roundStats.mBlocksPlaced;

    case eIntArg_MobKills:
        return roundStats.mMobKills;

    case eIntArg_ItemsCollected:
        return roundStats.mItemsCollected;

    case eIntArg_BlocksMined:
        return roundStats.mBlocksMined;

    case eIntArg_TimePlayed:
        return roundStats.mTimePlayed;

    case eIntArg_TotalTime:
        return roundStats.mTotalTime;

    case eIntArg_LastCheckpointId:
        return roundStats.mLastCheckpointId;

    case eIntArg_LastBankedCheckpointId:
        return roundStats.mLastBankedCheckpointId;

    case eIntArg_CheckpointTime:
        if (roundStats.mCheckpointTimes.find(roundStats.mLastCheckpointId) != roundStats.mCheckpointTimes.end())
        {
            return roundStats.mCheckpointTimes.at(roundStats.mLastCheckpointId);
        }
        return -1;

    case eIntArg_LastCheckpointSequenceId:
        return roundStats.mLastCheckpointSequenceId;

    case eIntArg_LapsCompleted:
        return roundStats.mLapsCompleted;

    case eIntArg_CheckpointsPassed:
        return roundStats.mCheckpointsPassed;

    case eIntArg_KillsMelee:
        return roundStats.mScoresReceived[0];

    case eIntArg_KillsRanged:
        return roundStats.mScoresReceived[1];

    case eIntArg_KillsExplosive:
        return roundStats.mScoresReceived[2];

    case eIntArg_KillsOther:
        return roundStats.mScoresReceived[3];

    case eIntArg_DeathsMelee:
        return roundStats.mScoresGiven[0];

    case eIntArg_DeathsRanged:
        return roundStats.mScoresGiven[1];

    case eIntArg_DeathsExplosive:
        return roundStats.mScoresGiven[2];

    case eIntArg_DeathsOther:
        return roundStats.mScoresGiven[3];

    case eIntArg_WeaknessWeaponItem:
        return mWeaknessWeapon.mItemId;

    case eIntArg_WeaknessWeaponPotion:
        return mWeaknessWeapon.mPotionId;

    default:
        return 0;
    }
}

long long Scoreboard::PlayerStats::getRecord(eInt64ArgIndex index, int round)
{
    return 0;
}

void Scoreboard::PlayerStats::setRecord(eStringArgIndex index, int round, int life, std::wstring value)
{
    _RoundStats& roundStats = mRoundStats[round];

    switch (index)
    {
    case eStringArg_PlayerName:
        mName = value;
        break;

    case eStringArg_KillerName:
        roundStats.mPerLife[life].mKillerName = value;
        break;
    }
}

void Scoreboard::PlayerStats::setRecord(eBoolArgIndex index, int round, bool value)
{
    _RoundStats& roundStats = mRoundStats[round];

    switch (index)
    {
    case eBoolArg_Active:
        roundStats.mActive = value;
        break;

    case eBoolArg_Finished:
        roundStats.mFinished = value;
        break;

    case eBoolArg_Winner:
        roundStats.mWinner = value;
        break;
    }
}

void Scoreboard::PlayerStats::setRecord(eFloatArgIndex index, int round, float value)
{
    _RoundStats& roundStats = mRoundStats[round];

    switch (index)
    {
    case eFloatArg_DistanceTravelled:
        roundStats.mDistanceTravelled = value;
        break;

    case eFloatArg_LongestShot:
        roundStats.mLongestShot = value;
        break;

    case eFloatArg_FallDistance:
        roundStats.mFallDistance = value;
        break;

    case eFloatArg_BestTime:
        roundStats.mBestTime = value;
        break;
    }
}

void Scoreboard::PlayerStats::setRecord(eIntArgIndex index, int round, int value)
{
    _RoundStats& roundStats = mRoundStats[round];

    switch (index)
    {
    case eIntArg_MedalId:
        mMedal = MiniGameMedals::getMedalById(value);
        break;

    case eIntArg_FavouriteWeaponItem:
        mFavouriteWeapon.mItemId = value;
        break;

    case eIntArg_FavouriteWeaponPotion:
        mFavouriteWeapon.mPotionId = value;
        break;

    case eIntArg_Score:
        roundStats.mScore = value;
        break;

    case eIntArg_Placement:
        roundStats.mPlacement = value;
        break;

    case eIntArg_KillerWeaponItem:
        roundStats.mKillerWeapon.mItemId = value;
        break;

    case eIntArg_KillerWeaponPotion:
        roundStats.mKillerWeapon.mPotionId = value;
        break;

    case eIntArg_Kills:
        roundStats.mKills = value;
        break;

    case eIntArg_Deaths:
        roundStats.mDeaths = value;
        break;

    case eIntArg_Lives:
        roundStats.mLives = value;
        break;

    case eIntArg_DamageDealt:
        roundStats.mDamageDealt = value;
        break;

    case eIntArg_DamageTaken:
        roundStats.mDamageTaken = value;
        break;

    case eIntArg_ArrowsFired:
        roundStats.mArrowsFired = value;
        break;

    case eIntArg_ArrowsHit:
        roundStats.mArrowsHit = value;
        break;

    case eIntArg_BlocksPlaced:
        roundStats.mBlocksPlaced = value;
        break;

    case eIntArg_MobKills:
        roundStats.mMobKills = value;
        break;

    case eIntArg_ItemsCollected:
        roundStats.mItemsCollected = value;
        break;

    case eIntArg_BlocksMined:
        roundStats.mBlocksMined = value;
        break;

    case eIntArg_TimePlayed:
        roundStats.mTimePlayed = value;
        break;

    case eIntArg_TotalTime:
        roundStats.mTotalTime = value;
        break;

    case eIntArg_LastCheckpointId:
        roundStats.mLastCheckpointId = value;
        break;

    case eIntArg_LastBankedCheckpointId:
        roundStats.mLastBankedCheckpointId = value;
        break;

    case eIntArg_CheckpointTime:
        if (value > 0)
        {
            roundStats.mCheckpointTimes[roundStats.mLastCheckpointId] = value;
        }
        break;

    case eIntArg_LastCheckpointSequenceId:
        roundStats.mLastCheckpointSequenceId = value;
        break;

    case eIntArg_LapsCompleted:
        roundStats.mLapsCompleted = value;
        break;

    case eIntArg_CheckpointsPassed:
        roundStats.mCheckpointsPassed = value;
        break;

    case eIntArg_KillsMelee:
        roundStats.mScoresReceived[0] = (unsigned short)value;
        break;

    case eIntArg_KillsRanged:
        roundStats.mScoresReceived[1] = (unsigned short)value;
        break;

    case eIntArg_KillsExplosive:
        roundStats.mScoresReceived[2] = (unsigned short)value;
        break;

    case eIntArg_KillsOther:
        roundStats.mScoresReceived[3] = (unsigned short)value;
        break;

    case eIntArg_DeathsMelee:
        roundStats.mScoresGiven[0] = (unsigned short)value;
        break;

    case eIntArg_DeathsRanged:
        roundStats.mScoresGiven[1] = (unsigned short)value;
        break;

    case eIntArg_DeathsExplosive:
        roundStats.mScoresGiven[2] = (unsigned short)value;
        break;

    case eIntArg_DeathsOther:
        roundStats.mScoresGiven[3] = (unsigned short)value;
        break;

    case eIntArg_WeaknessWeaponItem:
        mWeaknessWeapon.mItemId = value;
        break;

    case eIntArg_WeaknessWeaponPotion:
        mWeaknessWeapon.mPotionId = value;
        break;

    default:
        break;
    }
}

void Scoreboard::PlayerStats::setRecord(eInt64ArgIndex, int, long long)
{
}

bool Scoreboard::PlayerStats::isRecordSame(PlayerStats* pOther, int record, int round, int life)
{
    if (record == eRecord_SUID)
    {
        return getRecordSUID() == pOther->getRecordSUID();
    }

    record -= eRecord_StringFirst;

    if (record < eStringArg_Count)
    {
        return getRecord((eStringArgIndex)record, round, life)
               == pOther->getRecord((eStringArgIndex)record, round, life);
    }

    record -= eStringArg_Count;

    if (record < eBoolArg_Count)
    {
        return getRecord((eBoolArgIndex)record, round) == pOther->getRecord((eBoolArgIndex)record, round);
    }

    record -= eBoolArg_Count;

    if (record < eFloatArg_Count)
    {
        return getRecord((eFloatArgIndex)record, round) == pOther->getRecord((eFloatArgIndex)record, round);
    }

    record -= eFloatArg_Count;

    if (record < eIntArg_Count)
    {
        return getRecord((eIntArgIndex)record, round) == pOther->getRecord((eIntArgIndex)record, round);
    }

    record -= eIntArg_Count;

    if (record < eInt64Arg_Count)
    {
        return getRecord((eInt64ArgIndex)record, round) == pOther->getRecord((eInt64ArgIndex)record, round);
    }

    return false;
}

unsigned long long Scoreboard::PlayerStats::getChangedBitfield(PlayerStats* pOther, int round, int life)
{
    unsigned long long changed = 0;

    for (int record = 0; record < eRecord_Count; record++)
    {
        if (!isRecordSame(pOther, record, round, life))
        {
            changed |= 1LL << (unsigned int)record;
        }
    }

    return changed;
}

void Scoreboard::PlayerStats::updateFromOtherStats(PlayerStats* pOther, int round, int life, unsigned long long changed)
{
    for (unsigned long long record = 0; record < eRecord_Count; record++)
    {
        if (((changed >> record) & 1) == 0)
        {
            continue;
        }

        unsigned long long index = record;

        if (record == eRecord_SUID)
        {
            setRecordSUID(pOther->getRecordSUID());
            continue;
        }

        index -= eRecord_StringFirst;

        if ((unsigned int)life >= mRoundStats[round].mPerLife.size())
        {
            int missing = (int)mRoundStats[round].mPerLife.size() - life + 1;

            for (int i = 0; i < missing; i++)
            {
                mRoundStats[round].mPerLife.push_back(_RoundStats::_PerLife());
            }
        }

        if (index < eStringArg_Count)
        {
            setRecord((eStringArgIndex)index, round, life, pOther->getRecord((eStringArgIndex)index, round, life));
            continue;
        }

        index -= eStringArg_Count;

        if (index < eBoolArg_Count)
        {
            setRecord((eBoolArgIndex)index, round, pOther->getRecord((eBoolArgIndex)index, round));
            continue;
        }

        index -= eBoolArg_Count;

        if (index < eFloatArg_Count)
        {
            setRecord((eFloatArgIndex)index, round, pOther->getRecord((eFloatArgIndex)index, round));
            continue;
        }

        index -= eFloatArg_Count;

        if (index < eIntArg_Count)
        {
            setRecord((eIntArgIndex)index, round, pOther->getRecord((eIntArgIndex)index, round));
        }
    }
}

void Scoreboard::PlayerStats::readRecord(DataInputStream* pInput, int record, int round, int life)
{
    if (record == eRecord_SUID)
    {
        setRecordSUID(StatsUID(pInput->readPlayerUID()));
        return;
    }

    record -= eRecord_StringFirst;

    if ((unsigned int)life >= mRoundStats[round].mPerLife.size())
    {
        int missing = life - (int)mRoundStats[round].mPerLife.size() + 1;

        for (int i = 0; i < missing; i++)
        {
            mRoundStats[round].mPerLife.push_back(_RoundStats::_PerLife());
        }
    }

    if (record < eStringArg_Count)
    {
        setRecord((eStringArgIndex)record, round, life, pInput->readUTF());
        return;
    }

    record -= eStringArg_Count;

    if (record < eBoolArg_Count)
    {
        setRecord((eBoolArgIndex)record, round, pInput->readBoolean());
        return;
    }

    record -= eBoolArg_Count;

    if (record < eFloatArg_Count)
    {
        setRecord((eFloatArgIndex)record, round, pInput->readFloat());
        return;
    }

    record -= eFloatArg_Count;

    if (record < eIntArg_Count)
    {
        setRecord((eIntArgIndex)record, round, pInput->readVarInt());
        return;
    }

    record -= eIntArg_Count;

    if (record < eInt64Arg_Count)
    {
        setRecord((eInt64ArgIndex)record, round, pInput->readLong());
    }
}

void Scoreboard::PlayerStats::writeRecord(DataOutputStream* pOutput, int record, int round, int life)
{
    if (record == eRecord_SUID)
    {
        pOutput->writePlayerUID(getRecordSUID().GetID());
        return;
    }

    record -= eRecord_StringFirst;

    if (record < eStringArg_Count)
    {
        pOutput->writeUTF(getRecord((eStringArgIndex)record, round, life));
        return;
    }

    record -= eStringArg_Count;

    if (record < eBoolArg_Count)
    {
        pOutput->writeBoolean(getRecord((eBoolArgIndex)record, round));
        return;
    }

    record -= eBoolArg_Count;

    if (record < eFloatArg_Count)
    {
        pOutput->writeFloat(getRecord((eFloatArgIndex)record, round));
        return;
    }

    record -= eFloatArg_Count;

    if (record < eIntArg_Count)
    {
        pOutput->writeVarInt(getRecord((eIntArgIndex)record, round));
        return;
    }

    record -= eIntArg_Count;

    if (record < eInt64Arg_Count)
    {
        pOutput->writeLong(getRecord((eInt64ArgIndex)record, round));
    }
}

Scoreboard::PlayerStats::_RoundStats::_RoundStats()
{
    mQuit = false;
    mWinner = false;
    mActive = false;
    mPlacement = Minecraft::InMiniGame(eMiniGameId_Glide, true) ? -1 : 0;
    mScore = 0;
    mKills = 0;
    mBlocksMined = 0;
    mBestTime = 0.0;
    mDistanceTravelled = 0.0;
    mLongestShot = 0.0;
    mFallDistance = 0.0;
    mLapsCompleted = 0;
    mTimePlayed = 0;
    mTotalTime = 0;
    mItemsCollected = 0;
    mLastCheckpointId = 0;
    mMobKills = 0;
    mLastCheckpointSequenceId = 0;
    mArrowsHit = 0;
    mLastBankedCheckpointId = 0;
    mArrowsFired = 0;
    mCheckpointsPassed = 0;
    mDamageTaken = 0;
    mBlocksPlaced = 0;
    mDamageDealt = 0;
    mDeaths = 0;
    mCheckpointTimes = boost::unordered_map<int, int>();
    memset(mScoresGiven, 0, sizeof(mScoresGiven));
    memset(mScoresReceived, 0, sizeof(mScoresReceived));
}
