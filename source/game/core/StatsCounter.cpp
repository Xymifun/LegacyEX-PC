#include "game/core/StatsCounter.hpp"

#include "game/core/GenericStats.hpp"
#include "game/core/LeaderboardManager.hpp"
#include "game/core/Stat.hpp"
#include "game/core/Stats.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"

#include <climits>
#include <cstring>

extern C_4JProfile* gProfile;
extern C4JStorage* gStorage;

boost::unordered_map<Stat*, boost::unordered_map<unsigned int, StatsCounter::LEADERBOARD_FLAG> >
    StatsCounter::statBoards;

StatsCounter::StatsCounter()
{
    mSavePending = 0;
    mSaveDelay = 0;
    mLeaderboardDelay = 0;
}

void StatsCounter::setupStatBoards()
{
    statBoards[Stats::killsZombie][0] = eLeaderboard0;
    statBoards[Stats::killsZombie][1] = eLeaderboard1;
    statBoards[Stats::killsZombie][2] = eLeaderboard2;
    statBoards[Stats::killsZombie][3] = eLeaderboard3;

    statBoards[Stats::killsSkeleton][0] = eLeaderboard0;
    statBoards[Stats::killsSkeleton][1] = eLeaderboard1;
    statBoards[Stats::killsSkeleton][2] = eLeaderboard2;
    statBoards[Stats::killsSkeleton][3] = eLeaderboard3;

    statBoards[Stats::killsCreeper][0] = eLeaderboard0;
    statBoards[Stats::killsCreeper][1] = eLeaderboard1;
    statBoards[Stats::killsCreeper][2] = eLeaderboard2;
    statBoards[Stats::killsCreeper][3] = eLeaderboard3;

    statBoards[Stats::killsSpider][0] = eLeaderboard0;
    statBoards[Stats::killsSpider][1] = eLeaderboard1;
    statBoards[Stats::killsSpider][2] = eLeaderboard2;
    statBoards[Stats::killsSpider][3] = eLeaderboard3;

    statBoards[Stats::killsSpiderJockey][0] = eLeaderboard0;
    statBoards[Stats::killsSpiderJockey][1] = eLeaderboard1;
    statBoards[Stats::killsSpiderJockey][2] = eLeaderboard2;
    statBoards[Stats::killsSpiderJockey][3] = eLeaderboard3;

    statBoards[Stats::killsZombiePigman][0] = eLeaderboard0;
    statBoards[Stats::killsZombiePigman][1] = eLeaderboard1;
    statBoards[Stats::killsZombiePigman][2] = eLeaderboard2;
    statBoards[Stats::killsZombiePigman][3] = eLeaderboard3;

    statBoards[Stats::killsNetherZombiePigman][0] = eLeaderboard0;
    statBoards[Stats::killsNetherZombiePigman][1] = eLeaderboard1;
    statBoards[Stats::killsNetherZombiePigman][2] = eLeaderboard2;
    statBoards[Stats::killsNetherZombiePigman][3] = eLeaderboard3;

    statBoards[Stats::killsSlime][0] = eLeaderboard0;
    statBoards[Stats::killsSlime][1] = eLeaderboard1;
    statBoards[Stats::killsSlime][2] = eLeaderboard2;
    statBoards[Stats::killsSlime][3] = eLeaderboard3;

    statBoards[Stats::blockMinedStats[Block::getId(Blocks::dirt)]][0] = eLeaderboard4;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::dirt)]][1] = eLeaderboard5;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::dirt)]][2] = eLeaderboard6;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::dirt)]][3] = eLeaderboard7;

    statBoards[Stats::blockMinedStats[Block::getId(Blocks::cobblestone)]][0] = eLeaderboard4;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::cobblestone)]][1] = eLeaderboard5;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::cobblestone)]][2] = eLeaderboard6;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::cobblestone)]][3] = eLeaderboard7;

    statBoards[Stats::blockMinedStats[Block::getId(Blocks::sand)]][0] = eLeaderboard4;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::sand)]][1] = eLeaderboard5;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::sand)]][2] = eLeaderboard6;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::sand)]][3] = eLeaderboard7;

    statBoards[Stats::blockMinedStats[Block::getId(Blocks::stone)]][0] = eLeaderboard4;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::stone)]][1] = eLeaderboard5;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::stone)]][2] = eLeaderboard6;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::stone)]][3] = eLeaderboard7;

    statBoards[Stats::blockMinedStats[Block::getId(Blocks::gravel)]][0] = eLeaderboard4;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::gravel)]][1] = eLeaderboard5;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::gravel)]][2] = eLeaderboard6;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::gravel)]][3] = eLeaderboard7;

    statBoards[Stats::blockMinedStats[Block::getId(Blocks::clay)]][0] = eLeaderboard4;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::clay)]][1] = eLeaderboard5;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::clay)]][2] = eLeaderboard6;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::clay)]][3] = eLeaderboard7;

    statBoards[Stats::blockMinedStats[Block::getId(Blocks::obsidian)]][0] = eLeaderboard4;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::obsidian)]][1] = eLeaderboard5;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::obsidian)]][2] = eLeaderboard6;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::obsidian)]][3] = eLeaderboard7;

    statBoards[Stats::itemCollectedStats[Item::getId(Items::egg)]][0] = eLeaderboard12;
    statBoards[Stats::itemCollectedStats[Item::getId(Items::egg)]][1] = eLeaderboard13;
    statBoards[Stats::itemCollectedStats[Item::getId(Items::egg)]][2] = eLeaderboard14;
    statBoards[Stats::itemCollectedStats[Item::getId(Items::egg)]][3] = eLeaderboard15;

    statBoards[Stats::blockMinedStats[Block::getId(Blocks::wheat)]][0] = eLeaderboard12;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::wheat)]][1] = eLeaderboard13;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::wheat)]][2] = eLeaderboard14;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::wheat)]][3] = eLeaderboard15;

    statBoards[Stats::blockMinedStats[Block::getId(Blocks::brown_mushroom)]][0] = eLeaderboard12;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::brown_mushroom)]][1] = eLeaderboard13;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::brown_mushroom)]][2] = eLeaderboard14;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::brown_mushroom)]][3] = eLeaderboard15;

    statBoards[Stats::blockMinedStats[Block::getId(Blocks::reeds)]][0] = eLeaderboard12;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::reeds)]][1] = eLeaderboard13;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::reeds)]][2] = eLeaderboard14;
    statBoards[Stats::blockMinedStats[Block::getId(Blocks::reeds)]][3] = eLeaderboard15;

    statBoards[Stats::cowsMilked][0] = eLeaderboard12;
    statBoards[Stats::cowsMilked][1] = eLeaderboard13;
    statBoards[Stats::cowsMilked][2] = eLeaderboard14;
    statBoards[Stats::cowsMilked][3] = eLeaderboard15;

    statBoards[Stats::itemCollectedStats[Block::getId(Blocks::pumpkin)]][0] = eLeaderboard12;
    statBoards[Stats::itemCollectedStats[Block::getId(Blocks::pumpkin)]][1] = eLeaderboard13;
    statBoards[Stats::itemCollectedStats[Block::getId(Blocks::pumpkin)]][2] = eLeaderboard14;
    statBoards[Stats::itemCollectedStats[Block::getId(Blocks::pumpkin)]][3] = eLeaderboard15;

    statBoards[Stats::walkOneM][0] = eLeaderboard16;
    statBoards[Stats::walkOneM][1] = eLeaderboard17;
    statBoards[Stats::walkOneM][2] = eLeaderboard18;
    statBoards[Stats::walkOneM][3] = eLeaderboard19;

    statBoards[Stats::fallOneM][0] = eLeaderboard16;
    statBoards[Stats::fallOneM][1] = eLeaderboard17;
    statBoards[Stats::fallOneM][2] = eLeaderboard18;
    statBoards[Stats::fallOneM][3] = eLeaderboard19;

    statBoards[Stats::minecartOneM][0] = eLeaderboard16;
    statBoards[Stats::minecartOneM][1] = eLeaderboard17;
    statBoards[Stats::minecartOneM][2] = eLeaderboard18;
    statBoards[Stats::minecartOneM][3] = eLeaderboard19;

    statBoards[Stats::boatOneM][0] = eLeaderboard16;
    statBoards[Stats::boatOneM][1] = eLeaderboard17;
    statBoards[Stats::boatOneM][2] = eLeaderboard18;
    statBoards[Stats::boatOneM][3] = eLeaderboard19;

    statBoards[Stats::mg03BestTime][4] = eLeaderboard37;
    statBoards[Stats::mg03BestTime][5] = eLeaderboard38;
    statBoards[Stats::mg03BestTime][6] = eLeaderboard39;
    statBoards[Stats::mg03BestTime][7] = eLeaderboard40;
    statBoards[Stats::mg03BestTime][8] = eLeaderboard41;
    statBoards[Stats::mg03BestTime][9] = eLeaderboard42;
    statBoards[Stats::mg03BestTime][10] = eLeaderboard43;
    statBoards[Stats::mg03BestTime][11] = eLeaderboard44;
    statBoards[Stats::mg03BestTime][12] = eLeaderboard45;
    statBoards[Stats::mg03BestTime][13] = eLeaderboard46;
    statBoards[Stats::mg03BestTime][14] = eLeaderboard47;
    statBoards[Stats::mg03BestTime][15] = eLeaderboard48;

    statBoards[Stats::mg03BestTimeScoreChallenge][4] = eLeaderboard25;
    statBoards[Stats::mg03BestTimeScoreChallenge][5] = eLeaderboard26;
    statBoards[Stats::mg03BestTimeScoreChallenge][6] = eLeaderboard27;
    statBoards[Stats::mg03BestTimeScoreChallenge][7] = eLeaderboard28;
    statBoards[Stats::mg03BestTimeScoreChallenge][8] = eLeaderboard29;
    statBoards[Stats::mg03BestTimeScoreChallenge][8] = eLeaderboard30;
    statBoards[Stats::mg03BestTimeScoreChallenge][10] = eLeaderboard31;
    statBoards[Stats::mg03BestTimeScoreChallenge][11] = eLeaderboard32;
    statBoards[Stats::mg03BestTimeScoreChallenge][12] = eLeaderboard33;
    statBoards[Stats::mg03BestTimeScoreChallenge][13] = eLeaderboard34;
    statBoards[Stats::mg03BestTimeScoreChallenge][14] = eLeaderboard35;
    statBoards[Stats::mg03BestTimeScoreChallenge][15] = eLeaderboard36;

    statBoards[Stats::mg03Score][4] = eLeaderboard25;
    statBoards[Stats::mg03Score][5] = eLeaderboard26;
    statBoards[Stats::mg03Score][6] = eLeaderboard27;
    statBoards[Stats::mg03Score][7] = eLeaderboard28;
    statBoards[Stats::mg03Score][8] = eLeaderboard29;
    statBoards[Stats::mg03Score][9] = eLeaderboard30;
    statBoards[Stats::mg03Score][10] = eLeaderboard31;
    statBoards[Stats::mg03Score][11] = eLeaderboard32;
    statBoards[Stats::mg03Score][12] = eLeaderboard33;
    statBoards[Stats::mg03Score][13] = eLeaderboard34;
    statBoards[Stats::mg03Score][14] = eLeaderboard35;
    statBoards[Stats::mg03Score][15] = eLeaderboard36;

    statBoards[Stats::mg03RingCount][4] = eLeaderboard25;
    statBoards[Stats::mg03RingCount][5] = eLeaderboard26;
    statBoards[Stats::mg03RingCount][6] = eLeaderboard27;
    statBoards[Stats::mg03RingCount][7] = eLeaderboard28;
    statBoards[Stats::mg03RingCount][8] = eLeaderboard29;
    statBoards[Stats::mg03RingCount][8] = eLeaderboard30;
    statBoards[Stats::mg03RingCount][10] = eLeaderboard31;
    statBoards[Stats::mg03RingCount][11] = eLeaderboard32;
    statBoards[Stats::mg03RingCount][12] = eLeaderboard33;
    statBoards[Stats::mg03RingCount][13] = eLeaderboard34;
    statBoards[Stats::mg03RingCount][14] = eLeaderboard35;
    statBoards[Stats::mg03RingCount][15] = eLeaderboard36;
}

