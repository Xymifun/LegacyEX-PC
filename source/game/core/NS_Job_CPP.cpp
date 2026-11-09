#include "game/core/NS_Job_CPP.hpp"

namespace NS_Job_CPP
{

void InvokeCallbackOnCompletion(nn::nex::Job* pJob, const nn::nex::UserContext* pUserContext)
{
    nn::nex::CallbackRoot* pCallback = static_cast<nn::nex::CallbackRoot*>(pUserContext->GetPointer());
    pCallback->Call();
    delete pCallback;
}

}
