#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ERRORDESCRIPTIONTABLE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ERRORDESCRIPTIONTABLE_HPP

#include "nn/nex/RootObject.hpp"

enum SubSystemType
{
    SUBSYSTEM_TYPE_0,
    SUBSYSTEM_TYPE_1,
    SUBSYSTEM_TYPE_COUNT
};

namespace nn
{
namespace nex
{

class ErrorDescriptionTable : public RootObject
{
public:
    ErrorDescriptionTable(wchar_t** ppDescriptions, SubSystemType subSystem);
    virtual ~ErrorDescriptionTable();

private:
    static ErrorDescriptionTable* s_tables[SUBSYSTEM_TYPE_COUNT];

    SubSystemType m_subSystem;
    wchar_t** m_ppDescriptions;
};

}
}

#endif