unsigned int StatsCounter::isLargeStat(Stat* stat)
{
    static Stat** largeStats[] = {&Stats::walkOneM,     &Stats::swimOneM, &Stats::fallOneM, &Stats::climbOneM,
                                  &Stats::minecartOneM, &Stats::boatOneM, &Stats::pigOneM,  &Stats::portalsUsed};
    for (Stat*** largeStat = largeStats; largeStat != largeStats + sizeof(largeStats) / sizeof(largeStats[0]);
         ++largeStat)
    {
        if (**largeStat == stat)
        {
            return 1;
        }
    }
    return 0;
}

void StatsCounter::award(Stat* stat, ESubTypes subtype, unsigned int amount, bool accumulate)
{
    if (stat->isAchievement())
    {
        subtype = eSubtype0;
    }

    boost::unordered_map<Stat*, boost::unordered_map<unsigned int, unsigned int> >::iterator statIt = find(stat);
    if (statIt == end())
    {
        boost::unordered_map<unsigned int, unsigned int> values;
        values[static_cast<unsigned int>(subtype)] = amount;
        (*this)[stat] = values;
    }
    else if (accumulate)
    {
        statIt->second[static_cast<unsigned int>(subtype)] += amount;
        gStats->get_timePlayed();
        if (statIt->second[static_cast<unsigned int>(subtype)]
            < statIt->second[static_cast<unsigned int>(subtype)] - amount)
        {
            statIt->second[static_cast<unsigned int>(subtype)] = UINT_MAX;
        }
        if (statIt->second[static_cast<unsigned int>(subtype)] > USHRT_MAX && !isLargeStat(stat))
        {
            statIt->second[static_cast<unsigned int>(subtype)] = USHRT_MAX;
        }
    }
    else
    {
        statIt->second[static_cast<unsigned int>(subtype)] = amount;
    }

    mSavePending = 1;
    boost::unordered_map<Stat*, boost::unordered_map<unsigned int, LEADERBOARD_FLAG> >::iterator boardIt =
        statBoards.find(stat);
    if (boardIt != statBoards.end())
    {
        boost::unordered_map<unsigned int, LEADERBOARD_FLAG>::iterator flagIt = boardIt->second.find(
            static_cast<unsigned int>(subtype));
        if (flagIt != boardIt->second.end())
        {
            mLeaderboardPending[static_cast<unsigned int>(flagIt->second)] = true;
            if (mLeaderboardDelay == 0)
            {
                mLeaderboardDelay = 9000;
            }
        }
    }
}

