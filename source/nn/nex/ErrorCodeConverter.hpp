#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ERRORCODECONVERTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ERRORCODECONVERTER_HPP

namespace nn
{
namespace nex
{

class qResult;

extern unsigned int g_aNetworkErrorCodeTable[];

class ErrorCodeConverter
{
public:
    static unsigned int ConvertToNetworkErrorCode(const qResult& rResult);

private:
    static unsigned int MakeNetworkErrorCode(unsigned int uiCategory, int iIndex);

    enum Facility
    {
        FACILITY_CORE = 0x1,
        FACILITY_DDL = 0x2,
        FACILITY_RENDEZVOUS = 0x3,
        FACILITY_PYTHON_CORE = 0x4,
        FACILITY_TRANSPORT = 0x5,
        FACILITY_DO_CORE = 0x6,
        FACILITY_FPD = 0x65,
        FACILITY_RANKING = 0x67,
        FACILITY_AUTHENTICATION = 0x68,
        FACILITY_DATASTORE = 0x69,
        FACILITY_SERVICE_ITEM = 0x6C,
        FACILITY_MATCHMAKE_REFEREE = 0x6F,
        FACILITY_SUBSCRIBER = 0x70,
        FACILITY_RANKING2 = 0x71
    };

    enum NetworkErrorCategory
    {
        CATEGORY_CORE = 1,
        CATEGORY_DDL = 2,
        CATEGORY_RENDEZVOUS = 3,
        CATEGORY_PYTHON_CORE = 4,
        CATEGORY_TRANSPORT = 5,
        CATEGORY_DO_CORE = 6,
        CATEGORY_FPD = 7,
        CATEGORY_AUTHENTICATION = 8,
        CATEGORY_RANKING = 11,
        CATEGORY_DATASTORE = 12,
        CATEGORY_SERVICE_ITEM = 15,
        CATEGORY_MATCHMAKE_REFEREE = 18,
        CATEGORY_SUBSCRIBER = 19,
        CATEGORY_RANKING2 = 20,
        CATEGORY_CODE_RANGE = 100
    };

    enum Result
    {
        RESULT_ERROR_MASK = static_cast<int>(0x80000000u),
        CORE_UNKNOWN = static_cast<int>(0x80010001u),
        DDL_INVALID_SIGNATURE = static_cast<int>(0x80020001u),
        RENDEZVOUS_CONNECTION_FAILURE = static_cast<int>(0x80030001u),
        RENDEZVOUS_INVALID_USERNAME = static_cast<int>(0x80030064u),
        RENDEZVOUS_SESSION_FULL = static_cast<int>(0x800300C8u),
        PYTHON_CORE_EXCEPTION = static_cast<int>(0x80040001u),
        TRANSPORT_UNKNOWN = static_cast<int>(0x80050001u),
        DO_CORE_UNKNOWN = static_cast<int>(0x80060001u),
        FPD_UNKNOWN = static_cast<int>(0x80650000u),
        RANKING_FIRST_ERROR = static_cast<int>(0x80670001u),
        AUTHENTICATION_FIRST_ERROR = static_cast<int>(0x80680001u),
        DATASTORE_UNKNOWN = static_cast<int>(0x80690001u),
        SERVICE_ITEM_FIRST_ERROR = static_cast<int>(0x806C0001u),
        MATCHMAKE_REFEREE_FIRST_ERROR = static_cast<int>(0x806F0001u),
        SUBSCRIBER_FIRST_ERROR = static_cast<int>(0x80700001u),
        RANKING2_FIRST_ERROR = static_cast<int>(0x80710001u)
    };

    enum
    {
        RENDEZVOUS_CONNECTION_ERROR_COUNT = 2,
        RENDEZVOUS_ACCOUNT_ERROR_COUNT = 19
    };
};

}
}

#endif
