#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ROUTINGTABLE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ROUTINGTABLE_HPP

#include <functional>
#include <map>
#include <utility>

#include "nn/nex/InetAddress.hpp"
#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class RoutingTable : public RootObject
{
public:
    class RTEntry
    {
    public:
        RTEntry(const InetAddress& rAddress, unsigned int uiTimeRemaining)
            : m_address(rAddress),
              m_timeRemaining(uiTimeRemaining)
        {
        }

        bool IsExpired(unsigned int uiElapsed) const;

        InetAddress m_address;
        mutable unsigned int m_timeRemaining;
    };

    typedef std::map<InetAddress, RTEntry, std::less<InetAddress>,
                     MemAllocator<std::pair<const InetAddress, RTEntry> > >
        EntryMap;

    explicit RoutingTable(unsigned int uiTimeout);

    bool Add(const InetAddress& rKey, const InetAddress& rAddress, bool bReplace);
    bool Remove(const InetAddress& rKey);
    bool Find(const InetAddress& rKey, InetAddress& rAddress) const;
    void Clean();

    unsigned int GetSize() const { return m_entries.size(); }

    void PeriodicHouseCleaning();

private:
    unsigned int m_unknown0;
    EntryMap m_entries;
    unsigned int m_timeout;
    Time m_lastCleaning;
};

}
}

#endif
