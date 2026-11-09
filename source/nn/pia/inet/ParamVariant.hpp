#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_PARAM_VARIANT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_PARAM_VARIANT_HPP

#include <stdint.h>

namespace nn
{
namespace pia
{
namespace inet
{

class NexMatchmakeSession;
class NexSessionSearchCriteria;

class ParamVariant
{
public:
    enum Type
    {
        cType_None = 0,
        cType_UInt64 = 1,
        cType_Int64 = 2,
        cType_UInt32 = 3,
        cType_Int32 = 4,
        cType_Bool = 5,
        cType_Double = 6
    };

    ParamVariant()
    {
        m_uint64Value = 0;
        m_type = 0;
    }

private:
    friend class NexMatchmakeSession;
    friend class NexSessionSearchCriteria;

    union
    {
        unsigned long long m_uint64Value;
        long long m_int64Value;
        unsigned int m_uint32Value;
        int m_int32Value;
        bool m_boolValue;
        double m_doubleValue;
    };
    unsigned int m_type;
};

}
}
}

#endif
