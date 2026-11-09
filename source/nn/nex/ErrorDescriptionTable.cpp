#include "nn/nex/ErrorDescriptionTable.hpp"

namespace nn
{
namespace nex
{

ErrorDescriptionTable* ErrorDescriptionTable::s_tables[SUBSYSTEM_TYPE_COUNT];

namespace
{

wchar_t* s_systemErrorDescriptions[] = {
    L"Insufficient Memory",
    L"Illegal Memory Access",
    L"Win32 function returned an error",
    L"Index out of bound",
    L"Duplicated key within the collection",
    L"The item was in use, and could not be deleted",
    L"The item was not found",
    L"An access beyond the end of the collection has occured",
    L"Object not initialized",
    L"Feature not implemented",
    L"IDGenerator is out of ID",
    L"Internal Exception",
    L"Timeout occured",
    L"Child process could not be launched",
    L"Invalid operation",
    L"Multiple instances not allowed",
    L"Multiple exception handlers not allowed",
    L"This product has expired",
    L"Parameter out of range",
    L"Assertion Failed",
    L"Deadlock detected",
    L"NetZ has not been initialized",
    L"Invalid parameter",
    L"IO error",
    L"Product key does not exist or is invalid",
    L"Buffer extraction overflow",
    L"Invalid wait",
    L"Access denied",
    L"Operation denied",
    L"Gen invalid operation warning",
    L"Gen CTR error",
};

ErrorDescriptionTable s_systemErrorDescriptionTable(s_systemErrorDescriptions, SUBSYSTEM_TYPE_0);

}

ErrorDescriptionTable::ErrorDescriptionTable(wchar_t** ppDescriptions, SubSystemType subSystem)
{
    m_ppDescriptions = ppDescriptions;
    m_subSystem = subSystem;
    s_tables[subSystem] = this;
}

ErrorDescriptionTable::~ErrorDescriptionTable()
{
    m_ppDescriptions = 0;
    s_tables[m_subSystem] = 0;
}

}
}
