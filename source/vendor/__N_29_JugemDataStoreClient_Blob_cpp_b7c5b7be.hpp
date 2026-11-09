#ifndef SOURCE_VENDOR___N_29_JUGEMDATASTORECLIENT_BLOB_CPP_B7C5B7BE_HPP
#define SOURCE_VENDOR___N_29_JUGEMDATASTORECLIENT_BLOB_CPP_B7C5B7BE_HPP

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
    static unsigned long long GetDataCodeChecksum(const nn::nex::String&, unsigned long long);
    static unsigned long long DataIdToDataCode(const nn::nex::String&, unsigned long long);
    static int DataCodeCharComp(const void*, const void*);
    static nn::nex::String DataCodeStr(const nn::nex::String&);
    static bool CheckDataCode(const nn::nex::String&, unsigned long long);
    static nn::nex::String DataCodeRawToDataCodeStr(unsigned long long, bool);
};

#endif
