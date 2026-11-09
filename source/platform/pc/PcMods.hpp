#ifndef PLATFORM_PC_PCMODS_HPP
#define PLATFORM_PC_PCMODS_HPP

#include <string>
#include <vector>

const int kPcModApi = 2;

struct PcMod
{
    std::string id;
    std::string name;
    std::string version;
    std::string description;
    std::vector<std::string> authors;
    int priority;
    int api;
    std::string side;
    std::string directory;
    unsigned int files;
    unsigned int gameplayFiles;
    unsigned int scriptFiles;
    unsigned char gameplayHash[8];
};

struct PcModArchiveFile
{
    std::wstring name;
    std::string path;
};

void PcModsSetDirectory(const std::string& directory);
void PcModsSetEnabled(bool enabled);
void PcModsLoad();
const std::vector<PcMod>& PcModsLoaded();

bool PcModsIsWorldData(const std::string& contentRelative);
bool PcModsCountsForGameplay(const std::string& contentRelative, const std::string& side);
std::string PcModsFindContent(const std::string& contentRelative);
void PcModsArchiveFiles(const char* archiveGamePath, std::vector<PcModArchiveFile>* files);

#endif
