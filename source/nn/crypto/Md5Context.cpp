#include "nn/crypto/Md5Context.hpp"

#include "vendor/__N_14_crypto_Md5_cpp_c46079df.hpp"

namespace nn
{
namespace crypto
{

namespace
{

const unsigned int InitialStateA = 0x67452301;
const unsigned int InitialStateB = 0xEFCDAB89;
const unsigned int InitialStateC = 0x98BADCFE;
const unsigned int InitialStateD = 0x10325476;

const unsigned int RoundConstants[64] = {
    0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE, 0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501,
    0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE, 0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821,
    0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA, 0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8,
    0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED, 0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A,
    0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C, 0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70,
    0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05, 0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665,
    0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039, 0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1,
    0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1, 0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391};

const unsigned int WordOrder[48] = {1, 6, 11, 0,  5,  10, 15, 4,  9,  14, 3, 8,  13, 2,  7,  12,
                                    5, 8, 11, 14, 1,  4,  7,  10, 13, 0,  3, 6,  9,  12, 15, 2,
                                    0, 7, 14, 5,  12, 3,  10, 1,  8,  15, 6, 13, 4,  11, 2,  9};

const unsigned char PaddingByte = 0x80;

inline unsigned int SwapBytes(unsigned int uiValue)
{
    return (uiValue << 24) | ((uiValue & 0x0000FF00) << 8) | (uiValue >> 24) | ((uiValue >> 8) & 0x0000FF00);
}

inline unsigned int RotateLeft(unsigned int uiValue, unsigned int uiShift)
{
    return (uiValue << uiShift) | (uiValue >> (32 - uiShift));
}

inline unsigned int CalcRound3(unsigned int a, unsigned int b, unsigned int c, unsigned int d, unsigned int uiWord,
                               unsigned int uiShift, unsigned int uiConstant)
{
    return b + RotateLeft(a + (b ^ c ^ d) + SwapBytes(uiWord) + uiConstant, uiShift);
}

}

void Md5Context::Initialize()
{
    m_state[0] = InitialStateA;
    m_state[1] = InitialStateB;
    m_state[2] = InitialStateC;
    m_state[3] = InitialStateD;
    m_length = 0;
}

void Md5Context::Finalize()
{
}

void Md5Context::Update(const void* pData, unsigned int uiSize)
{
    const unsigned char* pSource = static_cast<const unsigned char*>(pData);
    unsigned int uiOffset = m_length & (BlockSize - 1);
    unsigned int uiRoom = BlockSize - uiOffset;
    m_length += uiSize;

    if (uiRoom > uiSize)
    {
        if (uiSize != 0)
        {
            unsigned char* pDest = &m_block.bytes[uiOffset];
            for (int i = 0; i < static_cast<int>(uiSize); ++i)
                pDest[i] = pSource[i];
        }
    }
    else
    {
        unsigned char* pDest = &m_block.bytes[uiOffset];
        for (int i = 0; i < static_cast<int>(uiRoom); ++i)
            pDest[i] = pSource[i];
        ProcessBlock();
        pSource += uiRoom;

        uiSize -= uiRoom;
        unsigned int uiBlocks = uiSize / BlockSize;
        for (int block = 0; block < static_cast<int>(uiBlocks); ++block)
        {
            pDest = m_block.bytes;
            for (int i = 0; i < BlockSize; ++i)
                pDest[i] = pSource[i];
            pSource += BlockSize;
            ProcessBlock();
        }

        unsigned int uiRemainder = uiSize & (BlockSize - 1);
        if (uiRemainder != 0)
        {
            pDest = m_block.bytes;
            for (int i = 0; i < static_cast<int>(uiRemainder); ++i)
                pDest[i] = pSource[i];
        }
    }
}

unsigned int Md5Context::GetHashSize()
{
    return 16;
}

void Md5Context::GetHash(void* pHash)
{
    unsigned int uiBitLength = m_length << 3;
    unsigned int uiBitLengthHigh = 0;
    Update(&PaddingByte, 1);

    unsigned int uiOffset = m_length & (BlockSize - 1);
    unsigned int uiRoom = BlockSize - uiOffset;
    if (uiRoom < 8)
    {
        unsigned char* pDest = &m_block.bytes[uiOffset];
        for (int i = 0; i < static_cast<int>(uiRoom); ++i)
            pDest[i] = 0;
        ProcessBlock();
        uiOffset = 0;
        uiRoom = BlockSize;
    }

    unsigned char* pDest = &m_block.bytes[uiOffset];
    for (int i = 0; i < static_cast<int>(uiRoom - 8); ++i)
        pDest[i] = 0;

    m_block.words[BlockWordCount - 2] = SwapBytes(uiBitLength);
    m_block.words[BlockWordCount - 1] = SwapBytes(uiBitLengthHigh);
    ProcessBlock();

    unsigned char* pOutput = static_cast<unsigned char*>(pHash);
    unsigned int uiState = m_state[0];
    pOutput[0] = static_cast<unsigned char>(uiState);
    pOutput[1] = static_cast<unsigned char>(uiState >> 8);
    pOutput[2] = static_cast<unsigned char>(uiState >> 16);
    pOutput[3] = static_cast<unsigned char>(uiState >> 24);
    uiState = m_state[1];
    pOutput[4] = static_cast<unsigned char>(uiState);
    pOutput[5] = static_cast<unsigned char>(uiState >> 8);
    pOutput[6] = static_cast<unsigned char>(uiState >> 16);
    pOutput[7] = static_cast<unsigned char>(uiState >> 24);
    uiState = m_state[2];
    pOutput[8] = static_cast<unsigned char>(uiState);
    pOutput[9] = static_cast<unsigned char>(uiState >> 8);
    pOutput[10] = static_cast<unsigned char>(uiState >> 16);
    pOutput[11] = static_cast<unsigned char>(uiState >> 24);
    uiState = m_state[3];
    pOutput[12] = static_cast<unsigned char>(uiState);
    pOutput[13] = static_cast<unsigned char>(uiState >> 8);
    pOutput[14] = static_cast<unsigned char>(uiState >> 16);
    pOutput[15] = static_cast<unsigned char>(uiState >> 24);
}

void Md5Context::ProcessBlock()
{
    unsigned int a = m_state[0];
    unsigned int b = m_state[1];
    unsigned int c = m_state[2];
    unsigned int d = m_state[3];
    unsigned int uiStep = 0;

    for (unsigned int group = 0; group < 4; ++group)
    {
        a = __N_14_crypto_Md5_cpp_c46079df::CalcRound1(a, b, c, d, m_block.words[group * 4], 7, RoundConstants[uiStep]);
        d = __N_14_crypto_Md5_cpp_c46079df::CalcRound1(d, a, b, c, m_block.words[group * 4 + 1], 12,
                                                       RoundConstants[uiStep + 1]);
        c = __N_14_crypto_Md5_cpp_c46079df::CalcRound1(c, d, a, b, m_block.words[group * 4 + 2], 17,
                                                       RoundConstants[uiStep + 2]);
        b = __N_14_crypto_Md5_cpp_c46079df::CalcRound1(b, c, d, a, m_block.words[group * 4 + 3], 22,
                                                       RoundConstants[uiStep + 3]);
        uiStep += 4;
    }

    for (unsigned int group = 0; group < 4; ++group)
    {
        a = __N_14_crypto_Md5_cpp_c46079df::CalcRound2(a, b, c, d, m_block.words[WordOrder[group * 4]], 5,
                                                       RoundConstants[uiStep]);
        d = __N_14_crypto_Md5_cpp_c46079df::CalcRound2(d, a, b, c, m_block.words[WordOrder[group * 4 + 1]], 9,
                                                       RoundConstants[uiStep + 1]);
        c = __N_14_crypto_Md5_cpp_c46079df::CalcRound2(c, d, a, b, m_block.words[WordOrder[group * 4 + 2]], 14,
                                                       RoundConstants[uiStep + 2]);
        b = __N_14_crypto_Md5_cpp_c46079df::CalcRound2(b, c, d, a, m_block.words[WordOrder[group * 4 + 3]], 20,
                                                       RoundConstants[uiStep + 3]);
        uiStep += 4;
    }

    for (unsigned int group = 4; group < 8; ++group)
    {
        a = CalcRound3(a, b, c, d, m_block.words[WordOrder[group * 4]], 4, RoundConstants[uiStep]);
        d = CalcRound3(d, a, b, c, m_block.words[WordOrder[group * 4 + 1]], 11, RoundConstants[uiStep + 1]);
        c = CalcRound3(c, d, a, b, m_block.words[WordOrder[group * 4 + 2]], 16, RoundConstants[uiStep + 2]);
        b = CalcRound3(b, c, d, a, m_block.words[WordOrder[group * 4 + 3]], 23, RoundConstants[uiStep + 3]);
        uiStep += 4;
    }

    for (unsigned int group = 8; group < 12; ++group)
    {
        a = __N_14_crypto_Md5_cpp_c46079df::CalcRound4(a, b, c, d, m_block.words[WordOrder[group * 4]], 6,
                                                       RoundConstants[uiStep]);
        d = __N_14_crypto_Md5_cpp_c46079df::CalcRound4(d, a, b, c, m_block.words[WordOrder[group * 4 + 1]], 10,
                                                       RoundConstants[uiStep + 1]);
        c = __N_14_crypto_Md5_cpp_c46079df::CalcRound4(c, d, a, b, m_block.words[WordOrder[group * 4 + 2]], 15,
                                                       RoundConstants[uiStep + 2]);
        b = __N_14_crypto_Md5_cpp_c46079df::CalcRound4(b, c, d, a, m_block.words[WordOrder[group * 4 + 3]], 21,
                                                       RoundConstants[uiStep + 3]);
        uiStep += 4;
    }

    m_state[0] += a;
    m_state[1] += b;
    m_state[2] += c;
    m_state[3] += d;
}

unsigned int Md5Context::GetHashLength()
{
    return GetHashSize();
}

}
}
