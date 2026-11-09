#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_AES_IMPL_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_AES_IMPL_HPP

#include <stdint.h>
#include <string.h>
#include <coreinit/memory.h>
#include "nn/pia/common/Crypto.hpp"

namespace nn
{
namespace pia
{
namespace common
{

namespace aes
{

template <unsigned int Value, unsigned int Count>
struct RotateByteLeft
{
    static const unsigned int value = ((Value << Count) | (Value >> (8 - Count))) & 0xff;
};

template <unsigned int Value>
struct Double
{
    static const unsigned int value = ((Value << 1) ^ ((Value & 0x80) != 0 ? 0x11b : 0)) & 0xff;
};

template <unsigned int Left, unsigned int Right>
struct Multiply
{
    static const unsigned int value = ((Right & 1) != 0 ? Left : 0)
                                      ^ Multiply<Double<Left>::value, (Right >> 1)>::value;
};

template <unsigned int Left>
struct Multiply<Left, 0>
{
    static const unsigned int value = 0;
};

template <unsigned int Base, unsigned int Exponent>
struct Power
{
    static const unsigned int half = Power<Base, Exponent / 2>::value;
    static const unsigned int square = Multiply<half, half>::value;
    static const unsigned int value = (Exponent & 1) != 0 ? Multiply<Base, square>::value : square;
};

template <unsigned int Base>
struct Power<Base, 0>
{
    static const unsigned int value = 1;
};

template <unsigned int Value>
struct Inverse
{
    static const unsigned int value = Power<Value, 254>::value;
};

template <unsigned int Value>
struct Substitute
{
    static const unsigned int inverse = Inverse<Value>::value;
    static const unsigned int value = (inverse ^ RotateByteLeft<inverse, 1>::value ^ RotateByteLeft<inverse, 2>::value
                                       ^ RotateByteLeft<inverse, 3>::value ^ RotateByteLeft<inverse, 4>::value ^ 0x63)
                                      & 0xff;
};

template <unsigned int Value>
struct InverseSubstitute
{
    static const unsigned int value = Inverse<(RotateByteLeft<Value, 1>::value ^ RotateByteLeft<Value, 3>::value
                                               ^ RotateByteLeft<Value, 6>::value ^ 0x05)>::value;
};

template <unsigned int Value>
struct EncryptionColumn
{
    static const unsigned int byte = Substitute<Value>::value;
    static const unsigned int value = (byte << 24) | (byte << 16) | (Multiply<byte, 3>::value << 8)
                                      | Multiply<byte, 2>::value;
};

template <unsigned int Value>
struct DecryptionColumn
{
    static const unsigned int byte = InverseSubstitute<Value>::value;
    static const unsigned int value = (Multiply<byte, 9>::value << 24) | (Multiply<byte, 13>::value << 16)
                                      | (Multiply<byte, 11>::value << 8) | Multiply<byte, 14>::value;
};

template <unsigned int Index>
struct RoundConstant
{
    static const unsigned int value = Double<RoundConstant<Index - 1>::value>::value;
};

template <>
struct RoundConstant<0>
{
    static const unsigned int value = 1;
};

}

#define NN_PIA_AES_ENTRIES_4(Entry, first) Entry(first), Entry((first) + 1), Entry((first) + 2), Entry((first) + 3)
#define NN_PIA_AES_ENTRIES_16(Entry, first) \
    NN_PIA_AES_ENTRIES_4(Entry, first), NN_PIA_AES_ENTRIES_4(Entry, (first) + 4), \
        NN_PIA_AES_ENTRIES_4(Entry, (first) + 8), NN_PIA_AES_ENTRIES_4(Entry, (first) + 12)
#define NN_PIA_AES_ENTRIES_64(Entry, first) \
    NN_PIA_AES_ENTRIES_16(Entry, first), NN_PIA_AES_ENTRIES_16(Entry, (first) + 16), \
        NN_PIA_AES_ENTRIES_16(Entry, (first) + 32), NN_PIA_AES_ENTRIES_16(Entry, (first) + 48)
#define NN_PIA_AES_ENTRIES_256(Entry) \
    NN_PIA_AES_ENTRIES_64(Entry, 0), NN_PIA_AES_ENTRIES_64(Entry, 64), NN_PIA_AES_ENTRIES_64(Entry, 128), \
        NN_PIA_AES_ENTRIES_64(Entry, 192)
#define NN_PIA_AES_SUBSTITUTE(index) aes::Substitute<(index)>::value
#define NN_PIA_AES_INVERSE_SUBSTITUTE(index) aes::InverseSubstitute<(index)>::value
#define NN_PIA_AES_ENCRYPTION_COLUMN(index) aes::EncryptionColumn<(index)>::value
#define NN_PIA_AES_DECRYPTION_COLUMN(index) aes::DecryptionColumn<(index)>::value
#define NN_PIA_AES_ROUND_CONSTANT(index) aes::RoundConstant<(index)>::value

template <unsigned int KeySize>
class AesImpl
{
public:
    ~AesImpl();
    void Initialize(const void* key, unsigned int length, bool encrypt);
    void EncryptBlock(void* destination, unsigned int destinationLength, const void* source,
                      unsigned int sourceLength) const;
    void DecryptBlock(void* destination, unsigned int destinationLength, const void* source,
                      unsigned int sourceLength) const;

private:
    static uint32_t RotateLeft(uint32_t word, unsigned int count);
    static uint32_t SubstituteWord(uint32_t word);
    static uint32_t DoubleBytes(uint32_t word);
    static uint32_t InverseMixColumn(uint32_t word);
    static uint32_t EncryptionColumn(uint32_t first, uint32_t second, uint32_t third, uint32_t fourth);
    static uint32_t DecryptionColumn(uint32_t first, uint32_t second, uint32_t third, uint32_t fourth);

