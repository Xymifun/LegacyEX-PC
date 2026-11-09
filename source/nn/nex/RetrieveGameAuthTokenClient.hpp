#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RETRIEVEGAMEAUTHTOKENCLIENT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RETRIEVEGAMEAUTHTOKENCLIENT_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class CallContext;
class String;

class RetrieveGameAuthTokenClient : public RootObject
{
public:
    RetrieveGameAuthTokenClient();
    virtual ~RetrieveGameAuthTokenClient();

    qResult RetrieveGameAuthToken(CallContext* pCallContext, unsigned int uiPrincipalId, const String& rPassword,
                                  String* pToken, String* pKey);

private:
    enum RetrieveResult
    {
        RETRIEVE_CALL_SUCCEEDED_RESULT = 0x10001,
        RETRIEVE_CALL_FAILED_RESULT = static_cast<int>(0x8001000Du)
    };
};

}
}

#endif
