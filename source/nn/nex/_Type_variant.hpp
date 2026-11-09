#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__TYPE_VARIANT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__TYPE_VARIANT_HPP

namespace nn
{
namespace nex
{

class Message;
class Variant;

class _Type_variant
{
public:
    typedef Variant CPPType;

    static void Add(Message* pMessage, const Variant& rSource);
    static bool Extract(Message* pMessage, Variant* pDestination);
};

}
}

#endif