int StatsCounter::getTotalValue(Stat* stat)
{
    boost::unordered_map<Stat*, boost::unordered_map<unsigned int, unsigned int> >::iterator statIt = find(stat);
    if (statIt != end())
    {
        int total = 0;
        for (unsigned int subtype = 0; subtype < statIt->second.size(); ++subtype)
        {
            total += statIt->second[subtype];
        }
        return total;
    }
    return 0;
}

void StatsCounter::save(int profile, bool force, unsigned char* data)
{
    if (!gProfile->IsFullVersion())
    {
        return;
    }

    unsigned char* buffer;
    if (data != NULL)
    {
        buffer = data;
    }
    else
    {
        buffer = gStorage->GetGameDefinedProfileData(profile);
    }

    unsigned char* cursor = buffer + PROFILE_STATS_OFFSET;
    std::memset(cursor, 0, PROFILE_STATS_SIZE);
    unsigned long long* woolBits = reinterpret_cast<unsigned long long*>(cursor);
    cursor += sizeof(unsigned long long);
    unsigned long long* biomeBits = reinterpret_cast<unsigned long long*>(cursor);
    cursor += sizeof(unsigned long long);
    unsigned char* armourBits = cursor;
    cursor += sizeof(unsigned char);
    unsigned int woolIndex = 0;
    unsigned int biomeIndex = 0;
    unsigned int armourIndex = 0;

    std::vector<Stat*>::iterator statsEnd = Stat::stats->end();
    for (std::vector<Stat*>::iterator it = Stat::stats->begin(); it != statsEnd; it++)
    {
        Stat* stat = *it;
        boost::unordered_map<Stat*, boost::unordered_map<unsigned int, unsigned int> >::iterator statIt = find(stat);

        if (stat->isAchievement())
        {
            continue;
        }
        else if (stat->isWoolStat())
        {
            if (statIt != end())
            {
                *woolBits |= static_cast<unsigned long long>(statIt->second[0] & 1) << woolIndex;
            }
            ++woolIndex;
        }
        else if (stat->isBiomeStat())
        {
            if (statIt != end())
            {
                *biomeBits |= static_cast<unsigned long long>(statIt->second[0] & 1) << biomeIndex;
            }
            ++biomeIndex;
        }
        else if (stat->isArmourStat())
        {
            if (statIt != end())
            {
                *armourBits |= (statIt->second[0] & 1) << armourIndex;
            }
            ++armourIndex;
        }
        else if (!isLargeStat(stat))
        {
            unsigned short* smallValues = reinterpret_cast<unsigned short*>(cursor);
            if (statIt != end())
            {
                smallValues[0] = statIt->second[0];
                smallValues[1] = statIt->second[1];
                smallValues[2] = statIt->second[2];
                smallValues[3] = statIt->second[3];
            }
            cursor += 4 * sizeof(unsigned short);
        }
        else
        {
            unsigned int* largeValues = reinterpret_cast<unsigned int*>(cursor);
            if (statIt != end())
            {
                largeValues[0] = statIt->second[0];
                largeValues[1] = statIt->second[1];
                largeValues[2] = statIt->second[2];
                largeValues[3] = statIt->second[3];
            }
            cursor += 4 * sizeof(unsigned int);
        }
    }

    if (data == NULL)
    {
        gStorage->WriteToProfile(profile, true, force);
    }

    mSaveDelay = SAVE_DELAY;
}

