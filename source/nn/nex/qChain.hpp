#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_QCHAIN_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_QCHAIN_HPP

#include <cstddef>
#include <iterator>
#include <boost/type_traits/remove_pointer.hpp>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

template <class T>
class qChainLink
{
public:
    qChainLink() : m_pNext(0), m_pPrevious(0) {}

    T m_pNext;
    T m_pPrevious;
};

template <class T>
class DefaultChainPolicy
{
public:
    static T& Next(T element) { return element->m_link.m_pNext; }

    static T& Previous(T element) { return element->m_link.m_pPrevious; }
};

template <class T, class TPolicy>
class qChain : public RootObject
{
public:
    class iterator
    {
    public:
        typedef std::forward_iterator_tag iterator_category;
        typedef T value_type;
        typedef std::ptrdiff_t difference_type;
        typedef T* pointer;
        typedef T& reference;

        iterator() : m_element(0) {}

        explicit iterator(T element) : m_element(element) {}

        iterator(const iterator& other) : m_element(other.m_element) {}

        iterator& operator=(const iterator& other)
        {
            m_element = other.m_element;
            return *this;
        }

        T& operator*() { return m_element; }

        const T& operator*() const { return m_element; }

        iterator& operator++()
        {
            m_element = TPolicy::Next(m_element);
            return *this;
        }

        bool operator==(const iterator& other) const { return m_element == other.m_element; }

        bool operator!=(const iterator& other) const { return m_element != other.m_element; }

    private:
        T m_element;
    };

    qChain();

    virtual ~qChain()
    {
        iterator last = end();
        iterator position = begin();

        while (position != last)
            position = erase(position);
    }

    iterator begin() const { return iterator(m_pFirst); }

    iterator end() const { return iterator(m_pEnd); }

    bool empty() const { return m_pFirst == m_pEnd; }

    unsigned int size() const { return m_size; }

    iterator find(const T& element) const
    {
        iterator position = begin();
        while (position != end() && *position != element)
            ++position;
        return position;
    }

    iterator find(const typename boost::remove_pointer<T>::type* element) const
    {
        iterator position = begin();
        while (position != end() && *position != element)
            ++position;
        return position;
    }

    T& front() { return m_pFirst; }

    T& back() { return m_pLast; }

    void push_back(const T& element)
    {
        if (m_pFirst != m_pEnd)
        {
            TPolicy::Next(m_pLast) = element;
            TPolicy::Previous(element) = m_pLast;
            m_pLast = element;
            ++m_size;
        }
        else
        {
            m_pFirst = element;
            m_pLast = element;
            ++m_size;
        }
    }

    void push_front(const T& element)
    {
        if (m_pFirst != m_pEnd)
        {
            TPolicy::Next(element) = m_pFirst;
            TPolicy::Previous(m_pFirst) = element;
            m_pFirst = element;
            ++m_size;
        }
        else
        {
            m_pFirst = element;
            m_pLast = element;
            ++m_size;
        }
    }

    iterator erase(iterator position)
    {
        T element = *position;
        T previous = TPolicy::Previous(element);
        iterator next(TPolicy::Next(element));

        if (previous != 0)
            TPolicy::Next(previous) = *next;

        TPolicy::Previous(element) = 0;

        if (*next != 0)
            TPolicy::Previous(*next) = previous;

        TPolicy::Next(element) = 0;

        if (m_pFirst == element)
            m_pFirst = *next;

        if (m_pLast == element)
            m_pLast = previous;

        --m_size;
        return next;
    }

    void remove(const T& element)
    {
        iterator position = begin();

        while (position != end())
        {
            while (position != end() && *position != element)
                ++position;

            if (position == end())
                return;

            position = erase(position);
        }
    }

private:
    T m_pFirst;
    T m_pLast;
    T m_pEnd;
    unsigned int m_size;
};

template <class T, class TPolicy>
qChain<T, TPolicy>::qChain() : m_pFirst(0),
                               m_pLast(0),
                               m_pEnd(0),
                               m_size(0)
{
}

}
}

#endif
