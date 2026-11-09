#include "nn/nex/MutexPrimitive.hpp"

namespace nn
{
namespace nex
{

MutexPrimitive::MutexPrimitive()
{
    OSInitMutex(&m_mutex);
}

MutexPrimitive::~MutexPrimitive()
{
}

}
}