void StatsCounter::writeStats()
{
    gProfile->IsFullVersion();
}

void StatsCounter::flushLeaderboards()
{
    if (g_pLeaderboardManager->OpenSession())
    {
        writeStats();
        g_pLeaderboardManager->FlushStats();
    }
    mLeaderboardPending.clear();
}

void StatsCounter::tick(int profile)
{
    if (mSaveDelay > 0)
    {
        --mSaveDelay;
    }
    if (mSavePending != 0 && mSaveDelay == 0)
    {
        save(profile, false, NULL);
    }
    if (mLeaderboardDelay > 0)
    {
        --mLeaderboardDelay;
        if (mLeaderboardDelay == 0)
        {
            flushLeaderboards();
        }
    }
}

void StatsCounter::parse(void* data)
{
    unsigned char* cursor = static_cast<unsigned char*>(data) + PROFILE_STATS_OFFSET;
    unsigned long long* woolBits = reinterpret_cast<unsigned long long*>(cursor);
    cursor += sizeof(unsigned long long);
    unsigned long long* biomeBits = reinterpret_cast<unsigned long long*>(cursor);
    cursor += sizeof(unsigned long long);
    char* armourBits = reinterpret_cast<char*>(cursor);
    cursor += sizeof(char);
    unsigned int woolIndex = 0;
    unsigned int biomeIndex = 0;
    unsigned int armourIndex = 0;
    boost::unordered_map<unsigned int, unsigned int> values;
    std::vector<Stat*>::iterator statsEnd = Stat::stats->end();
    for (std::vector<Stat*>::iterator it = Stat::stats->begin(); it != statsEnd; ++it)
    {
        Stat* stat = *it;
        if (stat->isAchievement())
        {
            continue;
        }
        else if (stat->isWoolStat())
        {
            if (*woolBits != 0)
            {
                values[0] = 0;
                values[0] |= (*woolBits >> woolIndex) & 1;
                values[1] = 0;
                values[2] = 0;
                values[3] = 0;
                insert(std::make_pair(stat, values));
            }
            ++woolIndex;
        }
        else if (stat->isBiomeStat())
        {
            if (*biomeBits != 0)
            {
                values[0] = 0;
                values[0] |= (*biomeBits >> biomeIndex) & 1;
                values[1] = 0;
                values[2] = 0;
                values[3] = 0;
                insert(std::make_pair(stat, values));
            }
            ++biomeIndex;
        }
        else if (stat->isArmourStat())
        {
            if (*armourBits != 0)
            {
                values[0] = 0;
                values[0] |= (*armourBits >> armourIndex) & 1;
                values[1] = 0;
                values[2] = 0;
                values[3] = 0;
                insert(std::make_pair(stat, values));
            }
            ++armourIndex;
        }
        else
        {
            if (!isLargeStat(stat))
            {
                unsigned short* smallValues = reinterpret_cast<unsigned short*>(cursor);
                if (smallValues[0] != 0 || smallValues[1] != 0 || smallValues[2] != 0 || smallValues[3] != 0)
                {
                    values[0] = smallValues[0];
                    values[1] = smallValues[1];
                    values[2] = smallValues[2];
                    values[3] = smallValues[3];
                    insert(std::make_pair(stat, values));
                }
                cursor += 4 * sizeof(unsigned short);
            }
            else
            {
                unsigned int* largeValues = reinterpret_cast<unsigned int*>(cursor);
                if (largeValues[0] != 0 || largeValues[1] != 0 || largeValues[2] != 0 || largeValues[3] != 0)
                {
                    values[0] = largeValues[0];
                    values[1] = largeValues[1];
                    values[2] = largeValues[2];
                    values[3] = largeValues[3];
                    insert(std::make_pair(stat, values));
                }
                cursor += 4 * sizeof(unsigned int);
            }
        }
    }
}

