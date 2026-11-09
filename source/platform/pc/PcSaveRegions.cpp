#include "platform/pc/PcSaveRegions.hpp"

#include "platform/pc/PcOs.hpp"
#include "platform/pc/PcPaths.hpp"

#include <dirent.h>
#include <nn/act/client_cpp.h>
#include <stdio.h>
#include <string.h>

namespace
{
const char REGION_DIRECTORY_SUFFIX[] = ".regions";

bool CopyFile(const std::string& from, const std::string& to)
{
    FILE* input = fopen(from.c_str(), "rb");
    if (input == NULL)
        return false;
    FILE* output = fopen(to.c_str(), "wb");
    if (output == NULL)
    {
        fclose(input);
        return false;
    }
    static char buf[1 << 16];
    bool ok = true;
    for (;;)
    {
        size_t n = fread(buf, 1, sizeof(buf), input);
        if (n == 0)
            break;
        if (fwrite(buf, 1, n, output) != n)
        {
            ok = false;
            break;
        }
    }
    ok = ok && !ferror(input);
    fclose(input);
    ok = fclose(output) == 0 && ok;
    return ok;
}
}

std::string PcSaveRegionDirectory(const std::string& saveName)
{
    char account[16];
    snprintf(account, sizeof(account), "%08x", 0x80000000u | nn::act::GetSlotNo());
    return PcSaveRoot() + "/" + account + "/" + saveName + REGION_DIRECTORY_SUFFIX;
}

bool PcListDirectory(const std::string& path, std::vector<std::string>* names)
{
    DIR* dir = opendir(path.c_str());
    if (dir == NULL)
        return false;
    struct dirent* e;
    while ((e = readdir(dir)) != NULL)
    {
        if (strcmp(e->d_name, ".") != 0 && strcmp(e->d_name, "..") != 0)
            names->push_back(e->d_name);
    }
    closedir(dir);
    return true;
}

void PcSaveRegionsDelete(const std::string& saveName)
{
    if (saveName.empty())
        return;
    const std::string dir = PcSaveRegionDirectory(saveName);
    std::vector<std::string> names;
    if (!PcListDirectory(dir, &names))
        return;
    for (size_t i = 0; i < names.size(); ++i)
        PcOsRemoveFile((dir + "/" + names[i]).c_str());
    PcOsRemoveDirectory(dir.c_str());
}

bool PcSaveRegionsCopy(const std::string& fromSave, const std::string& toSave)
{
    const std::string from = PcSaveRegionDirectory(fromSave);
    std::vector<std::string> names;
    if (!PcListDirectory(from, &names))
        return true;
    const std::string to = PcSaveRegionDirectory(toSave);
    PcOsMakeDirectory(to.c_str());
    bool ok = true;
    for (size_t i = 0; i < names.size(); ++i)
    {
        const std::string& name = names[i];
        if (name.size() > 5 && name.compare(name.size() - 5, 5, ".work") == 0)
            continue;
        ok = CopyFile(from + "/" + name, to + "/" + name) && ok;
    }
    return ok;
}
