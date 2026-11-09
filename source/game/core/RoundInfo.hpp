#ifndef GAME_CORE_ROUNDINFO_HPP
#define GAME_CORE_ROUNDINFO_HPP

#include <string>
#include <stdint.h>

class RoundInfo
{
public:
    RoundInfo();

    void reset();
    void startNewRound();
    void startNewSubRound();

    std::wstring getRoundId() const;
    std::wstring getSubRoundId() const;
    int getCurrentSubRoundNumber() const;
    unsigned int getThemeId() const;
    unsigned int getThemeSubId() const;
    unsigned int getThemeUniqueId() const;

    void setTheme(int themeId, int themeSubId);
    void setUniqueThemeId(int uniqueThemeId);
    void setRoundAndSubRound(const std::wstring& roundId, const std::wstring& subRoundId);

private:
    std::wstring mRoundId;
    std::wstring mSubRoundId;
    int mCurrentSubRoundNumber;
    int mThemeId;
    int mThemeSubId;
};

#endif
