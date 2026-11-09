#include "nn/nex/ErrorCodeConverter.hpp"

#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

unsigned int g_aNetworkErrorCodeTable[] = {1060000, 2, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1};

inline unsigned int ErrorCodeConverter::MakeNetworkErrorCode(unsigned int uiCategory, int iIndex)
{
    return g_aNetworkErrorCodeTable[0] + uiCategory * CATEGORY_CODE_RANGE + iIndex
           + g_aNetworkErrorCodeTable[uiCategory];
}

unsigned int ErrorCodeConverter::ConvertToNetworkErrorCode(const qResult& rResult)
{
    int iResult = rResult.GetReturnCode();
    unsigned int uiFacility = rResult.GetFacility();
    if ((iResult & RESULT_ERROR_MASK) == 0)
        return 0;

    unsigned int uiNetworkErrorCode;
    switch (uiFacility)
    {
    case FACILITY_CORE:
        uiNetworkErrorCode = MakeNetworkErrorCode(CATEGORY_CORE, iResult - CORE_UNKNOWN);
        break;
    case FACILITY_DDL:
        uiNetworkErrorCode = MakeNetworkErrorCode(CATEGORY_DDL, iResult - DDL_INVALID_SIGNATURE);
        break;
    case FACILITY_RENDEZVOUS:
    {
        int iIndex;
        if (iResult < RENDEZVOUS_INVALID_USERNAME)
            iIndex = iResult - RENDEZVOUS_CONNECTION_FAILURE;
        else if (iResult < RENDEZVOUS_SESSION_FULL)
            iIndex = iResult - RENDEZVOUS_INVALID_USERNAME + RENDEZVOUS_CONNECTION_ERROR_COUNT;
        else
            iIndex = iResult - RENDEZVOUS_SESSION_FULL + RENDEZVOUS_CONNECTION_ERROR_COUNT
                     + RENDEZVOUS_ACCOUNT_ERROR_COUNT;
        uiNetworkErrorCode = MakeNetworkErrorCode(CATEGORY_RENDEZVOUS, iIndex);
        break;
    }
    case FACILITY_PYTHON_CORE:
        uiNetworkErrorCode = MakeNetworkErrorCode(CATEGORY_PYTHON_CORE, iResult - PYTHON_CORE_EXCEPTION);
        break;
    case FACILITY_TRANSPORT:
        uiNetworkErrorCode = MakeNetworkErrorCode(CATEGORY_TRANSPORT, iResult - TRANSPORT_UNKNOWN);
        break;
    case FACILITY_DO_CORE:
        uiNetworkErrorCode = MakeNetworkErrorCode(CATEGORY_DO_CORE, iResult - DO_CORE_UNKNOWN);
        break;
    case FACILITY_FPD:
        uiNetworkErrorCode = MakeNetworkErrorCode(CATEGORY_FPD, iResult - FPD_UNKNOWN);
        break;
    case FACILITY_RANKING:
        uiNetworkErrorCode = MakeNetworkErrorCode(CATEGORY_RANKING, iResult - RANKING_FIRST_ERROR);
        break;
    case FACILITY_AUTHENTICATION:
        uiNetworkErrorCode = MakeNetworkErrorCode(CATEGORY_AUTHENTICATION, iResult - AUTHENTICATION_FIRST_ERROR);
        break;
    case FACILITY_DATASTORE:
        uiNetworkErrorCode = MakeNetworkErrorCode(CATEGORY_DATASTORE, iResult - DATASTORE_UNKNOWN);
        break;
    case FACILITY_SERVICE_ITEM:
        uiNetworkErrorCode = MakeNetworkErrorCode(CATEGORY_SERVICE_ITEM, iResult - SERVICE_ITEM_FIRST_ERROR);
        break;
    case FACILITY_MATCHMAKE_REFEREE:
        uiNetworkErrorCode = MakeNetworkErrorCode(CATEGORY_MATCHMAKE_REFEREE, iResult - MATCHMAKE_REFEREE_FIRST_ERROR);
        break;
    case FACILITY_SUBSCRIBER:
        uiNetworkErrorCode = MakeNetworkErrorCode(CATEGORY_SUBSCRIBER, iResult - SUBSCRIBER_FIRST_ERROR);
        break;
    case FACILITY_RANKING2:
        uiNetworkErrorCode = MakeNetworkErrorCode(CATEGORY_RANKING2, iResult - RANKING2_FIRST_ERROR);
        break;
    default:
        uiNetworkErrorCode = g_aNetworkErrorCodeTable[0];
        break;
    }

    return uiNetworkErrorCode;
}

}
}
