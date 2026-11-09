#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ANYOBJECTADAPTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ANYOBJECTADAPTER_HPP

namespace nn
{
namespace nex
{

class Message;

template <typename T, typename KeyType>
class AnyObjectAdapter
{
public:
    AnyObjectAdapter() {}

    virtual ~AnyObjectAdapter() {}

    virtual void StreamIn(Message* pMessage, const T& rObject) const = 0;
    virtual void StreamOut(Message* pMessage, T** ppObject) const = 0;

    static AnyObjectAdapter<T, KeyType>* s_pInstance;
};

template <typename T, typename KeyType>
AnyObjectAdapter<T, KeyType>* AnyObjectAdapter<T, KeyType>::s_pInstance = 0;

}
}

#endif
