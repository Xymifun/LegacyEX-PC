#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORERATINGLOG_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTORERATINGLOG_HPP

#include "nn/nex/_DDL_DataStoreRatingLog.hpp"

namespace nn
{
namespace nex
{

class DataStoreRatingLog : public _DDL_DataStoreRatingLog
{
public:
    DataStoreRatingLog() { m_bIsRated = false; }

    virtual ~DataStoreRatingLog() {}
};

}
}

#endif
