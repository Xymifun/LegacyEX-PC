#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RESULTRANGE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RESULTRANGE_HPP

#include <stdint.h>

#include "nn/nex/_DDL_ResultRange.hpp"

namespace nn
{
namespace nex
{

class ResultRange : public _DDL_ResultRange
{
public:
    ResultRange(uint32_t uiOffset = 0, uint32_t uiSize = 20)
    {
        m_uiOffset = uiOffset;
        m_uiSize = uiSize;
    }

    virtual ~ResultRange() {}

    virtual uint32_t GetOffset() const { return m_uiOffset; }

    virtual uint32_t GetSize() const { return m_uiSize; }
};

}
}

#endif
