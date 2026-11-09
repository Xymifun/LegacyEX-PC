#include "nn/nex/Data.hpp"

namespace nn
{
namespace nex
{

Data::Data()
{
}

Data::~Data()
{
}

Data* _DDL_Data::Clone() const
{
    return new Data;
}

}
}
