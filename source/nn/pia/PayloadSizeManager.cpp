#include "nn/pia/PayloadSizeManager.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/Memory.hpp"
#include "platform/wiiu/pead.hpp"

#include <new>

namespace nn

{
namespace pia
{
namespace common
{

PayloadSizeManager* g_payloadSizeManager = 0;

namespace
{
enum
{
    cResultAllocationFailed = 0x10c01,
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cResultNotInitialized = 0x10c0c,
    cResultAlreadyCreated = 0x10c0f
};

unsigned char* g_payloadSizeManagerStorage = 0;
}

PayloadSizeManager::PayloadSizeManager()
    : m_mtuSize(0x5c0),
      m_minMtuSize(0xffffffffU),
      m_maxMtuSize(0),
      m_payloadSizeLimit(0),
      m_signatureSize(0)
{
}

PayloadSizeManager::~PayloadSizeManager()
{
}

Result PayloadSizeManager::CreateInstance()
{
    if (IsInitialized() == 0)
        return cResultNotInitialized;
    if (IsInSetupMode() == 0)
        return cResultInvalidState;
    if (g_payloadSizeManager != 0)
        return cResultAlreadyCreated;

    g_payloadSizeManagerStorage = NewArray<unsigned char>(sizeof(PayloadSizeManager), 8);
    if (g_payloadSizeManagerStorage == 0)
        return cResultAllocationFailed;

    g_payloadSizeManager = new (g_payloadSizeManagerStorage) PayloadSizeManager();
    return 0;
}

void PayloadSizeManager::DestroyInstance()
{
    if (g_payloadSizeManager != 0)
    {
        g_payloadSizeManager->~PayloadSizeManager();
        g_payloadSizeManager = 0;
    }
    if (g_payloadSizeManagerStorage != 0)
    {
        DeleteArray(g_payloadSizeManagerStorage);
        g_payloadSizeManagerStorage = 0;
    }
}

Result PayloadSizeManager::SetMtuSize(unsigned int mtuSize)
{
    if (mtuSize < m_minMtuSize)
        return Result(cResultInvalidArgument);
    if (mtuSize > m_maxMtuSize)
        return ResultInvalidArgument();
    m_mtuSize = mtuSize;
    return 0;
}

}
}
}
