#include "nn/pia/transport/TransportAnalysisData.hpp"

#include "nn/pia/transport/AnalysisPrinter.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

void TransportAnalysisData::Print(bool isDetail, bool skipIntermediate) const
{
    m_packetAnalysisData.Print(isDetail);
    if (!skipIntermediate)
    {
        m_receivedPacketAnalysisData.Print(isDetail);
        m_sentPacketAnalysisData.Print(isDetail);
    }
    m_retransmittedPacketAnalysisData.Print(isDetail);
    m_connectionAnalysisData.Print(isDetail);
    AnalysisPrinter::Write("[Analysis] ------ DispatchCount:%d ------", m_dispatchCount);
}

}
}
}