void StatsCounter::parseOld(unsigned short, void* data)
{
    unsigned char* cursor = static_cast<unsigned char*>(data) + PROFILE_STATS_OFFSET;
    boost::unordered_map<unsigned int, unsigned int> values;
    std::vector<Stat*>::iterator statsEnd = Stat::stats->end();
    for (std::vector<Stat*>::iterator it = Stat::stats->begin(); it != statsEnd; it++)
    {
        if (!(*it)->isAchievement())
        {
            if (!isLargeStat(*it))
            {
                unsigned short* smallValues = reinterpret_cast<unsigned short*>(cursor);
                if (smallValues[0] != 0 || smallValues[1] != 0 || smallValues[2] != 0 || smallValues[3] != 0)
                {
                    values[0] = smallValues[0];
                    values[1] = smallValues[1];
                    values[2] = smallValues[2];
                    values[3] = smallValues[3];
                    insert(std::make_pair(*it, values));
                }
                cursor += 4 * sizeof(unsigned short);
            }
            else
            {
                unsigned int* largeValues = reinterpret_cast<unsigned int*>(cursor);
                if (largeValues[0] != 0 || largeValues[1] != 0 || largeValues[2] != 0 || largeValues[3] != 0)
                {
                    values[0] = largeValues[0];
                    values[1] = largeValues[1];
                    values[2] = largeValues[2];
                    values[3] = largeValues[3];
                    insert(std::make_pair(*it, values));
                }
                cursor += 4 * sizeof(unsigned int);
            }
        }
        else
        {
            unsigned short* achievementValue = reinterpret_cast<unsigned short*>(cursor);
            if (achievementValue[0] != 0)
            {
                values[0] = achievementValue[0];
                values[1] = 0;
                values[2] = 0;
                values[3] = 0;
                insert(std::make_pair(*it, values));
            }
            cursor += sizeof(unsigned short);
        }
    }
}

void StatsCounter::clear()
{
    boost::unordered_map<Stat*, boost::unordered_map<unsigned int, unsigned int> >::clear();
    mLeaderboardPending.clear();
}

void StatsCounter::saveLeaderboards()
{
    if (!gProfile->IsFullVersion())
    {
        return;
    }
    if (g_pLeaderboardManager->OpenSession())
    {
        writeStats();
        g_pLeaderboardManager->CloseSession();
    }
    mLeaderboardPending.clear();
}
