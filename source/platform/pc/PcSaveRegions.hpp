#ifndef PLATFORM_PC_PCSAVEREGIONS_HPP
#define PLATFORM_PC_PCSAVEREGIONS_HPP

#include <string>
#include <vector>

std::string PcSaveRegionDirectory(const std::string& saveName);
bool PcListDirectory(const std::string& path, std::vector<std::string>* names);
void PcSaveRegionsDelete(const std::string& saveName);
bool PcSaveRegionsCopy(const std::string& fromSave, const std::string& toSave);

#endif