    static const uint8_t s_roundConstants[30];
    static const uint8_t s_substitution[256];
    static const uint32_t s_encryptionColumns[256];
    static const uint8_t s_inverseSubstitution[256];
    static const uint32_t s_decryptionColumns[256];

    uint32_t m_roundKeys[44];
};

template <unsigned int KeySize>
const uint8_t AesImpl<KeySize>::s_roundConstants[30] = {NN_PIA_AES_ENTRIES_16(NN_PIA_AES_ROUND_CONSTANT, 0),
                                                        NN_PIA_AES_ENTRIES_4(NN_PIA_AES_ROUND_CONSTANT, 16),
                                                        NN_PIA_AES_ENTRIES_4(NN_PIA_AES_ROUND_CONSTANT, 20),
                                                        NN_PIA_AES_ENTRIES_4(NN_PIA_AES_ROUND_CONSTANT, 24),
                                                        NN_PIA_AES_ROUND_CONSTANT(28),
                                                        NN_PIA_AES_ROUND_CONSTANT(29)};

template <unsigned int KeySize>
const uint8_t AesImpl<KeySize>::s_substitution[256] = {NN_PIA_AES_ENTRIES_256(NN_PIA_AES_SUBSTITUTE)};

template <unsigned int KeySize>
const uint32_t AesImpl<KeySize>::s_encryptionColumns[256] = {NN_PIA_AES_ENTRIES_256(NN_PIA_AES_ENCRYPTION_COLUMN)};

template <unsigned int KeySize>
const uint8_t AesImpl<KeySize>::s_inverseSubstitution[256] = {NN_PIA_AES_ENTRIES_256(NN_PIA_AES_INVERSE_SUBSTITUTE)};

template <unsigned int KeySize>
const uint32_t AesImpl<KeySize>::s_decryptionColumns[256] = {NN_PIA_AES_ENTRIES_256(NN_PIA_AES_DECRYPTION_COLUMN)};

#undef NN_PIA_AES_ROUND_CONSTANT
#undef NN_PIA_AES_DECRYPTION_COLUMN
#undef NN_PIA_AES_ENCRYPTION_COLUMN
#undef NN_PIA_AES_INVERSE_SUBSTITUTE
#undef NN_PIA_AES_SUBSTITUTE
#undef NN_PIA_AES_ENTRIES_256
#undef NN_PIA_AES_ENTRIES_64
#undef NN_PIA_AES_ENTRIES_16
#undef NN_PIA_AES_ENTRIES_4

template <unsigned int KeySize>
AesImpl<KeySize>::~AesImpl()
{
    crypto::ClearMemory(m_roundKeys, sizeof(m_roundKeys));
}

template <unsigned int KeySize>
inline uint32_t AesImpl<KeySize>::RotateLeft(uint32_t word, unsigned int count)
{
    return (word << count) | (word >> (32 - count));
}

template <unsigned int KeySize>
inline uint32_t AesImpl<KeySize>::SubstituteWord(uint32_t word)
{
    return (static_cast<uint32_t>(s_substitution[word >> 24]) << 24)
           | (static_cast<uint32_t>(s_substitution[(word >> 16) & 0xff]) << 16)
           | (static_cast<uint32_t>(s_substitution[(word >> 8) & 0xff]) << 8) | s_substitution[word & 0xff];
}

template <unsigned int KeySize>
inline uint32_t AesImpl<KeySize>::DoubleBytes(uint32_t word)
{
    return ((word & 0x7f7f7f7f) << 1) ^ (((word & 0x80808080) >> 7) * 0x1b);
}

template <unsigned int KeySize>
inline uint32_t AesImpl<KeySize>::InverseMixColumn(uint32_t word)
{
    uint32_t twice = DoubleBytes(word);
    uint32_t fourfold = DoubleBytes(twice);
    uint32_t mixed = word ^ DoubleBytes(fourfold);
    mixed = mixed ^ RotateLeft(mixed, 8) ^ fourfold;
    mixed = mixed ^ RotateLeft(mixed, 8) ^ twice;
    return mixed ^ RotateLeft(mixed, 8) ^ word;
}

template <unsigned int KeySize>
inline uint32_t AesImpl<KeySize>::EncryptionColumn(uint32_t first, uint32_t second, uint32_t third, uint32_t fourth)
{
    return RotateLeft(s_encryptionColumns[first >> 24], 24) ^ RotateLeft(s_encryptionColumns[(second >> 16) & 0xff], 16)
           ^ RotateLeft(s_encryptionColumns[(third >> 8) & 0xff], 8) ^ s_encryptionColumns[fourth & 0xff];
}

template <unsigned int KeySize>
inline uint32_t AesImpl<KeySize>::DecryptionColumn(uint32_t first, uint32_t second, uint32_t third, uint32_t fourth)
{
    return RotateLeft(s_decryptionColumns[first >> 24], 24) ^ RotateLeft(s_decryptionColumns[(second >> 16) & 0xff], 16)
           ^ RotateLeft(s_decryptionColumns[(third >> 8) & 0xff], 8) ^ s_decryptionColumns[fourth & 0xff];
}

template <unsigned int KeySize>
void AesImpl<KeySize>::Initialize(const void* key, unsigned int length, bool encrypt)
{
    int keyWords = length / 4;
    OSBlockMove(m_roundKeys, key, length, FALSE);
    uint32_t word = m_roundKeys[keyWords - 1];
    for (int index = keyWords; index < 44; ++index)
    {
        if (index % keyWords == 0)
        {
            word = s_substitution[word >> 24] ^ (static_cast<uint32_t>(s_substitution[(word >> 16) & 0xff]) << 24)
                   ^ (static_cast<uint32_t>(s_substitution[(word >> 8) & 0xff]) << 16)
                   ^ (static_cast<uint32_t>(s_substitution[word & 0xff]) << 8)
                   ^ (static_cast<uint32_t>(s_roundConstants[index / keyWords - 1]) << 24);
        }
        else if (keyWords > 6 && index % keyWords == 4)
        {
            word = ((static_cast<uint32_t>(s_substitution[(word >> 8) & 0xff]) << 8)
                    | (static_cast<uint32_t>(s_substitution[(word >> 16) & 0xff]) << 16)
                    | (static_cast<uint32_t>(s_substitution[word >> 24]) << 24))
                   ^ s_substitution[word & 0xff];
        }
        word ^= m_roundKeys[index - keyWords];
        m_roundKeys[index] = word;
    }
    if (!encrypt)
    {
        for (unsigned int round = 1; round < 10; ++round)
        {
            for (unsigned int column = 0; column < 4; ++column)
                m_roundKeys[round * 4 + column] = InverseMixColumn(m_roundKeys[round * 4 + column]);
        }
    }
}

template <unsigned int KeySize>
void AesImpl<KeySize>::EncryptBlock(void* destination, unsigned int, const void* source, unsigned int) const
{
    const uint32_t* input = static_cast<const uint32_t*>(source);
    uint32_t* output = static_cast<uint32_t*>(destination);
    uint32_t state0 = input[0] ^ m_roundKeys[0];
    uint32_t state1 = input[1] ^ m_roundKeys[1];
    uint32_t state2 = input[2] ^ m_roundKeys[2];
    uint32_t state3 = input[3] ^ m_roundKeys[3];
    const uint32_t* roundKey = m_roundKeys;
    for (int round = 9; round > 0; --round)
    {
        roundKey += 4;
        uint32_t next0 = EncryptionColumn(state0, state1, state2, state3) ^ roundKey[0];
        uint32_t next1 = EncryptionColumn(state1, state2, state3, state0) ^ roundKey[1];
        uint32_t next2 = EncryptionColumn(state2, state3, state0, state1) ^ roundKey[2];
        uint32_t next3 = EncryptionColumn(state3, state0, state1, state2) ^ roundKey[3];
        state0 = next0;
        state1 = next1;
        state2 = next2;
        state3 = next3;
    }
    roundKey += 4;
    output[0] = roundKey[0]
                ^ ((static_cast<uint32_t>(s_substitution[state0 >> 24]) << 24)
                   | (static_cast<uint32_t>(s_substitution[(state1 >> 16) & 0xff]) << 16)
                   | (static_cast<uint32_t>(s_substitution[(state2 >> 8) & 0xff]) << 8)
                   | s_substitution[state3 & 0xff]);
    output[1] = roundKey[1]
                ^ ((static_cast<uint32_t>(s_substitution[state1 >> 24]) << 24)
                   | (static_cast<uint32_t>(s_substitution[(state2 >> 16) & 0xff]) << 16)
                   | (static_cast<uint32_t>(s_substitution[(state3 >> 8) & 0xff]) << 8)
                   | s_substitution[state0 & 0xff]);
    output[2] = roundKey[2]
                ^ ((static_cast<uint32_t>(s_substitution[state2 >> 24]) << 24)
                   | (static_cast<uint32_t>(s_substitution[(state3 >> 16) & 0xff]) << 16)
                   | (static_cast<uint32_t>(s_substitution[(state0 >> 8) & 0xff]) << 8)
                   | s_substitution[state1 & 0xff]);
    output[3] = roundKey[3]
                ^ ((static_cast<uint32_t>(s_substitution[state3 >> 24]) << 24)
                   | (static_cast<uint32_t>(s_substitution[(state0 >> 16) & 0xff]) << 16)
                   | (static_cast<uint32_t>(s_substitution[(state1 >> 8) & 0xff]) << 8)
                   | s_substitution[state2 & 0xff]);
}

template <unsigned int KeySize>
void AesImpl<KeySize>::DecryptBlock(void* destination, unsigned int, const void* source, unsigned int) const
{
    const uint32_t* input = static_cast<const uint32_t*>(source);
    uint32_t* output = static_cast<uint32_t*>(destination);
    const uint32_t* roundKey = m_roundKeys + 40;
    uint32_t state0 = input[0] ^ roundKey[0];
    uint32_t state1 = input[1] ^ roundKey[1];
    uint32_t state2 = input[2] ^ roundKey[2];
    uint32_t state3 = input[3] ^ roundKey[3];
    for (int round = 9; round > 0; --round)
    {
        roundKey -= 4;
        uint32_t next0 = DecryptionColumn(state0, state3, state2, state1) ^ roundKey[0];
        uint32_t next1 = DecryptionColumn(state1, state0, state3, state2) ^ roundKey[1];
        uint32_t next2 = DecryptionColumn(state2, state1, state0, state3) ^ roundKey[2];
        uint32_t next3 = DecryptionColumn(state3, state2, state1, state0) ^ roundKey[3];
        state0 = next0;
        state1 = next1;
        state2 = next2;
        state3 = next3;
    }
    roundKey -= 4;
    output[0] = roundKey[0]
                ^ ((static_cast<uint32_t>(s_inverseSubstitution[state0 >> 24]) << 24)
                   | (static_cast<uint32_t>(s_inverseSubstitution[(state3 >> 16) & 0xff]) << 16)
                   | (static_cast<uint32_t>(s_inverseSubstitution[(state2 >> 8) & 0xff]) << 8)
                   | s_inverseSubstitution[state1 & 0xff]);
    output[1] = roundKey[1]
                ^ ((static_cast<uint32_t>(s_inverseSubstitution[state1 >> 24]) << 24)
                   | (static_cast<uint32_t>(s_inverseSubstitution[(state0 >> 16) & 0xff]) << 16)
                   | (static_cast<uint32_t>(s_inverseSubstitution[(state3 >> 8) & 0xff]) << 8)
                   | s_inverseSubstitution[state2 & 0xff]);
    output[2] = roundKey[2]
                ^ ((static_cast<uint32_t>(s_inverseSubstitution[state2 >> 24]) << 24)
                   | (static_cast<uint32_t>(s_inverseSubstitution[(state1 >> 16) & 0xff]) << 16)
                   | (static_cast<uint32_t>(s_inverseSubstitution[(state0 >> 8) & 0xff]) << 8)
                   | s_inverseSubstitution[state3 & 0xff]);
    output[3] = roundKey[3]
                ^ ((static_cast<uint32_t>(s_inverseSubstitution[state3 >> 24]) << 24)
                   | (static_cast<uint32_t>(s_inverseSubstitution[(state2 >> 16) & 0xff]) << 16)
                   | (static_cast<uint32_t>(s_inverseSubstitution[(state1 >> 8) & 0xff]) << 8)
                   | s_inverseSubstitution[state0 & 0xff]);
}

template <unsigned int KeySize>
class AesEncryptor
{
public:
    void Initialize(const void* key, unsigned int length) { m_aes.Initialize(key, length, true); }

    void EncryptBlock(void* destination, const void* source) const
    {
        m_aes.EncryptBlock(destination, 0x10, source, 0x10);
    }

private:
    AesImpl<KeySize> m_aes;
};

}
}
}

#endif
