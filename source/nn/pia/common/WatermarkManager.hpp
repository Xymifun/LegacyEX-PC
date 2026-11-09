#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_WATERMARKMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_WATERMARKMANAGER_HPP

#include <stdint.h>
#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class Watermark
{
public:
    enum
    {
        cNameLengthMax = 63
    };
    Watermark();
    ~Watermark();
    void SetName(const char*);
    void Reset();
    void Update(long long);

private:
    friend class WatermarkManager;
    long long m_minimum;
    long long m_maximum;
    long long m_current;
    unsigned long long m_updateCount;
    bool m_enabled;
    char m_name[cNameLengthMax + 1];
};

class WatermarkManager
{
public:
    enum
    {
        cWatermarkNum = 10
    };
    WatermarkManager();
    ~WatermarkManager();
    static Result CreateInstance();
    static void DestroyInstance();
    static void DisableAllWatermark();
    static WatermarkManager* GetInstance();
    Watermark* GetWatermark(int);
    const Watermark* GetWatermark(int) const;
    void EnableAllWatermark();
    void Trace(unsigned long) const;

private:
    Watermark m_watermarks[cWatermarkNum];
};

}
}
}
namespace nn
{
namespace pia
{
namespace transport
{
extern common::WatermarkManager* g_piaWatermarkManager;
}
}
}

namespace nn
{
namespace pia
{
namespace common
{

inline WatermarkManager* WatermarkManager::GetInstance()
{
    return transport::g_piaWatermarkManager;
}

}
}
}

#endif
