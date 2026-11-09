#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__TYPE_QRESULT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__TYPE_QRESULT_HPP

namespace nn
{
namespace nex
{

class Message;
class qResult;

class _Type_qresult
{
public:
    typedef qResult CPPType;

    static void Add(Message* pMessage, const qResult& rSource);
    static void Extract(Message* pMessage, qResult* pDestination);
};

}
}

#endif
