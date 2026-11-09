#ifndef PLATFORM_PC_PCGOLDENAGE_HPP
#define PLATFORM_PC_PCGOLDENAGE_HPP

#include <string>
#include <vector>

class BufferedImage;
class SoundEvent;

bool PcGoldenAgeInstalled();
bool PcGoldenAgeOn(int group);
void PcGoldenAgeSetGroups(int groups);
void PcGoldenAgeRefresh(int previousGroups);
void PcGoldenAgePatchAtlas(int iconType, BufferedImage* image);
void PcGoldenAgeRecolourAnimation(const std::wstring& path, BufferedImage* image);
BufferedImage* PcGoldenAgeReplaceAnimation(const std::wstring& path, BufferedImage* image);
void PcGoldenAgeAnimationFrames(const std::wstring& path, std::wstring* animation);
BufferedImage* PcGoldenAgeReadImage(const std::wstring& name);
int PcGoldenAgeTerrainTexture();
const char* PcGoldenAgeEventName(const SoundEvent* sound);
const std::vector<char>* PcGoldenAgeSoundFile(const SoundEvent* sound);

#endif
