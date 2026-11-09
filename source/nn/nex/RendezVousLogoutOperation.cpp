#include "nn/nex/RendezVousLogoutOperation.hpp"

namespace nn
{
namespace nex
{

RendezVousLogoutOperation::RendezVousLogoutOperation(BackEndServices& rBackEndServices)
    : RendezVousOperation(rBackEndServices)
{
}

RendezVousLogoutOperation::~RendezVousLogoutOperation()
{
}

unsigned int RendezVousLogoutOperation::GetType() const
{
    return 22;
}

const wchar_t* RendezVousLogoutOperation::GetClassNameString() const
{
    return L"RendezVousLogout";
}

void RendezVousLogoutOperation::ForceImplOperationCommonMethodsMacro()
{
}

}
}
