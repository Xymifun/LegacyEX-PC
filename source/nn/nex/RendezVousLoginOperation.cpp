#include "nn/nex/RendezVousLoginOperation.hpp"

namespace nn
{
namespace nex
{

RendezVousLoginOperation::RendezVousLoginOperation(BackEndServices& rBackEndServices)
    : RendezVousOperation(rBackEndServices)
{
}

RendezVousLoginOperation::~RendezVousLoginOperation()
{
}

unsigned int RendezVousLoginOperation::GetType() const
{
    return 21;
}

const wchar_t* RendezVousLoginOperation::GetClassNameString() const
{
    return L"RendezVousLogin";
}

void RendezVousLoginOperation::ForceImplOperationCommonMethodsMacro()
{
}

}
}
