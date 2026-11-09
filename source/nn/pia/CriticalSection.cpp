#include "nn/pia.h"
namespace nn
{
namespace pia
{
namespace common
{

CriticalSection::CriticalSection(int)
{
    OSFastMutex_Init(&m_mutex, 0);
}
CriticalSection::~CriticalSection()
{
}
void CriticalSection::Lock()
{
    OSFastMutex_Lock(&m_mutex);
}
void CriticalSection::Unlock()
{
    OSFastMutex_Unlock(&m_mutex);
}
}
}
}
