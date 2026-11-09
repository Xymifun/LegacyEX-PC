#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DYNAMICDATA_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DYNAMICDATA_HPP

#include "nn/nex/String.hpp"
#include "nn/nex/_DDL_DynamicData.hpp"

namespace nn
{
namespace nex
{

class DynamicData : public _DDL_DynamicData
{
public:
    DynamicData() : _DDL_DynamicData(), m_Type(L"DynamicData"), m_bCompactFormat(false) {}

    virtual ~DynamicData() {}

    virtual String GetDataType() const { return m_Type; }

    String m_Type;
    bool m_bCompactFormat;
};

}
}

#endif
