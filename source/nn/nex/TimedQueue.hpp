#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_TIMEDQUEUE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_TIMEDQUEUE_HPP

#include <utility>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/Time.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

template <typename T>
class TimedQueue : public RootObject
{
public:
    typedef std::pair<T, Time> Entry;
    typedef qList<Entry> EntryList;

    TimedQueue() {}

    virtual ~TimedQueue();

    virtual void Queue(const T& rItem, Time time)
    {
        typename EntryList::iterator position = m_entries.end();
        while (position != m_entries.begin())
        {
            typename EntryList::iterator previous = position;
            --previous;
            if (static_cast<unsigned long long>(previous->second) <= static_cast<unsigned long long>(time))
            {
                break;
            }

            position = previous;
        }

        m_entries.insert(position, Entry(rItem, time));
    }

    virtual bool Ready(Time time)
    {
        return !m_entries.empty()
               && static_cast<unsigned long long>(m_entries.front().second) <= static_cast<unsigned long long>(time);
    }

    virtual T GetHead(Time time) { return m_entries.front().first; }

    virtual void Dequeue(const T& rItem)
    {
        if (!m_entries.empty())
            m_entries.pop_front();
    }

    void Trace(unsigned long long uiFilter)
    {
        for (typename EntryList::iterator entry = m_entries.begin(); entry != m_entries.end(); ++entry)
        {
        }
    }

private:
    EntryList m_entries;
};

template <typename T>
TimedQueue<T>::~TimedQueue()
{
    m_entries.clear();
}

}
}

#endif
