#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DDLCLASSMETATYPE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DDLCLASSMETATYPE_HPP

namespace nn
{
namespace nex
{

class Message;

template <typename T>
class DDLClassMetaType
{
public:
    typedef T CPPType;

    static void Add(Message* pMessage, const CPPType& rSource) { CPPType::Add(pMessage, rSource); }

    static void Extract(Message* pMessage, CPPType* pDestination) { CPPType::Extract(pMessage, pDestination); }
};

}
}

#endif
