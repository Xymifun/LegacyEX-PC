#include "nn/nex/RandomNumberGenerator.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int STATE_SIZE = 624;
const unsigned int STATE_SHIFT = 397;
const unsigned int STATE_UNSEEDED_INDEX = STATE_SIZE + 1;
const unsigned int MATRIX_A = 0x9908B0DFu;
const unsigned int UPPER_MASK = 0x80000000u;
const unsigned int LOWER_MASK = 0x7FFFFFFFu;
const unsigned int TEMPERING_MASK_B = 0x9D2C5680u;
const unsigned int TEMPERING_MASK_C = 0xEFC60000u;
const unsigned int DEFAULT_SEED = 4357;
const unsigned int SEED_MULTIPLIER = 69069;
const unsigned int REAL_RANDOM_RANGE = 0x10000;
const float REAL_RANDOM_SCALE = 1.0f / 65536.0f;

}

RandomNumberGenerator::RandomNumberGenerator()
{
    m_aMagic[0] = 0;
    m_aMagic[1] = MATRIX_A;
    m_uiIndex = STATE_UNSEEDED_INDEX;
    SetRandomNumberSeed(1);
}

void RandomNumberGenerator::SetRandomNumberSeed(unsigned int uiSeed)
{
    if (uiSeed == 0)
        uiSeed = DEFAULT_SEED;

    m_aState[0] = uiSeed;
    for (m_uiIndex = 1; m_uiIndex < STATE_SIZE; ++m_uiIndex)
        m_aState[m_uiIndex] = SEED_MULTIPLIER * m_aState[m_uiIndex - 1];
}

unsigned int RandomNumberGenerator::GetRandomNumber(unsigned int uiLimit)
{
    if (uiLimit == 0)
        return 0;

    if (m_uiIndex >= STATE_SIZE)
    {
        if (m_uiIndex == STATE_UNSEEDED_INDEX)
            SetRandomNumberSeed(DEFAULT_SEED);

        unsigned int uiPosition = 0;
        unsigned int uiValue;
        for (; uiPosition < STATE_SIZE - STATE_SHIFT; ++uiPosition)
        {
            uiValue = (m_aState[uiPosition] & UPPER_MASK) | (m_aState[uiPosition + 1] & LOWER_MASK);
            m_aState[uiPosition] = m_aState[uiPosition + STATE_SHIFT] ^ (uiValue >> 1) ^ m_aMagic[uiValue & 1];
        }

        for (; uiPosition < STATE_SIZE - 1; ++uiPosition)
        {
            uiValue = (m_aState[uiPosition] & UPPER_MASK) | (m_aState[uiPosition + 1] & LOWER_MASK);
            m_aState[uiPosition] = m_aState[uiPosition + STATE_SHIFT - STATE_SIZE] ^ (uiValue >> 1)
                                   ^ m_aMagic[uiValue & 1];
        }

        uiValue = (m_aState[STATE_SIZE - 1] & UPPER_MASK) | (m_aState[0] & LOWER_MASK);
        m_aState[STATE_SIZE - 1] = m_aState[STATE_SHIFT - 1] ^ (uiValue >> 1) ^ m_aMagic[uiValue & 1];
        m_uiIndex = 0;
    }

    unsigned int uiValue = m_aState[m_uiIndex++];
    uiValue ^= uiValue >> 11;
    uiValue ^= (uiValue << 7) & TEMPERING_MASK_B;
    uiValue ^= (uiValue << 15) & TEMPERING_MASK_C;
    uiValue ^= uiValue >> 18;
    return uiValue % uiLimit;
}

float RandomNumberGenerator::GetRealRandomNumber(float fLimit)
{
    if (fLimit > 0.0f)
        return GetRandomNumber(REAL_RANDOM_RANGE) * REAL_RANDOM_SCALE * fLimit;

    return 0.0f;
}

}
}
