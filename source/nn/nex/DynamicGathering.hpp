#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DYNAMICGATHERING_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DYNAMICGATHERING_HPP

#include "nn/nex/_DDL_DynamicGathering.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

class DynamicGathering : public _DDL_DynamicGathering
{
public:
    DynamicGathering() : _DDL_DynamicGathering(), m_Type(L"DynamicGathering") {}

    virtual ~DynamicGathering() {}

    virtual String GetGatheringType() const { return m_Type; }

    String m_Type;
};

}
}

#endif
