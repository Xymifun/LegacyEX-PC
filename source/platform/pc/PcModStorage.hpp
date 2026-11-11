#ifndef PLATFORM_PC_PCMODSTORAGE_HPP
#define PLATFORM_PC_PCMODSTORAGE_HPP

#include <map>
#include <string>
#include <vector>

struct PcNativeRegister;
struct PcNativeHost;
class CompoundTag;

typedef std::map<std::string, std::vector<unsigned char> > PcModData;

void PcModStorageRead(CompoundTag* tag, PcModData* out);
void PcModStorageWrite(CompoundTag* tag, const PcModData& data);

void PcModStorageFillRegister(PcNativeRegister* reg);
void PcModStorageFillHost(PcNativeHost* host);

#endif
