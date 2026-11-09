#include "nn/pia/common/SessionStateMonitoringContent.hpp"

#include <coreinit/memory.h>

namespace nn
{
namespace pia
{
namespace common
{

static const unsigned char cUnsetU8 = 0xff;
static const unsigned short cUnsetU16 = 0xffff;
static const unsigned int cUnsetU32 = 0xffffffff;
static const unsigned long long cUnsetU64 = 0xffffffffffffffffULL;

SessionStateMonitoringContent g_sessionStateMonitoringContent;

void SessionStateMonitoringContent::Initialize()
{
    OSBlockSet(this, cUnsetU8, sizeof(*this));
    m_contentType = 0x12;
    m_contentVersion = 1;
    m_contentSize = static_cast<unsigned short>(GetSerializedSize());
    m_marker = 0x5069614d;
}

void SessionStateMonitoringContent::Cleanup()
{
    m_schedulerDispatchCount = cUnsetU32;
    m_metric1[2] = cUnsetU32;
    m_metric223[1] = cUnsetU8;
    m_metric223[2] = cUnsetU8;
    for (unsigned int index = 0; index < 16; ++index)
    {
        m_metric1[3 + index] = cUnsetU32;
        m_metric20[index] = cUnsetU32;
        m_metric36[index] = cUnsetU64;
        m_metric52[index] = cUnsetU32;
        m_metric68[index] = cUnsetU32;
        m_metric84[index] = cUnsetU64;
    }
    for (unsigned int index = 0; index < 15; ++index)
    {
        m_metric100[index] = cUnsetU32;
        m_metric115[index] = cUnsetU8;
        m_metric130[index] = cUnsetU16;
        m_metric145[index] = cUnsetU16;
    }

    m_sendThreadSleepMilliSecond = cUnsetU8;
    m_metric163 = cUnsetU32;
    m_metric165 = cUnsetU32;
    m_receiveThreadSleepMilliSecond = cUnsetU8;
    m_metric162 = cUnsetU16;
    m_metric164 = cUnsetU16;
    for (unsigned int index = 1; index < 5; ++index)
        m_metric167[index] = cUnsetU32;
    for (unsigned int index = 0; index < 4; ++index)
        m_metric172[index] = cUnsetU32;
    m_metric176 = cUnsetU32;
    m_metric177 = cUnsetU32;
    m_metric178 = cUnsetU8;
    m_metric179 = cUnsetU32;
    m_metric180 = cUnsetU8;
    m_metric181 = cUnsetU8;
    m_metric182 = cUnsetU32;
    m_metric183 = cUnsetU32;
    m_metric184 = cUnsetU16;
    m_metric185 = cUnsetU16;
    m_metric186 = cUnsetU32;
    m_metric187[0] = cUnsetU8;
    m_metric187[1] = cUnsetU8;
    m_metric187[2] = cUnsetU8;
    m_metric187[3] = cUnsetU8;
    m_metric187[4] = cUnsetU8;
    m_metric187[5] = cUnsetU8;
    m_metric193 = cUnsetU32;
    m_metric194 = cUnsetU8;
    m_metric195 = cUnsetU32;
    m_metric196 = cUnsetU8;
    m_metric197 = cUnsetU8;
    m_metric198 = cUnsetU32;
    m_metric199 = cUnsetU32;
    m_metric222 = cUnsetU32;
    m_metric223[0] = cUnsetU8;
    m_leaveProcessingTime = cUnsetU32;
    m_metric225[0] = cUnsetU8;
    m_metric225[1] = cUnsetU8;
    m_metric225[2] = cUnsetU8;
    for (unsigned int index = 0; index < 4; ++index)
        m_metric200[index] = cUnsetU8;
    for (unsigned int index = 0; index < 5; ++index)
        m_metric201[index] = cUnsetU8;
    m_metric203 = cUnsetU16;
    m_metric202 = cUnsetU32;
    m_metric206 = cUnsetU32;
    m_metric205 = cUnsetU32;
}

unsigned int SessionStateMonitoringContent::GetSerializedSize()
{
    return MonitoringContentHeader::GetSerializedSize() + 883;
}

Result SessionStateMonitoringContent::Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const
{
    if (buffer == 0)
        return 0x10c07;
    if (size == 0)
        return 0x10c07;
    if (maxSize < GetSerializedSize())
        return 0x10c07;

    unsigned int headerSize;
    MonitoringContentHeader::Serialize(buffer, &headerSize, maxSize);
    unsigned char* cursor = buffer + headerSize;

    serializeU32(cursor, m_schedulerDispatchCount);
    cursor += 4;
    serializeU32(cursor, m_metric1[0]);
    cursor += 4;
    serializeU32(cursor, m_metric1[1]);
    cursor += 4;
    serializeU32(cursor, m_metric1[2]);
    cursor += 4;
    for (int index = 0; index < 16; ++index)
    {
        serializeU32(cursor, m_metric1[3 + index]);
        cursor += 4;
    }
    for (int index = 0; index < 16; ++index)
    {
        serializeU32(cursor, m_metric20[index]);
        cursor += 4;
    }
    for (int index = 0; index < 16; ++index)
    {
        serializeU64(cursor, m_metric36[index]);
        cursor += 8;
    }
    for (int index = 0; index < 16; ++index)
    {
        serializeU32(cursor, m_metric52[index]);
        cursor += 4;
    }
    for (int index = 0; index < 16; ++index)
    {
        serializeU32(cursor, m_metric68[index]);
        cursor += 4;
    }
    for (int index = 0; index < 16; ++index)
    {
        serializeU64(cursor, m_metric84[index]);
        cursor += 8;
    }
    for (int index = 0; index < 15; ++index)
    {
        serializeU32(cursor, m_metric100[index]);
        cursor += 4;
    }
    for (int index = 0; index < 15; ++index)
    {
        serializeU8(cursor, m_metric115[index]);
        cursor += 1;
    }
    for (int index = 0; index < 15; ++index)
    {
        serializeU16(cursor, m_metric130[index]);
        cursor += 2;
    }
    for (int index = 0; index < 15; ++index)
    {
        serializeU16(cursor, m_metric145[index]);
        cursor += 2;
    }
    serializeU8(cursor, m_sendThreadSleepMilliSecond);
    cursor += 1;
    serializeU8(cursor, m_receiveThreadSleepMilliSecond);
    cursor += 1;
    serializeU16(cursor, m_metric162);
    cursor += 2;
    serializeU32(cursor, m_metric163);
    cursor += 4;
    serializeU16(cursor, m_metric164);
    cursor += 2;
    serializeU32(cursor, m_metric165);
    cursor += 4;
    serializeU32(cursor, m_receiveThreadMetric);
    cursor += 4;
    serializeU32(cursor, m_metric167[0]);
    cursor += 4;
    for (int index = 0; index < 4; ++index)
    {
        serializeU32(cursor, m_metric167[1 + index]);
        cursor += 4;
    }
    for (int index = 0; index < 4; ++index)
    {
        serializeU32(cursor, m_metric172[index]);
        cursor += 4;
    }
    serializeU32(cursor, m_metric176);
    cursor += 4;
    serializeU32(cursor, m_metric177);
    cursor += 4;
    serializeU8(cursor, m_metric178);
    cursor += 1;
    serializeU32(cursor, m_metric179);
    cursor += 4;
    serializeU8(cursor, m_metric180);
    cursor += 1;
    serializeU8(cursor, m_metric181);
    cursor += 1;
    serializeU32(cursor, m_metric182);
    cursor += 4;
    serializeU32(cursor, m_metric183);
    cursor += 4;
    serializeU16(cursor, m_metric184);
    cursor += 2;
    serializeU16(cursor, m_metric185);
    cursor += 2;
    serializeU32(cursor, m_metric186);
    cursor += 4;
    serializeU8(cursor, m_metric187[0]);
    cursor += 1;
    serializeU8(cursor, m_metric187[1]);
    cursor += 1;
    serializeU8(cursor, m_metric187[2]);
    cursor += 1;
    serializeU8(cursor, m_metric187[3]);
    cursor += 1;
    serializeU8(cursor, m_metric187[4]);
    cursor += 1;
    serializeU8(cursor, m_metric187[5]);
    cursor += 1;
    serializeU32(cursor, m_metric193);
    cursor += 4;
    serializeU8(cursor, m_metric194);
    cursor += 1;
    serializeU32(cursor, m_metric195);
    cursor += 4;
    serializeU8(cursor, m_metric196);
    cursor += 1;
    serializeU8(cursor, m_metric197);
    cursor += 1;
    serializeU32(cursor, m_metric198);
    cursor += 4;
    serializeU32(cursor, m_metric199);
    cursor += 4;
    for (int index = 0; index < 4; ++index)
    {
        serializeU8(cursor, m_metric200[index]);
        cursor += 1;
    }
    for (int index = 0; index < 5; ++index)
    {
        serializeU8(cursor, m_metric201[index]);
        cursor += 1;
    }
    serializeU32(cursor, m_metric202);
    cursor += 4;
    serializeU16(cursor, m_metric203);
    cursor += 2;
    serializeU8(cursor, m_metric204);
    cursor += 1;
    serializeU32(cursor, m_metric205);
    cursor += 4;
    serializeU32(cursor, m_metric206);
    cursor += 4;
    serializeU16(cursor, m_metric207);
    cursor += 2;
    serializeU8(cursor, m_metric208);
    cursor += 1;
    serializeU16(cursor, m_metric209);
    cursor += 2;
    serializeU16(cursor, m_metric210);
    cursor += 2;
    serializeU32(cursor, m_metric211);
    cursor += 4;
    serializeU16(cursor, m_metric212);
    cursor += 2;
    serializeU16(cursor, m_metric213);
    cursor += 2;
    serializeU8(cursor, m_metric214);
    cursor += 1;
    serializeU32(cursor, m_metric215);
    cursor += 4;
    serializeU32(cursor, m_metric216);
    cursor += 4;
    serializeU32(cursor, m_metric217);
    cursor += 4;
    serializeU32(cursor, m_metric218);
    cursor += 4;
    serializeU64(cursor, m_metric219);
    cursor += 8;
    serializeU64(cursor, m_metric220);
    cursor += 8;
    serializeU8(cursor, m_metric221[0]);
    cursor += 1;
    serializeU8(cursor, m_metric221[1]);
    cursor += 1;
    serializeU8(cursor, m_metric221[2]);
    cursor += 1;
    serializeU8(cursor, m_metric221[3]);
    cursor += 1;
    serializeU32(cursor, m_metric222);
    cursor += 4;
    serializeU8(cursor, m_metric223[0]);
    cursor += 1;
    serializeU8(cursor, m_metric223[1]);
    cursor += 1;
    serializeU8(cursor, m_metric223[2]);
    cursor += 1;
    serializeU32(cursor, m_leaveProcessingTime);
    cursor += 4;
    serializeU8(cursor, m_metric225[0]);
    cursor += 1;
    serializeU8(cursor, m_metric225[1]);
    cursor += 1;
    serializeU8(cursor, m_metric225[2]);
    cursor += 1;
    for (int index = 0; index < 16; ++index)
    {
        serializeU8(cursor, m_metric226[index]);
        cursor += 1;
    }
    serializeU32(cursor, m_marker);
    cursor += 4;

    *size = static_cast<unsigned int>(cursor - buffer);
    return 0;
}

}
}
}
