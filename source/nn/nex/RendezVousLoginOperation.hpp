#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RENDEZVOUSLOGINOPERATION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RENDEZVOUSLOGINOPERATION_HPP

#include "nn/nex/RendezVousOperation.hpp"

namespace nn
{
namespace nex
{

class RendezVousLoginOperation : public RendezVousOperation
{
public:
    explicit RendezVousLoginOperation(BackEndServices& rBackEndServices);
    virtual ~RendezVousLoginOperation();
    virtual unsigned int GetType() const;
    virtual const wchar_t* GetClassNameString() const;
    virtual void ForceImplOperationCommonMethodsMacro();
};

}
}

#endif
