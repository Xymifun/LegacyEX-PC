#include "nn/nex/SandboxConnectionInfo.hpp"

namespace nn
{
namespace nex
{

SandboxConnectionInfo::~SandboxConnectionInfo()
{
}

const SandboxConnectionInfo::HostCredentials& SandboxConnectionInfo::GetCredentialsProd() const
{
    return m_credentialsProd;
}

void SandboxConnectionInfo::HostCredentials::Trace(uint64_t uiFlag) const
{
}

}
}
