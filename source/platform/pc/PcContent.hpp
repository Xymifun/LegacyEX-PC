#ifndef PLATFORM_PC_PCCONTENT_HPP
#define PLATFORM_PC_PCCONTENT_HPP

#include <string>
#include <vector>

struct PcContentMod
{
    std::string id;
    std::string version;
    unsigned char hash[8];
};

struct PcContentIdentity
{
    bool present;
    bool detailed;
    unsigned char game[8];
    unsigned int buildTime;
    unsigned char mods[8];
    std::vector<PcContentMod> modList;
    PcContentIdentity();
};

void PcContentInitialise();
const PcContentIdentity& PcContentLocal();
std::string PcContentGameVersion(const PcContentIdentity& identity);

void PcContentAppendSummary(std::vector<unsigned char>* data);
void PcContentAppendDetail(std::vector<unsigned char>* data);
void PcContentParse(const std::vector<unsigned char>& data, size_t start, PcContentIdentity* identity);

bool PcContentMatches(const PcContentIdentity& other);
bool PcContentModded(const PcContentIdentity& identity);
std::vector<std::string> PcContentDifferences(const PcContentIdentity& other, bool otherIsHost);
std::wstring PcContentListLabel(const PcContentIdentity& other);

void PcContentSetMismatch(const PcContentIdentity& host);
const wchar_t* PcContentMismatchText();

#endif
