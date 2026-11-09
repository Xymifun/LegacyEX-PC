#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RENDEZVOUSOPERATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RENDEZVOUSOPERATION_HPP

#include "nn/nex/Operation.hpp"

namespace nn
{
namespace nex
{

class BackEndServices;

class RendezVousOperation : public Operation
{
public:
    explicit RendezVousOperation(BackEndServices& rBackEndServices);
    virtual ~RendezVousOperation();

    BackEndServices* GetBackEndServices() { return m_pBackEndServices; }

    void TraceImpl(Event event, unsigned long long uiFilter) const;

protected:
    BackEndServices* m_pBackEndServices;
};

}
}

#endif
