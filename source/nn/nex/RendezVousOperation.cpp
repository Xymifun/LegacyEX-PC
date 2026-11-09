#include "nn/nex/RendezVousOperation.hpp"

namespace nn
{
namespace nex
{

RendezVousOperation::RendezVousOperation(BackEndServices& rBackEndServices)
    : Operation(0),
      m_pBackEndServices(&rBackEndServices)
{
}

RendezVousOperation::~RendezVousOperation()
{
}

void RendezVousOperation::TraceImpl(Event, unsigned long long) const
{
}

}
}
