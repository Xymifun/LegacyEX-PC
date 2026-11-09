#ifndef PLATFORM_PC_PCACCOUNT_HPP
#define PLATFORM_PC_PCACCOUNT_HPP

#include <stddef.h>

#include <string>

void PcSetPlayerName(const std::string& name);
const std::string& PcPlayerName();
unsigned int PcPrincipalId();
void PcAccountUuid(unsigned char uuid[16]);
bool PcAccountSign(const unsigned char* message, size_t size, unsigned char publicKey[32],
                   unsigned char signature[64]);
void PcSetIdentityDirectory(const std::string& directory);
const int kPcGuestIdentities = 3;
void PcGuestAccountUuid(int guest, unsigned char uuid[16]);
std::string PcGuestName(int guest);
bool PcGuestAccountSign(int guest, const unsigned char* message, size_t size, unsigned char publicKey[32],
                        unsigned char signature[64]);

#endif
