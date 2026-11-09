#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBDATASTOREGETRATING_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBDATASTOREGETRATING_HPP

#include <stdint.h>

#include "nn/nex/DataStoreRatingInfo.hpp"
#include "nn/nex/DataStoreRatingInfoWithSlot.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/StepSequenceJob.hpp"
#include "nn/nex/qMap.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class DataStoreLogicServerClientInterface;

class JobDataStoreGetRating : public StepSequenceJob
{
public:
    JobDataStoreGetRating();
    virtual ~JobDataStoreGetRating();

    virtual qResult Initialize(unsigned int uiCallId, DataStoreLogicServerClientInterface* pClient,
                               const uint64_t& uiDataId, qMap<signed char, DataStoreRatingInfo>* pRatings,
                               uint64_t uiAccessPassword);
    virtual qResult Initialize(unsigned int uiCallId, DataStoreLogicServerClientInterface* pClient,
                               const qVector<uint64_t>& rDataIds,
                               qVector<qMap<signed char, DataStoreRatingInfo> >* pRatings, qVector<qResult>* pResults);

    void CompleteJob(const qResult& rResult);
    void StepGetRating();
    void StepConvertType();

private:
    enum Result
    {
        RESULT_SUCCESS = 0x10001,
        RESULT_CORE_OPERATION_ABORTED = static_cast<int>(0x80010004u),
        RESULT_CORE_CALL_FAILED = static_cast<int>(0x8001000Du),
        RESULT_DATASTORE_UNKNOWN = static_cast<int>(0x80690001u)
    };

    unsigned int m_uiCallId;
    DataStoreLogicServerClientInterface* m_pClient;
    qVector<uint64_t> m_dataIds;
    qVector<qVector<DataStoreRatingInfoWithSlot> > m_ratingInfos;
    qVector<qMap<signed char, DataStoreRatingInfo> >* m_pRatingsList;
    qMap<signed char, DataStoreRatingInfo>* m_pRatings;
    qVector<qResult>* m_pResults;
    qVector<qResult> m_results;
    bool m_bSingleTarget;
    ProtocolCallContext m_getRatingContext;
    uint64_t m_uiAccessPassword;
};

}
}

#endif
