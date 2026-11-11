#ifndef PLATFORM_PC_PCMODSOUNDS_HPP
#define PLATFORM_PC_PCMODSOUNDS_HPP

#include <vector>

struct PcNativeRegister;
class SoundEvent;

const std::vector<char>* PcModSoundFile(const SoundEvent* sound);

void PcModSoundsFillRegister(PcNativeRegister* reg);

#endif
