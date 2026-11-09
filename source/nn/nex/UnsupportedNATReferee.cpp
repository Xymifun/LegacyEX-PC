#include "nn/nex/UnsupportedNATReferee.hpp"

namespace nn
{
namespace nex
{

UnsupportedNATReferee::UnsupportedNATReferee() : m_uiFailureCount(0)
{
}

void UnsupportedNATReferee::Succeeded()
{
    m_uiFailureCount = 0;
}

}
}
