#include "nn/pia/common/WatermarkManager.hpp"

#include "nn/pia/Common.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/Memory.hpp"
#include "platform/wiiu/pead.hpp"

#include <new>

namespace nn
{
namespace pia
{
namespace transport
{
common::WatermarkManager* g_piaWatermarkManager = 0;
}
}
}

namespace nn
{
namespace pia
{
namespace common
{

namespace
{
enum
{
    cResultAllocationFailed = 0x10c01,
    cResultInvalidState = 0x10c08,
    cResultNotInitialized = 0x10c0c,
    cResultAlreadyCreated = 0x10c0f
};

unsigned char* s_watermarkStorage = 0;
}

WatermarkManager::WatermarkManager()
{
}

WatermarkManager::~WatermarkManager()
{
}

Result WatermarkManager::CreateInstance()
{
    if (IsInitialized() == 0)
        return cResultNotInitialized;
    if (IsInSetupMode() == 0)
        return cResultInvalidState;
    if (transport::g_piaWatermarkManager != 0)
        return cResultAlreadyCreated;

    s_watermarkStorage = NewArray<unsigned char>(sizeof(WatermarkManager), 8);
    if (s_watermarkStorage == 0)
        return cResultAllocationFailed;

    transport::g_piaWatermarkManager = new (s_watermarkStorage) WatermarkManager();
    return 0;
}

void WatermarkManager::DestroyInstance()
{
    if (transport::g_piaWatermarkManager != 0)
    {
        transport::g_piaWatermarkManager->~WatermarkManager();
        transport::g_piaWatermarkManager = 0;
    }
    if (s_watermarkStorage != 0)
    {
        DeleteArray(s_watermarkStorage);
        s_watermarkStorage = 0;
    }
}

void WatermarkManager::EnableAllWatermark()
{
    Watermark* watermark = m_watermarks;
    for (unsigned int index = 0; index != cWatermarkNum; ++index)
    {
        watermark->m_enabled = true;
        ++watermark;
    }
}

Watermark* WatermarkManager::GetWatermark(int index)
{
    if (static_cast<unsigned int>(index) >= cWatermarkNum)
        return 0;
    return &m_watermarks[index];
}

}
}
}
