#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_QSORTEDVECTOR_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_QSORTEDVECTOR_HPP

#include <algorithm>
#include <utility>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

template <typename K, typename V>
class qSortedVector : public RootObject
{
public:
    typedef std::pair<K, V> value_type;
    typedef qVector<value_type> VectorType;
    typedef typename VectorType::iterator iterator;
    typedef typename VectorType::const_iterator const_iterator;
    typedef typename VectorType::size_type size_type;

    qSortedVector() {}

    explicit qSortedVector(size_type count) { m_elements.reserve(count); }

    iterator begin() { return m_elements.begin(); }

    iterator end() { return m_elements.end(); }

    const_iterator begin() const { return m_elements.begin(); }

    const_iterator end() const { return m_elements.end(); }

    bool empty() const { return m_elements.empty(); }

    size_type size() const { return m_elements.size(); }

    value_type& front() { return m_elements.front(); }

    void reserve(size_type count) { m_elements.reserve(count); }

    iterator find(const K& rKey)
    {
        iterator position = std::lower_bound(m_elements.begin(), m_elements.end(), rKey, KeyLess());
        if (position != end() && rKey < (*position).first)
            position = end();

        return position;
    }

    std::pair<iterator, bool> insert(const value_type& rElement)
    {
        iterator position = std::lower_bound(m_elements.begin(), m_elements.end(), rElement.first, KeyLess());
        if (position == m_elements.end() || rElement.first < position->first)
        {
            position = m_elements.insert(position, rElement);
            return std::pair<iterator, bool>(position, true);
        }

        return std::pair<iterator, bool>(position, false);
    }

    size_type erase(const K& rKey);

private:
    struct KeyLess
    {
        bool operator()(const value_type& rElement, const K& rKey) const { return rElement.first < rKey; }
    };

    VectorType m_elements;
};

template <typename K, typename V>
typename qSortedVector<K, V>::size_type qSortedVector<K, V>::erase(const K& rKey)
{
    iterator position = find(rKey);
    if (position != m_elements.end())
    {
        m_elements.erase(position);
        return 1;
    }

    return 0;
}

}
}

#endif
