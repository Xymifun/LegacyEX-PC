#include "nn/nex/_Type_variant.hpp"

#include "nn/nex/DDLSerialization.hpp"
#include "nn/nex/DateTime.hpp"
#include "nn/nex/Message.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/Variant.hpp"

namespace nn
{
namespace nex
{

void _Type_variant::Add(Message* pMessage, const Variant& rSource)
{
    _Type_byte::Add(pMessage, rSource.GetType());
    unsigned long long uiValue;
    switch (rSource.GetType())
    {
    case Variant::TYPE_STRING:
    {
        const String strValue = rSource.GetStringValue();
        _Type_string::Add(pMessage, strValue);
        break;
    }
    case Variant::TYPE_DOUBLE:
    {
        const double dValue = rSource.GetDoubleValue();
        pMessage->Append(reinterpret_cast<const unsigned char*>(&dValue), sizeof(dValue), 1);
        break;
    }
    case Variant::TYPE_INTEGER:
    {
        const long long iValue = rSource.GetInt64Value();
        pMessage->Append(reinterpret_cast<const unsigned char*>(&iValue), sizeof(iValue), 1);
        break;
    }
    case Variant::TYPE_UNSIGNED_INTEGER:
        uiValue = rSource.GetUInt64Value();
        pMessage->Append(reinterpret_cast<const unsigned char*>(&uiValue), sizeof(uiValue), 1);
        break;
    case Variant::TYPE_BOOL:
    {
        const bool bValue = rSource.GetBoolValue();
        *pMessage << bValue;
        break;
    }
    case Variant::TYPE_DATE_TIME:
    {
        DateTime dateTime = rSource.GetDateTimeValue();
        uiValue = dateTime;
        pMessage->Append(reinterpret_cast<const unsigned char*>(&uiValue), sizeof(uiValue), 1);
        break;
    }
    case Variant::TYPE_NONE:
        break;
    }
}

bool _Type_variant::Extract(Message* pMessage, Variant* pDestination)
{
    unsigned char uiType;
    pMessage->ExtractPrimitive(&uiType);
    switch (uiType)
    {
    case Variant::TYPE_STRING:
    {
        String strValue;
        if (!_Type_string::Extract(pMessage, &strValue))
            return false;
        *pDestination = Variant(strValue);
        break;
    }
    case Variant::TYPE_DOUBLE:
    {
        double dValue;
        pMessage->ExtractPrimitive(&dValue);
        *pDestination = Variant(dValue);
        break;
    }
    case Variant::TYPE_INTEGER:
    {
        long long iValue;
        pMessage->ExtractPrimitive(&iValue);
        *pDestination = Variant(iValue);
        break;
    }
    case Variant::TYPE_UNSIGNED_INTEGER:
    {
        unsigned long long uiValue;
        pMessage->ExtractPrimitive(&uiValue);
        *pDestination = Variant(uiValue);
        break;
    }
    case Variant::TYPE_BOOL:
    {
        bool bValue;
        *pMessage >> bValue;
        *pDestination = Variant(bValue);
        break;
    }
    case Variant::TYPE_DATE_TIME:
    {
        DateTime dateTime;
        unsigned long long uiValue;
        pMessage->ExtractPrimitive(&uiValue);
        dateTime = DateTime(uiValue);
        *pDestination = Variant(dateTime);
        break;
    }
    case Variant::TYPE_NONE:
        *pDestination = Variant();
        break;
    }
    return true;
}

}
}
