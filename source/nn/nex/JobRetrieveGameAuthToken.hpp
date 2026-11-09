#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBRETRIEVEGAMEAUTHTOKEN_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBRETRIEVEGAMEAUTHTOKEN_HPP

#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class JobRetrieveGameAuthToken : public StepSequenceJob
{
public:
    JobRetrieveGameAuthToken();
    virtual ~JobRetrieveGameAuthToken();

    qResult Initialize(unsigned int uiCallId, unsigned int uiPrincipalId, const String& rPassword, String* pToken,
                       String* pKey);
    void CompleteJob(const qResult& rResult);
    void StepDeriveKey();
    void StepDummyToComplete();

private:
    unsigned int m_uiCallId;
    unsigned int m_uiPrincipalId;
    String m_password;
    String* m_pKey;
};

}
}

#endif
