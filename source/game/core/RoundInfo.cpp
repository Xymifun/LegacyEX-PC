#include "game/core/RoundInfo.hpp"
#include "game/util/Random.hpp"
#include "game/core/StringConversion.hpp"
#include "game/world/LevelGenerationOptions.hpp"

std::wstring CreateRoundGuid()
{
    return _toString<int>(Random().nextInt());
}

RoundInfo::RoundInfo() : mRoundId(L""), mSubRoundId(L"")
{
    mCurrentSubRoundNumber = -1;
    mThemeId = -1;
    mThemeSubId = -1;
}

void RoundInfo::reset()
{
    *this = RoundInfo();
}

void RoundInfo::startNewRound()
{
    mRoundId = CreateRoundGuid();
    mSubRoundId = CreateRoundGuid();
    mCurrentSubRoundNumber = 0;
}

void RoundInfo::startNewSubRound()
{
    mSubRoundId = CreateRoundGuid();
    ++mCurrentSubRoundNumber;
}

std::wstring RoundInfo::getRoundId() const
{
    return mRoundId;
}

std::wstring RoundInfo::getSubRoundId() const
{
    return mSubRoundId;
}

int RoundInfo::getCurrentSubRoundNumber() const
{
    return mCurrentSubRoundNumber;
}

unsigned int RoundInfo::getThemeId() const
{
    return mThemeId;
}

unsigned int RoundInfo::getThemeSubId() const
{
    return mThemeSubId;
}

unsigned int RoundInfo::getThemeUniqueId() const
{
    if (mThemeId >= 0 && mThemeSubId >= 0)
    {
        return LevelGenerationOptions::CreateUniqueId((int)mThemeId, (int)mThemeSubId);
    }

    return 0xffffffffU;
}

void RoundInfo::setTheme(int themeId, int themeSubId)
{
    mThemeId = themeId;
    mThemeSubId = themeSubId;
}

void RoundInfo::setUniqueThemeId(int uniqueThemeId)
{
    if (uniqueThemeId >= 0)
    {
        int themeId = LevelGenerationOptions::GetThemeFromUniqueId(uniqueThemeId);
        int themeSubId = LevelGenerationOptions::GetMapSizeFromUniqueId(uniqueThemeId);
        setTheme(themeId, themeSubId);
    }
    else
    {
        mThemeId = -1;
        mThemeSubId = -1;
    }
}

void RoundInfo::setRoundAndSubRound(const std::wstring& roundId, const std::wstring& subRoundId)
{
    mRoundId = roundId;
    mSubRoundId = subRoundId;
}
