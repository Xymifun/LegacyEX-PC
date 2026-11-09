#include "nn/nex/CIDLogFilter.hpp"

#include "nn/nex/ProtocolRequestBrokerInterface.hpp"

namespace nn
{
namespace nex
{

bool CIDLogFilter::OutputCurrentTrace()
{
    unsigned int current;
    if (g_protocolCurrentCID != 0 && (current = g_protocolCurrentCID()) < m_upperBound && current > m_lowerBound)
        return true;
    return false;
}

}
}
