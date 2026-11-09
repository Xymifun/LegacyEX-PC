#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RANDOMNUMBERGENERATOR_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RANDOMNUMBERGENERATOR_HPP

namespace nn
{
namespace nex
{

class RandomNumberGenerator
{
public:
    RandomNumberGenerator();

    void SetRandomNumberSeed(unsigned int uiSeed);
    unsigned int GetRandomNumber(unsigned int uiLimit);
    float GetRealRandomNumber(float fLimit);

private:
    enum
    {
        StateSize = 624,
        MagicTableSize = 2
    };

    unsigned int m_aState[StateSize];
    unsigned int m_uiIndex;
    unsigned int m_aMagic[MagicTableSize];
};

}
}

#endif
