#include "nn/nex/FileStorageUnit.hpp"

namespace nn
{
namespace nex
{

unsigned int FileStorageUnit::GetSize() const
{
    return 0;
}

unsigned int FileStorageUnit::Read(unsigned int, unsigned int, unsigned char*) const
{
    return 0;
}

unsigned int FileStorageUnit::Write(unsigned int, unsigned int, unsigned char*)
{
    return 0;
}

}
}
