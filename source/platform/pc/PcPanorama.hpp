#ifndef PLATFORM_PC_PCPANORAMA_HPP
#define PLATFORM_PC_PCPANORAMA_HPP

int PcPanoramaTexture(const char* name, int textureId);
const wchar_t* PcPanoramaName(int panorama);
bool PcPanoramaAvailable(int panorama);

#endif
