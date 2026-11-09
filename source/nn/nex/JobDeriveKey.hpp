#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBDERIVEKEY_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBDERIVEKEY_HPP

#include "nn/nex/Buffer.hpp"
#include "nn/nex/Job.hpp"
#include "nn/nex/Key.hpp"

namespace nn
{
namespace nex
{

class ChecksumAlgorithm;
class KeyCache;

class JobDeriveKey : public Job
{
public:
    JobDeriveKey(JobType jobType, unsigned int uiCallId, ChecksumAlgorithm* pChecksumAlgorithm, const Buffer* pBuffer,
                 unsigned int uiIterations, Key* pKey);
    virtual ~JobDeriveKey();

    virtual void Execute();

    void CompleteJob(const Key& rKey);

private:
    friend class ChecksumAlgorithm;

    unsigned int m_uiCallId;
    ChecksumAlgorithm* m_pChecksumAlgorithm;
    Buffer m_current;
    unsigned int m_uiCompletedIterations;
    unsigned int m_uiIterations;
    Key* m_pKey;
    Buffer m_cacheKey;
    KeyCache* m_pKeyCache;
};

}
}

#endif
