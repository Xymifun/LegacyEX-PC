#ifndef GAME_CORE_STAT_HPP
#define GAME_CORE_STAT_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/StatFormatter.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class DecimalFormat;
class LocalPlayer;

class Stat
{
public:
    class DefaultFormat : public StatFormatter
    {
    public:
        virtual ~DefaultFormat() {}

        virtual std::wstring format(int value);
    };

    class TimeFormatter : public StatFormatter
    {
    public:
        virtual ~TimeFormatter() {}

        virtual std::wstring format(int value);
    };

    class DistanceFormatter : public StatFormatter
    {
    public:
        virtual ~DistanceFormatter() {}

        virtual std::wstring format(int value);
    };

    static std::vector<Stat*>* stats;
    static boost::unordered_map<int, Stat*>* statsById;
    static StatFormatter* defaultFormatter;
    static StatFormatter* timeFormatter;
    static DistanceFormatter* distanceFormatter;
    static DecimalFormat* decimalFormat;

    Stat(int id, const std::wstring& name, StatFormatter* formatter);
    Stat(int id, const std::wstring& name);
    virtual ~Stat() {}

    virtual Stat* postConstruct(bool addToList);
    virtual bool isAchievement();
    virtual bool isWoolStat();
    virtual bool isBiomeStat();
    virtual bool isArmourStat();
    virtual void handleParamBlob(boost::shared_ptr<LocalPlayer> player, arrayWithLength<unsigned char> params);

    Stat* setWoolStat(bool woolStat);
    Stat* setBiomeStat(bool biomeStat);
    Stat* setArmourStat(bool armourStat);

    Stat* setExtended();
    bool isExtended() { return extended; }

    Stat* setAwardLocallyOnly();
    bool isAwardedFromServer() const;

    std::wstring format(int value);
    std::wstring toString();

    int id;

protected:
    void _init();

    std::wstring name;
    StatFormatter* formatter;
    bool armourStat;
    bool woolStat;
    bool biomeStat;
    bool extended;
    bool awardLocallyOnly;
};

#endif
