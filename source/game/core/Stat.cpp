#include "game/core/Stat.hpp"

#include "game/core/DecimalFormat.hpp"

#include <utility>
#include <wchar.h>

template <class T>
std::wstring _toString(T value);

StatFormatter* Stat::defaultFormatter = new DefaultFormat();
StatFormatter* Stat::timeFormatter = new TimeFormatter();
Stat::DistanceFormatter* Stat::distanceFormatter = new DistanceFormatter();
DecimalFormat* Stat::decimalFormat = new DecimalFormat(L"%0(3).2f");
boost::unordered_map<int, Stat*>* Stat::statsById = new boost::unordered_map<int, Stat*>();
std::vector<Stat*>* Stat::stats = new std::vector<Stat*>();

Stat::Stat(int id, const std::wstring& name, StatFormatter* formatter) : id(id), name(name), formatter(formatter)
{
    _init();
}

Stat::Stat(int id, const std::wstring& name) : id(id), name(name), formatter(defaultFormatter)
{
    _init();
}

void Stat::_init()
{
    awardLocallyOnly = false;
    woolStat = false;
    armourStat = false;
    biomeStat = false;
    extended = false;
}

Stat* Stat::postConstruct(bool addToList)
{
    if (addToList)
    {
        stats->push_back(this);
    }

    std::pair<int, Stat*> entry(id, this);
    statsById->emplace(entry);

    return this;
}

bool Stat::isAchievement()
{
    return false;
}

bool Stat::isWoolStat()
{
    return woolStat;
}

bool Stat::isBiomeStat()
{
    return biomeStat;
}

bool Stat::isArmourStat()
{
    return armourStat;
}

void Stat::handleParamBlob(boost::shared_ptr<LocalPlayer> player, arrayWithLength<unsigned char> params)
{
}

Stat* Stat::setWoolStat(bool woolStat)
{
    this->woolStat = woolStat;
    return this;
}

Stat* Stat::setBiomeStat(bool biomeStat)
{
    this->biomeStat = biomeStat;
    return this;
}

Stat* Stat::setArmourStat(bool armourStat)
{
    this->armourStat = armourStat;
    return this;
}

Stat* Stat::setExtended()
{
    extended = true;
    return this;
}


Stat* Stat::setAwardLocallyOnly()
{
    awardLocallyOnly = true;
    return this;
}

bool Stat::isAwardedFromServer() const
{
    return !awardLocallyOnly;
}

std::wstring Stat::format(int value)
{
    return formatter->format(value);
}

std::wstring Stat::DefaultFormat::format(int value)
{
    wchar_t buffer[256];

    swprintf(buffer, 256, L"%d", value);

    std::wstring text(buffer);

    return text;
}

std::wstring Stat::TimeFormatter::format(int value)
{
    double seconds = (double)value / 20.0;
    double minutes = seconds / 60.0;
    double hours = minutes / 60.0;
    double days = hours / 24.0;
    double years = days / 365.0;

    if (years > 0.5)
    {
        return Stat::decimalFormat->format(years) + L" y";
    }
    else if (days > 0.5)
    {
        return Stat::decimalFormat->format(days) + L" d";
    }
    else if (hours > 0.5)
    {
        return Stat::decimalFormat->format(hours) + L" h";
    }
    else if (minutes > 0.5)
    {
        return Stat::decimalFormat->format(minutes) + L" m";
    }
    else
    {
        return _toString(seconds) + L" s";
    }
}

std::wstring Stat::DistanceFormatter::format(int value)
{
    double metres = (double)value / 100.0;
    double kilometres = metres / 1000.0;

    if (kilometres > 0.5)
    {
        return Stat::decimalFormat->format(kilometres) + L" km";
    }
    else if (metres > 0.5)
    {
        return Stat::decimalFormat->format(metres) + L" m";
    }
    else
    {
        return _toString(value) + L" cm";
    }
}
