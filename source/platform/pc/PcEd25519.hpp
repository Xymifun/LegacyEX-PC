#ifndef PLATFORM_PC_PCED25519_HPP
#define PLATFORM_PC_PCED25519_HPP

#include <stddef.h>
#include <stdint.h>

void PcEd25519PublicKey(const unsigned char seed[32], unsigned char publicKey[32]);
void PcEd25519Sign(const unsigned char seed[32], const unsigned char* message, size_t size,
                   unsigned char signature[64]);
bool PcEd25519Verify(const unsigned char publicKey[32], const unsigned char* message, size_t size,
                     const unsigned char signature[64]);
void PcSha512(const unsigned char* data, size_t size, unsigned char digest[64]);

struct PcSha512Context
{
    uint64_t state[8];
    unsigned char buffer[128];
    size_t used;
    uint64_t total;
};

void PcSha512Begin(PcSha512Context* context);
void PcSha512Add(PcSha512Context* context, const unsigned char* data, size_t size);
void PcSha512End(PcSha512Context* context, unsigned char digest[64]);

#endif
