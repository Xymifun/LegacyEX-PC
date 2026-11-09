#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_KEYEDCHECKSUMALGORITHM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_KEYEDCHECKSUMALGORITHM_HPP

#include "nn/nex/ChecksumAlgorithm.hpp"
#include "nn/nex/Key.hpp"

namespace nn
{
namespace nex
{

class KeyedChecksumAlgorithm : public ChecksumAlgorithm
{
public:
    KeyedChecksumAlgorithm(unsigned int uiMinKeyLength, unsigned int uiMaxKeyLength);
    virtual ~KeyedChecksumAlgorithm();

    virtual bool IsReady() const;
    virtual void KeyHasChanged() {}

    bool SetKey(const Key& rKey);

protected:
    Key m_key;
    unsigned int m_uiMinKeyLength;
    unsigned int m_uiMaxKeyLength;
};

}
}

#endif
