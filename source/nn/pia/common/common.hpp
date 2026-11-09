#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_COMMON_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_COMMON_HPP
#include <stdint.h>
#include <string.h>
#include <coreinit/memory.h>
#include "nn/pia/common/AesImpl.hpp"
namespace nn
{
namespace pia
{
namespace common
{
struct GcmModeGenericHelper
{
};

template <unsigned int Index> struct GcmReduction
{
    static const uint32_t value = (((Index & 1) != 0) ? 0xe1000000u >> 3 : 0)
                                  ^ (((Index & 2) != 0) ? 0xe1000000u >> 2 : 0)
                                  ^ (((Index & 4) != 0) ? 0xe1000000u >> 1 : 0)
                                  ^ (((Index & 8) != 0) ? 0xe1000000u : 0);
};

template <typename Helper> class GcmModeImpl
{
public:
    union Block
    {
        uint8_t bytes[16];
        uint32_t words[4];
        uint64_t quads[2];
    };
    typedef void (*BlockFunction)(void*, const void*, const void*);
    GcmModeImpl() : m_state(0) {}
    template <typename Cipher> static void BlockCallback(void*, const void*, const void*);
    template <typename Cipher> void Initialize(const Cipher&);
    ~GcmModeImpl();
    void GfMultH(Block*);
    void Reset(const void*, unsigned int);
    unsigned int EncryptUpdate(void*, unsigned int, const void*, unsigned int);
    unsigned int DecryptUpdate(void*, unsigned int, const void*, unsigned int);
    void GenerateMac();
    void GetMac(void*, unsigned int);
    void InitializeHashKey();

private:
    static const uint32_t s_reduction[16];

    uint32_t m_state;
    const void* m_cipher;
    BlockFunction m_encryptBlock;
    uint32_t m_layoutPadding;
    Block m_hash;
    Block m_counter;
    Block m_keystream;
    Block m_mac;
    Block m_work;
    uint64_t m_aadSize;
    uint64_t m_textSize;
    uint32_t m_partialSize;
    uint32_t m_keystreamOffset;
    uint32_t m_blockCounter;
    uint32_t m_statePadding;
    uint32_t m_table[16][4];
};

template <typename Helper>
const uint32_t GcmModeImpl<Helper>::s_reduction[16] = {
    GcmReduction<0>::value,  GcmReduction<1>::value,  GcmReduction<2>::value,  GcmReduction<3>::value,
    GcmReduction<4>::value,  GcmReduction<5>::value,  GcmReduction<6>::value,  GcmReduction<7>::value,
    GcmReduction<8>::value,  GcmReduction<9>::value,  GcmReduction<10>::value, GcmReduction<11>::value,
    GcmReduction<12>::value, GcmReduction<13>::value, GcmReduction<14>::value, GcmReduction<15>::value};

template <typename Helper>
template <typename Cipher>
void GcmModeImpl<Helper>::BlockCallback(void* output, const void* input, const void* context)
{
    static_cast<const Cipher*>(context)->EncryptBlock(output, input);
}

template <typename Helper>
template <typename Cipher>
void GcmModeImpl<Helper>::Initialize(const Cipher& cipher)
{
    m_cipher = &cipher;
    m_encryptBlock = &BlockCallback<Cipher>;
    InitializeHashKey();
    m_state = 1;
}

template <typename Helper>
GcmModeImpl<Helper>::~GcmModeImpl()
{
    OSBlockSet(this, 0, sizeof(*this));
}

template <typename Helper>
void GcmModeImpl<Helper>::GfMultH(Block* block)
{
    for (int index = 15; index >= 0; --index)
    {
        unsigned int low = block->bytes[index] & 0x0f;
        unsigned int high = block->bytes[index] >> 4;
        const uint32_t* lowEntry = m_table[low];
        const uint32_t* highEntry = m_table[high];
        if (index == 15)
        {
            m_work.words[0] = lowEntry[0];
            m_work.words[1] = lowEntry[1];
            m_work.words[2] = lowEntry[2];
            m_work.words[3] = lowEntry[3];
        }
        else
        {
            unsigned int remainder = m_work.words[3] & 0x0f;
            m_work.words[3] = (m_work.words[2] << 28) | (m_work.words[3] >> 4);
            m_work.words[2] = (m_work.words[1] << 28) | (m_work.words[2] >> 4);
            m_work.words[1] = (m_work.words[0] << 28) | (m_work.words[1] >> 4);
            m_work.words[0] = m_work.words[0] >> 4;
            m_work.words[0] ^= s_reduction[remainder];
            m_work.words[0] ^= lowEntry[0];
            m_work.words[1] ^= lowEntry[1];
            m_work.words[2] ^= lowEntry[2];
            m_work.words[3] ^= lowEntry[3];
        }
        unsigned int remainder = m_work.words[3] & 0x0f;
        m_work.words[3] = (m_work.words[2] << 28) | (m_work.words[3] >> 4);
        m_work.words[2] = (m_work.words[1] << 28) | (m_work.words[2] >> 4);
        m_work.words[1] = (m_work.words[0] << 28) | (m_work.words[1] >> 4);
        m_work.words[0] = m_work.words[0] >> 4;
        m_work.words[0] ^= s_reduction[remainder];
        m_work.words[0] ^= highEntry[0];
        m_work.words[1] ^= highEntry[1];
        m_work.words[2] ^= highEntry[2];
        m_work.words[3] ^= highEntry[3];
    }
    block->words[0] = m_work.words[0];
    block->words[1] = m_work.words[1];
    block->words[2] = m_work.words[2];
    block->words[3] = m_work.words[3];
}

template <typename Helper>
void GcmModeImpl<Helper>::Reset(const void* iv, unsigned int size)
{
    m_counter.words[3] = 0;
    m_partialSize = 0;
    m_counter.words[1] = 0;
    m_hash.words[2] = 0;
    m_hash.words[0] = 0;
    m_textSize = 0;
    m_counter.words[0] = 0;
    m_keystreamOffset = 0;
    m_hash.words[3] = 0;
    m_aadSize = 0;
    m_hash.words[1] = 0;
    m_counter.words[2] = 0;
    if (size == 12)
    {
        OSBlockMove(m_counter.bytes, iv, 12, FALSE);
        m_counter.bytes[15] = 1;
        m_blockCounter = 1;
        m_encryptBlock(m_mac.words, m_counter.bytes, m_cipher);
        m_state = 2;
    }
    else
    {
        uint64_t bits = size << 3;
        const uint8_t* bytes = static_cast<const uint8_t*>(iv);
        for (; size >= 16; size -= 16)
        {
            for (unsigned int index = 0; index < 16; ++index)
                m_counter.bytes[index] ^= bytes[index];
            GfMultH(&m_counter);
            bytes += 16;
        }
        if (size != 0)
        {
            for (unsigned int index = 0; index < size; ++index)
                m_counter.bytes[index] ^= bytes[index];
            GfMultH(&m_counter);
        }
        m_counter.quads[1] ^= bits;
        GfMultH(&m_counter);
        m_blockCounter = m_counter.words[3];
        m_encryptBlock(m_mac.words, m_counter.bytes, m_cipher);
        m_state = 2;
    }
}

template <typename Helper>
unsigned int GcmModeImpl<Helper>::EncryptUpdate(void* output, unsigned int, const void* input, unsigned int size)
{
    unsigned int total = size;
    uint8_t* destination = static_cast<uint8_t*>(output);
    const uint8_t* source = static_cast<const uint8_t*>(input);
    if (m_partialSize != 0)
    {
        GfMultH(&m_hash);
        m_partialSize = 0;
    }
    m_state = 3;
    m_textSize += size;
    for (; size != 0; --size)
    {
        if (m_keystreamOffset == 0)
        {
            ++m_blockCounter;
            m_counter.words[3] = m_blockCounter;
            m_encryptBlock(m_keystream.words, m_counter.words, m_cipher);
        }
        uint8_t value = *source ^ m_keystream.bytes[m_keystreamOffset];
        *destination = value;
        m_hash.bytes[m_keystreamOffset] ^= value;
        ++source;
        m_keystreamOffset = (m_keystreamOffset + 1) & 0x0f;
        ++destination;
        if (m_keystreamOffset == 0)
            GfMultH(&m_hash);
    }
    return total;
}

template <typename Helper>
unsigned int GcmModeImpl<Helper>::DecryptUpdate(void* output, unsigned int, const void* input, unsigned int size)
{
    unsigned int total = size;
    uint8_t* destination = static_cast<uint8_t*>(output);
    const uint8_t* source = static_cast<const uint8_t*>(input);
    if (m_partialSize != 0)
    {
        GfMultH(&m_hash);
        m_partialSize = 0;
    }
    m_state = 4;
    m_textSize += size;
    for (; size != 0; --size)
    {
        if (m_keystreamOffset == 0)
        {
            ++m_blockCounter;
            m_counter.words[3] = m_blockCounter;
            m_encryptBlock(m_keystream.words, m_counter.words, m_cipher);
        }
        uint8_t value = *source;
        *destination = value ^ m_keystream.bytes[m_keystreamOffset];
        m_hash.bytes[m_keystreamOffset] ^= value;
        ++source;
        m_keystreamOffset = (m_keystreamOffset + 1) & 0x0f;
        ++destination;
        if (m_keystreamOffset == 0)
            GfMultH(&m_hash);
    }
    return total;
}

template <typename Helper>
void GcmModeImpl<Helper>::GenerateMac()
{
    if (m_keystreamOffset != 0 || m_partialSize != 0)
        GfMultH(&m_hash);
    m_work.quads[0] = m_aadSize << 3;
    m_work.quads[1] = m_textSize << 3;
    m_hash.words[0] ^= m_work.words[0];
    m_hash.words[1] ^= m_work.words[1];
    m_hash.words[2] ^= m_work.words[2];
    m_hash.words[3] ^= m_work.words[3];
    GfMultH(&m_hash);
    m_hash.words[0] ^= m_mac.words[0];
    m_hash.words[1] ^= m_mac.words[1];
    m_hash.words[2] ^= m_mac.words[2];
    m_hash.words[3] ^= m_mac.words[3];
}

template <typename Helper>
void GcmModeImpl<Helper>::GetMac(void* output, unsigned int)
{
    if (m_state != 5)
    {
        GenerateMac();
        m_state = 5;
    }
    OSBlockMove(output, m_hash.bytes, sizeof(m_hash.bytes), FALSE);
}

template <typename Helper>
void GcmModeImpl<Helper>::InitializeHashKey()
{
    m_work.words[0] = 0;
    m_work.words[1] = 0;
    m_work.words[2] = 0;
    m_work.words[3] = 0;
    m_encryptBlock(m_work.words, m_work.words, m_cipher);
    m_table[0][0] = 0;
    m_table[0][1] = 0;
    m_table[0][2] = 0;
    m_table[0][3] = 0;
    m_table[8][0] = m_work.words[0];
    m_table[8][1] = m_work.words[1];
    m_table[8][2] = m_work.words[2];
    m_table[8][3] = m_work.words[3];
    int index = 4;
    do
    {
        unsigned int lowBit = m_work.words[3] & 1;
        m_work.words[3] = (m_work.words[2] << 31) | (m_work.words[3] >> 1);
        m_work.words[2] = (m_work.words[1] << 31) | (m_work.words[2] >> 1);
        m_work.words[1] = (m_work.words[0] << 31) | (m_work.words[1] >> 1);
        m_work.words[0] = m_work.words[0] >> 1;
        if (lowBit != 0)
            m_work.words[0] ^= 0xe1000000u;
        m_table[index][0] = m_work.words[0];
        m_table[index][1] = m_work.words[1];
        m_table[index][2] = m_work.words[2];
        m_table[index][3] = m_work.words[3];
        index >>= 1;
    } while (index != 0);
    index = 2;
    do
    {
        m_work.words[0] = m_table[index][0];
        m_work.words[1] = m_table[index][1];
        m_work.words[2] = m_table[index][2];
        m_work.words[3] = m_table[index][3];
        for (int offset = 1; offset < index; ++offset)
        {
            m_table[index + offset][0] = m_work.words[0] ^ m_table[offset][0];
            m_table[index + offset][1] = m_work.words[1] ^ m_table[offset][1];
            m_table[index + offset][2] = m_work.words[2] ^ m_table[offset][2];
            m_table[index + offset][3] = m_work.words[3] ^ m_table[offset][3];
        }
        index <<= 1;
    } while (index < 9);
}

typedef GcmModeImpl<GcmModeGenericHelper> GcmMode;
}
}
}
#endif
