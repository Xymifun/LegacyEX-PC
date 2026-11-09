#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_BITON_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_BITON_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

template <typename T>
class qList;

class BitOn : public RootObject
{
public:
    BitOn();
    ~BitOn();

    static unsigned int GetBitOn(unsigned int uiValue, qList<unsigned short>* pBits);
    static unsigned int GetBitOn(unsigned int uiValue);
};

}
}

#endif
