#ifndef SOURCE_VENDOR_N_29_JUGEMDATASTORECLIENT_BLOB_CPP_B7C5B7BE_HPP
#define SOURCE_VENDOR_N_29_JUGEMDATASTORECLIENT_BLOB_CPP_B7C5B7BE_HPP

namespace nn
{
namespace nex
{
class String;
}
}

class __N_29_JugemDataStoreClient_Blob_cpp_b7c5b7be
{
public:
    static unsigned long long GetDataCodeChecksum(const nn::nex::String& dataStorePassword, unsigned long long dataId);
    static unsigned long long DataIdToDataCode(const nn::nex::String& dataStorePassword, unsigned long long dataId);
    static int DataCodeCharComp(const void* lhs, const void* rhs);
    static nn::nex::String DataCodeStr(const nn::nex::String& dataCode);
    static bool CheckDataCode(const nn::nex::String& dataStorePassword, unsigned long long dataCode);
    static nn::nex::String DataCodeRawToDataCodeStr(unsigned long long dataCode, bool zeroPad);
};

#endif
