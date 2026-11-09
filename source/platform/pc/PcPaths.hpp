#ifndef PLATFORM_PC_PCPATHS_HPP
#define PLATFORM_PC_PCPATHS_HPP

#include <string>

void PcSetContentRoot(const std::string& root);
void PcSetSaveRoot(const std::string& root);
bool PcPrepareSaveRoot();
const std::string& PcContentRoot();
const std::string& PcSaveRoot();

std::string PcResolvePath(const char* gamePath);
std::string PcResolveWritePath(const char* gamePath);
std::string PcContentRelativePath(const char* gamePath);

#endif
