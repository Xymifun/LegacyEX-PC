#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSIONINFOLIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_SESSIONINFOLIST_HPP

#include "nn/pia/NetworkFactory.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/session/ISessionInfo.hpp"
#include "nn/pia/session/ISessionInfoList.hpp"

namespace nn
{
namespace pia
{
namespace session
{

template <typename T>
class SessionInfoList : public ISessionInfoList
{
public:
    explicit SessionInfoList(unsigned short maxSize)
        : m_size(0),
          m_maxSize(maxSize),
          m_items(common::NewArray<ISessionInfo*>(maxSize))
    {
        for (unsigned short index = 0; index < m_maxSize; ++index)
            m_items[index] = transport::CreateObject<T>();
    }

    virtual ~SessionInfoList();
    virtual const ISessionInfo** Begin() const;
    virtual ISessionInfo** Begin();
    virtual const ISessionInfo** End() const;
    virtual ISessionInfo** End();
    virtual unsigned short GetSize() const;
    virtual unsigned short GetMaxSize() const;
    virtual void Clear();

    unsigned short m_size;
    unsigned short m_maxSize;
    ISessionInfo** m_items;
};

template <typename T>
SessionInfoList<T>::~SessionInfoList()
{
    for (unsigned short index = 0; index < m_maxSize; ++index)
        transport::DestroyObject(m_items[index]);
    if (m_items != 0)
        common::DeleteArray(m_items);
}

template <typename T>
const ISessionInfo** SessionInfoList<T>::Begin() const
{
    return const_cast<const ISessionInfo**>(m_items);
}

template <typename T>
ISessionInfo** SessionInfoList<T>::Begin()
{
    return m_items;
}

template <typename T>
const ISessionInfo** SessionInfoList<T>::End() const
{
    return const_cast<const ISessionInfo**>(m_items + m_size);
}

template <typename T>
ISessionInfo** SessionInfoList<T>::End()
{
    return m_items + m_size;
}

template <typename T>
unsigned short SessionInfoList<T>::GetSize() const
{
    return m_size;
}

template <typename T>
unsigned short SessionInfoList<T>::GetMaxSize() const
{
    return m_maxSize;
}

template <typename T>
void SessionInfoList<T>::Clear()
{
    m_size = 0;
    if (m_items != 0)
    {
        for (unsigned short index = 0; index < m_maxSize; ++index)
            m_items[index]->Clear();
    }
}

}
}
}

#endif
