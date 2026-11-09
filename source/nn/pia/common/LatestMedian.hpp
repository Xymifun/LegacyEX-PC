#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_LATEST_MEDIAN_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_LATEST_MEDIAN_HPP

#include <algorithm>

namespace nn
{
namespace pia
{
namespace common
{

template <typename T, T N>
class LatestMedian
{
public:
    LatestMedian() : m_count(0), m_index(0) {}
    T GetMedian() const;

    unsigned int m_count;
    unsigned int m_index;
    T m_values[N];
};

template <typename T, T N>
T LatestMedian<T, N>::GetMedian() const
{
    unsigned int count = m_count;
    if (count == 0)
        return 0;
    if (count > N)
        count = N;

    T values[N];
    for (unsigned int index = 0; index < count; ++index)
    {
        const unsigned int source = (m_index + m_count - index - 1) % N;
        values[index] = m_values[source];
    }
    std::sort(values, values + count);

    const unsigned int middle = count / 2;
    if ((count & 1) != 0)
        return values[middle];
    return (values[middle - 1] + values[middle]) / 2;
}

}
}
}

#endif
