#include "nn/pia/common/CryptoSecurePseudoRandomGenerator.hpp"

#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace common
{

CryptoSecurePseudoRandomGenerator::CryptoSecurePseudoRandomGenerator() : m_counter(0)
{
    pead::Random random;
    m_setting0 = random.getU64();
    m_setting1 = random.getU64();
    m_seed = random.getU64();
    m_counter = 0;
}

CryptoSecurePseudoRandomGenerator::~CryptoSecurePseudoRandomGenerator()
{
}

void CryptoSecurePseudoRandomGenerator::UpdateSetting(unsigned long long first, unsigned long long second)
{
    m_setting0 = first;
    m_setting1 = second;
}

}
}
}
