#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RENDEZVOUSLOGOUTOPERATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RENDEZVOUSLOGOUTOPERATION_HPP

#include "nn/nex/RendezVousOperation.hpp"

namespace nn
{
namespace nex
{

class RendezVousLogoutOperation : public RendezVousOperation
{
public:
    explicit RendezVousLogoutOperation(BackEndServices& rBackEndServices);
    virtual ~RendezVousLogoutOperation();
    virtual unsigned int GetType() const;
    virtual const wchar_t* GetClassNameString() const;
    virtual void ForceImplOperationCommonMethodsMacro();
};

}
}

#endif
